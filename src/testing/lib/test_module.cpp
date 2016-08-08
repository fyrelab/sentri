/*
  test_messagehandler.cpp
  Copyright 2016 fyrelab
 */
 /*
  Test cases dont have to be as extensive as the module class is mostly a summary
  of the methods from the messagehandler and the configloader
*/

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <string>
#include <exception>
#include <utility>
#include <map>

#include "lib_module/module.h"
#include "lib_module/messagehandler.h"
#include "testing/test_util/test_logger.h"
#include "testing/test_util/test_logger.h"

using lib_module::MessageHandler;
using lib_module::Message;
using lib_module::MessageType;

BOOST_AUTO_TEST_SUITE(Test_Module)

// ====================================================================================================================
//                                          Constructor test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(contructorTest) {

  TestLogger logger = TestLogger("Test_Module:Constructor test");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("test_module", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  // Create a new test module
  BOOST_WARN_THROW(TestModule module = TestModule("test_in_queue", "test_out_queue"), std::exception);
}

// ====================================================================================================================
//                                          Get Module Name Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(getModuleNameTest) {

  TestLogger logger = TestLogger("Test_Module:getModuleNameTest");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("module_test", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  TestModule module = TestModule("test_in_queue", "test_out_queue");

  BOOST_CHECK_EQUAL(module.getModuleName(), "module_test");

}

// ====================================================================================================================
//                                          Get Config Loader Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(getConfigLoaderTest) {

  TestLogger logger = TestLogger("Test_Module:getConfigLoaderTest");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("test_module", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  TestModule module = TestModule( "test_in_queue", "test_out_queue");

  // Check whether the config_loader actually works and returns the right value
  BOOST_WARN_THROW(lib_module::ConfigLoaderJ loader = module.getConfigLoader(), std::exception);
  lib_module::ConfigLoaderJ loader = module.getConfigLoader();
  BOOST_CHECK(&loader);
}

// ====================================================================================================================
//                                          Receive Message Blocking Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(receiveMessageBlockingTest) {

  TestLogger logger = TestLogger("Test_Module:receiveMessageBlockingTest");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("test_module", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  TestModule module = TestModule( "test_in_queue", "test_out_queue");

  MessageHandler *test_sender = MessageHandler::open("test_out_queue", logger);
  lib_module::Message send_message;
  lib_module::Message test_message;
  std::map<std::string, std::string> test_map;
  send_message.type = lib_module::MSG_SYS;

  test_sender->sendMessage(send_message);

  BOOST_WARN_THROW(module.receiveMessageBlocking(&test_message, 3000, &test_map), std::exception);
  BOOST_CHECK_EQUAL(test_message.type, lib_module::MSG_SYS);

  delete test_sender;
}

// ====================================================================================================================
//                                          Receive Discard Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(receiveDiscardTest) {

  TestLogger logger = TestLogger("Test_Module:receiveDiscardTest");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("test_module", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  TestModule module = TestModule( "test_in_queue", "test_out_queue");

  MessageHandler *test_sender = MessageHandler::open("test_out_queue", logger);
  lib_module::Message send_message;
  send_message.type = lib_module::MSG_SYS;

  test_sender->sendMessage(send_message);

  BOOST_CHECK_EQUAL(module.receiveDiscard(), true);
  BOOST_CHECK_EQUAL(module.receiveDiscard(), false);

  delete test_sender;
}

// ====================================================================================================================
//                                          Acknowledge Event Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(ackowledgeEventTest) {

  TestLogger logger = TestLogger("Test_Module:ackowledgeEventTest");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("test_module", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  TestModule module = TestModule( "test_in_queue", "test_out_queue");

  MessageHandler *test_receiver = MessageHandler::open("test_in_queue", logger);

  Message m;
  std::map<std::string, std::string> test_map;
  BOOST_WARN_THROW(module.acknowledgeEvent(43), std::exception);
  BOOST_CHECK_EQUAL(test_receiver->receiveMessageBlocking(&m, 3000, &test_map), true);

  BOOST_CHECK_EQUAL(m.type, lib_module::MSG_ACKAC);
  BOOST_CHECK_EQUAL(m.body.ack.event_id, 43);

  delete test_receiver;

}

// ====================================================================================================================
//                                          Send Event Message Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(sendEventMessageTest) {

  TestLogger logger = TestLogger("Test_Module:ackowledgeEventTest");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("test_module", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  TestModule module = TestModule( "test_in_queue", "test_out_queue");

  MessageHandler *test_receiver = MessageHandler::open("test_in_queue", logger);

  Message m;
  BOOST_WARN_THROW(module.sendEventMessage(42, "stringwithvars!"), std::exception);
  BOOST_WARN_THROW(test_receiver->receiveMessageNonBlocking(&m), std::exception);

  BOOST_CHECK_EQUAL(m.type, lib_module::MSG_EVENT);
  BOOST_CHECK_EQUAL(m.body.event.rule_id, 42);
  BOOST_CHECK_EQUAL(m.body.event.vars, "stringwithvars!");

  delete test_receiver;

}

// ====================================================================================================================
//                                          Set Hearbeat Interval Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(setHeartbeatIntervalTest) {

  TestLogger logger = TestLogger("Test_Module:setHeartbeatIntervalTest");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("test_module", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  TestModule module = TestModule( "test_in_queue", "test_out_queue");

  BOOST_CHECK_EQUAL(module.setHeartbeatInterval(14500), 14500);
}

// ====================================================================================================================
//                                          Send Hearbeat Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(setHearbeatTest) {

  TestLogger logger = TestLogger("Test_Module:setHearbeatTest");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("test_module", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  TestModule module = TestModule( "test_in_queue", "test_out_queue");

  BOOST_CHECK_EQUAL(module.sendHeartbeat(), true);
}

// ====================================================================================================================
//                                          Write Hook Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(writeHookTest) {

  TestLogger logger = TestLogger("Test_Module:systemMessageTest");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("test_module", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  TestModule module = TestModule( "test_in_queue", "test_out_queue");

  BOOST_CHECK_EQUAL(module.writeHook("This is a test string"), false);
}

// ====================================================================================================================
//                                          Log Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(logTest) {

  TestLogger logger = TestLogger("Test_Module:logTest");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("test_module", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  TestModule module = TestModule( "test_in_queue", "test_out_queue");

  BOOST_CHECK_EQUAL(module.log(lib_module::INFO, "This is a test string"), true);
}

// ====================================================================================================================
//                                          Replace Vars Test
// ====================================================================================================================

BOOST_AUTO_TEST_CASE(replaceVarsTest) {

  TestLogger logger = TestLogger("Test_Module:replaceVarsTest");
  MessageHandler::create("test_in_queue", logger);
  MessageHandler::create("test_out_queue", logger);

  class TestModule : public lib_module::Module {
  public:
    TestModule(const std::string &sending_queue, const std::string &receiving_queue)
      : lib_module::Module("test_module", sending_queue, receiving_queue) {};
    ~TestModule(){};
  };
  TestModule module = TestModule( "test_in_queue", "test_out_queue");

  std::map<std::string, std::string> test_map;
  test_map.insert(std::pair<std::string, std::string>("replace1", "value1"));
  test_map.insert(std::pair<std::string, std::string>("replace2", ""));
  test_map.insert(std::pair<std::string, std::string>("replace3", "==="));
  test_map.insert(std::pair<std::string, std::string>("replace4", "%"));

  BOOST_CHECK_EQUAL(module.replaceVars("%replace1, %replace2, %replace3, %replace4", test_map),"value1, , ===, %");
}




BOOST_AUTO_TEST_SUITE_END()
