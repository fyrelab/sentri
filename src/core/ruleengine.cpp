/*
  ruleengine.cpp
  Copyright 2016 fyrelab
 */

#include "core/ruleengine.h"

#include <iostream>
#include <cmath>

#include "testing/def.h"

#define LOGGING_NAME "rule engine"

RuleEngine::RuleEngine(const std::vector<lib_module::RuleInfo> &rules, const Messenger &messenger,
                       const lib_module::Logger& logger)
                  : rules_(rules),
                    rule_timestamps_(std::vector<Timepoint>(rules.size(), Timepoint(boost::chrono::duration<int>(0)))),
                    logger_(logger),
                    messenger_(messenger) {
}

RuleEngine::~RuleEngine() {
  // End ack checker thread if existing
  if (ack_thread_) {
    ack_thread_->interrupt();
    delete ack_thread_;
  }
}

void RuleEngine::enqueueEvent(const EventEntry &entry) {
  EventEntry *e = new EventEntry(entry);

  try {
    bool trigger = true;

    const lib_module::RuleInfo &rule = getRule(entry.event.rule_id);

    // Check if disabled
    if (trigger && rule.is_disabled) {
      trigger = false;
    }

    // Check for fire limit
    if (trigger && rule.fire_limit > 0.0) {
      auto now = boost::chrono::high_resolution_clock::now();
      auto limit = boost::chrono::milliseconds(std::lround(rule.fire_limit * 1000.0));

      if (rule_timestamps_[entry.event.rule_id] + limit > now) {
        trigger = false;
      }
    }

    // Check if constraints are fulfilled event, if no apply action
    if (trigger) {
      // Set number of tries to 0
      e->num_tries = 0;

      // Take action for event

      ack_mtx_.lock();

      // Add to ack queue and wait for acknowdlegement
      e->id = event_id_++;
      ack_queue_.push_back(e);

      // Send action message to action module
      takeAction(*e);

      ack_mtx_.unlock();
    } else {
      // Entry not needed any more, since not enqueued
      delete e;

      // Report violated constraints to event module
      try {
        const lib_module::RuleInfo &rule = getRule(entry.event.rule_id);
        messenger_.sendEventAbort(rule.event.module, entry);
      } catch(RuleNotFoundException &ex) {
        logger_.log(LOGGING_NAME, lib_module::LogSeverity::WARNING, ex.what());
      }
    }
  } catch(RuleNotFoundException &ex) {
    logger_.log(LOGGING_NAME, lib_module::LogSeverity::WARNING, ex.what());

    // Entry not needed any more, since not enqueued
    delete e;
  }
}

void RuleEngine::acceptAck(lib_module::EventID event_id) {
  EventEntry *entry = NULL;
  bool ackValid = false;  // Ack is valid and exists

  ack_mtx_.lock();

#ifdef DEV
  size_t num = ack_queue_.size();
#endif

  // Find event with ID and mark as acknowledged
  for (std::list<EventEntry *>::iterator it = ack_queue_.begin(); it != ack_queue_.end(); ++it) {
    entry = *it;
    if (entry->id == event_id) {
      // Remove event from queue
      ack_queue_.erase(it);
      ackValid = true;

      break;
    }
  }

  assert(num - (ackValid ? 1 : 0) == ack_queue_.size());

  ack_mtx_.unlock();

  if (ackValid) {
    // Redirect acknowdlegement to event module
    try {
      const lib_module::RuleInfo &rule = getRule(entry->event.rule_id);
      messenger_.sendEventAck(rule.event.module, *entry);
    } catch(RuleNotFoundException &ex) {
      logger_.log(LOGGING_NAME, lib_module::LogSeverity::WARNING, ex.what());
    }

    delete entry;
  }
}

void RuleEngine::takeAction(EventEntry &entry) {
  // Get rule for rule ID
  try {
    const lib_module::RuleInfo &rule = getRule(entry.event.rule_id);

    // Note: Check if exists is not necessary because takeAction is only called before checking that
    // Set last trigger of rule to now
    rule_timestamps_[entry.event.rule_id] = boost::chrono::high_resolution_clock::now();

    // Increment number of tries
    entry.num_tries++;
    entry.action_sent = boost::chrono::high_resolution_clock::now();

    // Send action to action module
    messenger_.sendAction(rule.action.module, entry);
  } catch(RuleNotFoundException &ex) {
    logger_.log(LOGGING_NAME, lib_module::LogSeverity::WARNING, ex.what());
  }
}

void RuleEngine::startAckCheckAsync() {
  ack_thread_ = new boost::thread(boost::bind(&RuleEngine::ackThread, this));
}

void RuleEngine::ackThread() {
  while (true) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(ACK_CHECK_INTERVAL));

    auto now = boost::chrono::high_resolution_clock::now();
    auto limit = boost::chrono::milliseconds(ACK_TIMEOUT);

    ack_mtx_.lock();

    for (std::list<EventEntry *>::iterator it = ack_queue_.begin(); it != ack_queue_.end();) {
      EventEntry *entry = *it;

      if ((now - entry->action_sent) >= limit) {
        if (entry->num_tries <= NUM_ACTION_RETRIES) {
          std::stringstream message;
          message << "No ack received, retry applying action with RULE_ID " << entry->event.rule_id;
          logger_.log(LOGGING_NAME, lib_module::LogSeverity::WARNING, message.str());

          // Take action if retries are still left
          takeAction(*entry);
        } else {
          // Remove element from queue
          it = ack_queue_.erase(it);

          std::stringstream message;
          message << "Abort retrying applying action with RULE_ID " << entry->event.rule_id;
          logger_.log(LOGGING_NAME, lib_module::LogSeverity::WARNING, message.str());

          // Send abort event
          try {
            const lib_module::RuleInfo &rule = getRule(entry->event.rule_id);
            messenger_.sendEventAbort(rule.event.module, *entry);
          } catch(RuleNotFoundException &ex) {
            logger_.log(LOGGING_NAME, lib_module::LogSeverity::WARNING, ex.what());
          }

          delete entry;

          // Skip loop here to avoid increment after iterator was reset
          continue;
        }
      }

       ++it;
    }

    ack_mtx_.unlock();
  }
}

void RuleEngine::systemMessage(const std::string &message) {
  // find system message rules
  lib_module::RuleID sm_id;

  for (auto &r : rules_) {
    if (r.event.module == SYSTEM_MESSAGE_MODULE && r.event.key == SYSTEM_MESSAGE) {
      // build event entry for system message rule and execute
      sm_id = r.id;
      lib_module::Event sm_event;
      sm_event.rule_id = sm_id;
      snprintf(sm_event.vars, sizeof(sm_event.vars), SYSTEM_MESSAGE "=%s", message.c_str());

      EventEntry sm_entry;
      sm_entry.event = sm_event;
      sm_entry.id = sm_id;
      enqueueEvent(sm_entry);
    }
  }
}

const lib_module::RuleInfo &RuleEngine::getRule(lib_module::RuleID id) {
  // Is rule ID within mapping boundries?
  if (rules_.size() > id) {
    return rules_[id];
  }

  throw RuleNotFoundException();
}
