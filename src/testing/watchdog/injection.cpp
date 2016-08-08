/*
  main.cpp
  Copyright 2016 fyrelab
 */


 #define BOOST_TEST_MODULE Watchdog

#include <boost/test/included/unit_test.hpp>

#include <stdlib.h>

#include "core/sentri.h"
#include "core/watchdog.h"
#include "core/defs.h"
#include "lib_module/defs.h"

using boost::filesystem::exists;
using boost::filesystem::path;


#define ANSI_COLOR_RED "\x1b[31;1m"
#define ANSI_COLOR_RESET "\x1b[0m"

Sentri *sentri;
boost::thread *thread;


/**
 * Analysing health after system was running with faulty modules
 */

BOOST_AUTO_TEST_SUITE(Health_Test)

std::map<std::string, ModuleData> moduleData;

BOOST_AUTO_TEST_CASE(Pre_Test) {
  sentri = new Sentri(lib_module::getRulesConfPath(), MODULE_PATH);

  Watchdog &wd = sentri->getWatchdog();

  thread = new boost::thread(boost::bind(&Sentri::start, sentri));

  std::cout << ANSI_COLOR_RED
            << "fyrelab sentri started with test injection" << std::endl
            << ANSI_COLOR_RESET;

  // Let system run before analysis results
  sleep(30);

  // Gather data
  moduleData = wd.getModuleData();

  // Stop system
  thread->interrupt();
  sleep(5);

  delete sentri;
  delete thread;

  // Present results
  std::cout << ANSI_COLOR_RED
            << "######################" << std::endl
            << "Watchdog test results:" << std::endl
            << ANSI_COLOR_RESET;
}

/**
 * This module doesn't send any heartbeats
 * Therefore the system is expected to bury module
 */
BOOST_AUTO_TEST_CASE(No_Heartbeats) {
  BOOST_REQUIRE(moduleData.find("module_test1") != moduleData.end());

  ModuleData data = moduleData["module_test1"];
  BOOST_CHECK(data.heartbeat_interval == 100);
  BOOST_CHECK(data.buried);
  BOOST_CHECK(data.mortality_rate >= CRITICAL_MORTALITY_RATE);
}

/**
 * This module floods the watchdog with heartbeats permanently
 * The system is expected to keep working with the module running
 */
BOOST_AUTO_TEST_CASE(Flood_Heartbeats) {
  BOOST_REQUIRE(moduleData.find("module_test2") != moduleData.end());

  ModuleData data = moduleData["module_test2"];
  BOOST_CHECK(data.running);
  BOOST_CHECK(data.mortality_rate == DEFAULT_MORTALITY_RATE);
}

/**
 * This module does not follow the naming conventions
 * It is expected not be listed in the watchdog
 */
BOOST_AUTO_TEST_CASE(Faulty_Name) {
  // Make sure module is not in map
  BOOST_CHECK(moduleData.find("mod_test3") == moduleData.end());
}

/**
 * This module is healthy (sends heartbeats)
 * It is expected to be running, not buried and without any mortality
 */
BOOST_AUTO_TEST_CASE(Healthy_Module) {
  BOOST_REQUIRE(moduleData.find("module_test4") != moduleData.end());

  ModuleData data = moduleData["module_test4"];
  BOOST_CHECK(data.heartbeat_interval == 100);
  BOOST_CHECK(data.running);
  BOOST_CHECK(!data.buried);
  BOOST_CHECK(data.mortality_rate == DEFAULT_MORTALITY_RATE);
}

/**
 * This module is healthy and keeps on sending and receiving its own messages
 * This is expected to be running, not buried and without any mortality
 */
BOOST_AUTO_TEST_CASE(Healthy_Communicator) {
  BOOST_REQUIRE(moduleData.find("module_test5") != moduleData.end());

  ModuleData data = moduleData["module_test5"];
  BOOST_CHECK(data.heartbeat_interval == 100);
  BOOST_CHECK(data.running);
  BOOST_CHECK(!data.buried);
  BOOST_CHECK(data.mortality_rate == DEFAULT_MORTALITY_RATE);
}

/**
 * This module keep sending events but doesn't collect its actions
 * This is expected to be running and have a healthy core (if not other tests might fail)
 */
BOOST_AUTO_TEST_CASE(Queue_Neglecting) {
  BOOST_REQUIRE(moduleData.find("module_test6") != moduleData.end());

  ModuleData data = moduleData["module_test6"];
  BOOST_CHECK(data.heartbeat_interval == 100);
  BOOST_CHECK(data.running);
  BOOST_CHECK(!data.buried);
  BOOST_CHECK(data.mortality_rate == DEFAULT_MORTALITY_RATE);
}

/**
 * This module sends a message with an identifier and waits for abort repeatedly
 * This is expected to be running and have a healthy core
 */
BOOST_AUTO_TEST_CASE(Ack_Denier) {
  BOOST_REQUIRE(moduleData.find("module_test7") != moduleData.end());

  ModuleData data = moduleData["module_test7"];
  BOOST_CHECK(data.heartbeat_interval == 100);
  BOOST_CHECK(data.running);
  BOOST_CHECK(!data.buried);
  BOOST_CHECK(data.mortality_rate == DEFAULT_MORTALITY_RATE);
}

/**
 * This check simply checks if all modules in the map follow the naming convetions
 */
BOOST_AUTO_TEST_CASE(Check_Naming) {
  std::map<std::string, ModuleData>::iterator it;

  for (it = moduleData.begin(); it != moduleData.end(); it++) {
    BOOST_CHECK(boost::starts_with(it->first, "module_"));
  }
}

BOOST_AUTO_TEST_SUITE_END()
