/*
  modulelogger.cpp
  Copyright 2016 fyrelab
*/

#include "lib_module/modulelogger.h"

#include <string>

namespace lib_module {

ModuleLogger::ModuleLogger(const std::string &logging_name) : Logger(logging_name) {
}

bool ModuleLogger::log(const std::string &module_name, const lib_module::LogSeverity severity,
                       const std::string log_message) const {
  if (severity == lib_module::LogSeverity::LOCAL) {
    std::cerr << "[local][" << module_name << "]" << " " << log_message << std::endl;
    return true;
  } else {
    if (sender_)
      return sender_->log(module_name, severity, log_message);
  }

  return false;
}

bool ModuleLogger::log(const lib_module::LogSeverity severity, const std::string log_message) const {
  return Logger::log(severity, log_message);
}

bool ModuleLogger::systemMessage(const std::string message) const {
  if (sender_) {
    Message m;
    m.type = MSG_SYS;
    snprintf(m.body.sys.message, sizeof(m.body.sys.message), "%s", message.c_str());
    return sender_->sendMessage(m);
  }

  return false;
}

void ModuleLogger::setSender(MessageHandler *sender) {
  sender_ = sender;
}

}  // namespace lib_module
