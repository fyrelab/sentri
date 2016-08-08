/*
  backgroundsubtractormog.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_BACKGROUNDSUBTRACTORMOG_H_
#define SRC_MODULES_MODULE_VIDEO_BACKGROUNDSUBTRACTORMOG_H_

#include <opencv2/bgsegm.hpp>
#include "modules/module_video/backgroundsubtractor.h"

/**
 * Uses the BackgroundSubtractorMOG from OpenCV.
 * For futher information visit: http://docs.opencv.org/3.1.0/d6/da7/classcv_1_1bgsegm_1_1BackgroundSubtractorMOG.html#gsc.tab=0
 */

class BackgroundSubtractorMOG : public BackgroundSubtractor {
 public:
   /**
    * @param  history The number of consecutive frames that affect the background model.
    */
  explicit BackgroundSubtractorMOG(int history = 50);
  ~BackgroundSubtractorMOG() = default;
  void apply(cv::Mat &in, cv::Mat &out);

 private:
  cv::Ptr<cv::BackgroundSubtractor> backgroundSubstractorMOG_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_BACKGROUNDSUBTRACTORMOG_H_
