/**
 * Healthy message send/receive module
 */

#include "lib_module/module.h"
#include <boost/thread.hpp>

#define MODULE_NAME "module_test5"

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

  while (true) {
    sendHeartbeat();

    if (!sendEventMessage(0, "")) {  // send rule 0
      std::cerr << MODULE_NAME << ": Could not send event" << std::endl;
      return;
    }

    if (!receiveMessageBlocking(&msg, waitTime, &map)) {
      std::cerr << MODULE_NAME << ": No action message" << std::endl;
      return;
    }
    if (msg.type != lib_module::MSG_ACTIO) {
      std::cerr << MODULE_NAME << ": Expected action, got " << msg.type << std::endl;
      return;
    }

    sendHeartbeat();

    if (!acknowledgeEvent(msg.body.action.event_id)) {
      std::cerr << MODULE_NAME << ": Could not send ack" << std::endl;
      return;
    }

    if (!receiveMessageBlocking(&msg, waitTime, &map)) {
      std::cerr << MODULE_NAME << ": No ack message" << std::endl;
      return;
    }
    if (msg.type != lib_module::MSG_ACKEV) {
      std::cerr << MODULE_NAME << ": Expected ack, got " << msg.type << std::endl;
      return;
    }

    sendHeartbeat();
    boost::this_thread::sleep(boost::posix_time::milliseconds(waitTime));
  }
}

int main(int argc, char* argv[]) {
  // Create module
  TestModule module = TestModule(argv[1], argv[2]);
  module.work();

  return 0;
}
