/*
  main.cpp
  Copyright 2016 fyrelab
 */

#define BOOST_TEST_NO_MAIN
#define FILE_PATH "../../../../src/testing/test_rules.conf"
#include <boost/test/unit_test.hpp>
#include <exception>

#include "core/ruleengine.h"
#include "lib_module/configloader.h"
#include "testing/test_util/test_logger.h"
#include "core/sentri.h"
#include "lib_module/configloader.h"

BOOST_AUTO_TEST_SUITE(Test_Ruleengine)

// ====================================================================================================================
//                                          Enqueue Event Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(enqueueEvent_test) {
  TestLogger logger = TestLogger("Test_Ruleengine:enqueueEvent_test");

  // We need a message queue to test what the rule engine returns
  Messenger *messenger = new Messenger(logger);
  messenger->addQueue("module_test4");
  messenger->addQueue("module_test3");
  std::string action_module_id = messenger->getUcid() + "-module_test4";
  std::string event_module_id = messenger->getUcid() + "-module_test3";
  message_queue *action_mq = new message_queue(boost::interprocess::open_only, action_module_id.c_str());
  message_queue *event_mq = new message_queue(boost::interprocess::open_only, event_module_id.c_str());

  lib_module::ConfigLoaderJ configLoader = lib_module::ConfigLoaderJ("", "", FILE_PATH, logger);
  EventEntry entry;

  entry.event.rule_id = 2;
  RuleEngine *engine_ = new RuleEngine(*configLoader.getRules("main"), *messenger, logger);
  BOOST_WARN_THROW(engine_->RuleEngine::enqueueEvent(entry), boost::interprocess::interprocess_exception);
  BOOST_CHECK_EQUAL(entry.event.rule_id, 2);

  // Check the message queue for the incoming action message
  unsigned int priority = 1;
  message_queue::size_type recvd_size;
  lib_module::Message message;
  action_mq->try_receive(&message, sizeof(lib_module::Message), recvd_size, priority);
  BOOST_CHECK_EQUAL(message.type, lib_module::MSG_ACTIO);
  BOOST_CHECK_EQUAL(message.body.action.rule_id, 2);

  // Shoud send abort message because the event was enqueued too fast, violating the fire limit
  BOOST_WARN_THROW(engine_->RuleEngine::enqueueEvent(entry), boost::interprocess::interprocess_exception);
  BOOST_CHECK_EQUAL(entry.event.rule_id, 2);
  lib_module::Message message2;

  event_mq->try_receive(&message2, sizeof(lib_module::Message), recvd_size, priority);
  BOOST_CHECK_EQUAL(message2.type, lib_module::MSG_EVABT);

  // sleep for the time of the firelimit
  logger.log("enqueueEvent_test", lib_module::INFO, "Sleeps for 6s to test firelimit. Be patient!");
  sleep(6);

  // Should work again because we slept some time
  BOOST_WARN_THROW(engine_->RuleEngine::enqueueEvent(entry), boost::interprocess::interprocess_exception);
  BOOST_CHECK_EQUAL(entry.event.rule_id, 2);
  lib_module::Message message3;

  action_mq->try_receive(&message3, sizeof(lib_module::Message), recvd_size, priority);
  BOOST_CHECK_EQUAL(message3.type, lib_module::MSG_ACTIO);

  delete engine_;
  delete messenger;
}


// ====================================================================================================================
//                                      Accept Acknowledgement Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(acceptAck_test) {
  TestLogger logger = TestLogger("Test_Ruleengine:acceptAck_test");

  // We need a message queue to test what the rule engine returns
  Messenger *messenger = new Messenger(logger);
  messenger->addQueue("module_test4");
  messenger->addQueue("module_test3");
  std::string id = messenger->getUcid() + "-module_test3";
  message_queue *mq = new message_queue(boost::interprocess::open_only, id.c_str());

  lib_module::ConfigLoaderJ configLoader = lib_module::ConfigLoaderJ("", "", FILE_PATH, logger);
  EventEntry entry;
  lib_module::EventID event_id = 0;
  entry.event.rule_id = 2;

  RuleEngine *engine_ = new RuleEngine(*configLoader.getRules("main"), *messenger, logger);
  // Enqueue an event and let it send back an acknowledged
  engine_->RuleEngine::enqueueEvent(entry);
  BOOST_WARN_THROW(engine_->RuleEngine::acceptAck(event_id), boost::interprocess::interprocess_exception);

  // Check the message queue for the incoming event acknowledgement message
  unsigned int priority = 1;
  message_queue::size_type recvd_size;
  lib_module::Message message;
  mq->try_receive(&message, sizeof(lib_module::Message), recvd_size, priority);
  BOOST_CHECK_EQUAL(message.type, lib_module::MSG_ACKEV);



  delete engine_;
  delete messenger;
}


// ====================================================================================================================
//                                    Acknowledgement Check Async Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(ackCheckAsync_test) {
  TestLogger logger = TestLogger("Test_Ruleengine:ackCheckAsync_test");
  Messenger *messenger = new Messenger(logger);
  messenger->addQueue("module_test4");
  messenger->addQueue("module_test3");
  std::string action_module_id = messenger->getUcid() + "-module_test4";
  message_queue *action_mq = new message_queue(boost::interprocess::open_only, action_module_id.c_str());

  lib_module::ConfigLoaderJ configLoader = lib_module::ConfigLoaderJ("", "", FILE_PATH, logger);
  EventEntry entry;
  entry.event.rule_id = 2;

  RuleEngine *engine_ = new RuleEngine(*configLoader.getRules("main"), *messenger, logger);
  BOOST_WARN_THROW(engine_->startAckCheckAsync(), boost::interprocess::interprocess_exception);

  BOOST_WARN_THROW(engine_->enqueueEvent(entry), boost::interprocess::interprocess_exception);

  for (int i = 0; i < NUM_ACTION_RETRIES + 1; i++) {
    logger.log("ackCheckAsync_test", lib_module::INFO, "Error message: 'No ack received, retry applying action with RULE_ID 2' expected");

    // Check the message queue for the incoming action message
    unsigned int priority = 1;
    message_queue::size_type recvd_size;
    lib_module::Message message;
    BOOST_CHECK_EQUAL(action_mq->try_receive(&message, sizeof(lib_module::Message), recvd_size, priority), true);
    BOOST_CHECK_EQUAL(message.type, lib_module::MSG_ACTIO);
    BOOST_CHECK_EQUAL(message.body.action.rule_id, 2);
    sleep(ACK_TIMEOUT / 1000 + 1);
  }

  // There should be no other message
  unsigned int priority = 1;
  message_queue::size_type recvd_size;
  lib_module::Message message;
  BOOST_CHECK_EQUAL(action_mq->try_receive(&message, sizeof(lib_module::Message), recvd_size, priority), false);

  delete engine_;
  delete messenger;
}

// ====================================================================================================================
//                                   System Message Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(systemMessage_test) {
  TestLogger logger = TestLogger("Test_Ruleengine:ackCheckAsync_test");
  Messenger *messenger = new Messenger(logger);
  messenger->addQueue("module_test4");
  std::string action_module_id = messenger->getUcid() + "-module_test4";
  message_queue *action_mq = new message_queue(boost::interprocess::open_only, action_module_id.c_str());

  lib_module::ConfigLoaderJ configLoader = lib_module::ConfigLoaderJ("", "", FILE_PATH, logger);

  RuleEngine *engine_ = new RuleEngine(*configLoader.getRules("main"), *messenger, logger);
  BOOST_WARN_THROW(engine_->systemMessage("Test Message"), boost::interprocess::interprocess_exception);

  // Check the message queue for the incoming action message
  unsigned int priority = 1;
  message_queue::size_type recvd_size;
  lib_module::Message message;
  BOOST_CHECK_EQUAL(action_mq->try_receive(&message, sizeof(lib_module::Message), recvd_size, priority), true);

  BOOST_CHECK_EQUAL(message.body.action.vars, "system_message=Test Message");

  delete engine_;
  delete messenger;
}



BOOST_AUTO_TEST_SUITE_END()
