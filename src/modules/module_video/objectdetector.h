/*
  objectdetector.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_OBJECTDETECTOR_H_
#define SRC_MODULES_MODULE_VIDEO_OBJECTDETECTOR_H_

#include <vector>
#include <string>
#include "modules/module_video/image.h"
#include "modules/module_video/detector.h"

/**
 * Detects objects with haar cascade classifier.
 * Read http://docs.opencv.org/2.4/modules/objdetect/doc/cascade_classification.html for further information.
 */

class ObjectDetector : public Detector{
 public:
  ObjectDetector() = delete;
  ~ObjectDetector() = default;

  /**
   * @param module The module object.
   * @param cascadeFile Path to the cascade file.
   * @param Parameter specifying how much the image size is reduced at each image scale.
   */
  ObjectDetector(Engine &module, const std::string &cascadeFile, double scale = 1.1);
  void detect(Image &input, std::vector<cv::Rect> &rects);
 private:
  cv::CascadeClassifier classifier_;
  double scale_;
  cv::Mat gray_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_OBJECTDETECTOR_H_
