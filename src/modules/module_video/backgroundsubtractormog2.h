/*
  backgroundsubtractormog2.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_BACKGROUNDSUBTRACTORMOG2_H_
#define SRC_MODULES_MODULE_VIDEO_BACKGROUNDSUBTRACTORMOG2_H_

#include "modules/module_video/backgroundsubtractor.h"

/**
 * Uses the BackgroundSubtractorMOG2 from OpenCV.
 * For futher information visit:
 * http://docs.opencv.org/3.1.0/d7/d7b/classcv_1_1BackgroundSubtractorMOG2.html#gsc.tab=0
 */

class BackgroundSubtractorMOG2 : public BackgroundSubtractor {
 public:
   /**
    * @param  history             The number of consecutive frames that affect the background model.
    * @param  backgroundThreshold Threshold on the squared Mahalanobis distance to decide whether it is well described
    * by the background model.
    * @param  shadow              Defining whether shadow detection should be enabled.
    */
  explicit BackgroundSubtractorMOG2(int history = 150, double backgroundThreshold = 16, bool shadow = false);
  ~BackgroundSubtractorMOG2() = default;
  void apply(cv::Mat &in, cv::Mat &out);

 private:
  cv::Ptr<cv::BackgroundSubtractor> backgroundSubstractorMOG2_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_BACKGROUNDSUBTRACTORMOG2_H_
