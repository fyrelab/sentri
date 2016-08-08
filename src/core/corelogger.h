/*
  corelogger.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_CORE_CORELOGGER_H_
#define SRC_CORE_CORELOGGER_H_

#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>

#include <fstream>
#include <cstddef>
#include <string>

#include "core/defs.h"
#include "core/ruleengine.h"
#include "lib_module/defs.h"
#include "lib_module/logger.h"

class RuleEngine;

class CoreLogger : public lib_module::Logger {
 public:
  CoreLogger();
  ~CoreLogger();

  /**
   * @see lib_module::Logger::log
   */
  bool log(const std::string &sender, const lib_module::LogSeverity sev, const std::string log_message) const;
  /**
   * Set the rule engine needed for system messages
   * @param rule_engine rule engine
   */
  void setRuleEngine(RuleEngine *rule_engine) const;
  /**
   * @see lib_module::Logger::systemMessage
   */
  bool systemMessage(const std::string message) const;

 private:
  mutable boost::log::sources::severity_logger<boost::log::trivial::severity_level> lg;  ///< severity logging object
  mutable RuleEngine *rule_engine_ = NULL;  ///< rule engine needed for system messages
};

#endif  // SRC_CORE_CORELOGGER_H_
