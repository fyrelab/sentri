/*
  ipcam.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_IPCAM_H_
#define SRC_MODULES_MODULE_VIDEO_IPCAM_H_

#include <string>
#include "modules/module_video/cam.h"

using boost::chrono::milliseconds;

/**
 * IP Cam
 */

class IPCam : public Cam {
 public:
  IPCam() = delete;
  ~IPCam();

  /**
   * @param delay_grab Grabbing delay
   * @param url The URL is used in combination with IP cameras.
   * @param camID Unique ID to distinguish multiple capturing devices.
   * @param module the module object.
   */
  IPCam(milliseconds delay_grab, const std::string &url, ID camID, Engine &module);
  Image* retrieve();

 private:
  std::string url_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_IPCAM_H_
