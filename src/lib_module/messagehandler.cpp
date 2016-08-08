/*
  messagehandler.cpp
  Copyright 2016 fyrelab
 */

#include "lib_module/messagehandler.h"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <core/defs.h>
#include <iostream>

#define DEFAULT_QUEUE_SIZE 5000

using std::pair;
using boost::interprocess::message_queue;
using boost::interprocess::interprocess_exception;
using boost::interprocess::create_only;
using boost::interprocess::open_only;
using boost::interprocess::interprocess_exception;

namespace lib_module {
MessageHandler::MessageHandler(message_queue *queue, const Logger &logger)
                               : mq_(queue), logger_(logger) {
}

MessageHandler::~MessageHandler() {
  try {
    message_queue::remove(msg_queue_id_.c_str());
  }
  catch (interprocess_exception &ex) {
    logger_.log(LogSeverity::ERROR, "Unable to remove message queue: " + std::string(ex.what()));
  }

  delete mq_;
}

MessageHandler *MessageHandler::create(const std::string &id, const Logger &logger) {
  try {
    message_queue::remove(id.c_str());
    message_queue* mq = new message_queue(create_only, id.c_str(), DEFAULT_QUEUE_SIZE, sizeof(Message));

    return new MessageHandler(mq, logger);
  }
  catch (interprocess_exception &ex) {
    logger.log(LogSeverity::ERROR, "Unable to create message handler: " + std::string(ex.what()));
  }

  return NULL;
}

MessageHandler *MessageHandler::open(const std::string &id, const Logger &logger) {
  try {
    message_queue* mq = new message_queue(open_only, id.c_str());
    return new MessageHandler(mq, logger);
  }
  catch (interprocess_exception &ex) {
    logger.log(LogSeverity::ERROR, "Unable to open message queue: " + std::string(ex.what()));
  }

  return NULL;
}

bool MessageHandler::sendMessage(const Message &message) {
  try {
    return mq_->try_send(&message, sizeof(Message), 0);
  }
  catch (interprocess_exception &ex) {
    logger_.log(LogSeverity::ERROR, "Unable to send message with type: " + std::to_string(message.type) + ex.what());
  }

  return false;
}

bool MessageHandler::receiveMessageNonBlocking(Message *message) {
  unsigned int priority = 0;
  message_queue::size_type recvd_size;

  try {
    return mq_->try_receive(message, sizeof(Message), recvd_size, priority);
  }
  catch (interprocess_exception &ex) {
    logger_.log(LogSeverity::ERROR, "Unable to receive " + std::string(ex.what()));
  }

  return false;
}

bool MessageHandler::receiveMessageBlocking(Message *message, uint32_t timeout) {
  using boost::posix_time::ptime;
  using boost::posix_time::millisec;

  ptime timestamp = boost::date_time::microsec_clock<ptime>::universal_time() + millisec(timeout);

  unsigned int priority = 0;
  message_queue::size_type recvd_size;
  try {
    return mq_->timed_receive(message, sizeof(Message), recvd_size, priority, timestamp);
  }
  catch (interprocess_exception &ex) {
    logger_.log(LogSeverity::ERROR, "Unable to receive " + std::string(ex.what()));
  }

  return false;
}

bool MessageHandler::receiveMessageNonBlocking(Message *message, std::map<std::string, std::string> *varMap) {
  if (receiveMessageNonBlocking(message)) {
    switch (message->type) {
      case MSG_EVENT: {
        parseVars(message->body.event.vars, varMap);
        break;
      }
      case MSG_ACTIO: {
        parseVars(message->body.action.vars, varMap);
        break;
      }
      case MSG_ACKEV: {
        parseVars(message->body.action.vars, varMap);
        break;
      }
      case MSG_EVABT: {
        parseVars(message->body.action.vars, varMap);
        break;
      }
      default: {
        char empty = 0;
        parseVars(&empty, varMap);
      }
    }
    return true;
  }
  return false;
}

bool MessageHandler::receiveMessageBlocking(Message *message, uint32_t timeout,
                                            std::map<std::string, std::string> *varMap) {
  if (receiveMessageBlocking(message, timeout)) {
    switch (message->type) {
      case MSG_EVENT: {
        parseVars(message->body.event.vars, varMap);
        break;
      }
      case MSG_ACTIO: {
        parseVars(message->body.action.vars, varMap);
        break;
      }
      case MSG_ACKEV: {
        parseVars(message->body.action.vars, varMap);
        break;
      }
      case MSG_EVABT: {
        parseVars(message->body.action.vars, varMap);
        break;
      }
      default: {
        char empty = 0;
        parseVars(&empty, varMap);
      }
    }

    return true;
  }
  return false;
}

void MessageHandler::parseVars(char *varsPointer, std::map<std::string, std::string> *varMap) {
  *varMap = std::map<std::string, std::string>();

  std::string vars(varsPointer);

  std::vector<std::string> variables;
  boost::split(variables, vars, boost::is_any_of(","));

  for (size_t i = 0; i < variables.size(); i++) {
    unsigned int pos;

    // loop while pos is the position of the first '='
    for (pos = 0; pos < variables[i].length() && variables[i].at(pos) != '='; pos++) {}

    // If no '=' was found it's not clear what is the value and key, so do nothing
    if (pos < variables[i].length()) {
      // This has to be done with insert instead of operator[]
      varMap->insert(pair<std::string, std::string>(variables[i].substr(0, pos),
                                                    variables[i].substr(pos + 1, variables[i].length() - pos - 1)));
    }
  }
}

bool MessageHandler::sendHeartbeat(const std::string &name) {
  Message m;
  m.type = MSG_HBEAT;
  snprintf(m.body.hbeat.module, sizeof(m.body.hbeat.module), "%s", name.c_str());

  return sendMessage(m);
}

int MessageHandler::setHeartbeatInterval(const std::string &module, unsigned int interval) {
  // returns requested value if interval was in valid range, returns MAX_HEARTBEAT_INTERVAL otherwise
  if (interval > MAX_HEARTBEAT_INTERVAL) {
    interval = MAX_HEARTBEAT_INTERVAL;
  }

  Message m;
  m.type = MSG_HBINT;
  snprintf(m.body.hbint.module, sizeof(m.body.hbint.module), "%s", module.c_str());
  m.body.hbint.interval = interval;

  if (sendMessage(m)) {
    return interval;
  }

  // Heartbeat updated failed
  return -1;
}

bool MessageHandler::log(const std::string &module, const lib_module::LogSeverity severity,
                         const std::string log_message) {
  Message m;
  m.type = MSG_LOG;
  snprintf(m.body.log.module, sizeof(m.body.log.module), "%s", module.c_str());
  m.body.log.severity = severity;
  snprintf(m.body.log.log_message, sizeof(m.body.log.log_message), "%s", log_message.c_str());

  return sendMessage(m);
}

bool MessageHandler::acknowledgeEvent(EventID id) {
  Message m;
  m.type = MSG_ACKAC;
  m.body.ack.event_id = id;

  return sendMessage(m);
}

bool MessageHandler::sendEventMessage(RuleID rule_id, std::string vars) {
  Message m;
  m.type = MSG_EVENT;
  m.body.event.rule_id = rule_id;
  snprintf(m.body.event.vars, sizeof(m.body.event.vars), "%s", vars.c_str());

  return sendMessage(m);
}

}  // namespace lib_module
