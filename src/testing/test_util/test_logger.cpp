/*
  test_logger.cpp
  Copyright 2016 fyrelab
*/

#include "testing/test_util/test_logger.h"
#include "boost/date_time/posix_time/posix_time.hpp"


TestLogger::TestLogger(const std::string& logging_name) : Logger(logging_name) {
  std::string logName = logging_name;
}

TestLogger::~TestLogger() {

}

bool TestLogger::log(const std::string& logName, const lib_module::LogSeverity severity, const std::string logMessage) const{

  boost::posix_time::ptime t = boost::posix_time::microsec_clock::local_time();
  std::string logTime = to_simple_string(t);

  std::cout << "[" << logTime << "]" << "[" << test_logger::sevTypes[severity] << "]" << "[" << logName << "]"
    << logMessage << std::endl;

  return true;
}

bool TestLogger::systemMessage(const std::string message) const{
  return true;
}
