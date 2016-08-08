/*
  trigger_module.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_trigger/trigger_module.h"
#include <boost/thread.hpp>

TriggerModule::TriggerModule(const std::string &sending_queue, const std::string &receiving_queue)
                             : lib_module::Module(MODULE_NAME, sending_queue, receiving_queue) {
  // get the config loader and get the rules
  lib_module::ConfigLoaderJ &configL = getConfigLoader();
  std::vector<lib_module::RuleInfo> *rules = configL.getRules(MODULE_NAME);

  // create a new vector to hold the EventTrigger objects
  event_triggers_ = new std::vector<EventTrigger *>();

  // create for every rule a EventTrigger object
  for (auto rule : *rules) {
    // if the event type is auto_event the trigger module can handle this event
    if (rule.event.key.compare("auto_event") == 0) {
      // get the configuration of the event
      int timeout;
      int timeout_between;
      std::string vars;

      try {
        timeout = std::stoi(rule.event.configuration["timeout"]);
        timeout_between = std::stoi(rule.event.configuration["timeout-between"]);
        vars = rule.event.configuration["vars"];
      } catch (const std::out_of_range &oor) {
        log(lib_module::FATAL, "Invalid value in configuration: " + std::string(oor.what()));
        exit(EXIT_FAILURE);
      } catch (const std::invalid_argument &ir) {
        log(lib_module::FATAL, "No conversion possible: " + std::string(ir.what()));
        exit(EXIT_FAILURE);
      }

      // create new object of EventTrigger and add it to the vector
      EventTrigger *et = new EventTrigger(this, timeout, timeout_between, rule.id, vars);
      event_triggers_->push_back(et);

    } else {
      log(lib_module::WARNING, "Unknown event: " + rule.event.key);
    }
  }
  delete rules;
}

TriggerModule::~TriggerModule() {
  for (size_t i = 0; i < event_triggers_->size(); i++) {
    delete (*event_triggers_)[i];
  }
  delete event_triggers_;
}

void TriggerModule::startSendingEvents() {
  for (size_t i = 0; i < event_triggers_->size(); i++) {
    // start the async sending of events
    boost::thread(boost::bind(&EventTrigger::startSendingEventsAsync, (*event_triggers_)[i]));
  }
}

void TriggerModule::sendHeartbeatsForever() {
  // Set heartbeat interval with tolerance range
  setHeartbeatInterval(HEARTBEAT_INTERVAL + 100);

  // start endless loop
  while (true) {
    sendHeartbeat();
    boost::this_thread::sleep(boost::posix_time::milliseconds(HEARTBEAT_INTERVAL));

    // clear receiving queue. Maybe there are some acknowledgements
    while (receiveDiscard()) {}
  }
}



EventTrigger::EventTrigger(lib_module::Module *module, int timeout, int timeout_between,
          lib_module::RuleID rule_id, const std::string vars) :
          module_(module), timeout_(timeout), timeout_between_(timeout_between), rule_id_(rule_id), vars_(vars) {
}

EventTrigger::~EventTrigger() {
}

void EventTrigger::startSendingEventsAsync() {
  // Debug log to determine that a thread for this rule has startet
  {
    std::stringstream logM;
    logM << "Thread started with rule id: " << rule_id_;
    module_->log(lib_module::DEBUG, logM.str());
  }

  // sleep the initial timeout
  boost::this_thread::sleep(boost::posix_time::seconds(timeout_));

  // if timeout_between is 0 only one Message should be sent
  bool endless = timeout_between_ > 0;

  // start endless loop sending events
  do {
    // Debug log that a event was triggerd
    {
      std::stringstream logM;
      logM << "Send message with rule id: " << rule_id_;
      module_->log(lib_module::DEBUG, logM.str());
    }

    // send message and sleep again
    module_->sendEventMessage(rule_id_, vars_);
    boost::this_thread::sleep(boost::posix_time::seconds(timeout_between_));
  } while (endless);
}
