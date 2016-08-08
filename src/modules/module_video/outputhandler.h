/*
  outputhandler.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_OUTPUTHANDLER_H_
#define SRC_MODULES_MODULE_VIDEO_OUTPUTHANDLER_H_

#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>
#include <string>
#include <map>

#include "modules/module_video/image.h"
#include "modules/module_video/output.h"
#include "lib_module/configloader.h"

/**
 * Outputhandler can send videos and images.
 */

class Engine;  // Forward declaration


class OutputHandler : public Output {
 public:
  OutputHandler() = delete;
  ~OutputHandler() = default;

  /**
   * @param module The module object.
   * @param path   Path to the directory where files to be saved.
   * @param        The maximum number of frames of a video.
   * @param        Several rules combined to one if the event configuration is the same.
   */
  OutputHandler(Engine &module, const std::string &path, int size,
                std::map<lib_module::RuleID, lib_module::ActionInfo> map);
  void out(Image &img, bool b);
  void handlePendingAttachments(lib_module::Message &m, std::map<std::string, std::string> *map);

 private:
  boost::circular_buffer<Image> buf_;
  boost::circular_buffer<Image> buf2_;  //!< ten frames before motion detection
  boost::circular_buffer<Image> buf3_;  //!< ten frames after motion detection
  const std::string path_;
  void writeVideo(std::string pathVideo);
  void writeImage(std::string pathImage, Image &img);
  std::map<lib_module::RuleID, lib_module::ActionInfo> map_;
  std::map<std::string, int> pendingAttachments_;     //!< Map of all sent attachments.
  boost::mutex mtx_;             //!< Ensures that race conditions dont't occur when accessing buf_. (multi threading)
  bool write_;
  bool image_;
  void forkVideoWriter();
  void forkImageWriter(Image &img);
  bool pre_;
  bool post_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_OUTPUTHANDLER_H_
