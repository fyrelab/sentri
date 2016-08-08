/*
  mail_trigger.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_MODULES_MODULE_TRIGGER_TRIGGER_MODULE_H_
#define SRC_MODULES_MODULE_TRIGGER_TRIGGER_MODULE_H_

// Includes
#include <string>
#include <vector>

#include "lib_module/module.h"

// Constants
#define MODULE_NAME "module_trigger"
#define HEARTBEAT_INTERVAL 10000  // 10 second
#define ACTION_1_TITLE "send"

/**
 * Class to send an event with a given timeout and given vars
 */
class EventTrigger {
 public:
  /**
   * @param *module Pointer to instance of the module to send Messages and log
   * @param timeout Time to wait before the first event
   * @param timeout_between Time to wait between the evnets
   * @param rule_id ID of the rule (rule.id)
   * @param vars Vars to be sent with the event
   */
  EventTrigger(lib_module::Module *module, int timeout, int timeout_between,
                lib_module::RuleID rule_id, const std::string vars);
  ~EventTrigger();

  /**
   * Starts sending Events, first waits timeout_ and then waits
   * timeout_between_ between the events
   * Note: Is blockin and should be called in an extra thread
   */
  void startSendingEventsAsync();

 private:
  lib_module::Module *module_;
  int timeout_;
  int timeout_between_;
  lib_module::RuleID rule_id_;
  const std::string vars_;
};


class TriggerModule : lib_module::Module {
 public:
  /**
   * Constructor of TriggerModule
   * @param sending_queue Name of the sending queue.
   * Normally argument number 1 when a module is startet by the watchdog
   * @param receiving_queue Name of the receiving queue.
   * Normally argument number 2 when a module is startet by the watchdog
   */
  TriggerModule(const std::string &sending_queue, const std::string &receiving_queue);
  ~TriggerModule();

  /**
   * Starts startSendingEventsAsync of all Objects in event_triggers_ in it's own
   * threads.
   */
  void startSendingEvents();

  /**
   * Sends heartbeats in a endless while loop
   * Note: this method is blocking
   */
  void sendHeartbeatsForever();

 private:
  std::vector<EventTrigger *> *event_triggers_;
};

#endif  // SRC_MODULES_MODULE_TRIGGER_TRIGGER_MODULE_H_
