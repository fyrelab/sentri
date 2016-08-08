/*
  engine.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_MODULES_MODULE_VIDEO_ENGINE_H_
#define SRC_MODULES_MODULE_VIDEO_ENGINE_H_

#include <boost/thread.hpp>
#include <vector>
#include <tuple>
#include <string>
#include <map>

#include "modules/module_video/cam.h"
#include "modules/module_video/detector.h"
#include "modules/module_video/collider.h"
#include "modules/module_video/highlighter.h"
#include "modules/module_video/heuristic.h"
#include "modules/module_video/output.h"
#include "modules/module_video/collidergeometry.h"

#include "lib_module/module.h"

#define MODULE_VIDEO_NAME "module_video"

/**
 * The engine of the video analysing module. Frames are retrieved or read from the capturing device. Afterwards they
 * are processed by the detector. The calculated foregroundmask is then forwarded to the intersection testing algorithm.
 * After rectangles were filtered out which are not in the region of interest they can be highlighted in the current frame.
 * Finally, the outputhander sends event messages which include videos or pictures of the corresponding motion detections.
 * The stages of the processing pipeline are:
 * 1. Detector
 * 2. Collider
 * 3. Heuristic
 * 4. Highlighter
 * 5. Outputhandler
 */

class Engine : public lib_module::Module {
 public:
  /**
   * @param  sending_queue
   * @param  receiving_queue
   */
  explicit Engine(const std::string &sending_queue, const std::string &receiving_queue);
  ~Engine();
  void start();

 private:
  int height_;
  int width_;
  bool enabled_;
  void init();
  Cam *cam_;
  //!< One detector used for all rules (Several detectors may lead to very high cpu usage on the Raspberry PI).
  std::unique_ptr<Detector> detector_;

  /**
   * For every rule there is one route with the corresponding settings which are defined in the rules.
   */
  std::vector<std::tuple<std::vector<std::unique_ptr<Collider>>,
    std::vector<std::unique_ptr<Heuristic>>, std::vector<std::unique_ptr<Highlighter>>,
    std::vector<std::unique_ptr<Output>>>> routes_;

  milliseconds delay_take_;
  bool isNumber(const std::string &s);

  /**
   * Starts the thread which handles the incoming messages asynchronously and forwards them to the outputhandlers.
   */
  void handleIncomingMsgAsync();
  boost::thread *th_;

  /**
   * The camera needs to warm up to prevent artifacts in the first frames.
   */
  void warmUp();
  void configure();
  void applyRules();
  void addRoute(lib_module::EventInfo event, std::map<lib_module::RuleID, lib_module::ActionInfo> map);
  void writeHookData();
  std::string videoimgpath_;
};

#endif  // SRC_MODULES_MODULE_VIDEO_ENGINE_H_
