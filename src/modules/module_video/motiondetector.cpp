/*
  motiondetectormog2.cpp
  Copyright 2016 fyrelab
*/

#include "modules/module_video/motiondetector.h"

#include <string>

#include "modules/module_video/engine.h"

MotionDetector::MotionDetector(Engine &module, std::shared_ptr<BackgroundSubtractor> backgroundSub,
                               unsigned int blurSize, unsigned int motionThreshold, unsigned int dilationIterations,
                               unsigned int dilationSize, double contoursThreshold)
                               : Detector(module),
                                 backgroundSub_(backgroundSub),
                                 blurSize_(blurSize),
                                 motionThreshold_(motionThreshold),
                                 dilationIterations_(dilationIterations),
                                 dilationKernel_(getStructuringElement(cv::MORPH_RECT,
                                                                  cv::Size(2 * dilationSize + 1, 2 * dilationSize + 1),
                                 cv :: Point(-1, -1))),
                                 contoursThreshold_(contoursThreshold) {
}

MotionDetector::~MotionDetector() {
}

void MotionDetector::detect(Image &input, std::vector<cv::Rect> &rects) {
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  try {
    // convert to grayscale image
    cv::cvtColor(input.getImage(), foreGroundMask_, CV_BGR2GRAY);

    // noise reduction in the image
    cv::GaussianBlur(foreGroundMask_, foreGroundMask_, cv::Size(blurSize_, blurSize_), 0, 0);

    // apply backgroundsubtractor and save calculated foreground mask
    backgroundSub_->apply(foreGroundMask_, foreGroundMask_);

    // apply threshold to the image. Pixel(x,y) >= motionThreshold_ then Pixel(x,y) = 255 (white)
    cv::threshold(foreGroundMask_, foreGroundMask_, motionThreshold_, 255, CV_THRESH_BINARY);

    // dilate white regions
    cv::dilate(foreGroundMask_, foreGroundMask_, dilationKernel_, cv::Point(-1, -1), dilationIterations_);
    // find contours
    cv::findContours(foreGroundMask_.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    // create bounding rects and filter out too small regions
    for (auto &i : contours) {
      if (cv::contourArea(i) >= contoursThreshold_) {
        rects.push_back(cv::boundingRect(i));
      }
    }
  }
  catch(cv::Exception &e) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}
