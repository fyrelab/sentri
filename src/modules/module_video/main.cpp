/*
  main.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_video/engine.h"
#include "lib_module/module.h"


int main(int argc, char* argv[]) {
  if (argc < 3) {
    exit(EXIT_FAILURE);
  }

  Engine e(argv[1], argv[2]);
  e.start();
  return 0;
}
