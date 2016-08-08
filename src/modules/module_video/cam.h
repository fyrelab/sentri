/*
  cam.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_CAM_H_
#define SRC_MODULES_MODULE_VIDEO_CAM_H_

#include <boost/circular_buffer.hpp>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <string>
#include <vector>
#include "modules/module_video/image.h"

using boost::chrono::milliseconds;

class Engine;  // Forward declaration

/**
 * The interface for all capturing devices. It can handle the RaspberryPI camera, USB and IP cameras.
 */
class Cam {
 protected:
  milliseconds delay_grab_;    //!< Grabs a new frame from the camere every delay_grab_ milliseconds.
  boost::mutex mtx_cam_;       //!< Ensures that race conditions dont't occur when grabbing and retrieving frames.
  cv::VideoCapture cap_;       //!< OpenCV VideoCapture class.
                               //!< ttp://docs.opencv.org/3.1.0/d8/dfe/classcv_1_1VideoCapture.html#gsc.tab=0
  boost::thread* grabThread_;  //!< Asynchronous thread that handles the grabbing of frames.
  ID imageID_;                 //!< Unique ID for every retrieved frame.
  ID camID_;                   //!< Unique ID for every capturing device.
  Engine &module_;

 public:
  /**
   * @param delay_grab Grabbing interval.
   * @param camID Unique ID to distinguish multiple capturing devices.
   * @param module The module object.
   */
  Cam(milliseconds delay_grab, ID camID, Engine &module);

  /**
   * @param delay_grab Grabbing interval.
   * @param device Specifies the device responsible for capturing.
   * @param camID Unique ID to distinguish multiple capturing devices.
   * @param module The module object.
   */
  Cam(milliseconds delay_grab, int device, ID camID, Engine &module);

  /**
   * @param delay_grab Grabbing interval.
   * @param url The URL is used in combination with IP cameras.
   * @param camID Unique ID to distinguish multiple capturing devices.
   * @param module The module object.
   */
  Cam(milliseconds delay_grab, const std::string &url, ID camID, Engine &module);
  virtual ~Cam();

  virtual Image* retrieve() = 0;
  Image* readImage();

  double getWidth();
  double getHeight();
  double getFPS();
  double getRealFPS();
  double getCodec();
  double getBrightness();
  double getContrast();
  double getSaturation();
  double getHue();
  double getGain();

  void setWidth(double width);
  void setHeight(double height);
  void setFPS(double fps);
  void setCodec(double codec);
  void setBrightness(double brightness);
  void setContrast(double contrast);
  void setSaturation(double saturation);
  void setHue(double hue);
  void setGain(double gain);

  bool isOpened();
  void grab();

  /**
   * Starts the grabbing thread.
   */
  void grabAsync();

  /**
   * Stops the grabbing thread.
   */
  void stopGrabAsync();
};

#endif  // SRC_MODULES_MODULE_VIDEO_CAM_H_
