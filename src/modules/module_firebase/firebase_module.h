/*
  firebase_module.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_MODULES_MODULE_FIREBASE_FIREBASE_MODULE_H_
#define SRC_MODULES_MODULE_FIREBASE_FIREBASE_MODULE_H_

#include <string>
#include <map>
#include <vector>

#include "lib_module/module.h"

// Constants
#define MODULE_NAME "module_firebase"
#define HEARTBEAT_INTERVAL 1000  // 1 second
#define REQ_SYS_REG_URI "/register/system"
#define REQ_DEV_ADD_URI(token) ("/notify/" + (token))
#define EVT_NOTIFY_KEY "notify"
#define INVALID_TOKEN_ERROR "Invalid token"

class UrlRequest;

class FirebaseModule : lib_module::Module {
 public:
  /**
   * @see lib_module::Module::Module
   */
  FirebaseModule(const std::string &sending_queue, const std::string &receiving_queue);
  ~FirebaseModule();

  /**
   * Start the loop to read messages and send notifications
   */
  void handleMessages();

 private:
  /**
   * Reads the service token for the notification server from the token file
   * @return service token
   */
  std::string readToken();

  /**
   * Writes the currently active token to the token file
   */
  void writeToken();

  /**
   * Request a new token from the server and saves it
   * On error this sets an empty string as token
   */
  void requestToken();

  /**
   * Create a JSON request with server, port, header etc preset
   * @return Prepared request
   */
  UrlRequest createRequest();

  /**
   * React to an action message
   * @param m Message itself
   * @param vars Variable map of the message
   */
  void takeAction(lib_module::Message &m, std::map<std::string, std::string> &vars);

  /**
   * Send a push notification to all registered devices
   * @param title The title of the notification
   * @param message The message body of the notification
   * @param useStream If the stream should be on focus of the notification
   * @return success
   */
  bool notify(std::string &title, std::string &message, bool useStream);

  std::map<std::string, std::string> *config_;  ///< Module config
  std::vector<lib_module::RuleInfo> *rules_;  ///< Module rules

  std::string token_;  ///< Service token
};

#endif  // SRC_MODULES_MODULE_FIREBASE_FIREBASE_MODULE_H_
