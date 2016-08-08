/*
  module.cpp
  Copyright 2016 fyrelab
 */

#include <math.h>

#include <string>
#include <map>

#include "lib_module/module.h"
#include "core/defs.h"

using std::string;

#define FLOOD_MSG_PER_SECOND 15

namespace lib_module {

Module::Module(const std::string &name, const std::string &sending_queue, const std::string &receiving_queue)
              : message_density_(0),
                module_name_(name),
                logger_(new ModuleLogger(module_name_)),
                sender_(MessageHandler::open(sending_queue, *logger_)),
                receiver_(MessageHandler::open(receiving_queue, *logger_)),
                configloader_(new ConfigLoaderJ(getModulesInfoPath() + "/" + module_name_ + "/" + module_name_
                                                + EXT_INFO, getModulesConfPath() + "/" + module_name_ + "/"
                                                + module_name_ + EXT_CONF, getRulesConfPath(), *logger_)) {
  logger_->setSender(sender_);
}

Module::~Module() {
  delete sender_;
  delete receiver_;
  delete configloader_;
  delete logger_;
}

std::string Module::getModuleName() const {
  return module_name_;
}

ConfigLoaderJ &Module::getConfigLoader() const {
  return *configloader_;
}

bool Module::receiveMessageBlocking(Message *message, uint32_t timeout, std::map<std::string, std::string> *map) const {
  return receiver_->receiveMessageBlocking(message, timeout, map);
}

bool Module::receiveDiscard() const {
  Message m;
  return receiver_->receiveMessageNonBlocking(&m);
}

bool Module::acknowledgeEvent(EventID eventID) const {
  // Check if module is currently flooding the core
  if (isFlooding() > 0) {
    return false;
  }

  return sender_->acknowledgeEvent(eventID);
}

bool Module::sendEventMessage(RuleID ruleID, std::string vars) const {
  // Check if module is currently flooding the core
  if (isFlooding() > 0) {
    return false;
  }

  return sender_->sendEventMessage(ruleID, vars);
}

int Module::setHeartbeatInterval(unsigned int interval) const {
  // Check if module is currently flooding the core
  if (isFlooding() > 0) {
    return false;
  }

  return sender_->setHeartbeatInterval(module_name_, interval);
}

bool Module::sendHeartbeat() const {
  // Check if module is currently flooding the core
  unsigned int time = isFlooding();

  if (time > 0) {
    // Wait until message can be sent
    boost::this_thread::sleep(boost::posix_time::milliseconds(time));

    // Update values
    isFlooding();
  }

  return sender_->sendHeartbeat(module_name_);
}

bool Module::log(const lib_module::LogSeverity severity, const std::string &log_message) const {
  return logger_->log(severity, log_message);
}

bool Module::systemMessage(const std::string &message) const {
  // Check if module is currently flooding the core
  if (isFlooding() > 0) {
    return false;
  }

  return logger_->systemMessage(message);
}

bool Module::writeHook(const std::string &hook_data) const {
  std::ofstream hook;
  hook.exceptions(std::ofstream::failbit | std::ofstream::eofbit | std::ofstream::badbit);

  try {
    hook.open(getModulesConfPath() + "/" + module_name_ + "/" HOOK_FILE_NAME,
                         std::fstream::out | std::fstream::trunc);
    hook.seekp(0);
    hook << hook_data;
    hook.flush();
    hook.close();
    return true;
  } catch (...) {
    logger_->log(lib_module::LogSeverity::WARNING, "Couldn't write hook!");
    return false;
  }
}

std::string Module::replaceVars(const std::string &text, const std::map<std::string, std::string> &varsMap) {
  std::string result = text;
  for (const auto &variable : varsMap) {
    const std::string &key = variable.first;
    const std::string &value = variable.second;
    boost::replace_all(result, "%" + key, value);
  }
  return result;
}

unsigned int Module::isFlooding() const {
  typedef boost::chrono::nanoseconds ns;
  const ns second = ns(1000000000);

  auto now = boost::chrono::high_resolution_clock::now();
  ns timespan = now - last_density_reset_;
  ns limit = second / FLOOD_MSG_PER_SECOND;

  if (timespan > limit) {
    message_density_ = 0;
    last_density_reset_ = now;
  }

  if (message_density_ < FLOOD_MSG_PER_SECOND) {
    message_density_++;
    return 0;
  }

  return ceil(1000.0 / FLOOD_MSG_PER_SECOND);
}

}  // namespace lib_module
