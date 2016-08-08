/*
  firebase_module.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_firebase/firebase_module.h"

#include <UrlRequest.hpp>

#include <fstream>

#include "lib_module/jsonxx.h"

FirebaseModule::FirebaseModule(const std::string &sending_queue, const std::string &receiving_queue)
  : lib_module::Module(MODULE_NAME, sending_queue, receiving_queue) {
  // Get the config loader
  lib_module::ConfigLoaderJ &config_loader = getConfigLoader();

  // Get config and rules
  config_ = config_loader.getConfig();
  rules_  = config_loader.getRules(MODULE_NAME);

  std::string token = readToken();

  // Check if token exists
  if (token.length() > 0) {
    token_ = token;
  } else {
    // No token saved, get token from server
    requestToken();
  }

  // Update hook file to tell webtool
  std::string hook_data = "{ \"token\": \"" + token_ + "\" }\n";
  writeHook(hook_data);
}

FirebaseModule::~FirebaseModule() {
  delete config_;
  delete rules_;
}

void FirebaseModule::handleMessages() {
  // Init vars for receiving messages
  lib_module::Message m;  // Message buffer
  std::map<std::string, std::string> vars;  // Map for vars

  // Set heartbeat interval with tolerance range
  setHeartbeatInterval(HEARTBEAT_INTERVAL + 100);

  // Receive messages blocking (with HEARTBEAT_INTERVAL as timeout)
  while (true) {
    // Send heartbeat with each iteration of timeout
    sendHeartbeat();

    bool hasMessage = receiveMessageBlocking(&m, HEARTBEAT_INTERVAL, &vars);

    if (hasMessage) {
      // New message: read and react on message
      switch (m.type) {
        case lib_module::MSG_ACTIO: {
          // Log received message
          {
            std::stringstream message;
            message << "Received action message with EVENT_ID: " << m.body.action.event_id
                    << " and RULE_ID: " << m.body.action.rule_id;
            log(lib_module::DEBUG, message.str());
          }

          // Take action in incoming action
          takeAction(m, vars);

          break;
        }
        default: {
          // Ignore other messages
          break;
        }
      }
    }
  }
}

std::string FirebaseModule::readToken() {
  std::ifstream token_file = std::ifstream();
  token_file.open(lib_module::getModulesConfPath() + "/" + MODULE_NAME + "/token");

  if (token_file.is_open()) {
    // Get line for token
    std::string line;
    getline(token_file, line);

    token_file.close();

    return line;
  }

  return "";
}

void FirebaseModule::writeToken() {
  // Write token in token file
  std::ofstream token_file = std::ofstream();
  token_file.open(lib_module::getModulesConfPath() + "/" + MODULE_NAME + "/token");

  if (token_file.is_open()) {
    token_file << token_ << std::endl;
    token_file.close();
  } else {
    log(lib_module::ERROR, "Could not write token file");
    return;
  }
}

void FirebaseModule::takeAction(lib_module::Message &m, std::map<std::string, std::string> &vars) {
  // Search for rule
  lib_module::ActionInfo *action = NULL;
  for (auto &r : *rules_) {
    if (r.id == m.body.action.rule_id) {
      action = &r.action;
    }
  }

  // Abort if action was not found
  if (action == NULL) {
    std::stringstream message;
    message << "For action message with RULE_ID: " << m.body.action.rule_id
            << " was no action found in rules";
    log(lib_module::ERROR, message.str());

    return;
  }

  // Action found, continue with notifying
  if (action->key == EVT_NOTIFY_KEY) {
    // Prepare message values
    std::string title;
    std::string message;
    bool useStream;

    try {
      title = replaceVars(action->configuration["title"], vars);
      message = replaceVars(action->configuration["message"], vars);
      useStream = action->configuration["useStream"] == "true";
    } catch (const std::out_of_range &oor) {
      log(lib_module::LogSeverity::ERROR, "Invalid action configuration: " + std::string(oor.what()));
      exit(EXIT_FAILURE);
    }

    // Notify and get success
    bool success = notify(title, message, useStream);

    // Send ack if successful
    if (success) {
      acknowledgeEvent(m.body.action.event_id);
    }
  } else {
    std::stringstream message;
    message << "Invalid action " << action->key << "for RULE_ID: " << m.body.action.rule_id;
    log(lib_module::ERROR, message.str());
  }
}


bool FirebaseModule::notify(std::string &title, std::string &message, bool useStream) {
  UrlRequest req = createRequest();

  // Request settings
  std::string uri = REQ_DEV_ADD_URI(token_);

  req.uri(uri);

  req.body({
    { "title", title },
    { "message", message },
    { "useStream", useStream }
  });

  // Send request
  try {
    auto res = req.perform();

    if (res.statusCode() == 200) {
      // Parse JSON response
      jsonxx::Object parser;

      if (parser.parse(res.body())) {
        if (parser.has<jsonxx::Boolean>("success")) {
          bool success = parser.get<jsonxx::Boolean>("success");

          if (!success) {
            // No success: log error and do further handling
            if (parser.has<jsonxx::String>("error")) {
              std::string error = parser.get<jsonxx::String>("error");

              log(lib_module::WARNING, "Notify: " + error);

              if (error == INVALID_TOKEN_ERROR) {
                // Token invalid: delete old one, request new one and notify user
                systemMessage("Firebase module has an invalid token, a new token was requested."
                " You need to reregister your devices to receive notifications on your phone");

                // Get new token from server
                requestToken();
              }
            } else {
              log(lib_module::WARNING, "Notify: Unknown error");
            }
          }

          return success;  // Return if notify was successful
        }
      }
    } else {
      log(lib_module::ERROR, "Notify: Server faulty");
    }
  } catch (...) {
    log(lib_module::LogSeverity::ERROR, "Couldn't notify, server unreachable!");
  }

  return false;  // In all other cases notify was unsuccessful
}

void FirebaseModule::requestToken() {
  // Clear token
  token_ = "";

  // Log request
  log(lib_module::INFO, "Request new service token");

  UrlRequest req = createRequest();

  // Request settings
  req.uri(REQ_SYS_REG_URI);

  // Send request
  try {
    auto res = req.perform();

    if (res.statusCode() == 200) {
      // Parse JSON response
      jsonxx::Object parser;

      if (parser.parse(res.body())) {
        if (parser.has<jsonxx::String>("token")) {
          token_ = parser.get<jsonxx::String>("token");
          writeToken();
        }
      }
    } else {
      log(lib_module::ERROR, "Request token: Server faulty");
    }
  } catch (...) {
    log(lib_module::LogSeverity::FATAL, "Couldn't request token, server unreachable!");
    systemMessage("firebase module was unable to request a service token! Check your device's internet connection and"
                  " check the module configuration.");
    exit(EXIT_FAILURE);
  }
}

UrlRequest FirebaseModule::createRequest() {
  UrlRequest req;

  // General settings
  req.host((*config_)["server"]);
  req.port(stoi((*config_)["port"]));
  req.addHeader("Content-Type: application/json");
  req.method("POST");

  return req;
}
