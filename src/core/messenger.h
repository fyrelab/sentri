/*
  messenger.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_CORE_MESSENGER_H_
#define SRC_CORE_MESSENGER_H_

#include <map>
#include <string>

#include "core/evententry.h"
#include "lib_module/messagehandler.h"
#include "lib_module/logger.h"

class Messenger {
 public:
  /**
   * @param logger Logger instance to use for logging
   */
  explicit Messenger(const lib_module::Logger &logger);

  ~Messenger();

  /**
   * Send action to module
   * @param module Module name to send action to
   * @param entry EventEntry for further information
   * @return success of sending
   * @remark thread-safe
   */
  bool sendAction(const std::string &module, const EventEntry &entry) const;

  /**
   * Send event acknowledgement to module
   * @param module Module name to send acknowledgement to
   * @param entry EventEntry for further information
   * @return success of sending
   * @remark thread-safe
   */
  bool sendEventAck(const std::string &module, const EventEntry &entry) const;

  /**
   * Send event abort to module
   * @param module Module name to send abort to
   * @param entry EventEntry for further information
   * @return success of sending
   * @remark thread-safe
   */
  bool sendEventAbort(const std::string &module, const EventEntry &entry) const;

  /**
   * Create a new queue for a module and save it in a map
   * or replace an existing one with a new one (by also deleting the old one)
   * @param module Module name queue is there for
   * @return uqid to identify queue
   * @remark thread-safe
   */
  std::string addQueue(const std::string &module);

  /**
   * Removes and deletes (if existing) the message queue of a module
   * @param module Module name queue should be removed from
   * @remark thread-safe
   */
  void removeQueue(const std::string &module);

  /**
   * Get UCID (unique core queue id) for system
   * @return ucid
   */
  inline std::string getUcid() const {
    return ucid_;
  }

  /**
   * Get name of the core messaging queue
   * @return queue name
   */
  inline std::string getCoreQueueName() const {
    return core_queue_name_;
  }

 private:
  const lib_module::Logger &logger_;  ///< System logger
  std::string core_queue_name_;  ///< Name of the core queue for message receiving
  std::string ucid_;  ///< unique core identifier, used as prefix for message queues
  std::map<std::string, lib_module::MessageHandler*> messageQueues_;  ///< Map of module names and message queues

  /**
   * Send message to a module
   * @param module Name of module
   * @param message Message to be sent
   * @return success of sending
   * @remark thread-safe
   */
  bool sendMessage(const std::string &module, const lib_module::Message &message) const;

  mutable boost::mutex queue_mtx_;  ///< Mutex to make messenger thread safe
};

#endif  // SRC_CORE_MESSENGER_H_
