/*
  defs.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_LIB_MODULE_DEFS_H_
#define SRC_LIB_MODULE_DEFS_H_

#include <string>

#include "core/defs.h"

#define HOOK_FILE_NAME "hook_data.json"
#define EXT_INFO ".info"
#define EXT_CONF ".conf"

namespace lib_module {

typedef uint64_t RuleID;
typedef uint64_t EventID;

enum LogSeverity {LOCAL, TRACE, DEBUG, INFO, WARNING, ERROR, FATAL};

inline std::string getModulesConfPath() {
#ifdef DEV
  return "../../../res/conf";
#else
  return std::string(getenv("HOME")) + "/.config/" SYSTEM_NAME;
#endif
}

inline std::string getModulesInfoPath() {
#ifdef DEV
  #ifdef TESTING
    return "../../../src/testing/watchdog/modules";
  #else
    return "../../../res/share";
  #endif
#else
  return "/usr/share/" SYSTEM_NAME;
#endif
}

inline std::string getRulesConfPath() {
#ifdef DEV
  #ifdef TESTING
    return "../../../src/testing/watchdog/modules/rules.conf";
  #else
    return "../../../res/conf/rules.conf";
  #endif
#else
  return getModulesConfPath() + "/rules.conf";
#endif
}

inline std::string getTempPath() {
  return "/tmp/sentri";
}

}  // namespace lib_module

#endif  // SRC_LIB_MODULE_DEFS_H_
