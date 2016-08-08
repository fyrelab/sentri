/**
 * Module sends a message with an identifier and waits for abort repeatedly
 */

#include "lib_module/module.h"

#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#include "core/ruleengine.h"

#define MODULE_NAME "module_test7"

class TestModule : lib_module::Module {
 public:
  TestModule(const std::string &sending_queue, const std::string &receiving_queue);
  void work();
};

TestModule::TestModule(const std::string &sending_queue, const std::string &receiving_queue)
  : lib_module::Module(MODULE_NAME, sending_queue, receiving_queue) {
}

void TestModule::work() {
  int actualRate = setHeartbeatInterval(100);  // 100ms heartbeat rate
  lib_module::Message msg;
  std::map<std::string, std::string> map;

  if (actualRate == -1) {
    return;
  }

  int waitTime = actualRate - (0.1 * actualRate);

  for (int i = 0; true; i++) {
    sendHeartbeat();

    // Send message to itself with iterator
    if (!sendEventMessage(2, "i=" + std::to_string(i))) {
      std::cerr << MODULE_NAME << ": Could not send event" << std::endl;
      return;
    }

    // Catch resending messages
    for (int j = 0; j <= NUM_ACTION_RETRIES; j++) {
      while (!receiveMessageBlocking(&msg, waitTime, &map)) {
        sendHeartbeat();
      }

      if (msg.type != lib_module::MSG_ACTIO|| map["i"] != std::to_string(i)) {
        std::cerr << MODULE_NAME << ": Expected action for " << i << ", got " << msg.type << " with " << map["i"] << std::endl;
        return;
      }

      int numHearbeats = (ACK_TIMEOUT + ACK_CHECK_INTERVAL) / waitTime;

      for (int k = 0; k <= numHearbeats; k++) {
        sendHeartbeat();
        boost::this_thread::sleep_for(boost::chrono::milliseconds(waitTime));
      }
    }

    // Time for waiting for abort is up, check if abort is there
    while (!receiveMessageBlocking(&msg, waitTime, &map)) {
      sendHeartbeat();
    }

    if (msg.type != lib_module::MSG_EVABT || map["i"] != std::to_string(i)) {
      std::cerr << MODULE_NAME << ": Expected abort for " << i << ", got " << msg.type << " with " << map["i"] << std::endl;
      return;
    }

    std::cout << MODULE_NAME << ": Received abort" << std::endl;
  }
}

int main(int argc, char* argv[]) {
  // Create module
  TestModule module = TestModule(argv[1], argv[2]);
  module.work();

  return 0;
}
