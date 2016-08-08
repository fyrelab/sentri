/*
  engine.h
  Copyright 2016 fyrelab
*/

#include "modules/module_video/engine.h"

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <boost/chrono.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/thread/thread.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>
#include <streambuf>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include <iostream>

#include "modules/module_video/ipcam.h"
#include "modules/module_video/usbpicam.h"
#include "modules/module_video/highlighterrect.h"
#include "modules/module_video/objectdetector.h"
#include "modules/module_video/backgroundsubtractormog.h"
#include "modules/module_video/backgroundsubtractormog2.h"
#include "modules/module_video/backgroundsubtractor.h"
#include "modules/module_video/motiondetector.h"
#include "modules/module_video/outputhandler.h"

#define RECEIVE_MESSAGE_TIMEOUT 30000
#define HBEAT_INT 10000
#define JCROP_IMG_PATH "/tmp/sentri/img/videoimg.jpg"

Engine::Engine(const std::string &sending_queue, const std::string &receiving_queue)
               : lib_module::Module(MODULE_VIDEO_NAME, sending_queue, receiving_queue) {
}

Engine::~Engine() {
  if (th_) {
    th_->interrupt();
    delete th_;
  }
  delete cam_;
}

void Engine::init() {
  sendHeartbeat();
  configure();
  writeHookData();
  applyRules();
  if (enabled_) {
    cam_->grabAsync();
  }
  handleIncomingMsgAsync();
}

void Engine::start() {
  init();
  warmUp();

  Image *image;
  boost::posix_time::time_duration diff;
  boost::chrono::milliseconds sleepTime;
  boost::posix_time::ptime t1;
  boost::posix_time::ptime t2;
  boost::posix_time::ptime last_hbeat = boost::posix_time::microsec_clock::local_time();
  // cv::namedWindow("Video", 0);

  try {
    while (true) {
      t1 = boost::posix_time::microsec_clock::local_time();

      // retrieve grabbed frames
      image = cam_->retrieve();

      // resize to reduce cpu load
      if (image) {
        image->resize(width_, height_);
      }

      // only send a hearbeat and update the jcrop image every 5 seconds
      if ((boost::posix_time::microsec_clock::local_time() - last_hbeat).total_seconds() > 5) {
        sendHeartbeat();
        last_hbeat = boost::posix_time::microsec_clock::local_time();
        if (image) {
          image->save(JCROP_IMG_PATH, 0, 0);
        }
      }

      if (image && enabled_) {
        // contours
        std::vector<cv::Rect> rects;

        // apply detector
        detector_->detect(*image, rects);

        // iterate through rules
        for (auto &route : routes_) {
          Image img(*image);

          std::vector<std::unique_ptr<Collider>> &coll = std::get<0>(route);
          std::vector<std::unique_ptr<Heuristic>> &heur = std::get<1>(route);
          std::vector<std::unique_ptr<Highlighter>> &high = std::get<2>(route);
          std::vector<std::unique_ptr<Output>> &out = std::get<3>(route);

          std::vector<cv::Rect> nRects(rects);

          // Intersection test
          for (auto &i : coll) {
            i->apply(nRects);
          }

          // Heuristic test
          for (auto &i : heur) {
            i->apply(nRects);
          }

          // Highlighting
          for (auto &i : high) {
            i->highlight(img, nRects);
          }

          // Outputhandler
          for (auto &i : out) {
            i->out(img, nRects.size() > 0);
          }

          // cv::imshow("Video", img.getImage());
          // cv::waitKey(5);
        }

         t2 = boost::posix_time::microsec_clock::local_time();

         diff = t2 - t1;

        // if the calculations took more time than the processing rate do not sleep
        sleepTime = delay_take_ - boost::chrono::milliseconds(diff.total_milliseconds());
        if (sleepTime < boost::chrono::milliseconds(0)) {
          boost::this_thread::sleep_for(boost::chrono::milliseconds(0));
        } else {
          boost::this_thread::sleep_for(sleepTime);
        }

      // if there is no rule do nothing, but send heartbeats.
      } else if (!enabled_) {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(HBEAT_INT - 2000));
      }
      delete image;
    }
  } catch (boost::thread_interrupted&) {
    delete image;
  }
}

void Engine::handleIncomingMsgAsync() {
  th_ = new boost::thread(
    [this]() {
        lib_module::Message m;
        while (true) {
          std::map<std::string, std::string> *map = new std::map<std::string, std::string>;
          bool hasMessage = receiveMessageBlocking(&m, RECEIVE_MESSAGE_TIMEOUT, map);

          // Check if message received
          if (hasMessage) {
            // forward messages to all output handlers
            for (auto &route : routes_) {
              std::vector<std::unique_ptr<Output>> &out = std::get<3>(route);
              for (auto &i : out) {
                i->handlePendingAttachments(m, map);
              }
            }
          }
          delete map;
        }
      });
}

bool Engine::isNumber(const std::string &s) {
  return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

void Engine::warmUp() {
  boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
  for (int i = 0; i < 50; i++) {
    Image *image = cam_->readImage();
    delete image;
  }
}

void Engine::configure() {
  videoimgpath_ = lib_module::getModulesConfPath() + "/" + MODULE_VIDEO_NAME + "/videoimg.jpg";

  std::map<std::string, std::string> *conf = getConfigLoader().getConfig();
  setHeartbeatInterval(HBEAT_INT);

  try {
      delay_take_ = boost::chrono::milliseconds(std::stoi(conf->at("processing_rate")));
  } catch (const std::invalid_argument &ia) {
    log(lib_module::LogSeverity::FATAL, "Invalid conversion (processing_rate): " + std::string(ia.what()));
    exit(EXIT_FAILURE);
  } catch (const std::out_of_range &oor) {
    log(lib_module::LogSeverity::FATAL, "Invalid configuration reading 'processing_rate': " + std::string(oor.what()));
    exit(EXIT_FAILURE);
  }

  std::string device;

  try {
    device = conf->at("video");
  } catch (const std::out_of_range &oor) {
    log(lib_module::LogSeverity::FATAL, "Invalid configuration reading 'video': " + std::string(oor.what()));
    exit(EXIT_FAILURE);
  }

  int grabbing_rate;
  ID cam_id;

  try {
    grabbing_rate = std::stoi(conf->at("grab_rate"));
    cam_id = std::stoi(conf->at("cam_id"));
  } catch (const std::invalid_argument& ia) {
    log(lib_module::LogSeverity::FATAL,
        "Invalid conversion ('grab_rate' or 'cam_id'): " + std::string(ia.what()));
    exit(EXIT_FAILURE);
  } catch (const std::out_of_range& oor) {
    log(lib_module::LogSeverity::FATAL,
        "Invalid configuration reading 'grab_rate' or 'cam_id': " + std::string(oor.what()));
    exit(EXIT_FAILURE);
  }
  // real capturing device id
  if (isNumber(device)) {
    try {
      cam_ = new USBPICam(milliseconds(grabbing_rate), std::stoi(device), cam_id, *this);
    } catch (cv::Exception &e) {
        log(lib_module::FATAL, "OpenCV Exception: " + std::string(e.what()));
        exit(EXIT_FAILURE);
    }

  // ip camera
  } else if (boost::starts_with(device, "http") || boost::starts_with(device, "rtp")
                                                || boost::starts_with(device, "rtsp")
                                                || boost::starts_with(device, "rtmp")) {
    try {
      cam_ = new IPCam(milliseconds(grabbing_rate), device, cam_id, *this);
    } catch (cv::Exception &e) {
      log(lib_module::FATAL, "OpenCV Exception: " + std::string(e.what()));
      exit(EXIT_FAILURE);
    }
  }

  // check if camera is opened
  if (!cam_->isOpened()) {
    log(lib_module::FATAL, "Cannot open camera with ID: " + conf->at("cam_id"));
    exit(EXIT_FAILURE);
  }

  try {
    height_ = std::stoi(conf->at("height_frame"));
    width_ = std::stoi(conf->at("width_frame"));
  } catch (const std::invalid_argument& ia) {
    log(lib_module::LogSeverity::FATAL,
        "Invalid conversion ('height_frame' or 'width_frame'): " + std::string(ia.what()));
    exit(EXIT_FAILURE);
  } catch (const std::out_of_range &oor) {
    log(lib_module::LogSeverity::FATAL,
        "Invalid configuration reading 'height_frame' or 'width_frame': " + std::string(oor.what()));
    exit(EXIT_FAILURE);
  }

  // read settings from the info file and apply them
  try {
    int backgroundsubtractor_history = std::stoi(conf->at("backgroundsubtractor_history"));
    int motion_detector_blursize = std::stoi(conf->at("motion_detector_blursize"));
    int motion_detector_threshold = std::stoi(conf->at("motion_detector_threshold"));
    int motion_detector_dilationiterations = std::stoi(conf->at("motion_detector_dilationiterations"));
    int motion_detector_dilationsize = std::stoi(conf->at("motion_detector_dilationsize"));
    double motion_detector_contoursthreshold = std::stod(conf->at("motion_detector_contoursthreshold"));

    if (conf->at("backgroundsubtractor_type") == "1") {
      std::shared_ptr<BackgroundSubtractorMOG> mog(new BackgroundSubtractorMOG(backgroundsubtractor_history));

      std::unique_ptr<MotionDetector> m(new MotionDetector(*this, mog,
        motion_detector_blursize, motion_detector_threshold, motion_detector_dilationiterations,
        motion_detector_dilationsize, motion_detector_contoursthreshold));
      detector_ = std::move(m);
    } else if (conf->at("backgroundsubtractor_type") == "2") {
      bool shadow = conf->at("backgroundsubtractor_shadow") == "false" ? false : true;
      int backgroundsubtractor_threshold = std::stoi(conf->at("backgroundsubtractor_threshold"));

      std::shared_ptr<BackgroundSubtractorMOG2> mog(new BackgroundSubtractorMOG2(backgroundsubtractor_history,
        backgroundsubtractor_threshold, shadow));

      std::unique_ptr<MotionDetector> m(new MotionDetector(*this, mog,
        motion_detector_blursize, motion_detector_threshold, motion_detector_dilationiterations,
        motion_detector_dilationsize, motion_detector_contoursthreshold));
      detector_ = std::move(m);
    }
  } catch (const std::invalid_argument& ia) {
    log(lib_module::LogSeverity::FATAL,
        "Invalid conversion (motion_detector): " + std::string(ia.what()));
    exit(EXIT_FAILURE);
  } catch (const std::out_of_range &oor) {
    log(lib_module::LogSeverity::FATAL,
        "Invalid configuration reading 'motion_detector': " + std::string(oor.what()));
    exit(EXIT_FAILURE);
  }
  delete conf;
}

void Engine::applyRules() {
  std::vector<lib_module::RuleInfo> *rules = getConfigLoader().getRules(MODULE_VIDEO_NAME);

  // Filter out disabled rules
  boost::remove_erase_if(*rules, [] (const lib_module::RuleInfo r) {
    return r.is_disabled;
  });

  if (rules->size() == 0) {
    enabled_ = false;
  } else {
    enabled_ = true;

    // Combine rules with the same event configuration, but keep actions separated.
    // Get first rule and compare it. If there is a rule with the same configuration delete it from the rules and
    // add the rule id and the action struct to the map.
    while (rules->size() > 0) {
     std::map<lib_module::RuleID, lib_module::ActionInfo> map;
     lib_module::RuleInfo r = rules->back();
     rules->pop_back();
     lib_module::EventInfo event = r.event;
     map[r.id] = r.action;

     // look for same configuration
     for (unsigned int i = 0; i < rules->size(); i++) {
       if (r.event.key == rules->at(i).event.key && r.event.configuration == rules->at(i).event.configuration) {
         map[rules->at(i).id] = rules->at(i).action;
         rules->erase(rules->begin() + i);
         --i;
       }
     }
     addRoute(event, map);
    }
  }
  delete rules;
}

void Engine::addRoute(lib_module::EventInfo event, std::map<lib_module::RuleID, lib_module::ActionInfo> map) {
  // one route for every rule (combined rules included)
  std::vector<std::unique_ptr<Collider>> colliders;
  std::vector<std::unique_ptr<Highlighter>> highlighters;
  std::vector<std::unique_ptr<Heuristic>> heuristics;
  std::vector<std::unique_ptr<Output>> outputs;

  std::map<std::string, std::string> conf = event.configuration;

  // Colliders
  try {
    if (conf["collider_rio"] != "") {
      std::vector<std::string> strs;
      boost::split(strs, conf["collider_rio"], boost::is_any_of("#"));
      std::vector<Geometry> geo;
      for (auto &i : strs) {
        if (boost::starts_with(i, "POINT")) {
          point p;
          boost::geometry::read_wkt(i, p);
          geo.push_back(p);
        } else if (boost::starts_with(i, "LINESTRING")) {
          linestring l;
          boost::geometry::read_wkt(i, l);
          geo.push_back(l);
        } else if (boost::starts_with(i, "POLYGON")) {
          polygon p;
          boost::geometry::read_wkt(i, p);
          geo.push_back(p);
        } else if (boost::starts_with(i, "BOX")) {
          box b;
          boost::geometry::read_wkt(i, b);
          geo.push_back(b);
        }
      }
      std::unique_ptr<ColliderGeometry> gt(new ColliderGeometry(*this, geo));
      colliders.push_back(std::move(gt));
    }
  } catch (const std::out_of_range &oor) {
    log(lib_module::LogSeverity::FATAL,
        "Invalid configuration reading 'collider_rio': " + std::string(oor.what()));
    exit(EXIT_FAILURE);
  }

  // Highlighter
  try {
    if (conf["highlighter"] == "true") {
      std::unique_ptr<HighlighterRect> hr(new HighlighterRect(*this));
      highlighters.push_back(std::move(hr));
    }
  } catch (const std::out_of_range &oor) {
    log(lib_module::LogSeverity::FATAL,
        "Invalid configuration reading 'highlighter': " + std::string(oor.what()));
    exit(EXIT_FAILURE);
  }


  try {
    std::unique_ptr<OutputHandler> vr(new OutputHandler(*this, lib_module::getTempPath() + "/",
                                      std::stod(conf["video_output_number"]), map));
    outputs.push_back(std::move(vr));
  } catch (const std::invalid_argument& ia) {
    log(lib_module::LogSeverity::ERROR, "Invalid conversion (video_output_number): " + std::string(ia.what()));
    exit(EXIT_FAILURE);
  } catch (const std::out_of_range &oor) {
    log(lib_module::LogSeverity::FATAL,
      "Invalid configuration reading 'video_output_number': " + std::string(oor.what()));
    exit(EXIT_FAILURE);
  }

  // adding route
  routes_.push_back(std::make_tuple(std::move(colliders), std::move(heuristics),
                    std::move(highlighters), std::move(outputs)));
}

void Engine::writeHookData() {
  std::string hook = lib_module::getModulesConfPath() + "/" + MODULE_VIDEO_NAME + "/hook_data.json";

  boost::property_tree::ptree root;
  boost::property_tree::read_json(hook, root);

  // write hook for jcrop image, width, height, path
  boost::property_tree::ptree images;
  boost::property_tree::ptree areapicker;
  // /tmp/babyfon/img/videoimg.jpg
  areapicker.put("path", "data/img/videoimg.jpg");
  areapicker.put("width", width_);
  areapicker.put("height", height_);

  images.put_child("collider_rio", areapicker);

  boost::property_tree::ptree &forms = root.get_child("forms");
  forms.put_child("images", images);

  std::ofstream out(hook);
  boost::property_tree::write_json(out, root);
  out.close();
}
