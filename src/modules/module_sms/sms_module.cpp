/*
  module_sms.cpp
  Copyright 2016 fyrelab
*/

#include "modules/module_sms/sms_module.h"

#include <iostream>

#include "lib_module/configloader.h"
#include "curl/curl.h"

SMSModule::SMSModule(const std::string &sending_queue, const std::string &receiving_queue)
  : lib_module::Module(MODULE_NAME, sending_queue, receiving_queue) {
  // get config loader to get the smtp Settings and the rules
  lib_module::ConfigLoaderJ &configL = getConfigLoader();

  // get settings
  std::map<std::string, std::string> *moduleConfig = configL.getConfig();
  readSettings(moduleConfig);
  delete moduleConfig;

  // get rules
  rules_ = configL.getRules(MODULE_NAME);
}

SMSModule::~SMSModule() {
  delete rules_;
}

// Handles messages by starting a endless while loop and receive messages
void SMSModule::handleMessages() {
  // Message buffer
  lib_module::Message m;
  // map for vars
  std::map<std::string, std::string> vars;

  // Set heartbeat interval with tolerance range
  setHeartbeatInterval(HEARTBEAT_INTERVAL + 100);

  // Start endless loop waiting for a message
  while (true) {
    // send heartbeat
    sendHeartbeat();

    // wait for message
    bool success = receiveMessageBlocking(&m, HEARTBEAT_INTERVAL, &vars);

    if (success) {
      sendHeartbeat();

      // this module can only react to action messages
      switch (m.type) {
        case lib_module::MSG_ACTIO: {
          // Debug output to see when an action was received by the sms module
          {
            std::stringstream message;
            message << "SMS Module rceived action message with EVENT_ID: " << m.body.action.event_id
                    << " and RULE_ID: " << m.body.action.rule_id;
            log(lib_module::DEBUG, message.str());
          }

          // Now take Action (reads the configuration and sends the text message)
          if (takeAction(m, vars)) {
            // If this was successfull send a acknowledgement and log this
            acknowledgeEvent(m.body.action.event_id);
            {
              std::stringstream message;
              message << "Acknowledgement sent with event id: " << m.body.action.event_id;
              log(lib_module::DEBUG, message.str());
            }
          }
          break;
        }
        default: {
          // If the message received in the queue was not of the Type MSG_ACTIO this module
          // is not the right reciever. Log this.
          std::stringstream message;
          message << "Received messege of type: " << m.type << ". This is not an action message.";
          log(lib_module::WARNING, message.str());
        }
      }
    }
  }  // Endless while loop
}

// send the SMS
int SMSModule::sendSMS(const std::string &to, const std::string &body) {
  std::string encode = body;
  CURL *curl;
  CURLcode res;
  curl = curl_easy_init();

  std::string key = settings_.key.c_str();
  std::string text1 = "&pass=";
  std::string pass = settings_.pass.c_str();
  std::string text2 = "&from=";
  std::string from = settings_.from.c_str();
  std::string text3 = "&to=";
  std::string receiver = to;
  std::string text4 = "&text=";

  if (curl) {
    char *output = curl_easy_escape(curl, encode.c_str(), 0);
    if (output) {
      std::string link = URL + key + text1 + pass + text2 + from + text3 + receiver + text4 + output;

      curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
      curl_free(output);

      // perform request
      res = curl_easy_perform(curl);

      // check for errors
      if (res != CURLE_OK) {
        log(lib_module::ERROR, "curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
      }

      // clean up
      curl_easy_cleanup(curl);
    }
  } else {
    log(lib_module::ERROR, "curl_easy_init failed");
  }

  return 0;
}

// reads the settings out of a config map and writes it into the Settings struct
void SMSModule::readSettings(std::map<std::string, std::string> *config) {
  try {
    settings_.from = (*config)["from"];
    settings_.key = (*config)["user"];
    settings_.pass = (*config)["pass"];
  } catch (const std::out_of_range &oor) {
    log(lib_module::LogSeverity::FATAL, "Invalid settings: " + std::string(oor.what()));
    exit(EXIT_FAILURE);
  }
}

// reacts to an action
int SMSModule::takeAction(const lib_module::Message &m, const std::map<std::string, std::string> &vars) {
  // find the right rule to get the action of it, because the configuration of the action is needed
  lib_module::ActionInfo *action = NULL;
  for (auto &r : *rules_) {
    if (r.id == m.body.action.rule_id) {
      action = &r.action;
    }
  }

  if (action->key.compare(ACTION_1_TITLE) == 0) {
    // take Settings from 'action'
    std::string receiver = action->configuration["to"];
    std::string body = action->configuration["body"];

    // Replace vairables in receiver and message body
    receiver = replaceVars(receiver, vars);
    body = replaceVars(body, vars);


    // create a vector with all the receivers, so it can be used by sendSMS
    std::vector<std::string> receiverVec;
    boost::split(receiverVec, receiver, boost::is_any_of(", "));

    // send the text message to all receivers
    for (int i = 0; i < receiverVec.size(); i++) {
      sendSMS(receiverVec[i], body);
    }
  }

  return 0;
}
