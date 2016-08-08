/*
  module_noise_detection.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_MODULES_MODULE_NOISE_DETECTION_NOISE_DETECTION_MODULE_H_
#define SRC_MODULES_MODULE_NOISE_DETECTION_NOISE_DETECTION_MODULE_H_

#include <portaudio.h>
#include <string>
#include <map>
#include <vector>
#include "lib_module/module.h"

#define MODULE_NAME "module_noise_detection"
#define HEARTBEAT_INTERVAL 4000  // 4 seconds

class NoiseDetectionModule : lib_module::Module {
 public:
  NoiseDetectionModule(const std::string &sending_queue, const std::string &receiving_queue);
  ~NoiseDetectionModule();

  /**
   * The main class of the module. Initializes and starts an PortAudio-Stream.
   * During a short calibration period the threshold is set to a starting value.
   * Then sound is recorded and analysed in a loop, a event message is sent
   * when a noise is detected. Later deleting te tempoary wave files
   */
  void detectNoise();

 private:
  /**
   * Generates a wave file that contains the detected noise. Gets the pointer to the input
   * stream and a pointer to the buffer in which the audio samples are stored
   * @param stream The PortAudio Input stream
   * @param sampleBlock The memory block the audio is written to
   * @return returns the path of the created audio file as a string
   */
  std::string generateAudioFile(PaStream *stream, int16_t *sampleBlock);

  /**
   * Waits for an acknowledgement-message from the core and then deletes the wave file that
   * contains the noise. This method is calles asynchronous.
   * @param fileName Contains the file name of that file that has to be removed
   * @param numOfRules If multiple event messages were sent, wait for all to be acknowledged before deleting the file
   * @return returns a status code if sucessfull
   */
  int deleteFile(std::string fileName, int numOfRules);

  /**
   * Deletes all temporary audio files that may be in the tmp directory when starting the module
   */
  void deleteTmpFiles();

  /* The config map, that contains the module config values,
   * and the rules vecotr, that contains the module specific rules
   */
  std::map<std::string, std::string> *config_;
  std::vector<lib_module::RuleInfo> *rules_;
};

#endif  // SRC_MODULES_MODULE_NOISE_DETECTION_NOISE_DETECTION_MODULE_H_
