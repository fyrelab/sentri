/*
  mail_module.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_MODULES_MODULE_MAIL_MAIL_MODULE_H_
#define SRC_MODULES_MODULE_MAIL_MAIL_MODULE_H_

// Includes
#include <string>
#include <vector>
#include <map>

#include "lib_module/module.h"

// Constants
#define MODULE_NAME "module_mail"
#define HEARTBEAT_INTERVAL 10000  // 10 seconds
#define ACTION_1_TITLE "send"


/**
 * The struct SMTPSettings contains all relevent information to connect to a smtp
 * server. An instance of this struct is created in the constructor using the config
 * of the user.
 */
struct SMTPSettings {
  std::string from;
  std::string server;
  unsigned int port;
  std::string user;
  std::string pass;
};


class MailModule : lib_module::Module {
 public:
  /**
   * @param sending_queue Name of the sending queue.
   * Normally argument number 1 when a module is startet by the watchdog
   * @param receiving_queue Name of the receiving queue.
   * Normally argument number 2 when a module is startet by the watchdog
   */
  MailModule(const std::string &sending_queue, const std::string &receiving_queue);
  ~MailModule();

  /**
   * Starts an endless while loop in which messages are recieved and heartbeats are sent.
   * If a message is received it calls some private methods in order to take action.
   * Note: This method is blocking
   */
  void handleMessages();

 private:
  /**
   * Reads the SMTP Setting out of the module config and writes it into the
   * smtp_setting_ struct.
   * @param module_config
   */
  void readSMTPSettings(std::map<std::string, std::string> *config);

  /**
   * Is called when a message was received by the ::handleMessages function.
   * Detects which rule the event belongs to, constructs the subject, body and attachments using the
   * vars and calls ::sendMessage
   * @param message The message which was received
   * @param vars map containing key value pairs of the vars, filled by lib_module
   * @return 1 if sending of the mail was successfull and 0 else
   */
  int takeAction(const lib_module::Message &m, const std::map<std::string, std::string> &vars);

  /**
   * Sends the Mail using the smtp setting in the smpt_settings_ field and libquickmail.
   * @param receivers Vector containing the mail addesses of the receivers, should be at least one
   * @param subject Subject of the e-mail
   * @param body Body of the e-mail
   * @param attachments Vector containing file paths to the attachments
   */
  int sendMail(std::vector<std::string> *receivers, const std::string &subject, const std::string &body,
                std::vector<std::string> *attachments);

  SMTPSettings smtp_settings_;
  std::vector<lib_module::RuleInfo> *rules_;
};

#endif  // SRC_MODULES_MODULE_MAIL_MAIL_MODULE_H_
