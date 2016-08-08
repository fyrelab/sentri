/*
  backgroundsubstractormog2.h
  Copyright 2016 fyrelab
*/

#include "modules/module_video/backgroundsubtractormog2.h"

BackgroundSubtractorMOG2::BackgroundSubtractorMOG2(int history, double backgroundThreshold, bool shadow)
              : backgroundSubstractorMOG2_(cv::createBackgroundSubtractorMOG2(history, backgroundThreshold, shadow)) {
}

void BackgroundSubtractorMOG2::apply(cv::Mat &in, cv::Mat &out) {
  backgroundSubstractorMOG2_->apply(in, out);
}
