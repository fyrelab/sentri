/*
  streaming_module.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_MODULES_MODULE_STREAMING_STREAMING_MODULE_H_
#define SRC_MODULES_MODULE_STREAMING_STREAMING_MODULE_H_

#include <lib_module/module.h>

#include <string>

/**
* Segments the captured data to stream with HLS.
*/

class Streaming : public lib_module::Module {
 public:
  Streaming() = delete;

  /**
   * @param sending_queue
   * @param receiving_queue
   */
  Streaming(const std::string &sending_queue, const std::string &receiving_queue);
  void start();
};

#endif  // SRC_MODULES_MODULE_STREAMING_STREAMING_MODULE_H_
