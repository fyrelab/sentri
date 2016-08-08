/*
  module_temperature/main.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_temperature/temperature_module.h"

int main(int argc, char* argv[]) {
  TemperatureModule module = TemperatureModule(argv[1], argv[2]);
  module.start();

  return 0;
}
