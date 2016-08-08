/*
  modulelogger.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_LIB_MODULE_MODULELOGGER_H_
#define SRC_LIB_MODULE_MODULELOGGER_H_

#include <string>

#include "lib_module/logger.h"
#include "lib_module/messagehandler.h"

namespace lib_module {

class ModuleLogger : public Logger {
 public:
  /**
   * @param logging_name name used in logs
   */
  explicit ModuleLogger(const std::string &logging_name);
  ~ModuleLogger() = default;

  /**
   * @see Logger::log
   */
  bool log(const std::string &module_name, const lib_module::LogSeverity severity, const std::string log_message) const;

  /**
   * @see Logger::log
   */
  bool log(const lib_module::LogSeverity severity, const std::string log_message) const;

  /**
   * @see lib_module::Logger::systemMessage
   */
  bool systemMessage(const std::string message) const;

  /**
   * Sets sender message handler used for logging
   * @param sender
   */
  void setSender(MessageHandler *sender);

 private:
  MessageHandler *sender_;
};

}  // namespace lib_module

#endif  // SRC_LIB_MODULE_MODULELOGGER_H_
