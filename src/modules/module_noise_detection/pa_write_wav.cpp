/*
  pa_write_wav.cpp
  used/modified by fyrelab 2016
*/
/*
 * PortAudio Portable Real-Time Audio Library
 * Latest Version at: http://www.portaudio.com
 *
 * Copyright (c) 1999-2010 Phil Burk and Ross Bencina
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however,
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also
 * requested that these non-binding requests be included along with the
 * license above.
 */

/**
  * Very simple WAV file writer for saving captured audio.
  */

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include "modules/module_noise_detection/pa_write_wav.h"


/* Write long word data to a little endian format byte array. */
static void WriteLongLE(unsigned char **addrPtr, unsigned int data) {
  unsigned char *addr = *addrPtr;
  *addr++ =  (unsigned char) data;
  *addr++ =  (unsigned char) (data>>8);
  *addr++ =  (unsigned char) (data>>16);
  *addr++ =  (unsigned char) (data>>24);
  *addrPtr = addr;
}

/* Write short word data to a little endian format byte array. */
static void WriteShortLE(unsigned char **addrPtr,  uint16_t data) {
  unsigned char *addr = *addrPtr;
  *addr++ =  (unsigned char) data;
  *addr++ =  (unsigned char) (data>>8);
  *addrPtr = addr;
}

/* Write IFF ChunkType data to a byte array. */
static void WriteChunkType(unsigned char **addrPtr, uint32_t cktyp) {
  unsigned char *addr = *addrPtr;
  *addr++ =  (unsigned char) (cktyp>>24);
  *addr++ =  (unsigned char) (cktyp>>16);
  *addr++ =  (unsigned char) (cktyp>>8);
  *addr++ =  (unsigned char) cktyp;
  *addrPtr = addr;
}

#define WAV_HEADER_SIZE (4 + 4 + 4 + /* RIFF+size+WAVE */ \
        4 + 4 + 16 + /* fmt chunk */ \
        4 + 4 ) /* data chunk */


/*********************************************************************************
 * Open named file and write WAV header to the file.
 * The header includes the DATA chunk type and size.
 * Returns number of bytes written to file or negative error code.
 */
int Audio_WAV_OpenWriter(WAV_Writer *writer, const char *fileName, int frameRate, int samplesPerFrame) {
  unsigned int  bytesPerSecond;
    unsigned char header[WAV_HEADER_SIZE];
  unsigned char *addr = header;
    int numWritten;

    writer->dataSize = 0;
    writer->dataSizeOffset = 0;

    writer->fid = fopen(fileName, "wb");
    if (writer->fid == NULL) {
        return -1;
    }

/* Write RIFF header. */
  WriteChunkType(&addr, RIFF_ID);

/* Write RIFF size as zero for now. Will patch later. */
  WriteLongLE(&addr, 0);

/* Write WAVE form ID. */
  WriteChunkType(&addr, WAVE_ID);

/* Write format chunk based on AudioSample structure. */
  WriteChunkType(&addr, FMT_ID);
    WriteLongLE(&addr, 16);
    WriteShortLE(&addr, WAVE_FORMAT_PCM);
    bytesPerSecond = frameRate * samplesPerFrame * sizeof(int16_t);
  WriteShortLE(&addr, (int16_t) samplesPerFrame);
  WriteLongLE(&addr, frameRate);
  WriteLongLE(&addr,  bytesPerSecond);
  WriteShortLE(&addr, (int16_t) (samplesPerFrame * sizeof(int16_t))); /* bytesPerBlock */
  WriteShortLE(&addr, (int16_t) 16); /* bits per sample */

/* Write ID and size for 'data' chunk. */
  WriteChunkType(&addr, DATA_ID);
/* Save offset so we can patch it later. */
    writer->dataSizeOffset = static_cast<int>(addr - header);
  WriteLongLE(&addr, 0);

    numWritten = fwrite(header, 1, sizeof(header), writer->fid );
    if (numWritten != sizeof(header))
      return -1;

  return numWritten;
}

/*********************************************************************************
 * Write to the data chunk portion of a WAV file.
 * Returns bytes written or negative error code.
 */
int Audio_WAV_WriteShorts( WAV_Writer *writer, short *samples, int numSamples) {
  unsigned char buffer[2];
  unsigned char *bufferPtr;

  int i;

  short *p = samples;

  int numWritten;
  int bytesWritten;

  if (numSamples <= 0) {
  return -1;
  }

  for (i=0; i<numSamples; i++) {
    bufferPtr = buffer;
    WriteShortLE(&bufferPtr, *p++);
    numWritten = fwrite(buffer, 1, sizeof(buffer), writer->fid);

    if (numWritten != sizeof(buffer))
      return -1;
  }

  bytesWritten = numSamples * sizeof(short);
  writer->dataSize += bytesWritten;
  return bytesWritten;
 }

/*********************************************************************************
 * Close WAV file.
 * Update chunk sizes so it can be read by audio applications.
 */
int Audio_WAV_CloseWriter(WAV_Writer *writer) {
  unsigned char buffer[4];
  unsigned char *bufferPtr;
  int numWritten;
  int riffSize;

  /* Go back to beginning of file and update DATA size */
  int result = fseek(writer->fid, writer->dataSizeOffset, SEEK_SET);
  if (result < 0)
    return result;

  bufferPtr = buffer;
  WriteLongLE(&bufferPtr, writer->dataSize);
  numWritten = fwrite(buffer, 1, sizeof( buffer), writer->fid);
  if (numWritten != sizeof(buffer))
    return -1;

  /* Update RIFF size */
  result = fseek(writer->fid, 4, SEEK_SET);
  if (result < 0)
    return result;

  riffSize = writer->dataSize + (WAV_HEADER_SIZE - 8);
  bufferPtr = buffer;
  WriteLongLE(&bufferPtr, riffSize);
  numWritten = fwrite(buffer, 1, sizeof(buffer), writer->fid);
  if (numWritten != sizeof(buffer))
    return -1;

  fclose(writer->fid);
  writer->fid = NULL;
  return writer->dataSize;
}
