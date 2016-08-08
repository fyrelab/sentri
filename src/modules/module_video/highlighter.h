/*
  highlighter.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_HIGHLIGHTER_H_
#define SRC_MODULES_MODULE_VIDEO_HIGHLIGHTER_H_

#include <vector>
#include "modules/module_video/image.h"

class Engine;  // Forward declaration

/**
 * Specifies the interface for all highlighter.
 */

class Highlighter {
 public:
  /**
   * @param  module The module object.
   */
  explicit Highlighter(Engine &module);

  /**
   * @param image Highlights detections in the image.
   * @param rects The contours of the motion detection.
   */
  virtual void highlight(Image &image, std::vector<cv::Rect> &rects) = 0;

 protected:
  Engine &module_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_HIGHLIGHTER_H_
