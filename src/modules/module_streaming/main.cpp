/*
  main.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_streaming/streaming_module.h"

int main(int argc, char* argv[]) {
  if (argc < 3) {
    exit(EXIT_FAILURE);
  }

  Streaming stream(argv[1], argv[2]);
  stream.start();
  return 0;
}
