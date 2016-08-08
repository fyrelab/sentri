/*
  collider.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_COLLIDER_H_
#define SRC_MODULES_MODULE_VIDEO_COLLIDER_H_

#include <opencv2/opencv.hpp>
#include <vector>

class Engine;  // Forward declaration

/**
 * Specifies the interface for all collision/intersection detection algorithms.
 */

class Collider {
 public:
  /**
   * @param module The module object.
   */
  explicit Collider(Engine &module);

  /**
   * @param rects The detected contours as a vector of rectangles.
   */
  virtual void apply(std::vector<cv::Rect> &rects) = 0;

 protected:
  Engine &module_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_COLLIDER_H_
