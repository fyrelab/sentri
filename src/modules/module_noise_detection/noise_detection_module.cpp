/*
  noise_detection_module.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_noise_detection/noise_detection_module.h"

#include <math.h>
#include <dirent.h>
#include <stdio.h>
#include <portaudio.h>
#include <string>
#include <map>
#include <cmath>
#include <thread>
#include <future>
#ifdef __APPLE__
  #include <pa_mac_core.h>
#endif
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include "modules/module_noise_detection/pa_write_wav.h"

#define FRAMES_PER_BUFFER (2048)  // Frames/samples that are written in one buffer

#define WAV_HEADER_SIZE (4 + 4 + 4 + /* RIFF+size+WAVE */ \
        4 + 4 + 16 + /* fmt chunk */ \
        4 + 4 ) /* data chunk */

#define SAMPLE_RATE (44100)

NoiseDetectionModule::NoiseDetectionModule(const std::string &sending_queue, const std::string &receiving_queue)
  : lib_module::Module(MODULE_NAME, sending_queue, receiving_queue) {
  // Create module object
  lib_module::ConfigLoaderJ &configLoader = getConfigLoader();

  // Get config and rules
  config_ = configLoader.getConfig();
  rules_  = configLoader.getRules(MODULE_NAME);
}

NoiseDetectionModule::~NoiseDetectionModule() {
  delete config_;
  delete rules_;
}

std::string NoiseDetectionModule::generateAudioFile(PaStream *stream, int16_t *sampleBlock) {
  PaError err;

  std::string record_string = lib_module::getTempPath() +
    "/audio/noise" + boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()) + ".wav";
  const char *record_file = record_string.c_str();

  // The WAV_writer struct, see pa_write_wav.cpp
  WAV_Writer writer;
  int result;

  // Opens the WAV_writer from pa_write_vav
  result =  Audio_WAV_OpenWriter(&writer, record_file, SAMPLE_RATE, 1);
  if (result < 0) {
    log(lib_module::FATAL, "Cannot open Audio_WAV_OpenWriter");
    exit(EXIT_FAILURE);
  }

  // Given the sample rate 44100, 64 gives a ((SAMPLE_RATE/FRAMES_PER_BUFFER)*64) seconds
  // long sound clip, in this case around 3 seconds
  for (int i = 0; i <= 64; i++) {
    result =  Audio_WAV_WriteShorts(&writer, sampleBlock, FRAMES_PER_BUFFER);
    if (result < 0) {
      log(lib_module::FATAL, "Cannot write audio file");
      exit(EXIT_FAILURE);
    }
    err = Pa_ReadStream(stream, sampleBlock, FRAMES_PER_BUFFER);
  }

  result =  Audio_WAV_CloseWriter(&writer);
  if (result < 0) {
    log(lib_module::FATAL, "Cannot close Audio_WAV_OpenWriter");
    exit(EXIT_FAILURE);
  }

  (void) err;

  return record_string;
}

int NoiseDetectionModule::deleteFile(std::string fileName, int numOfRules) {
  lib_module::Message m;
  const char *record_file = fileName.c_str();
  int nrOfRules = numOfRules;
  bool hasMessage = false;

  // Wait blocking for an acknowledgement-message
  while(nrOfRules != 0) {
  std::map<std::string, std::string> *map = new std::map<std::string, std::string>;
  hasMessage = receiveMessageBlocking(&m, 30000, map);

  nrOfRules--;
  }

  // Check if a messages were received
  if (hasMessage) {
    log(lib_module::INFO, "Acknowledgement-messages received. Deleting tmp noise audio file");
  } else {
    log(lib_module::ERROR, "No acknowledgement-message or abort message received, still deleting the file");
  }

  // Delete the file in any case to prevent memory dump
  remove(record_file);

  return 0;
}

void NoiseDetectionModule::deleteTmpFiles() {
  // These are data types defined in the "dirent" header
  try {
    DIR *theFolder = opendir("/tmp/sentri/audio");
    if (theFolder) {
      struct dirent *next_file;
      char filepath[256];

      while ( (next_file = readdir(theFolder)) != NULL ) {
        // build the path for each file in the folder
        sprintf(filepath, "%s/%s", "/tmp/sentri/audio", next_file->d_name);
        remove(filepath);
      }
      closedir(theFolder);
    } else {
      log(lib_module::ERROR, "Cannot open /tmp/sentri/audio");
    }
  } catch (...) {
    log(lib_module::ERROR, "Cannot delete temporary audio files in /tmp/sentri/audio");
  }
}

void NoiseDetectionModule::detectNoise() {
  // Clean the tmp folder with the audio files when starting the module
  deleteTmpFiles();

  // set hearbeat interval and send first heartbeat
  setHeartbeatInterval(HEARTBEAT_INTERVAL);
  sendHeartbeat();

  // Initialize PortAudio, the audio input-handler on which the applicatio is based on
  PaError err = Pa_Initialize();
  if (err != paNoError) {
    log(lib_module::FATAL, "PortAudio-Error. Check for the package");
    exit(EXIT_FAILURE);
  }

  PaStream *stream;

  // Opens the PortAudio input stream from the default audio input device
  err = Pa_OpenDefaultStream(&stream,
    1,               // number of input channels
    0,               // number of output channels
    paInt16,       // 32 bit integer output
    SAMPLE_RATE,
    FRAMES_PER_BUFFER,  // frames per buffer
    NULL,  // this is your callback function
    NULL);

  if (err != paNoError) {
    std::stringstream logM;
    logM << "Error opening audio stream: " << err;
    log(lib_module::FATAL, logM.str());
    exit(EXIT_FAILURE);
  }

  err = Pa_StartStream(stream);

  if (err != paNoError) {
    std::stringstream logM;
    logM << "Error starting audio stream: " << err;
    log(lib_module::FATAL, logM.str());
    exit(EXIT_FAILURE);
  }

  // Instantiate the chunk of memory on which the audio samples are writen
  int16_t *sampleBlock = new int16_t[FRAMES_PER_BUFFER];

  // Gets the initial threshold from the info file, threshold is later dynamicly set
  float_t threshold = 1200000.0;  // Set default value if tof fails
  int initial_calibration_period = 10;  // Set default value
  try {
    threshold = std::stof((*config_)["threshold"]);
    initial_calibration_period = std::stoi((*config_)["initial_calibration_period"]);
  } catch (std::exception) {
    log(lib_module::ERROR, "stof/stoi-conversion error while reading a value" +
              (*config_)["treshold"] + "or" + (*config_)["initial_calibration_period"]);
  }

  /* Initial Calibration to silence threshold of the microphone */
  time_t timeInitStart = time(0);
  time_t timeInitEnd = time(0);
  time_t hearbeatTime = time(0);
  int sum = 0;
  // For given calibration intervall, read from the input stream and calculate threshold dynamically
  while (timeInitEnd - timeInitStart <= initial_calibration_period) {
    err = Pa_ReadStream(stream, reinterpret_cast<void*>(sampleBlock), FRAMES_PER_BUFFER);
    sum = 0;

    const int16_t *initIn = sampleBlock;

    // Calculate the amplitude of the sound clip
    for (uint32_t i = 0; i < FRAMES_PER_BUFFER; i++) {
      int16_t inVal = *initIn++;

      sum += abs(inVal);
    }

    // Calculates the threshold dynamicaly, so it sets to the silence level
    threshold = 0.9 * threshold + 0.1 * static_cast<float_t>(sum);

    if (timeInitEnd - hearbeatTime >= HEARTBEAT_INTERVAL / 1000) {
      sendHeartbeat();
      hearbeatTime = time(0);
    }

    timeInitEnd  = time(0);
  }

  std::stringstream logInitM;
  logInitM << "Initial threshold was set to: " << threshold;
  log(lib_module::INFO, logInitM.str());

  // Time variables/ Boolean used for sending heartbeats at the right time
  time_t time1;
  time_t time2;
  bool timestampSet = false;

  // Send hearbeat at the beginning of the program routine
  sendHeartbeat();

  /* The while loop contains the actual programm routine */
  while (true) {
    // Set timestamp at the begin of a new hearbeat interval
    if (timestampSet != true) {
      time1 = time(0);
      timestampSet = true;
    }

    // Reading the first 2048 samples/frames from the port audio input stream
    err = Pa_ReadStream(stream, reinterpret_cast<void*>(sampleBlock), FRAMES_PER_BUFFER);

    const int16_t *in = sampleBlock;

    int sum = 0;

    // Calculate the amplitude of the sound clip
    for (uint32_t i = 0; i < FRAMES_PER_BUFFER; i++) {
      int16_t inVal = *in++;

      sum += abs(inVal);
    }

    // Calculates the threshold dynamicaly, so it sets to the silence level
    threshold = 0.95 * threshold + 0.05 * static_cast<float_t>(sum);

    // If another event occurs, dont create a new file, and use the existing file
    bool fileGenerated = false;
    // The number of invoked rules for an event
    int numOfRules = 0;
    std::string file_path;

    // Invoke a indivual noise detection for every rule, e.g. with unique threshold_overhead
    for (lib_module::RuleInfo r : *rules_) {

      float threshold_overhead = 5.0;  // Set default threshold_overhead config value for stof failure

      // Get the  absolute value (n,..) that is multiplied with the silence threshold
      try{
        threshold_overhead = std::stof(r.event.configuration["threshold_overhead"]);
      } catch (std::exception) {
        log(lib_module::ERROR, "Convert to float failed: threshold_overhead");
      }

      // Event occured
      if (sum > (threshold * threshold_overhead)) {
        // If a wave file was generated for one of previous rule iterations, dont create a new wave file
        if (fileGenerated != true) {
          sendHeartbeat();
          log(lib_module::INFO, "Sound detected: Recording started");
          file_path = generateAudioFile(stream, sampleBlock);
          fileGenerated = true;
          log(lib_module::INFO, "Sound detected: Recording finished");
        }

        // Increase the number of invoked rules
        numOfRules++;

        // Send an event message
        sendEventMessage(r.id, "audioFile=" + file_path);
        sendHeartbeat();
      }
    }

    time2 = time(0);
    // If interval length is reached, send a heartbeat and admit setting a new timestamp
    if (time2 - time1 >= (HEARTBEAT_INTERVAL / 1000)) {
      sendHeartbeat();
      timestampSet = false;
    }

    if (fileGenerated) {
      auto deleteCode = std::async(&NoiseDetectionModule::deleteFile, this, file_path, numOfRules);
    }
  }

  err = Pa_StopStream(stream);
  if (err != paNoError) {
    std::stringstream logM;
    logM << "Error starting audio stream: " << err;
    log(lib_module::FATAL, logM.str());
    exit(EXIT_FAILURE);
  }

  delete[] sampleBlock;

  Pa_Terminate();
}
