/*
  defs.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_CORE_DEFS_H_
#define SRC_CORE_DEFS_H_

#include <boost/chrono.hpp>

#include <stdlib.h>

#include <string>

#define SYSTEM_NAME "sentri"

#define UCID_PREFIX SYSTEM_NAME "_"

// logging
#define LOG_SIZE 5  // in MB
#define ROTARIES 5  // number of rotary logs per session
#define MAX_LOGS 20  // maximum number of files in LOG_PATH

// MAKE ABSOLUTELY SURE NO OTHER FILES THAN LOGS ARE IN THIS DIRECTORY, THEY WOULD BE DELETED BY THE LOGGER
inline std::string getLogPath() {
#ifdef DEV
  return "../../../log";
#else
  return std::string(getenv("HOME")) + "/.log/" SYSTEM_NAME;
#endif
}

// watchdog
#ifdef DEV
#define MODULE_PATH "../modules"
#else
#define MODULE_PATH "/opt/" SYSTEM_NAME "/bin/release/modules"
#endif
#define MODULE_PREFIX "module_"
#define DEFAULT_HEARTBEAT_INTERVAL 10000  // interval in ms two heartbeats must arrive
#define MAX_HEARTBEAT_INTERVAL 15000

// sytem message
#define SYSTEM_MESSAGE "system_message"
#define SYSTEM_MESSAGE_MODULE MODULE_PREFIX SYSTEM_MESSAGE

// Typedefs
typedef boost::chrono::time_point<boost::chrono::high_resolution_clock> Timepoint;

#endif  // SRC_CORE_DEFS_H_
