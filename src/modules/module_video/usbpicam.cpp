/*
  usbpicam.cpp
  Copyright 2016 fyrelab
*/

#include "modules/module_video/usbpicam.h"
#include <string>
#include "modules/module_video/engine.h"

USBPICam::USBPICam(milliseconds delay_grab, int device, ID camID, Engine &module)
                   : Cam(delay_grab, device, camID, module) {
}

USBPICam::~USBPICam() {}

Image* USBPICam::retrieve() {
  Image *img = NULL;
  try {
    cv::Mat image_;
    mtx_cam_.lock();
    bool succ = cap_.retrieve(image_);

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
