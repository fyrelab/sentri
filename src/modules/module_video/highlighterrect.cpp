/*
  highlighterrect.cpp
  Copyright 2016 fyrelab
*/

#include "modules/module_video/highlighterrect.h"

#include <vector>

class Engine;  // Forward declaration

HighlighterRect::HighlighterRect(Engine &module, int thickness, cv::Scalar color)
                                 : Highlighter(module),
                                   thickness_(thickness),
                                   color_(color) {
}

void HighlighterRect::highlight(Image& image, std::vector<cv::Rect>& rects) {
  for (auto &i : rects) {
    image.drawRectangle(i, color_, thickness_, 8, 0);
  }
}
