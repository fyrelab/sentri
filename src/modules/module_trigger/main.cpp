/*
  main.cpp
  Copyright 2016 fyrelab
 */


#include "modules/module_trigger/trigger_module.h"


int main(int argc, char* argv[]) {
  // Create module
  TriggerModule module = TriggerModule(argv[1], argv[2]);

  // Starts Threads which send events
  module.startSendingEvents();

  // Start sending heartbeats (this is blocking)
  module.sendHeartbeatsForever();

  return 0;
}
