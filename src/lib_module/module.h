/*
  module.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_LIB_MODULE_MODULE_H_
#define SRC_LIB_MODULE_MODULE_H_

#include <string>
#include <map>

#include "lib_module/modulelogger.h"
#include "lib_module/configloader.h"
#include "lib_module/messagehandler.h"
#include "lib_module/defs.h"

namespace lib_module {

/**
 * Base class for a module of the system. Provides several helper functions to interact with the core.
 * @remark Inherit from this class if you implement a module!
 */
class Module {
 protected:
  /**
   * @param name the module name
   * @param sending_queue the name of the queue used for sending, modules are given this name as argv[1]
   * @param receiving_queue name of the queue used for receiving, argv[2]
   */
  explicit Module(const std::string &name, const std::string &sending_queue, const std::string &receiving_queue);
  ~Module();

 public:
  /**
   * Get the module name and return
   * @return module name
   */
  std::string getModuleName() const;

  /**
   * Get the configloader.
   * @return configloader
   */
  ConfigLoaderJ &getConfigLoader() const;

  /**
   * Receive a message blocking with given timeout
   * @param a message
   * @param timeout timeout in ms
   * @param map a map of key-value pairs where to store the vars the message provides
   * @return true if successful receive
   */
  bool receiveMessageBlocking(Message *message, uint32_t timeout, std::map<std::string, std::string> *map) const;

  /**
   * Receive a message and instantly discard it.
   * Useful to make sure incoming acks are removed from the queue.
   * @return true if a message was discarded, i.e. the queue was not empty before
   */
  bool receiveDiscard() const;

  /**
   * Acknowledge an event.
   * @param eventID
   * @return true if message was sent
   */
  bool acknowledgeEvent(EventID eventID) const;

  /**
   * Send an event message.
   * @param ruleID id of the rule
   * @param vars variables the event provides
   * @return true if message was sent
   */
  bool sendEventMessage(RuleID ruleID, std::string vars) const;

  /**
   * Set the heartbeat interval for a module. The heartbeat can be sent
   * by unsing the function sendHeartbeat();
   * @param interval is the interval in milliseconds.
   * @return value of the actuallty set interval
   */
  int setHeartbeatInterval(unsigned int interval) const;

  /**
   * Send a heartbeat. The interval of this heartbeat is given by
   * the function setHeartbeatInterval.
   * NOTE: This action waits when flood protection avoids sending
   * @return true if message was sent
   */
  bool sendHeartbeat() const;

  /**
   * Log to sentri system log.
   * @param severity how severe the log message is
   * @param log_message the actual log message
   * @return true if message was sent
   */
  bool log(const lib_module::LogSeverity severity, const std::string &log_message) const;

  /**
   * Send a system message.
   * @param message contains the message sent by the system
   * @return true if message was sent
   */
  bool systemMessage(const std::string &message) const;

  /**
   * Write to hook file used by webtool. (e.g. show current temperature reading in web interface)
   * @param string which contains the content of the hook
   * @return true if it was successful
   */
  bool writeHook(const std::string &hook_data) const;

  /**
    * Replace all occurrences of the var keys with '%'
    * in front of it with the value of the var.
    * @param text is a reference to the string which contains the text
    * @param varsMap is a reference to the Map which contains the vars with key value pairs
    * @return string with replaced keys
    */
  static std::string replaceVars(const std::string &text, const std::map<std::string, std::string> &varsMap);

 private:
   /**
    * Updates message density and last sent message to prevent message queue flooding on core
    * @return the number of milliseconds to wait for being allowed to send the message
    */
  unsigned int isFlooding() const;

  mutable int message_density_;
  mutable Timepoint last_density_reset_;

  std::string module_name_;
  ModuleLogger *logger_;
  MessageHandler *sender_;
  MessageHandler *receiver_;
  ConfigLoaderJ *configloader_;
};
}  // namespace lib_module

#endif  // SRC_LIB_MODULE_MODULE_H_
