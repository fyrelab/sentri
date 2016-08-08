/*
  collidergeometry.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_COLLIDERGEOMETRY_H_
#define SRC_MODULES_MODULE_VIDEO_COLLIDERGEOMETRY_H_

#include <opencv2/opencv.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <vector>

#include "modules/module_video/collider.h"

typedef boost::geometry::model::point<int, 2, boost::geometry::cs::cartesian> point;
typedef boost::geometry::model::box<point> box;
typedef boost::geometry::model::polygon<point, false, true> polygon;  // ccw, closed polygon
typedef boost::geometry::model::ring<point, false, true> ring;  // ccw, closed ring
typedef boost::geometry::model::linestring<point> linestring;
typedef boost::variant<polygon, ring, linestring, box, point> Geometry;

#include "modules/module_video/image.h"

/**
 * Filters rectangles out which don't intersect with the given geometries.
 */

class ColliderGeometry : public Collider {
 public:
  ColliderGeometry() = delete;
  ~ColliderGeometry() = default;

  /**
   * @param module The module object.
   * @param geometry Vector of geometries (polygon, box, ring, linestring).
   */
  ColliderGeometry(Engine &module, std::vector<Geometry> geometry);
  void apply(std::vector<cv::Rect> &rects);

 private:
  std::vector<Geometry> geometry_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_COLLIDERGEOMETRY_H_
