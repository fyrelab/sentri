/*
  messenger.cpp
  Copyright 2016 fyrelab
 */

#include "core/messenger.h"

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#include "core/defs.h"

#define CORE_QUEUE_POSTFIX "--events";

#define LOGGING_NAME "messenger"

using lib_module::MessageHandler;

Messenger::Messenger(const lib_module::Logger &logger) : logger_(logger) {
  // Symbols for random part of ucid
  const uint16_t kLength = 8;
  const char symbols[] = "0123456789ABCDEF";

  char random[kLength + 1];
  random[kLength] = 0;

  // Init random generator
  unsigned int seed = time(NULL);

  // Generate random part for ucid
  for (int i = 0; i < kLength; i++) {
    random[i] = symbols[rand_r(&seed) % sizeof(symbols)];
  }

  // Build ucid
  ucid_ = UCID_PREFIX + std::string(random);
  core_queue_name_ = ucid_ + CORE_QUEUE_POSTFIX;

  // Create message queue for module mapping
  messageQueues_ = std::map<std::string, lib_module::MessageHandler*>();
}

Messenger::~Messenger() {
  // Delete all message handlers
  for (std::map<std::string, MessageHandler*>::iterator it = messageQueues_.begin();
       it != messageQueues_.end(); ++it) {
    delete it->second;
  }
}

bool Messenger::sendAction(const std::string &module, const EventEntry &entry) const {
  // Create message
  lib_module::Message message;
  message.type = lib_module::MSG_ACTIO;
  message.body.action.rule_id = entry.event.rule_id;
  message.body.action.event_id = entry.id;

  memcpy(&message.body.action.vars, &entry.event.vars, MSG_VAR_SIZE);

  return sendMessage(module, message);
}

bool Messenger::sendEventAck(const std::string &module, const EventEntry &entry) const {
  // Create message
  lib_module::Message message;
  message.type = lib_module::MSG_ACKEV;
  message.body.action.rule_id = entry.event.rule_id;
  message.body.action.event_id = entry.id;

  memcpy(&message.body.action.vars, &entry.event.vars, MSG_VAR_SIZE);

  return sendMessage(module, message);
}

bool Messenger::sendEventAbort(const std::string &module, const EventEntry &entry) const {
  // Create message
  lib_module::Message message;
  message.type = lib_module::MSG_EVABT;
  message.body.action.rule_id = entry.event.rule_id;
  message.body.action.event_id = entry.id;

  memcpy(&message.body.action.vars, &entry.event.vars, MSG_VAR_SIZE);

  return sendMessage(module, message);
}

bool Messenger::sendMessage(const std::string &module, const lib_module::Message &message) const {
  // can't send to fake system message module
  if (module == SYSTEM_MESSAGE_MODULE) {
    return false;
  }

  // Search for module
  queue_mtx_.lock();

  std::map<std::string, MessageHandler*>::const_iterator it = messageQueues_.find(module);

  if (it != messageQueues_.end()) {
    // Message queue was found

    // Send message
    bool success = it->second->sendMessage(message);

    queue_mtx_.unlock();

    if (!success) {
      logger_.log(LOGGING_NAME, lib_module::LogSeverity::ERROR, "Could not send message to " + module);
    }

    return success;
  } else {
    queue_mtx_.unlock();

    logger_.log(LOGGING_NAME, lib_module::LogSeverity::ERROR, "Module " + module + " not found");
  }

  // No success
  return false;
}

std::string Messenger::addQueue(const std::string &module) {
  // Generate queue name
  std::string uqid = ucid_ + "-" + module;

  // Remove existing queue (if necessary)
  removeQueue(module);

  // Create message handler
  MessageHandler *handler = MessageHandler::create(uqid, logger_);

  // Add message handler to map
  queue_mtx_.lock();
  messageQueues_[module] = handler;
  queue_mtx_.unlock();

  return uqid;
}

void Messenger::removeQueue(const std::string &module) {
  queue_mtx_.lock();

  // Check if queue exists for module
  std::map<std::string, MessageHandler*>::const_iterator it = messageQueues_.find(module);

  if (it != messageQueues_.end()) {
    // Queue exists
    messageQueues_.erase(it);
    delete it->second;
  }

  queue_mtx_.unlock();
}
