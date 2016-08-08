/*
  test_logger.h
  Copyright 2016 fyrelab
 */
#ifndef SRC_TESTING_TEST_UTIL_TEST_LOGGER_H_
#define SRC_TESTING_TEST_UTIL_TEST_LOGGER_H_

#include <stdio.h>
#include "lib_module/logger.h"
#include "lib_module/defs.h"
#include <string>

namespace test_logger {
  enum LogSeverity {LOCAL, TRACE, DEBUG, INFO, WARNING, ERROR, FATAL, SIZE_OF_ENUM};
  static const char* sevTypes[] = {"local", "trace", "debug", "info", "warning", "error", "fatal"};

  // static_assert(sizeof(test_logger::LogSeverity)/sizeof(char*) == test_logger::SIZE_OF_ENUM, "sizes don't match");
}

class TestLogger : public lib_module::Logger {
 public:
   TestLogger(const std::string&);
   ~TestLogger();

   bool log(const std::string&, const lib_module::LogSeverity, const std::string) const;

   bool systemMessage(const std::string) const;
};

#endif  // SRC_TESTING_TEST_UTIL_TEST_LOGGER_H_
