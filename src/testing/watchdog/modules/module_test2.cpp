/**
 * Flood heartbeat module
 */

#include "lib_module/module.h"

#define MODULE_NAME "module_test2"

class TestModule : lib_module::Module {
 public:
  TestModule(const std::string &sending_queue, const std::string &receiving_queue);
  void work();
};

TestModule::TestModule(const std::string &sending_queue, const std::string &receiving_queue)
  : lib_module::Module(MODULE_NAME, sending_queue, receiving_queue) {
}

void TestModule::work() {
  while (true) {
    sendHeartbeat();
  }
}

int main(int argc, char* argv[]) {
  // Create module
  TestModule module = TestModule(argv[1], argv[2]);
  module.work();

  return 0;
}
