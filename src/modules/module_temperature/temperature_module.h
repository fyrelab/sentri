/*
  module_temperature/temperature_module.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_MODULES_MODULE_TEMPERATURE_TEMPERATURE_MODULE_H_
#define SRC_MODULES_MODULE_TEMPERATURE_TEMPERATURE_MODULE_H_

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "lib_module/module.h"

#define MODULE_NAME "module_temperature"

#define BUS_PATH "/sys/bus/w1/devices"
#define SENSOR_PREFIX "28-"
#define TEMPERATURE_FILE "w1_slave"
#define TEMPERATURE_PREFIX "t="
#define TEMPERATURE_WITH 5
#define ABSOLUTE_ZERO -274.0  // used to check invalid temperature readings

class TemperatureModule : lib_module::Module {
 public:
  /**
   * @see lib_module::Module::Module
   */
  TemperatureModule(const std::string &sending_queue, const std::string &receiving_queue);
  ~TemperatureModule();

  /**
   * Starts main temperature sensor reading and processing
   */
  void start();

 private:
  /**
   * Read temperature from sensor.
   * @param sensor file that the sensor writes its data to
   * @return temperature read from sensor
   */
  double readTemperature(std::ifstream *sensor);
  /**
   * Simulates a sensor reading. Used for debugging only.
   * @param min minimal temperature that can be generated
   * @param max maximum temperature that can be generated
   * @return simulated temperature reading
   */
  double simulateSensor(double min, double max);
  /**
   * Writes hook file used by webtool to display the current temperature.
   * @param temperature temperature to write
   * @returns true if write was successful
   */
  bool writeHookTemp(double temperature);

  unsigned int rand_seed = 42;  ///< used by rand_r() to store seed when simulating sensor readings on x86
};

#endif  // SRC_MODULES_MODULE_TEMPERATURE_TEMPERATURE_MODULE_H_
