/*
  messagehandler.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_LIB_MODULE_MESSAGEHANDLER_H_
#define SRC_LIB_MODULE_MESSAGEHANDLER_H_

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <utility>

#include "lib_module/message.h"
#include "lib_module/defs.h"
#include "lib_module/logger.h"

using boost::interprocess::message_queue;
using std::function;

namespace lib_module {
class MessageHandler {
 public:
  MessageHandler() = delete;
  /**
   * Creates a new queue
   * @param id the id of the new queue
   * @param logger for logging
   * @return pointer to message handler associated with new queue
   */
  static MessageHandler *create(const std::string &id, const Logger &logger);

  /**
   * Opens a queue
   * @param id id of the queue to open
   * @param logger for logging
   * @return pointer to message handler associated with opened queue
   */
  static MessageHandler *open(const std::string &id, const Logger &logger);
  ~MessageHandler();

  /**
   * Send a given message on the queue
   * @param message @see lib_module::Message
   * @return true if successful, false if queue is full or another error occured
   */
  bool sendMessage(const Message &message);

  /**
   * Sends a message, that a given event has occured
   * @param rule_id id of the rule that the event is part of
   * @param vars comma separated list of vars the event provides (e.g. var1=value1,var2=value2)
   * @return @see sendMessage
   */
  bool sendEventMessage(RuleID rule_id, std::string vars);

  /**
   * Receive a message, doesn't block
   * @param message pointer where to store received message
   * @return true if successful receive, false if queue is full/empty or an error occured
   */
  bool receiveMessageNonBlocking(Message *message);

  /**
   * Receive message, block for a maximum of timeout seconds
   * @param message where to store received message
   * @param timeout maximum time to block in ms
   * @return true if successful receive, false otherwise
   */
  bool receiveMessageBlocking(Message *message, uint32_t timeout);

  /**
   * Similar to @see receiveMessageNonBlocking(Message *message), store incoming vars in varMap
   * @param message where to store message
   * @param varMap where to store vars in map (map is overridden)
   * @return true if successful, false otherwise
   */
  bool receiveMessageNonBlocking(Message *message, std::map<std::string, std::string> *varMap);

  /**
   * Similar to receiveMessageBlocking(Message *message, uint32_t timeout) but incoming vars are stored in map
   * @param message where to store message
   * @param timeout max block time in ms
   * @param varMap where to store vars (map is overridden)
   * @return true if successful, false otherwise
   */
  bool receiveMessageBlocking(Message *message, uint32_t timeout, std::map<std::string, std::string> *varMap);

  /**
   * Send a heartbeat message
   * @param name name of the module which sends the heartbeat
   * @return @see sendMessage
   */
  bool sendHeartbeat(const std::string &name);

  /**
   * Set the heartbeat interval
   * @param module name of the module
   * @param interval interval time in ms
   * @return actual set interval (if interval > MAX_HEARTBEAT_INTERVAL return MAX_HEARTBEAT_INTERVAL)
   */
  int setHeartbeatInterval(const std::string &module, unsigned int interval);

  /**
   * Send log message
   * @param module name of the sending module
   * @param severity how severe the log message is
   * @param log_message the actual log message
   * @return @see sendMessage
   */
  bool log(const std::string &module, const lib_module::LogSeverity severity, const std::string log_message);

  /**
   * Send acknowledge message
   * @param id id of the event to acknowledge
   * @return @see sendMessage
   */
  bool acknowledgeEvent(EventID id);

 private:
  /**
   * @param queue message queue
   * @param logger for logging
   */
  explicit MessageHandler(message_queue *queue, const Logger &logger);
  std::string msg_queue_id_;
  message_queue *mq_;
  const Logger &logger_;
  /**
   * Convert vars string to key-value map
   * @param varPointer vars string
   * @param varMap output map (map is overridden)
   */
  void parseVars(char *varsPointer, std::map<std::string, std::string> *varMap);
};
}  // namespace lib_module

#endif  // SRC_LIB_MODULE_MESSAGEHANDLER_H_
