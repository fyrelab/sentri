/*
  highlighterrect.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_HIGHLIGHTERRECT_H_
#define SRC_MODULES_MODULE_VIDEO_HIGHLIGHTERRECT_H_

#include <vector>
#include "modules/module_video/highlighter.h"

/**
 * Draws a rectangle around the contours.
 */

class HighlighterRect : public Highlighter {
 public:
  /**
   * @param module The module object.
   * @param thickness The thickness of the line.
   * @param color The color of the line.
   */
  explicit HighlighterRect(Engine &module, int thickness = 1, cv::Scalar color = cv::Scalar(255, 0, 0));
  ~HighlighterRect() = default;
  void highlight(Image &image, std::vector<cv::Rect> &rects);

 private:
  int thickness_;
  cv::Scalar color_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_HIGHLIGHTERRECT_H_
