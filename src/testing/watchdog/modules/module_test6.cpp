/**
 * Module flooding events and heartbeats to itself but not clearing its queue
 */

#include "lib_module/module.h"
#include <boost/thread.hpp>

#define MODULE_NAME "module_test6"

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

  if (actualRate == -1) {
    return;
  }

  while (true) {
    sendHeartbeat();
    sendEventMessage(1, "");  // send rule 1
  }
}

int main(int argc, char* argv[]) {
  // Create module
  TestModule module = TestModule(argv[1], argv[2]);
  module.work();

  return 0;
}
