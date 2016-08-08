/*
  sms_module.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_MODULES_MODULE_SMS_SMS_MODULE_H_
#define SRC_MODULES_MODULE_SMS_SMS_MODULE_H_

// Includes
#include <cctype>
#include <iomanip>
#include <sstream>
#include <map>
#include <vector>
#include <string>

#include "lib_module/module.h"

// Constants
#define MODULE_NAME "module_sms"
#define HEARTBEAT_INTERVAL 10000
#define URL "http://http.gtx-messaging.net/smsc.php?user="
#define ACTION_1_TITLE "send"

struct Settings {
  std::string from;
  std::string key;
  std::string pass;
};


class SMSModule : lib_module::Module {
 public:
  /**
   * Constructor of SMSModule
   * @param sending_queue Name of the sending queue.
   * Normally argument number 1 when a module is startet by the watchdog
   * @param receiving_queue Name of the receiving queue.
   * Normally argument number 2 when a module is startet by the watchdog
   */
  SMSModule(const std::string &sending_queue, const std::string &receiving_queue);
  ~SMSModule();

  /**
   * Starts an endless while loop in which messages are recieved and heartbeats are sent.
   * If a message is received it calls some private methods in order to take action.
   * Note: This method is blocking
   */
  void handleMessages();

 private:
  /**
   * Sends the text message to the numbers which are read out of the rule file.
   * The numbers are handed to the method as a vector, sepetated by a comma (e.g. 491234567, 497654321)
   * The text message is also read out of the rule file. The message should not be longer than 160 characters
   * due to cost issues.
   * @param &to vector of receivers numbers
   * @param &body message which is sent as an text message to a phone. Should't be longer than 160 characters
   */
  int sendSMS(const std::string &to, const std::string &body);

  /**
   * Is called when a message was received by the ::handleMessages function.
   * Detects which rule the event belongs to, constructs the subject, body and attachments using the
   * vars and calls ::sendMessage
   * @param message The message which was received
   * @param vars map containing key value pairs of the vars, filled by lib_module
   * @return 1 if sending of the text message was successfull and 0 else
   */
  int takeAction(const lib_module::Message &m, const std::map<std::string, std::string> &vars);

  /**
   * Reads the settings out of the module config and writes it into the
   * settings_ struct
   * @param module_config
   */
  void readSettings(std::map<std::string, std::string> *config);


  Settings settings_;
  std::vector<lib_module::RuleInfo> *rules_;
};

#endif  // SRC_MODULES_MODULE_SMS_SMS_MODULE_H_
