/*
  sentri.cpp
  Copyright 2016 fyrelab
 */

#include "core/sentri.h"

#include <boost/chrono.hpp>
#include <iostream>
#include <string>
#include <cstdlib>

#include "testing/def.h"
#include "lib_module/configloader.h"
#include "lib_module/defs.h"

Sentri::Sentri(std::string rule_file_path, std::string module_path)
               : logger_(CoreLogger()),
               messenger_(new Messenger(logger_)),
               rules_(lib_module::ConfigLoaderJ("", "", rule_file_path, logger_).getRules("main")),
               engine_(new RuleEngine(*rules_, *messenger_, logger_)),
               watchdog_(new Watchdog(module_path, *messenger_, logger_)) {
  logger_.setRuleEngine(engine_);
}

Sentri::~Sentri() {
  delete watchdog_;
  delete rules_;
  delete engine_;
  delete messenger_;
}

void Sentri::start() {
  std::string queueName = messenger_->getCoreQueueName();
  lib_module::MessageHandler *handler = lib_module::MessageHandler::create(queueName, logger_);

  // Start async checker for acknowledgements
  engine_->startAckCheckAsync();
  watchdog_->start();

  // Listen for incoming messages
  lib_module::Message m;

  while (true) {
    bool hasMessage = handler->receiveMessageBlocking(&m, RECEIVE_TIMEOUT);

    // Check if message is received
    if (hasMessage) {
      readMessage(m);
    }
  }

  delete handler;

  nassert(false, "This code should not be reached");
}

void Sentri::readMessage(const lib_module::Message &msg) {
  // Read messages and redirect them to the controlling unit of the message type
  switch (msg.type) {
    case lib_module::MSG_EVENT: {
      EventEntry entry = { msg.body.event };
      engine_->enqueueEvent(entry);
      break;
    }
    case lib_module::MSG_HBEAT: {
      std::string module = std::string(msg.body.hbeat.module);
      watchdog_->heartbeat(module);
      break;
    }
    case lib_module::MSG_HBINT: {
      std::string module = std::string(msg.body.hbint.module);
      watchdog_->setHeartbeatInterval(module, msg.body.hbint.interval);
      break;
    }
    case lib_module::MSG_ACKAC: {
      engine_->acceptAck(msg.body.ack.event_id);
      break;
    }
    case lib_module::MSG_ACTIO: {
      logger_.log("core", lib_module::WARNING, "Illegal action message");
      break;
    }
    case lib_module::MSG_LOG: {
      std::string module = std::string(msg.body.log.module);
      logger_.log(module, msg.body.log.severity, std::string(msg.body.log.log_message));
      break;
    }
    case lib_module::MSG_SYS: {
      engine_->systemMessage(std::string(msg.body.sys.message));
      break;
    }
    default: {
      std::string message = "Unknown message type " + std::to_string(msg.type);
      logger_.log("core", lib_module::WARNING, message);
    }
  }
}
