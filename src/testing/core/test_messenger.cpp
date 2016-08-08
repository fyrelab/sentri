/*
  main.cpp
  Copyright 2016 fyrelab
 */

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/chrono.hpp>
#include <string>
#include <exception>

#include "core/messenger.h"
#include "lib_module/configloader.h"
#include "testing/test_util/test_logger.h"

using lib_module::MessageType;

BOOST_AUTO_TEST_SUITE(Test_Messenger)


// ====================================================================================================================
//                                          Create Queue Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(createQueueTest) {
  TestLogger logger = TestLogger("Test_Messenger:createQueueTest");
  Messenger *messenger = new Messenger(logger);
  BOOST_WARN_THROW(messenger->addQueue("TestQueue"), std::exception);
  std::string id = messenger->getUcid() + "-TestQueue";
  message_queue* mq;
  BOOST_WARN_THROW(mq = new message_queue(boost::interprocess::open_only, id.c_str()), std::exception);
  delete mq;
  delete messenger;
}


// ====================================================================================================================
//                                          Send Action Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(sendActionTest) {
  TestLogger logger = TestLogger("Test_Messenger:sendActionTest");
  Messenger *messenger = new Messenger(logger);
  BOOST_WARN_THROW(messenger->addQueue("TestQueue"), std::exception);
  std::string id = messenger->getUcid() + "-TestQueue";
  message_queue* mq;
  BOOST_WARN_THROW(mq = new message_queue(boost::interprocess::open_only, id.c_str()), std::exception);

  // create Evententry
  EventEntry entry;
  entry.event.rule_id = 42;
  std::snprintf(entry.event.vars, sizeof(entry.event.vars), "DassindmeineVars!\\%%79");
  entry.id = 56;

  unsigned int priority = 1;
  message_queue::size_type recvd_size;
  lib_module::Message message;

  BOOST_CHECK_EQUAL(messenger->sendAction("TestQueue", entry), true);

  BOOST_WARN_THROW(mq->try_receive(&message, sizeof(lib_module::Message), recvd_size, priority),
                                                      boost::interprocess::interprocess_exception);

  BOOST_CHECK_EQUAL(message.type, lib_module::MSG_ACTIO);
  BOOST_CHECK_EQUAL(message.body.action.event_id, 56);
  BOOST_CHECK_EQUAL(message.body.action.rule_id, 42);
  BOOST_CHECK_EQUAL(message.body.action.vars, "DassindmeineVars!\\%79");

  // try to send to non existing module, should return false
  logger.log("NoQueueTest", lib_module::INFO, "Error message: 'Module NoQueue not found expected'");
  BOOST_CHECK_EQUAL(messenger->sendAction("NoQueue", entry), false);

  delete mq;
  delete messenger;
}

// ====================================================================================================================
//                                          Send Event Acknowledgement Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(sendEventAcknowledgementTest) {
  TestLogger logger = TestLogger("Test_Messenger:sendEventAcknowledgementTest");
  Messenger *messenger = new Messenger(logger);
  BOOST_WARN_THROW(messenger->addQueue("TestQueue"), std::exception);
  std::string id = messenger->getUcid() + "-TestQueue";
  message_queue* mq;
  BOOST_WARN_THROW(mq = new message_queue(boost::interprocess::open_only, id.c_str()), std::exception);

  // create Evententry
  EventEntry entry;
  entry.event.rule_id = 42;
  std::snprintf(entry.event.vars, sizeof(entry.event.vars), "DassindmeineVars!\\%%79");
  entry.id = 56;

  unsigned int priority = 1;
  message_queue::size_type recvd_size;
  lib_module::Message message;

  BOOST_CHECK_EQUAL(messenger->sendEventAck("TestQueue", entry), true);

  BOOST_WARN_THROW(mq->try_receive(&message, sizeof(lib_module::Message), recvd_size, priority),
                                                      boost::interprocess::interprocess_exception);

  BOOST_CHECK_EQUAL(message.type, lib_module::MSG_ACKEV);
  BOOST_CHECK_EQUAL(message.body.action.event_id, 56);
  BOOST_CHECK_EQUAL(message.body.action.rule_id, 42);
  BOOST_CHECK_EQUAL(message.body.action.vars, "DassindmeineVars!\\%79");

  // try to send to non existing module, should return false
  logger.log("NoQueueTest", lib_module::INFO, "Error message: 'Module NoQueue not found expected'");
  BOOST_CHECK_EQUAL(messenger->sendEventAck("NoQueue", entry), false);

  delete mq;
  delete messenger;
}

// ====================================================================================================================
//                                          Send Event Abort Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(sendEventAbortTest) {
  TestLogger logger = TestLogger("Test_Messenger:sendEventAbortTest");
  Messenger *messenger = new Messenger(logger);
  BOOST_WARN_THROW(messenger->addQueue("TestQueue"), std::exception);
  std::string id = messenger->getUcid() + "-TestQueue";
  message_queue* mq;
  BOOST_WARN_THROW(mq = new message_queue(boost::interprocess::open_only, id.c_str()), std::exception);

  // create Evententry
  EventEntry entry;
  entry.event.rule_id = 42;
  std::snprintf(entry.event.vars, sizeof(entry.event.vars), "DassindmeineVars!\\%%79");
  entry.id = 56;

  unsigned int priority = 1;
  message_queue::size_type recvd_size;
  lib_module::Message message;

  BOOST_CHECK_EQUAL(messenger->sendEventAbort("TestQueue", entry), true);

  BOOST_WARN_THROW(mq->try_receive(&message, sizeof(lib_module::Message), recvd_size, priority),
                                                      boost::interprocess::interprocess_exception);

  BOOST_CHECK_EQUAL(message.type, lib_module::MSG_EVABT);
  BOOST_CHECK_EQUAL(message.body.action.event_id, 56);
  BOOST_CHECK_EQUAL(message.body.action.rule_id, 42);
  BOOST_CHECK_EQUAL(message.body.action.vars, "DassindmeineVars!\\%79");

  // try to send to non existing module, should return false
  logger.log("NoQueueTest", lib_module::INFO, "Error message: 'Module NoQueue not found expected'");
  BOOST_CHECK_EQUAL(messenger->sendEventAbort("NoQueue", entry), false);

  delete mq;
  delete messenger;
}


// ====================================================================================================================
//                                          Core Queue Name Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(coreQueueNameTest) {
  TestLogger logger = TestLogger("Test_Messenger:coreQueueNameTest");
  Messenger *messenger = new Messenger(logger);
  std::string coreQueue;
  coreQueue = messenger->getCoreQueueName();

  BOOST_WARN_THROW(message_queue(boost::interprocess::open_only, coreQueue.c_str()), std::exception);

  delete messenger;
}

BOOST_AUTO_TEST_SUITE_END()
