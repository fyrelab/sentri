/*
  main.cpp
  Copyright 2016 fyrelab
 */

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <string>
#include <exception>
#include <map>

#include "lib_module/messagehandler.h"
#include "lib_module/message.h"
#include "lib_module/configloader.h"
#include "testing/test_util/test_logger.h"

using lib_module::MessageHandler;
using lib_module::Message;
using lib_module::MessageType;


BOOST_AUTO_TEST_SUITE(Test_Messagehandler)

// ====================================================================================================================
//                                          Constructor Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(constructor_test) {
  TestLogger logger = TestLogger("Test_Messagehandler:constructor_test");
  std::string messageQueueNames[] = {"lala", "1", "&_#s^", "<>{}", "üöä", "MESSAGE_QUEUE!","$queue"};
  int messageQueueLength = 7;
  MessageHandler *(queues[(messageQueueLength-1)]);

  for (int i = 0; i < messageQueueLength; i++) {
    // create new message queues should work
    BOOST_WARN_THROW((queues[i] = MessageHandler::create(messageQueueNames[i], logger)), std::exception);
  }
  MessageHandler *tmpQueue;

  for (int i = 0; i< messageQueueLength; i++) {
    // open a message queue that exists should work
    BOOST_WARN_THROW(tmpQueue = MessageHandler::open(messageQueueNames[i], logger), std::exception);
  }

  // create a message queue that exists deletes the old queue and creates a new one
  BOOST_WARN_THROW(tmpQueue = MessageHandler::create(messageQueueNames[1], logger), std::exception);

  // open a message queue that not exists creates a new queue
  logger.log("Test_Messagehandler:constructor_test", lib_module::INFO, "Error Message 'Unable to open message queue' is expected");
  BOOST_WARN_THROW(tmpQueue = MessageHandler::open("not_existing", logger), std::exception);
  delete tmpQueue;

  for (int i = 0; i < messageQueueLength; i++) {
    delete queues[i];
  }
}

// ====================================================================================================================
//                                          Send and Receive Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(send_and_receive_test) {
  TestLogger logger = TestLogger("Test_Messagehandler:send_and_receive_test");
  MessageHandler *sender = MessageHandler::create("testQueue", logger);
  MessageHandler *receiver = MessageHandler::open("testQueue", logger);

  std::string vars1("lkgnlsnadvlksandlknaslknsdlkvnsdlvknalsknaoisdnaoicnsoainscoiansoicn");
  std::string vars2("jbdnasnlmlksmdlaksmclkamslkmlasmlcakmslkam");
  std::string module("dasklng");

  Message sendMessage;
  Message receiveMessage;

  // Check event message
  sendMessage.type = lib_module::MSG_EVENT;
  sendMessage.body.event.rule_id = 1;
  std::snprintf(sendMessage.body.event.vars, sizeof(sendMessage.body.event.vars), "%s", vars1.c_str());

  BOOST_WARN_THROW(sender->sendMessage(sendMessage), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&receiveMessage), std::exception);

  BOOST_CHECK_EQUAL(receiveMessage.type, sendMessage.type);
  BOOST_CHECK_EQUAL(receiveMessage.body.event.rule_id, sendMessage.body.event.rule_id);
  BOOST_CHECK_EQUAL(receiveMessage.body.event.vars, sendMessage.body.event.vars);

  // Check action message
  sendMessage.type = lib_module::MSG_ACTIO;
  sendMessage.body.action.event_id = 3;
  sendMessage.body.action.rule_id = 2;
  std::snprintf(sendMessage.body.action.vars, sizeof(sendMessage.body.action.vars), "%s", vars2.c_str());

  BOOST_WARN_THROW(sender->sendMessage(sendMessage), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&receiveMessage), std::exception);

  BOOST_CHECK_EQUAL(receiveMessage.type, sendMessage.type);
  BOOST_CHECK_EQUAL(receiveMessage.body.action.event_id, sendMessage.body.action.event_id);
  BOOST_CHECK_EQUAL(receiveMessage.body.action.rule_id, sendMessage.body.action.rule_id);
  BOOST_CHECK_EQUAL(receiveMessage.body.action.vars, sendMessage.body.action.vars);


  // Check heartbeat message
  sendMessage.type = lib_module::MSG_HBEAT;
  std::snprintf(sendMessage.body.hbeat.module, sizeof(sendMessage.body.hbeat.module), "%s", module.c_str());

  BOOST_WARN_THROW(sender->sendMessage(sendMessage), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&receiveMessage), std::exception);

  BOOST_CHECK_EQUAL(receiveMessage.type, sendMessage.type);
  BOOST_CHECK_EQUAL(receiveMessage.body.hbeat.module, sendMessage.body.hbeat.module);


  // Check acknowledge message
  sendMessage.type = lib_module::MSG_ACKAC;
  sendMessage.body.action.event_id = 5;

  BOOST_WARN_THROW(sender->sendMessage(sendMessage), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&receiveMessage), std::exception);

  BOOST_CHECK_EQUAL(receiveMessage.type, sendMessage.type);
  BOOST_CHECK_EQUAL(receiveMessage.body.ack.event_id, sendMessage.body.ack.event_id);

  delete sender;
  delete receiver;
}

void receiveLogMessages(MessageHandler *receiver) {
  Message m;
  while (receiver->receiveMessageNonBlocking(&m)) {}
}


// ====================================================================================================================
//                                          Var Parser Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(varparser_test) {
  TestLogger logger = TestLogger("Test_Messagehandler:varparser_test");
  MessageHandler *sender = MessageHandler::create("testQueue2", logger);
  MessageHandler *receiver = MessageHandler::open("testQueue2", logger);

  std::string vars("key1=value1,key2=value2,,"
    "longkeyyesareallylongkeynotjustthislongbutreallyreallylongkeynotenoughtyetbutthelineshouldnotbetolongsodonehere"
    "=longvalyesareallylongvalnotjustthislongbutreallyreallylongvalnotenoughtyetbutthelineshouldnotbetolongsodonehere,"
    "AKey!with?=/and\\afun\"yva1ue");
  std::string varsOneKey("key=value");
  std::string varsEndWithComma("key=value,,");
  std::string varsMoreValues("key=value=value2");
  std::string varsDoubleEqualSign("key==value");
  std::string varsNoValues("key");
  std::string varsEmpty("");

  Message sendMessage;
  Message receiveMessage;

  // create event message with vars
  sendMessage.type = lib_module::MSG_EVENT;
  sendMessage.body.event.rule_id = 1;
  std::snprintf(sendMessage.body.event.vars, sizeof(sendMessage.body.event.vars), "%s", vars.c_str());

  std::map<std::string, std::string> varMap;

  // send and receive message
  BOOST_WARN_THROW(sender->sendMessage(sendMessage), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&receiveMessage, &varMap), std::exception);

  BOOST_CHECK_EQUAL(varMap.size(), 4);
  BOOST_CHECK_EQUAL(varMap["key1"], "value1");
  BOOST_CHECK_EQUAL(varMap["key2"], "value2");
  BOOST_CHECK_EQUAL(varMap[
    "longkeyyesareallylongkeynotjustthislongbutreallyreallylongkeynotenoughtyetbutthelineshouldnotbetolongsodonehere"],
    "longvalyesareallylongvalnotjustthislongbutreallyreallylongvalnotenoughtyetbutthelineshouldnotbetolongsodonehere");
  BOOST_CHECK_EQUAL(varMap["AKey!with?"], "/and\\afun\"yva1ue");

  receiveLogMessages(receiver);

  // copy vars with one key into message
  std::snprintf(sendMessage.body.event.vars, sizeof(sendMessage.body.event.vars), "%s", varsOneKey.c_str());

  varMap.clear();

  // send and receive message
  BOOST_WARN_THROW(sender->sendMessage(sendMessage), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&receiveMessage, &varMap), std::exception);

  BOOST_CHECK_EQUAL(varMap.size(), 1);
  BOOST_CHECK_EQUAL(varMap["key"], "value");

  receiveLogMessages(receiver);

  // copy vars with comma in the end into message
  std::snprintf(sendMessage.body.event.vars, sizeof(sendMessage.body.event.vars), "%s", varsEndWithComma.c_str());

  varMap.clear();

  // send and receive message
  BOOST_WARN_THROW(sender->sendMessage(sendMessage), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&receiveMessage, &varMap), std::exception);

  BOOST_CHECK_EQUAL(varMap.size(), 1);
  BOOST_CHECK_EQUAL(varMap["key"], "value");

  receiveLogMessages(receiver);

  // copy vars with more values per key into message
  std::snprintf(sendMessage.body.event.vars, sizeof(sendMessage.body.event.vars), "%s", varsMoreValues.c_str());

  varMap.clear();

  // send and receive message
  BOOST_WARN_THROW(sender->sendMessage(sendMessage), std::exception);

  // the vars string is not parseable -> throw a exception
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&receiveMessage, &varMap), std::exception);

  BOOST_CHECK_EQUAL(varMap.size(), 1);
  BOOST_CHECK_EQUAL(varMap["key"], "value=value2");

  receiveLogMessages(receiver);

  // copy vars with double equal sign per key into message
  std::snprintf(sendMessage.body.event.vars, sizeof(sendMessage.body.event.vars), "%s", varsDoubleEqualSign.c_str());

  varMap.clear();

  // send and receive message
  BOOST_WARN_THROW(sender->sendMessage(sendMessage), std::exception);

  // the vars string is not parseable -> throw a exception
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&receiveMessage, &varMap), std::exception);

  BOOST_CHECK_EQUAL(varMap.size(), 1);
  BOOST_CHECK_EQUAL(varMap["key"], "=value");

  receiveLogMessages(receiver);

  // copy vars with no value per key into message
  std::snprintf(sendMessage.body.event.vars, sizeof(sendMessage.body.event.vars), "%s", varsNoValues.c_str());

  varMap.clear();

  // send and receive message
  BOOST_WARN_THROW(sender->sendMessage(sendMessage), std::exception);

  // the vars string is not parseable -> throw a exception
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&receiveMessage, &varMap), std::exception);

  BOOST_CHECK_EQUAL(varMap.size(), 0);

  receiveLogMessages(receiver);

  // copy empty vars into message
  std::snprintf(sendMessage.body.event.vars, sizeof(sendMessage.body.event.vars), "%s", varsEmpty.c_str());

  varMap.clear();

  // send and receive message
  BOOST_WARN_THROW(sender->sendMessage(sendMessage), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&receiveMessage, &varMap), std::exception);

  BOOST_CHECK_EQUAL(varMap.size(), 0);

  delete sender;
  delete receiver;
}

// ====================================================================================================================
//                                          Event Message Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(eventMsg_test) {
  TestLogger logger = TestLogger("Test_Messagehandler:eventMsg_test");
  MessageHandler *sender = MessageHandler::create("testQueue3", logger);
  MessageHandler *receiver = MessageHandler::open("testQueue3", logger);

  Message m;
  BOOST_WARN_THROW(sender->sendEventMessage(42, "stringwithvars!"), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&m), std::exception);

  BOOST_CHECK_EQUAL(m.type, lib_module::MSG_EVENT);
  BOOST_CHECK_EQUAL(m.body.event.rule_id, 42);
  BOOST_CHECK_EQUAL(m.body.event.vars, "stringwithvars!");

  BOOST_WARN_THROW(sender->sendEventMessage(0, ""), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&m), std::exception);

  BOOST_CHECK_EQUAL(m.type, lib_module::MSG_EVENT);
  BOOST_CHECK_EQUAL(m.body.event.rule_id, 0);
  BOOST_CHECK_EQUAL(m.body.event.vars, "");

  // Should throw exception if you want to send a negative event id
  // BOOST_CHECK_THROW(sender->sendEventMessage(-5, "var"), std::exception);

  delete sender;
  delete receiver;
}


// ====================================================================================================================
//                                          Send Heartbeat Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(sendHeartbeat_test) {
  TestLogger logger = TestLogger("Test_Messagehandler:sendHeartbeat_test");
  MessageHandler *sender = MessageHandler::create("testQueue3", logger);
  MessageHandler *receiver = MessageHandler::open("testQueue3", logger);

  Message m;
  BOOST_WARN_THROW(sender->sendHeartbeat("moduleName!"), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&m), std::exception);

  BOOST_CHECK_EQUAL(m.type, lib_module::MSG_HBEAT);
  BOOST_CHECK_EQUAL(m.body.hbeat.module, "moduleName!");

  delete sender;
  delete receiver;
}


// ====================================================================================================================
//                                          Acknowledge Event Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(acknowledgeEvent_test) {
  TestLogger logger = TestLogger("Test_Messagehandler:acknowledgeEvent_test");
  MessageHandler *sender = MessageHandler::create("testQueue3", logger);
  MessageHandler *receiver = MessageHandler::open("testQueue3", logger);

  Message m;
  BOOST_WARN_THROW(sender->acknowledgeEvent(43), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&m), std::exception);

  BOOST_CHECK_EQUAL(m.type, lib_module::MSG_ACKAC);
  BOOST_CHECK_EQUAL(m.body.ack.event_id, 43);

  delete sender;
  delete receiver;
}


// ====================================================================================================================
//                                         Set Heartbeat Interval Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(setHeartbeatInterval_test) {
  TestLogger logger = TestLogger("Test_Messagehandler:setHeartbeatInterval_test");
  MessageHandler *sender = MessageHandler::create("testQueue3", logger);
  MessageHandler *receiver = MessageHandler::open("testQueue3", logger);

  Message m;


  int hbeat_interval = 0;
  BOOST_WARN_THROW(hbeat_interval = sender->setHeartbeatInterval("meinModul!", 5000), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&m), std::exception);

  BOOST_CHECK_EQUAL(m.type, lib_module::MSG_HBINT);
  BOOST_CHECK_EQUAL(m.body.hbint.module, "meinModul!");
  BOOST_CHECK_EQUAL(m.body.hbint.interval, 5000);
  BOOST_CHECK_EQUAL(hbeat_interval, 5000);


  hbeat_interval = -1;
  BOOST_WARN_THROW(hbeat_interval = sender->setHeartbeatInterval("meinModul?", 120000), std::exception);
  BOOST_WARN_THROW(receiver->receiveMessageNonBlocking(&m), std::exception);

  BOOST_CHECK_EQUAL(m.type, lib_module::MSG_HBINT);
  BOOST_CHECK_EQUAL(m.body.hbint.module, "meinModul?");
  BOOST_CHECK_EQUAL(m.body.hbint.interval, 15000); //15000 is the global maximum for a heartbeat interval
  BOOST_CHECK_EQUAL(hbeat_interval, 15000);

  delete sender;
  delete receiver;
}


BOOST_AUTO_TEST_SUITE_END()
