/*
  backgroundsubstractor.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_BACKGROUNDSUBTRACTOR_H_
#define SRC_MODULES_MODULE_VIDEO_BACKGROUNDSUBTRACTOR_H_

#include <vector>
#include "modules/module_video/image.h"

/**
 * Specifies the interface for all kinds backgroundsubtractors. OpenCV 3.0 includes two types of them:
 * 1. BackgroundSubtractorMOG2
 * http://docs.opencv.org/3.1.0/d7/d7b/classcv_1_1BackgroundSubtractorMOG2.html#gsc.tab=0
 * 2. BackgroundSubtractorMOG
 * http://docs.opencv.org/3.1.0/d6/da7/classcv_1_1bgsegm_1_1BackgroundSubtractorMOG.html#gsc.tab=0
 */

class BackgroundSubtractor {
 public:
  /**
   * Applies the backgroundsubtractor.
   * @param in  Input for the backgroundsubtractor in order to update and calculate the new back- and foreground masks.
   * @param out The new foreground mask calculated by the backgroundsubtractor.
   */
  virtual void apply(cv::Mat &in, cv::Mat &out) = 0;
};

#endif  // SRC_MODULES_MODULE_VIDEO_BACKGROUNDSUBTRACTOR_H_
