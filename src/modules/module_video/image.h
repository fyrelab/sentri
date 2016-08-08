/*
  image.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_IMAGE_H_
#define SRC_MODULES_MODULE_VIDEO_IMAGE_H_

#include <boost/date_time.hpp>
#include <opencv2/opencv.hpp>
#include <string>

class Engine;  // Forward declaration

typedef uint64_t ID;

/**
 * Wrapper around the OpenCV cv::Mat class.
 */

class Image {
 private:
  cv::Mat image_;
  std::string name_;
  boost::posix_time::ptime timeStamp_;
  Engine &module_;

 public:
  Image() = delete;
  ~Image();

  /**
   * @param module The module object.
   * @param image The opencv::Mat image.
   * @param name The name for the image.
   * @param imageID The unique image id.
   * @param camID The unique capturing device id.
   */
  Image(Engine &module, cv::Mat &image, const std::string &name, const ID imageID, const ID camID);

  /**
   * The copy constructor.
   */
  Image(const Image &cop);

  /**
   * The copy assignment operator.
   */
  Image& operator=(Image cop);

  /**
   * @param center     Center of the ellipse
   * @param axes       Half of the size of the ellipse main axes.
   * @param angle      Ellipse rotation angle in degrees.
   * @param startAngle Starting angle of the elliptic arc in degrees.
   * @param endAngle   Ending angle of the elliptic arc in degrees.
   * @param color      Ellipse color.
   * @param thickness  Thickness of the ellipse arc outline, if positive. Otherwise, this indicates that a filled
   *                   ellipse sector is to be drawn.
   * @param lineType   Type of the ellipse boundary. LINE_8 (or omitted) - 8-connected line, LINE_4 - 4-connected line,
   *                   LINE_AA - antialiased line.
   * @param shift      Number of fractional bits in the point coordinates.
   */
  void drawEllipse(cv::Point center, cv::Size axes, double angle, double startAngle, double endAngle,
                   const cv::Scalar color, int thickness, int lineType, int shift);

  /**
   *
   * @param center    Center of the circle.
   * @param radius    Radius of the circle.
   * @param color     Circle color
   * @param thickness Thickness of the ellipse arc outline, if positive. Otherwise, this indicates that a filled
   *                  ellipse sector is to be drawn.
   * @param lineType  Type of the ellipse boundary. LINE_8 (or omitted) - 8-connected line, LINE_4 - 4-connected line,
   *                  LINE_AA - antialiased line.
   * @param shift     Number of fractional bits in the point coordinates.
   */
  void drawCircle(cv::Point center, int radius, const cv::Scalar color, int thickness, int lineType, int shift);

  /**
   * @param p1        First point of the line segment.
   * @param p2        Second point of the line segment.
   * @param color     Line color.
   * @param thickness Thickness of the ellipse arc outline, if positive. Otherwise, this indicates that a filled
   *                  ellipse sector is to be drawn.
   * @param lineType  Type of the ellipse boundary. LINE_8 (or omitted) - 8-connected line, LINE_4 - 4-connected line,
   *                  LINE_AA - antialiased line.
   * @param shift     Number of fractional bits in the point coordinates.
   */
  void drawLine(cv::Point p1, cv::Point p2, const cv::Scalar color, int thickness, int lineType, int shift);

  /**
   * @param p1        Vertex of the rectangle.
   * @param p2        Vertex of the rectangle opposite to pt1 .
   * @param color     Rectangle color or brightness (grayscale image).
   * @param thickness Thickness of lines that make up the rectangle. Negative values, like CV_FILLED , mean that the
   *                  function has to draw a filled rectangle.
   * @param lineType  Type of the line. LINE_8 (or omitted) - 8-connected line, LINE_4 - 4-connected line,
   *                  LINE_AA - antialiased line.
   * @param shift     Number of fractional bits in the point coordinates.
   */
  void drawRectangle(cv::Point p1, cv::Point p2, const cv::Scalar color, int thickness, int lineType, int shift);

  /**
   * @param rect      Rectangle to be drawn.
   * @param color     Rectangle color or brightness (grayscale image).
   * @param thickness Thickness of the ellipse arc outline, if positive. Otherwise, this indicates that a filled
   * ellipse sector is to be drawn.
   * @param lineType  Type of the ellipse boundary. LINE_8 (or omitted) - 8-connected line, LINE_4 - 4-connected line,
   * LINE_AA - antialiased line.
   * @param shift     Number of fractional bits in the point coordinates.
   */
  void drawRectangle(const cv::Rect rect, const cv::Scalar color, int thickness, int lineType, int shift);

  /**
   * @param pts       Array of polygonal curves.
   * @param npts      Array of polygon vertex counters.
   * @param ncontours Number of curves.
   * @param isClosed  Flag indicating whether the drawn polylines are closed or not. If they are closed, the function
   *                   draws a line from the last vertex of each curve to its first vertex.
   * @param color     Polyline color.
   * @param thickness Thickness of the polyline edges.
   * @param lineType  Type of the line segments. LINE_8 (or omitted) - 8-connected line, LINE_4 - 4-connected line,
   *                  LINE_AA - antialiased line.
   * @param shift     Number of fractional bits in the point coordinates.
   */
  void drawPolylines(const cv::Point **pts, const int *npts, int ncontours, bool isClosed, const cv::Scalar &color,
                     int thickness, int lineType, int shift);

  /**
   * @param text             Text string to be drawn.
   * @param org              Bottom-left corner of the text string in the image.
   * @param fontFace         Font type. One of FONT_HERSHEY_SIMPLEX, FONT_HERSHEY_PLAIN, FONT_HERSHEY_DUPLEX,
   *                         FONT_HERSHEY_COMPLEX, FONT_HERSHEY_TRIPLEX, FONT_HERSHEY_COMPLEX_SMALL,
   *                         FONT_HERSHEY_SCRIPT_SIMPLEX, or FONT_HERSHEY_SCRIPT_COMPLEX, where each of the font
   *                         ID’s can be combined with FONT_ITALIC to get the slanted letters.
   * @param fontScale        Font scale factor that is multiplied by the font-specific base size.
   * @param color            Text color.
   * @param thickness        Thickness of the lines used to draw a text.
   * @param lineType         Type of the Line. LINE_8 (or omitted) - 8-connected line, LINE_4 - 4-connected line,
   *                         LINE_AA - antialiased line.
   * @param bottomLeftOrigin When true, the image data origin is at the bottom-left corner. Otherwise,
   *                         it is at the top-left corner.
   */
  void drawString(const std::string &text, cv::Point org, int fontFace, double fontScale, cv::Scalar color,
                  int thickness, int lineType, bool bottomLeftOrigin);

  /**
   *
   * @param x Position x
   * @param y Position y
   *
   * @return Pixel value at Point(x,y).
   */
  float get(int x, int y);
  cv::Mat &getImage();
  int getRows();
  int getCols();
  void toGray();
  void rotate(double angle, double scale, cv::Point p);
  void crop(cv::Point p1, cv::Point p2);
  void resize(int width, int height);
  void thresholding(double thresh, double maxval, int type);

  /**
   * Calculates the per-element absolute difference between images.
   *
   * @param im     Input image
   * @param result Diff image
   */
  void diff(const Image &im, Image &result);

  /**
   * Calculates the per-element bit-wise conjunction of images.
   *
   * @param im     Input image
   * @param result Bitwise and image
   */
  void bitAnd(const Image &img, Image &result);

  /**
   * Saves the image.
   *
   * @param path        Name of the file.
   * @param type        1 = JPEG, 0 = PNG
   * @param compression JPEG: quality from 0 to 100 (the higher is the better), PNG: compression level from 0 to 9.
   *                    A higher value means a smaller size and longer compression time.
   */
  bool save(const std::string &path, int type, int compression);
  void setBrightness(const cv::Scalar &brightness);
  ID imageID_;
  ID camID_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_IMAGE_H_
