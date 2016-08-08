/*
  output.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_OUTPUT_H_
#define SRC_MODULES_MODULE_VIDEO_OUTPUT_H_

#include <map>
#include <string>

#include "modules/module_video/image.h"
#include "lib_module/message.h"

class Engine;  // Forward declaration

/**
 * Specifies the interface for all outputhandlers.
 */

class Output {
 public:
  explicit Output(Engine &module);

  /**
   * @param img The (highlighted) input image.
   * @param b   The number of detected contours.
   */
  virtual void out(Image &img, bool b) = 0;

  /**
   * Is called when ack or abort message is received to remove attachments (images, video).
   *
   * @param m   The Message received from the core module.
   * @param map Map of parsed vars sent in the message.
   */
  virtual void handlePendingAttachments(lib_module::Message &m, std::map<std::string, std::string> *map) = 0;

 protected:
  Engine &module_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_OUTPUT_H_
