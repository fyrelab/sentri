/*
  module_video.h
  Copyright 2016 fyrelab
 */

#include "modules/module_video/image.h"
#include <vector>

#include "modules/module_video/engine.h"

Image::Image(Engine &module, cv::Mat &image, const std::string &name, const ID imageID, const ID camID)
             : image_(image),
               name_(name),
               timeStamp_(boost::posix_time::microsec_clock::local_time()),
               module_(module),
               imageID_(imageID),
               camID_(camID) {
}

Image::~Image() {
  image_.release();
}

Image::Image(const Image &cop) : module_(cop.module_) {
  image_ = cop.image_.clone();
  name_ = cop.name_;
  timeStamp_ = cop.timeStamp_;
  imageID_ = cop.imageID_;
  camID_ = cop.camID_;
}

Image& Image::operator=(Image cop) {
  image_ = cop.image_.clone();
  name_ = cop.name_;
  timeStamp_ = cop.timeStamp_;
  imageID_ = cop.imageID_;
  camID_ = cop.camID_;

  return *this;
}

void Image::drawEllipse(cv::Point center, cv::Size axes, double angle, double startAngle, double endAngle,
const cv::Scalar color, int thickness, int lineType, int shift) {
  try {
    cv::ellipse(image_, center, axes, angle, startAngle, endAngle, color, thickness, lineType, shift);
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Image::drawCircle(cv::Point center, int radius, const cv::Scalar color, int thickness, int lineType, int shift) {
  try {
      cv::circle(image_, center, radius, color, thickness, lineType, shift);
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Image::drawLine(cv::Point p1, cv::Point p2, const cv::Scalar color, int thickness, int lineType, int shift) {
  try {
    cv::line(image_, p1, p2, color, thickness, lineType, shift);
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Image::drawRectangle(cv::Point p1, cv::Point p2, const cv::Scalar color, int thickness, int lineType, int shift) {
  try {
    cv::rectangle(image_, p1, p2, color, thickness, lineType, shift);
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Image::drawRectangle(const cv::Rect rect,  const cv::Scalar color, int thickness, int lineType, int shift) {
  try {
    cv::rectangle(image_, rect, color, thickness, lineType, shift);
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Image::drawPolylines(const cv::Point **pts, const int *npts, int ncontours, bool isClosed, const cv::Scalar &color,
int thickness, int lineType, int shift) {
  try {
    cv::polylines(image_, pts, npts, ncontours, isClosed, color, thickness, lineType, shift);
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Image::drawString(const std::string &text, cv::Point org, int fontFace, double fontScale, cv::Scalar color,
int thickness, int lineType, bool bottomLeftOrigin) {
  try {
    cv::putText(image_, text, org, fontFace, fontScale, color, thickness, lineType, bottomLeftOrigin);
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

float Image::get(int x, int y) {
    return image_.at<float>(x, y);
}

int Image::getRows() {
  return image_.rows;
}

int Image::getCols() {
  return image_.cols;
}

void Image::toGray() {
  cvtColor(image_, image_, CV_RGB2GRAY);
}

void Image::rotate(double angle, double scale, cv::Point p) {
  try {
    cv::Mat rot_mat(2, 3, CV_32FC1);
    rot_mat = cv::getRotationMatrix2D(p, angle, scale);
    cv::warpAffine(image_, image_, rot_mat, image_.size());
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Image::crop(cv::Point p1, cv::Point p2) {
  try {
    image_ = image_(cv::Rect(p1, p2));
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Image::resize(int width, int height) {
  try {
    cv::resize(image_, image_, cv::Size(width, height));
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Image::thresholding(double thresh, double maxval, int type) {
  try {
    cv::threshold(image_, image_, thresh, maxval, type);
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Image::diff(const Image & im, Image & result) {
  try {
    cv::absdiff(image_, im.image_, result.image_);
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Image::bitAnd(const Image & im, Image & result) {
  try {
      cv::bitwise_and(image_, im.image_, result.image_);
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

bool Image::save(const std::string & path, int type, int compression) {
  try {
    std::vector<int> compression_params;

    // type == 1 -> JPEG
    if (type) {
      compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
      // image quality between 0 and 100
      compression_params.push_back(compression);

    // PNG
    } else {
      compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
      // compression between 0 and 9
      compression_params.push_back(compression);
    }

    return imwrite(path, image_, compression_params);
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::ERROR, "OpenCV Exception: " + std::string(e.what()));
  }
  return false;
}

void Image::setBrightness(const cv::Scalar & brightness) {
  try {
      image_ += brightness;
  }
  catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

cv::Mat& Image::getImage() {
  return image_;
}
