/*
  cam.cpp
  Copyright 2016 fyrelab
*/

#include "modules/module_video/cam.h"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

#include "modules/module_video/engine.h"

using boost::chrono::milliseconds;

Cam::Cam(milliseconds delay_grab, ID camID, Engine &module) : delay_grab_(delay_grab), cap_(),
                                                      grabThread_(NULL), imageID_(0), camID_(camID), module_(module) {
}

Cam::Cam(milliseconds delay_grab, int device, ID camID, Engine &module) : delay_grab_(delay_grab),
                                        cap_(device), grabThread_(NULL), imageID_(0), camID_(camID), module_(module) {
}

Cam::Cam(milliseconds delay_grab, const std::string& url, ID camID, Engine &module) :
                  delay_grab_(delay_grab), cap_(url), grabThread_(NULL), imageID_(0), camID_(camID), module_(module) {
}

Cam::~Cam() {
  cap_.release();
}

double Cam::getWidth() {return cap_.get(CV_CAP_PROP_FRAME_WIDTH);}
double Cam::getHeight() {return cap_.get(CV_CAP_PROP_FRAME_HEIGHT);}
double Cam::getFPS() {return cap_.get(CV_CAP_PROP_FPS);}
double Cam::getCodec() {return cap_.get(CV_CAP_PROP_FOURCC);}
double Cam::getBrightness() {return cap_.get(CV_CAP_PROP_BRIGHTNESS);}
double Cam::getContrast() {return cap_.get(CV_CAP_PROP_CONTRAST);}
double Cam::getSaturation() {return cap_.get(CV_CAP_PROP_SATURATION);}
double Cam::getHue() {return cap_.get(CV_CAP_PROP_HUE);}
double Cam::getGain() {return cap_.get(CV_CAP_PROP_GAIN);}

void Cam::setWidth(double width) {
  try {
    cap_.set(CV_CAP_PROP_FRAME_WIDTH, width);
  } catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Cam::setHeight(double height) {
  try {
    cap_.set(CV_CAP_PROP_FRAME_HEIGHT, height);
  } catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Cam::setFPS(double fps) {
  try {
    cap_.set(CV_CAP_PROP_FPS, fps);
  } catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Cam::setCodec(double codec) {
  try {
    cap_.set(CV_CAP_PROP_FOURCC, codec);
  } catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Cam::setBrightness(double brightness) {
  try {
    cap_.set(CV_CAP_PROP_BRIGHTNESS, brightness);
  } catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Cam::setContrast(double contrast) {
  try {
    cap_.set(CV_CAP_PROP_CONTRAST, contrast);
  } catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Cam::setSaturation(double saturation) {
  try {
    cap_.set(CV_CAP_PROP_SATURATION, saturation);
  } catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Cam::setHue(double hue) {
  try {
    cap_.set(CV_CAP_PROP_HUE, hue);
  } catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Cam::setGain(double gain) {
  try {
    cap_.set(CV_CAP_PROP_GAIN, gain);
  } catch( cv::Exception& e ) {
    module_.log(lib_module::WARNING, "OpenCV Exception: " + std::string(e.what()));
  }
}

void Cam::grabAsync() {
  if (!grabThread_) {
    grabThread_ = new boost::thread(
          [this]() {
              try {
                while (true) {
                  grab();
                  // Interrupt point
                  boost::this_thread::sleep_for(delay_grab_);
                }
              }
              catch (boost::thread_interrupted&) {
                module_.log(lib_module::WARNING, "Grab Thread Interrupted");
              }
          });
  }
}

void Cam::stopGrabAsync() {
  if (grabThread_) {
    grabThread_->interrupt();
    delete grabThread_;
    grabThread_ = NULL;
  }
}

bool Cam::isOpened() {
  return cap_.isOpened();
}

void Cam::grab() {
  try {
    mtx_cam_.lock();
    // sometimes no frame can be grabbed
    if (!cap_.grab()) {
      module_.log(lib_module::WARNING, "Cam (ID:" + std::to_string(camID_) + ") could not grab next frame");
    }
    mtx_cam_.unlock();
  } catch( cv::Exception& e ) {
    mtx_cam_.unlock();
    module_.log(lib_module::FATAL, "OpenCV Exception: " + std::string(e.what()));
    exit(EXIT_FAILURE);
  }
}

// Used for webcams and cameras
double Cam::getRealFPS() {
  unsigned int num = 80;  // calculates the FPS rate for 80 read frames, grabbing and decoding included.
  boost::posix_time::ptime t1, t2;
  cv::Mat frame;

  // warm up camera
  for (int i = 0; i < 20; i++) {
    cap_ >> frame;
  }

  t1 = boost::posix_time::microsec_clock::local_time();

  for (unsigned int i = 0; i < num; i++) {
    cap_ >> frame;
  }

  t2 = boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration diff = t2 - t1;
  return static_cast<double>(num) / diff.total_seconds();
}

Image* Cam::readImage() {
  Image *img = NULL;
  cv::Mat image_;
  bool succ = cap_.read(image_);

  if (succ) {
    img = new Image(module_, image_, "", imageID_++, camID_);
  }

  return img;
}
