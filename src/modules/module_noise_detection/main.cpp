/*
  module_noise_detection main.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_noise_detection/noise_detection_module.h"

int main(int argc, char* argv[]) {
  // Create module
  NoiseDetectionModule module = NoiseDetectionModule(argv[1], argv[2]);

  // Start noise detection
  module.detectNoise();

  return 0;
}
