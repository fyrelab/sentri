/*
  main.cpp
  Copyright 2016 fyrelab
 */


#include "modules/module_mail/mail_module.h"


int main(int argc, char* argv[]) {
  // Create module
  MailModule module = MailModule(argv[1], argv[2]);

  // Start handling messages (this is blocking)
  module.handleMessages();

  return 0;
}
