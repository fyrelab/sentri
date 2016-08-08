/*
  module_temperature/temperature_module.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_temperature/temperature_module.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <iostream>
#include <string>
#include <map>
#include <cstring>

#include "lib_module/module.h"

using boost::filesystem::path;

TemperatureModule::TemperatureModule(const std::string &sending_queue, const std::string &receiving_queue)
                                     : lib_module::Module(MODULE_NAME, sending_queue, receiving_queue) {}

TemperatureModule::~TemperatureModule() {}

double TemperatureModule::readTemperature(std::ifstream *sensor) {
  double temperature = ABSOLUTE_ZERO;

  // jump to beginning, parse sensor file for temperature
  sensor->seekg(0);
  std::string line;
  size_t abs_pos = 0;
  bool found = false;
  while (!sensor->eof() && !found) {
    std::getline(*sensor, line);
    size_t pos = line.find(TEMPERATURE_PREFIX);
    if (pos != std::string::npos) {
      sensor->seekg(static_cast<std::streampos>(abs_pos + pos + strlen(TEMPERATURE_PREFIX) + 1));
      char buf[TEMPERATURE_WITH];
      sensor->read(buf, TEMPERATURE_WITH);
      try {
        temperature = std::stod(std::string(buf).insert(2, "."));
      } catch (const std::invalid_argument &ia) {
        log(lib_module::LogSeverity::ERROR, "Invalid temperature reading! " + std::string(ia.what()));
      }
      found = true;
    }
    abs_pos += line.length();
  }

  return temperature;
}

double TemperatureModule::simulateSensor(double min, double max) {
  double f = static_cast<double>(rand_r(&rand_seed)) / RAND_MAX;
  return min + f * (max - min);
}

bool TemperatureModule::writeHookTemp(double temperature) {
  std::string hook_data = "{ \"temperature\": " + std::to_string(temperature) + " }\n";
  return writeHook(hook_data);
}

void TemperatureModule::start() {
  // interval in which the sensor is read, read from config
  int polling_interval;
  if (getConfigLoader().getConfig()->find("polling_interval") != getConfigLoader().getConfig()->end()) {
    try {
      polling_interval = std::stoi(getConfigLoader().getConfig()->at("polling_interval"));
      if (polling_interval < 0)
       throw std::invalid_argument("negative value");
    } catch (...) {
      log(lib_module::LogSeverity::FATAL, "Invalid polling_interval in configuration!");
      exit(EXIT_FAILURE);
    }
  } else {
    log(lib_module::LogSeverity::FATAL, "polling_interval in config not found!");
    exit(EXIT_FAILURE);
  }

  int heartbeat_interval = DEFAULT_HEARTBEAT_INTERVAL / 1000;  // in seconds
  // make sure there is enough time to send a heartbeat
  setHeartbeatInterval((heartbeat_interval + 2) * 1000);

  #ifdef __arm__
  // initialize reading from sensor
  std::string sensor_path = "";
  // get first temperature sensor found with random iterator, NO support for multiple sensors!
  for (auto e : boost::filesystem::directory_iterator(path(BUS_PATH))) {
    if (boost::starts_with(e.path().filename().string(), SENSOR_PREFIX))
      sensor_path = e.path().string() + "/" TEMPERATURE_FILE;
  }
  if (sensor_path.empty()) {
    log(lib_module::LogSeverity::FATAL, "Couldn't find sensor!");
    exit(EXIT_FAILURE);
  }

  // file stream to sensor data
  std::ifstream sensor(sensor_path, std::ios_base::in);
  if (!sensor) {
    log(lib_module::LogSeverity::FATAL, "Couldn't open sensor for reading!");
    exit(EXIT_FAILURE);
  }
  #endif

  double currTemp = 0.0;

  bool poll = true;  // only poll when this is set true
  int sleep_time = polling_interval;  // determines how long the modules sleep,
                                      // calculation is based on polling interval, see loop
  int loop_time = 0;  // measures how long the loop took

  // main loop
  while (true) {
    time_t loop_begin = time(0);  // loop begin timepoint

    sendHeartbeat();

    while (receiveDiscard()) {}

    // only read temp and trigger if were are in the loop passage for next poll
    if (poll) {
      // restore sleep time
      if (polling_interval > heartbeat_interval)
        sleep_time = polling_interval;

      // read current temp from sensor
      // if we are on the RPi read from the real sensor
      #ifdef __arm__
      currTemp = readTemperature(&sensor);
      // otherwise simulate sensor to test it on x86
      #else
      currTemp = simulateSensor(0.0, 60.0);
      #endif

      log(lib_module::LogSeverity::TRACE, std::to_string(currTemp));

      // check rules
      // only check if valid temperature was read
      if (currTemp != ABSOLUTE_ZERO) {
        writeHookTemp(currTemp);

        for (auto r : *getConfigLoader().getRules(MODULE_NAME)) {
          if (!r.is_disabled) {
            if (r.event.key == "temp_exceeded") {
              try {
                if (r.event.configuration.find("max_temp") != r.event.configuration.end() &&
                      currTemp >= std::stod(r.event.configuration["max_temp"])) {
                  sendEventMessage(r.id, "temp=" + std::to_string(currTemp));
                }
              } catch (const std::invalid_argument &ia) {
                log(lib_module::LogSeverity::FATAL, "Invalid value for max_temp, rule corrupt!");
                exit(EXIT_FAILURE);
              }
            } else if (r.event.key == "temp_undershot") {
              try {
                if (r.event.configuration.find("min_temp") != r.event.configuration.end() &&
                    currTemp < std::stod(r.event.configuration["min_temp"])) {
                  sendEventMessage(r.id, "temp=" + std::to_string(currTemp));
                }
              } catch (const std::invalid_argument &ia) {
                log(lib_module::LogSeverity::FATAL, "Invalid value for min_temp, rule corrupt!");
                exit(EXIT_FAILURE);
              }
            } else {
                log(lib_module::LogSeverity::ERROR, "Unknown event!");
            }
          }
        }
      }
    }

    // make sure we maximally wait heartbeat_interval
    // this way if polling_interval > heartbeat_interval we only poll with specified frequency but remain consistent
    // heartbeats
    loop_time = time(0) - loop_begin;
    if (sleep_time > heartbeat_interval) {
      poll = false;
      sleep_time -= heartbeat_interval;
      if (heartbeat_interval - loop_time >= 0) {
        sleep(heartbeat_interval - loop_time);
      } else {
        sleep(0);
      }
    } else {
      poll = true;
      if (sleep_time - loop_time >= 0) {
        sleep(sleep_time - loop_time);
      } else {
        sleep(0);
      }
    }
  }
}
