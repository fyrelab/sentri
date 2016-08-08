/*
  sentri.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_CORE_SENTRI_H_
#define SRC_CORE_SENTRI_H_

#include <list>
#include <string>
#include <vector>

#include "lib_module/messagehandler.h"
#include "core/ruleengine.h"
#include "core/watchdog.h"
#include "core/messenger.h"
#include "core/corelogger.h"

#define RECEIVE_TIMEOUT 60000  // maximum time to block when receiving messages

class Sentri {
 public:
  /**
   * @param rule_file_path Path to rule file
   * @param module_path Path to module binaries
   */
  explicit Sentri(std::string rule_file_path, std::string module_path);
  ~Sentri();

  /**
   * Starts the sentri system
   * IMPORTANT: This blocks the entire thread and should not return under normal circumstances
   */
  void start();

  #ifdef TESTING

  /**
   * Get watchdog instance for module health analysing
   */
  inline Watchdog &getWatchdog() {
    return *watchdog_;
  }

  #endif

 private:
  /**
   * Takes message and hands it over to the dedicated handlers
   * @param msg Message to handle
   */
  void readMessage(const lib_module::Message &msg);

  CoreLogger logger_;  ///< Logger to log errors
  Messenger *messenger_;  ///< Messenger to create and set message queue prefixes

  std::vector<lib_module::RuleInfo> *rules_;  ///< List of all existing rules

  RuleEngine *engine_;  ///< Rule engine to process events
  Watchdog *watchdog_;  ///< Watchdog for module health related messages
};

#endif  // SRC_CORE_SENTRI_H_
