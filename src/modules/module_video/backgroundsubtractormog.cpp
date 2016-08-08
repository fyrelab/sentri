/*
  backgroundsubstractormog.h
  Copyright 2016 fyrelab
*/

#include "modules/module_video/backgroundsubtractormog.h"

BackgroundSubtractorMOG::BackgroundSubtractorMOG(int history)
                                    : backgroundSubstractorMOG_(cv::bgsegm::createBackgroundSubtractorMOG(history)) {
}

void BackgroundSubtractorMOG::apply(cv::Mat &in, cv::Mat &out) {
  backgroundSubstractorMOG_->apply(in, out);
}
