/*
  evententry.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_CORE_EVENTENTRY_H_
#define SRC_CORE_EVENTENTRY_H_

#include <boost/chrono.hpp>

#include "lib_module/message.h"

/**
 * Represents an event of the system
 */
struct EventEntry {
  lib_module::Event event;  ///< @see lib_module::Event

  uint64_t id;
  boost::chrono::time_point<boost::chrono::high_resolution_clock> action_sent;  ///< timestamp when action was executed
  uint16_t num_tries;  ///< number of tried the rule engine tried take take action
};

#endif  // SRC_CORE_EVENTENTRY_H_
