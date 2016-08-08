/*
  module_firebase/main.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_firebase/firebase_module.h"

int main(int argc, char* argv[]) {
  // Create module
  FirebaseModule module = FirebaseModule(argv[1], argv[2]);

  // Start handling messages (this is blocking)
  module.handleMessages();

  return 0;
}
