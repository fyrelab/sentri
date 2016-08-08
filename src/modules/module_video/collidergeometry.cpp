/*
  collidergeometry.cpp
  Copyright 2016 fyrelab
*/

#include "modules/module_video/collidergeometry.h"

#include <boost/range/algorithm_ext/erase.hpp>
#include <iostream>

ColliderGeometry::ColliderGeometry(Engine &module, std::vector<Geometry> geometry) : Collider(module),
                                                                                     geometry_(geometry) {
}

void ColliderGeometry::apply(std::vector<cv::Rect>& rects) {
  // removes rectangles which don't intersect one of the given geometries.
  boost::remove_erase_if(rects, [this] (const cv::Rect& rec) {
    // top left corner
    float x1 = rec.x;
    float y1 = rec.y;

    // bottom right corner
    float x2 = x1 + rec.width;
    float y2 = y1 + rec.height;
    box rect(point(x1, y1), point(x2, y2));
    bool intersect = false;
    for (auto &geo : geometry_) {
      if (boost::geometry::intersects(rect, geo)) {
        intersect = true;
      }
    }
    return !intersect;
  });
}
