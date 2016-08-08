/*
  main.cpp
  Copyright 2016 fyrelab
 */

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>

#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>

#include "lib_module/configloader.h"
#include "testing/test_util/test_logger.h"


BOOST_AUTO_TEST_SUITE(Test_Configloader)

// ====================================================================================================================
//                                          Load And Output Config
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(loadAndOutputConfig) {
  TestLogger logger = TestLogger("Test_Configloader:loadAndOutputConfig");
  lib_module::ConfigLoaderJ configL("../../../../src/testing/module-info_test.conf", "../../../../src/testing/module-conf_test.conf", "../../../../src/testing/test_rules.conf", logger);

  std::vector<lib_module::EventInfo> *events = configL.getEvents("audio");
  std::vector<lib_module::ActionInfo> *actions = configL.getActions("audio");
  std::vector<lib_module::RuleInfo> *rules = configL.getRules("main");
  std::vector<lib_module::RuleInfo> *trigger_rules = configL.getRules("module_test4");

  BOOST_CHECK_EQUAL(actions->at(0).key, "test_action");
  BOOST_CHECK_EQUAL(actions->at(0).description, "This is a test action.");
  BOOST_CHECK_EQUAL(actions->at(0).module, "audio");
  BOOST_CHECK_EQUAL(events->at(0).key, "test_event");
  BOOST_CHECK_EQUAL(events->at(0).description, "This is a test event.");
  BOOST_CHECK_EQUAL(rules->at(0).id, 0);
  BOOST_CHECK_EQUAL(rules->at(0).action.configuration["action_value_1"], "10.0");
  BOOST_CHECK_EQUAL(rules->at(1).id, 1);
  BOOST_CHECK_EQUAL(rules->at(1).event.configuration["collider_rio"], "BOX(224 81,481 309)");
  BOOST_CHECK_EQUAL(trigger_rules->at(0).id, 2);
  BOOST_CHECK_EQUAL(trigger_rules->at(0).action.configuration["title"], "sentri on your Android phone!");

  delete events;
  delete actions;
  delete rules;
  delete trigger_rules;
}


// ====================================================================================================================
//                                          Get Config Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(getConfig_test) {
  TestLogger logger = TestLogger("Test_Configloader:getConfig_test");
  lib_module::ConfigLoaderJ configL("../../../../src/testing/module-info_test.conf", "../../../../src/testing/module-conf_test.conf", "../../../../src/testing/test_rules.conf", logger);
  std::map<std::string, std::string> *config = configL.getConfig();
  std::vector<lib_module::RuleInfo> *rules = configL.getRules("main");

  BOOST_CHECK_EQUAL(config->at("test_value_1"), "1200000.0");
  BOOST_CHECK_EQUAL(config->at("test_value_2"), "otherValue");
  BOOST_CHECK_EQUAL(config->at("test_value_3"), "defaultValue");
  BOOST_CHECK_EQUAL(rules->at(0).action.configuration["action_value_1"], "10.0");
  BOOST_CHECK_EQUAL(rules->at(1).action.configuration["subject"], "sentri alert");
  BOOST_CHECK_EQUAL(rules->at(0).event.configuration["event_value_1"], "10.0");
  BOOST_CHECK_EQUAL(rules->at(1).event.configuration["video_output_number"], "200");
  BOOST_CHECK_EQUAL(rules->at(2).event.configuration["vars"], "value=key");

  delete config;
  delete rules;
}


// ====================================================================================================================
//                                          Get Events Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(getEvents_test) {
  TestLogger logger = TestLogger("Test_Configloader:getEvents_test");
  lib_module::ConfigLoaderJ configL("../../../../src/testing/module-info_test.conf", "../../../../src/testing/module-conf_test.conf", "../../../../src/testing/test_rules.conf", logger);
  std::vector<lib_module::EventInfo> *events = configL.getEvents("cl_test_module");

  BOOST_CHECK_EQUAL(events->at(0).key, "test_event");
  BOOST_CHECK_EQUAL(events->at(0).description, "This is a test event.");
  BOOST_CHECK_EQUAL(events->at(1).key, "test_event2");
  BOOST_CHECK_EQUAL(events->at(1).module, "cl_test_module");

  delete events;
}


// ====================================================================================================================
//                                          Get Actions Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(getActions_test) {
  TestLogger logger = TestLogger("Test_Configloader:getActions_test");
  lib_module::ConfigLoaderJ configL("../../../../src/testing/module-info_test.conf", "../../../../src/testing/module-conf_test.conf", "../../../../src/testing/test_rules.conf", logger);
  std::vector<lib_module::ActionInfo> *actions = configL.getActions("cl_test_module");

  BOOST_CHECK_EQUAL(actions->at(0).key, "test_action");
  BOOST_CHECK_EQUAL(actions->at(0).description, "This is a test action.");
  BOOST_CHECK_EQUAL(actions->at(0).module, "cl_test_module");

  delete actions;
}


// ====================================================================================================================
//                                          Get Rules Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(getRules_test) {
  TestLogger logger = TestLogger("Test_Configloader:getRules_test");
  lib_module::ConfigLoaderJ configL("../../../../src/testing/module-info_test.conf", "../../../../src/testing/module-conf_test.conf", "../../../../src/testing/test_rules.conf", logger);
  std::vector<lib_module::RuleInfo> *rules = configL.getRules("main");
  std::vector<lib_module::RuleInfo> *video_rules = configL.getRules("module_video");

  BOOST_CHECK_EQUAL(rules->at(0).id, 0);
  BOOST_CHECK_EQUAL(rules->at(0).action.configuration["action_value_1"], "10.0");
  BOOST_CHECK_EQUAL(rules->at(1).id, 1);
  BOOST_CHECK_EQUAL(rules->at(1).event.configuration["highlighter"], "false");
  BOOST_CHECK_EQUAL(rules->at(2).id, 2);
  BOOST_CHECK_EQUAL(rules->at(2).event.configuration["timeout-between"], "120");

  BOOST_CHECK_EQUAL(video_rules->size(), 1);
  BOOST_CHECK_EQUAL(video_rules->at(0).id, 1);
  BOOST_CHECK_EQUAL(video_rules->at(0).event.configuration["collider_rio"], "BOX(224 81,481 309)");

  delete rules;
  delete video_rules;
}


BOOST_AUTO_TEST_SUITE_END()
