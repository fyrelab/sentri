/*
  ipcam.h
  Copyright 2016 fyrelab
*/

#include "modules/module_video/ipcam.h"

#include <boost/algorithm/string/predicate.hpp>

#include "modules/module_video/engine.h"

IPCam::IPCam(milliseconds delay_grab, const std::string &url, ID camID, Engine &module) :
Cam(delay_grab, url, camID, module) {
}

IPCam::~IPCam() {}

Image* IPCam::retrieve() {
  Image *img = NULL;
  try {
    cv::Mat image_;
    mtx_cam_.lock();

    // read is by ip cameras necessary
    bool succ = cap_.read(image_);

    if (succ) {
      img = new Image(module_, image_, "", imageID_++, camID_);
    }
    mtx_cam_.unlock();
  } catch( cv::Exception& e ) {
    mtx_cam_.unlock();
    module_.log(lib_module::FATAL, "OpenCV Exception: " + std::string(e.what()));
    exit(EXIT_FAILURE);
  }
  return img;
}
