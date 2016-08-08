/*
  heuristic.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_HEURISTIC_H_
#define SRC_MODULES_MODULE_VIDEO_HEURISTIC_H_

#include <opencv2/opencv.hpp>
#include <vector>

class Engine;  // Forward declaration

/**
 * Specifies the interface for all heuristics.
 */

class Heuristic {
 public:
  /**
   * @param  module The module object.
   */
  explicit Heuristic(Engine &module);

  /**
   * @param rects The contours of the motion detection.
   */
  virtual void apply(std::vector<cv::Rect> &rects) = 0;

 protected:
  Engine &module_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_HEURISTIC_H_
