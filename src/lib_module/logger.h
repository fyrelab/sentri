/*
  logger.h
  Copyright 2016 fyrelab
*/

#ifndef SRC_LIB_MODULE_LOGGER_H_
#define SRC_LIB_MODULE_LOGGER_H_

#include <string>

#include "lib_module/defs.h"

namespace lib_module {

class Logger {
 public:
  /**
   * @param logging_name the sender name used in logs for a log message
   */
  explicit Logger(const std::string &logging_name) : logging_name_(logging_name) {}
  virtual ~Logger() = default;

  /**
   * Write log message
   * @param sender name of the part of the system (e.g. core, watchdog or module name) where the log came fro
   * @param sev severity level to indicate how critical a log message is
   * @param log_message what to log
   */
  virtual bool log(const std::string &sender, const lib_module::LogSeverity sev,
                                              const std::string log_message) const = 0;
  /**
   * Calls log with logging_name_
   * @see log
   */
  bool log(const lib_module::LogSeverity severity, const std::string log_message) const {
    return log(logging_name_, severity, log_message);
  }
  /**
   * Trigger the rules associated with system message, e.g. send the user a mail with the message provided
   * @param message what to write in system message
   */
  virtual bool systemMessage(const std::string) const = 0;

 protected:
  const std::string logging_name_;  ///< name used in log messages
};

}  // namespace lib_module

#endif  // SRC_LIB_MODULE_LOGGER_H_
