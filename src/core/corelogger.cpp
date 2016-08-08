/*
  logger.cpp
  Copyright 2016 fyrelab
 */

#include "core/corelogger.h"

#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/sinks/text_file_backend.hpp>

#include <iostream>

using boost::filesystem::path;
using boost::filesystem::directory_entry;
using boost::filesystem::directory_iterator;
using boost::filesystem::last_write_time;
using boost::filesystem::remove;
using boost::log::add_console_log;
using boost::log::add_file_log;
namespace keywords = boost::log::keywords;

#define CORE_LOGGING_NAME "core"

CoreLogger::CoreLogger() : Logger(CORE_LOGGING_NAME), rule_engine_(NULL) {
  boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");

  std::string startup_time = boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time());

  try {
    // iterate over directory to count log entries, delete oldest if there are too many
    if (boost::filesystem::exists(getLogPath())) {
      directory_iterator ld_it = directory_iterator(getLogPath());
      int log_count = 0;

      for (auto e : ld_it) {
        log_count++;
      }

      for (int i = log_count - MAX_LOGS + ROTARIES; i > 0; i--) {
        directory_entry oldest;
        for (auto e : directory_iterator(getLogPath())) {
            if ( oldest.path().empty() || last_write_time(e.path()) <= last_write_time(oldest.path()) )
              oldest = e;
        }
        remove(oldest.path());
      }
    } else {
      std::cerr << "Couldn't find log directory! I'm creating it for you..." << std::endl;
      if (!boost::filesystem::create_directories(getLogPath())) {
        std::cerr << "Unable to create log directory! Terminating" << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  } catch (...) {
    std::cerr << "Couldn't delete oldest log entries!" << std::endl;
  }

  // console sink
  auto console_sink = add_console_log(std::cout, keywords::format = "[%TimeStamp%][%Severity%]%Message%",
                                                 keywords::auto_flush = true);
  #ifndef DEV
  // reduce amount of log messages in release builds
  console_sink->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);
  #endif

  // logfile sink
  auto file_sink = add_file_log(keywords::file_name = getLogPath() + "/" + "log_" + startup_time + ".%N",
                                keywords::target = getLogPath(),
                                keywords::auto_flush = true,
                                keywords::format = "[%TimeStamp%][%Severity%]%Message%",
                                keywords::rotation_size = LOG_SIZE * 1024 * 1024,
                                keywords::max_size = (ROTARIES - 1) * 5 * 1024 * 1024,
                                keywords::scan_method = boost::log::sinks::file::scan_matching);
  #ifndef DEV
  // reduce amount of log messages in release builds
  file_sink->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
  #endif

  boost::log::add_common_attributes();
}

CoreLogger::~CoreLogger() {
}

bool CoreLogger::log(const std::string &sender, const lib_module::LogSeverity sev,
                     const std::string log_message) const {
  // match lib_module::LogSeverity to severity levels used by boost log
  boost::log::trivial::severity_level boost_sev = boost::log::trivial::severity_level::info;
  switch (sev) {
    case lib_module::LogSeverity::TRACE:
      boost_sev = boost::log::trivial::severity_level::trace;
      break;
    case lib_module::LogSeverity::DEBUG:
      boost_sev = boost::log::trivial::severity_level::debug;
      break;
    case lib_module::LogSeverity::INFO:
      boost_sev = boost::log::trivial::severity_level::info;
      break;
    case lib_module::LogSeverity::WARNING:
      boost_sev = boost::log::trivial::severity_level::warning;
      break;
    case lib_module::LogSeverity::ERROR:
      boost_sev = boost::log::trivial::severity_level::error;
      break;
    case lib_module::LogSeverity::FATAL:
      boost_sev = boost::log::trivial::severity_level::fatal;
      break;
    default:
      boost_sev = boost::log::trivial::severity_level::info;
      break;
  }

  // write log message
  BOOST_LOG_SEV(lg, boost_sev) << "[" << sender << "] " << log_message;

  return true;
}

void CoreLogger::setRuleEngine(RuleEngine *rule_engine) const {
  rule_engine_ = rule_engine;
}

bool CoreLogger::systemMessage(const std::string message) const {
  if (rule_engine_) {
    rule_engine_->systemMessage(message);
    return true;
  }

  return false;
}
