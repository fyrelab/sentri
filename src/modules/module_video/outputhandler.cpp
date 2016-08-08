/*
  outputhandler.cpp
  Copyright 2016 fyrelab
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>

#include <algorithm>

#include "modules/module_video/outputhandler.h"
#include "modules/module_video/engine.h"

#define VIDEO_WIDTH 480
#define VIDEO_HEIGHT 270

OutputHandler::OutputHandler(Engine &module, const std::string &path, int size, std::map<lib_module::RuleID,
                             lib_module::ActionInfo> map)
                             : Output(module),
                               buf_(size),
                               buf2_(10),
                               buf3_(10),
                               path_(path),
                               map_(map),
                               write_(false),
                               pre_(true),
                               post_(false) {
  for (auto &i : map_) {
    // are there any rules with video attachments
    if (i.second.configuration["attachments"].find("video") != std::string::npos) {
      write_ = true;
    }
    // are there any rules with image attachments
    if (i.second.configuration["attachments"].find("image") != std::string::npos) {
      image_ = true;
    }
  }
}

void OutputHandler::handlePendingAttachments(lib_module::Message &m, std::map<std::string, std::string> *map) {
  mtx_.lock();
  if (m.type == lib_module::MSG_EVABT || m.type == lib_module::MSG_ACKEV) {
    // Catch videos
    try {
      std::string name = map->at("video");
      try {
        // remove the video only if all event messages with the same attachment have been aborted or acknowledged.
        // this happens if several rules are combined to one if the event configuration is the same.
        int c = pendingAttachments_[name];
        pendingAttachments_[name] = c - 1;
        if (pendingAttachments_[name] == 0) {
          if (remove(name.c_str()) != 0) {
            module_.log(lib_module::WARNING, "Cannot delete file:" + map->at("video"));
          }
          pendingAttachments_.erase(name);
        }
      } catch (const std::out_of_range& oor) {
        module_.log(lib_module::WARNING, "No such value " + map->at("video") + " in pendingAttachments");
      }
    }
    catch (const std::out_of_range& oor) {
    }

    // Catch images
    try {
      std::string name = map->at("image");
      try {
        // remove the image only if all event messages with the same attachment have been aborted or acknowledged.
        // this happens if several rules are combined to one if the event configuration is the same.
        int c = pendingAttachments_[name];
        pendingAttachments_[name] = c - 1;
        if (pendingAttachments_[name] == 0) {
          if (remove(name.c_str()) != 0) {
            module_.log(lib_module::WARNING, "Cannot delete file:" + map->at("image"));
          }
          pendingAttachments_.erase(name);
        }
      } catch (const std::out_of_range& oor) {
        module_.log(lib_module::WARNING, "No such value " + map->at("images") + " in pendingAttachments");
      }
    }
    catch (const std::out_of_range& oor) {
    }
  } else {
      module_.log(lib_module::WARNING, "Cannot handle message of type: " + std::to_string(m.type));
  }
  mtx_.unlock();
}

void OutputHandler::out(Image &img, bool b) {
  // video as attachment in at least one rule
  if (write_) {
    if (b && post_) {
      // write video with at least 3 frames with motion
      if (buf_.size() + buf2_.size() + buf3_.size() >= 15 && buf_.size() >= 3) {
        forkVideoWriter();
      }
      buf_.clear();
      buf2_.clear();
      buf3_.clear();
      post_ = false;
      pre_ = true;
    }

    if (b) {
      pre_ = false;
      if (buf_.full()) {
        // if buffer is full, write the video in a new process
        forkVideoWriter();
        buf_.clear();
        buf2_.clear();
      }
      buf_.push_back(img);
    } else if (!pre_) {
      post_ = true;
    }

    // always keep the last ten frames with no detection in the buffer
    if (!b && pre_) {
      buf2_.push_back(img);
    }

    // keep the last 10 frames after the detection in the buffer
    if (!b && post_) {
      if (buf3_.full()) {
        if (buf_.size() + buf2_.size() + buf3_.size() >= 15 && buf_.size() >= 3) {
          forkVideoWriter();
        }
        buf_.clear();
        buf2_.clear();
        buf3_.clear();
        post_ = false;
        pre_ = true;
      } else {
        buf3_.push_back(img);
      }
    }
  }
  // image as attachment in at least one rule
  if (image_) {
    if (b) {
      forkImageWriter(img);
    }
  }

  // send event messages without attachments
  for (auto &i : map_) {
    if (i.second.configuration["attachments"].find("video") == std::string::npos &&
                          i.second.configuration["attachments"].find("image") == std::string::npos ) {
      module_.sendEventMessage(i.first, "");
    }
  }
}

void OutputHandler::writeImage(std::string pathImage, Image &img) {
  img.save(pathImage, 1, 50);  // Middle compression quality (0 low)
  for (auto &i : map_) {
    if (i.second.configuration["attachments"].find("image") != std::string::npos) {
      module_.sendEventMessage(i.first, "image=" + pathImage);
    }
  }
}

void OutputHandler::writeVideo(std::string pathVideo) {
  /*
  Another small note. If you are using Mac OS, then what you need to use is 'A','V','C','1' or 'M','P','4','V'.
  From experience, 'H','2','6','4'or 'X','2','6','4'when trying to specify the FourCC code doesn't seem to work.
  CV_FOURCC('X', '2', '6', '4')
  */
  cv::VideoWriter writer(pathVideo, cv::VideoWriter::fourcc('X', '2', '6', '4'),
                          10, cv::Size(VIDEO_WIDTH, VIDEO_HEIGHT), true);
  if (!writer.isOpened()) {
    module_.log(lib_module::ERROR, "Cannot open VideoWriter: " + path_);
    exit(EXIT_FAILURE);
  }
  // write pre frames
  for (unsigned int i = 0; i < buf2_.size(); i++) {
    cv::resize(buf2_[i].getImage(), buf2_[i].getImage(), cv::Size(VIDEO_WIDTH, VIDEO_HEIGHT));
    writer << buf2_[i].getImage();
  }

  // write frames with motion
  for (unsigned int i = 0; i < buf_.size(); i++) {
    cv::resize(buf_[i].getImage(), buf_[i].getImage(), cv::Size(VIDEO_WIDTH, VIDEO_HEIGHT));
    writer << buf_[i].getImage();
  }

  // write post frames
  for (unsigned int i = 0; i < buf3_.size(); i++) {
    cv::resize(buf3_[i].getImage(), buf3_[i].getImage(), cv::Size(VIDEO_WIDTH, VIDEO_HEIGHT));
    writer << buf3_[i].getImage();
  }
  writer.release();

  for (auto &i : map_) {
    if (i.second.configuration["attachments"].find("video") != std::string::npos) {
      module_.sendEventMessage(i.first, "video=" + pathVideo);
    }
  }
}

void OutputHandler::forkImageWriter(Image &img) {
    std::string name(path_ + "img/" + "cam_id_" + std::to_string(img.camID_) + "_img_id_" +
                     std::to_string(img.imageID_) + "_" +
                     boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()) + ".jpg");
    mtx_.lock();
    for (auto &i : map_) {
      if (i.second.configuration["attachments"].find("image") != std::string::npos) {
        try {
          // add atatchment to map
          int c = pendingAttachments_[name];
          pendingAttachments_[name] = c + 1;
        } catch (const std::out_of_range& oor) {
          pendingAttachments_[name] = 1;
        }
      }
    }
    mtx_.unlock();

    // double fork to prevent zombies
    pid_t pid1 = fork();
    int status;

    // double fork to prevent zombies
    if (pid1 < 0) {
      module_.log(lib_module::ERROR, "Cannot fork module_image_writer");
      exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
      pid_t pid2 = fork();
      if (pid2 > 0) {
        exit(EXIT_SUCCESS);
      }
      if (pid2 < 0) {
        module_.log(lib_module::ERROR, "Cannot fork module_image_writer");
        exit(EXIT_FAILURE);
      }
      // module_.log(lib_module::TRACE, "PID writer: " +  std::to_string(getpid()));
      writeImage(name, img);
      exit(EXIT_SUCCESS);
    }
    wait(&status);
}

void OutputHandler::forkVideoWriter() {
    std::string name(path_ + "video/" +"cam_id_" + std::to_string(buf_[0].camID_) + "_img_id_" +
                        std::to_string(buf_[0].imageID_) + "_" +
                        boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()) + ".mp4");
    mtx_.lock();
    for (auto &i : map_) {
      if (i.second.configuration["attachments"].find("video") != std::string::npos) {
        try {
          // add atatchment to map
          int c = pendingAttachments_[name];
          pendingAttachments_[name] = c + 1;
        } catch (const std::out_of_range& oor) {
          pendingAttachments_[name] = 1;
        }
      }
    }
    mtx_.unlock();
    pid_t pid1 = fork();
    int status;

    // double fork to prevent zombies
    if (pid1 < 0) {
      module_.log(lib_module::ERROR, "Cannot fork module_video_writer");
      exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
      pid_t pid2 = fork();
      if (pid2 > 0) {
        exit(EXIT_SUCCESS);
      }
      if (pid2 < 0) {
        module_.log(lib_module::ERROR, "Cannot fork module_video_writer");
        exit(EXIT_FAILURE);
      }
      // module_.log(lib_module::TRACE, "PID writer: " +  std::to_string(getpid()));
      writeVideo(name);
      exit(EXIT_SUCCESS);
    }
    wait(&status);
}
