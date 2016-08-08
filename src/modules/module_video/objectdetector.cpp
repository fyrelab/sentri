/*
  motiondetector.cpp
  Copyright 2016 fyrelab
*/

#include "modules/module_video/objectdetector.h"
#include "modules/module_video/engine.h"

ObjectDetector::ObjectDetector(Engine &module, const std::string &cascadeFile, double scale) :
Detector(module), scale_(scale) {
  if (!classifier_.load(cascadeFile)) {
    module_.log(lib_module::FATAL, "Cannot read file: " + std::string(cascadeFile));
    exit(EXIT_FAILURE);
  }
}

void ObjectDetector::detect(Image &input, std::vector<cv::Rect> &rects) {
  // convert image to grayscale
  cvtColor(input.getImage(), gray_, CV_BGR2GRAY);

  // prepocess the image to make the following object detection more precisely.
  // http://docs.opencv.org/2.4/modules/imgproc/doc/histograms.html?highlight=equalizehist
  cv::equalizeHist(gray_, gray_);
  // keep in mind that detectMultiScale clears input std::vector&
  std::vector<cv::Rect> v;

  // apply object detector
  try {
    // detects objects of different sizes in the input image. The detected objects are returned as a list of rectangles.
    classifier_.detectMultiScale(gray_, v, scale_, 8, 0, cv::Size(40, 40));
  } catch (cv::Exception& e) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
  // use move it avoids copying
  rects.insert(rects.end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
}
