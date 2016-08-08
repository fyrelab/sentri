/*
  usbpicam.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_USBPICAM_H_
#define SRC_MODULES_MODULE_VIDEO_USBPICAM_H_

#include "modules/module_video/cam.h"

/**
 * Captures frames from an USB or the Raspberry Pi camera.
 */

class USBPICam : public Cam {
 public:
  USBPICam() = delete;
  ~USBPICam();

  /**
   * @param delay_grab Grabbing delay
   * @param url The device id.
   * @param camID Unique ID to distinguish multiple capturing devices.
   * @param module The module object.
   */
  USBPICam(milliseconds delay_grab, int device, ID camID, Engine &module);
  Image* retrieve();
};

#endif  // SRC_MODULES_MODULE_VIDEO_USBPICAM_H_
