/*
  detector.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_DETECTOR_H_
#define SRC_MODULES_MODULE_VIDEO_DETECTOR_H_

#include <vector>
#include "modules/module_video/image.h"

class Engine;  // Forward declaration

/**
 * Specifies the interface for detectors.
 */

class Detector {
 public:
  /**
   * @param  module The module object.
   */
  explicit Detector(Engine &module);
  ~Detector() = default;

  /**
   * Applies a detector and returns the contours of the detection.
   *
   * @param image The image to process.
   * @param rects The vector where the detected contours will be saved.
   */
  virtual void detect(Image &image, std::vector<cv::Rect> &rects) = 0;

 protected:
  Engine &module_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_DETECTOR_H_
