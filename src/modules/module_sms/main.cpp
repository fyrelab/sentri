/*
  main.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_sms/sms_module.h"

int main(int argc, char* argv[]) {
  // Create module
  SMSModule module = SMSModule(argv[1], argv[2]);

  // Start handling messages (this is blocking)
  module.handleMessages();

  return 0;
}
