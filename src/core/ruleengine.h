/*
  ruleengine.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_CORE_RULEENGINE_H_
#define SRC_CORE_RULEENGINE_H_

#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#include <list>
#include <string>
#include <vector>

#include "core/evententry.h"
#include "core/messenger.h"

#include "lib_module/messagehandler.h"
#include "lib_module/configloader.h"
#include "lib_module/logger.h"

// Defines
#define ACK_CHECK_INTERVAL 1000  // 1 second
#define ACK_TIMEOUT 60000  // 60 seconds
#define NUM_ACTION_RETRIES 3

// Testing settings
#ifdef TESTING

#undef ACK_CHECK_INTERVAL
#define ACK_CHECK_INTERVAL 100

#undef ACK_TIMEOUT
#define ACK_TIMEOUT 1000

#undef NUM_ACTION_RETRIES
#define NUM_ACTION_RETRIES 2

#endif


class RuleNotFoundException : public std::exception {
 public:
  virtual const char* what() const throw() {
    return "Rule not found";
  }
};


class RuleEngine {
 public:
  /**
   * @param rules Vector of all existing rules
   * @param messenger Messenger for sending messages to modules
   * @param logger Logger to be used for logging
   */
  RuleEngine(const std::vector<lib_module::RuleInfo> &rules, const Messenger &messenger,
             const lib_module::Logger &logger);

  ~RuleEngine();

  /**
   * Enqueue event for handling
   * Copies event entry to list of pending events
   * existence of rules and constraints are checked and only events fulfilling those requirements are enqueued
   * System tries to send event NUM_ACTION_RETRIES times if ack has not been received within ACK_TIMEOUT ms
   * @param e Event information
   * @remark thread-safe
   */
  void enqueueEvent(const EventEntry &e);

  /**
  *
   * Acknowledge an event
   * This removes the event from the outstanding list and tells event module that event was acknowledged
   * @param event_id Event ID to acknowledge
   * @remark thread-safe
   */
  void acceptAck(lib_module::EventID event_id);

  /**
   * Creates and starts thread for asycnronous checking for acknowledgement timeouts
   * IMPORTANT: Do only call this once
   */
  void startAckCheckAsync();

  /**
   * Send a system message to notify user
   * This while initiate events for each rule involving system messages
   * @param message Message to be sent
   * @remark thread-safe
   */
  void systemMessage(const std::string &message);

 private:
  /**
   * Search for rule by rule ID and return its information
   * This while initiate events for each rule involving system messages
   * Throws exception is rule was not found
   * @param id Rule ID to look up
   * @return rule information
   * @throws RuleNotFoundException
   * @remark thread-safe
   */
  const lib_module::RuleInfo &getRule(lib_module::RuleID id);

  /**
   * Send action of event to action module
   * @param entry Event information
   * @remark thread-safe
   */
  void takeAction(EventEntry &entry);

  /**
   * Asycnronous checker thread worker
   */
  void ackThread();

  const std::vector<lib_module::RuleInfo> &rules_;  ///< List of all rule settings
  std::vector<Timepoint> rule_timestamps_;  ///< Vector of the last occurrence of rules

  const lib_module::Logger &logger_;  ///< Logger for errors
  const Messenger &messenger_;  ///< Messenger used to send messages to modules

  std::list<EventEntry*> ack_queue_;  ///< List of all pending events (either not sent or waiting for ack)

  boost::mutex ack_mtx_;  ///< Mutex for thread safety of accessing ack_queue_
  boost::thread *ack_thread_ = NULL;  ///< Async ack checker thread

  lib_module::EventID event_id_ = 0;  ///< Continous incremental event ID
};

#endif  // SRC_CORE_RULEENGINE_H_
