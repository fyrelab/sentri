/*
  mail_module.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_mail/mail_module.h"

#include <quickmail.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>

#include <iostream>

#include "lib_module/configloader.h"

MailModule::MailModule(const std::string &sending_queue, const std::string &receiving_queue)
  : lib_module::Module(MODULE_NAME, sending_queue, receiving_queue) {
  // get config loader to get the smtp Settings and the rules
  lib_module::ConfigLoaderJ &configL = getConfigLoader();

  // get smtp settings
  std::map<std::string, std::string> *moduleConfig = configL.getConfig();
  readSMTPSettings(moduleConfig);
  delete moduleConfig;

  // get rules
  rules_ = configL.getRules(MODULE_NAME);
}

MailModule::~MailModule() {
  delete rules_;
}

// Handles Messages by starting a endless while loop and receive messages
void MailModule::handleMessages() {
  // Message buffer
  lib_module::Message m;
  // map for vars
  std::map<std::string, std::string> vars;

  // Set heartbeat interval with tolerance range
  setHeartbeatInterval(HEARTBEAT_INTERVAL + 100);

  // Start endless loop waiting for a message
  while (true) {
    // map for vars
    std::map<std::string, std::string> vars;

    // send heartbeat
    sendHeartbeat();

    // wait for message
    bool success = receiveMessageBlocking(&m, HEARTBEAT_INTERVAL, &vars);

    if (success) {
      sendHeartbeat();

      // this module can only react to action messages
      switch (m.type) {
        case lib_module::MSG_ACTIO: {
          // Debug output to see when a action was received by the mail module
          {
            std::stringstream message;
            message << "Received action message with EVENT_ID: " << m.body.action.event_id
                    << " and RULE_ID: " << m.body.action.rule_id;
            log(lib_module::DEBUG, message.str());
          }

          // Now take Action (reads the configuration and sends the mail)
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
          std::cout << m.body.log.module << std::endl;
          log(lib_module::WARNING, message.str());
        }
      }
    }
  }  // Endless while loop
}

// reads the smtp Setting out of a config writes it into the smtp_settings_ struct
void MailModule::readSMTPSettings(std::map<std::string, std::string> *config) {
  try {
    smtp_settings_.from = (*config)["from"];
    smtp_settings_.server = (*config)["smtp_server"];
    smtp_settings_.port = std::stoi((*config)["smtp_port"]);
    smtp_settings_.user = (*config)["smtp_user"];
    smtp_settings_.pass = (*config)["smtp_pass"];
  } catch (const std::out_of_range &oor) {
    log(lib_module::LogSeverity::FATAL, "Invalid configuration: " + std::string(oor.what()));
    exit(EXIT_FAILURE);
  }
}

// reacts to an action
int MailModule::takeAction(const lib_module::Message &m, const std::map<std::string, std::string> &vars) {
  // find the right rule to get the action of it, because the configuration of the action is needed
  lib_module::ActionInfo *action = NULL;
  for (auto &r : *rules_) {
    if (r.id == m.body.action.rule_id)
      action = &r.action;
  }

  // If a action was found and it has the right Key (Module Mail only reacts to one key)
  // the config of the action can be loaded and the mail send
  if (action != NULL && action->key.compare(ACTION_1_TITLE) == 0) {
    // take Settings from 'action'
    std::string receiver;
    std::string subject;
    std::string body;
    std::string attachments;

    try {
      receiver = action->configuration["to"];
      subject = action->configuration["subject"];
      body = action->configuration["body"];
      attachments = action->configuration["attachments"];
    } catch (const std::out_of_range &oor) {
      log(lib_module::LogSeverity::FATAL, "Invalid configuration: " + std::string(oor.what()));
      exit(EXIT_FAILURE);
    }

    subject = replaceVars(subject, vars);
    // replace variables in subject, body and attachments
    body = replaceVars(body, vars);
    for (const auto &variable : vars) {
      const std::string &key = variable.first;
      const std::string &value = variable.second;
      boost::replace_all(attachments, key, value);
    }

    // create a vector with all the attachments, so it can be used by sendMail
    std::vector<std::string> attVec;
    boost::split(attVec, attachments, boost::is_any_of(","));


    // create a vector with all the receivers, so it can be used by sendMail
    std::vector<std::string> receiverVec;
    boost::split(receiverVec, receiver, boost::is_any_of(", "));

    // finally send the mail
    return sendMail(&receiverVec, subject, body, &attVec);
  } else if (action == NULL) {
    // If action == NULL this means that no action with connected to the rule id
    // sent in the message exists

    std::stringstream message;
    message << "For action message with RULE_ID: " << m.body.action.rule_id
            << " was no action found in rules";
    log(lib_module::ERROR, message.str());

    return 0;

  } else {
    // the action is not an email, but the MailModule can only send emails

    std::stringstream message;
    message << "For action message with RULE_ID: " << m.body.action.rule_id
            << " exists no action with this key";
    log(lib_module::ERROR, message.str());
    return 0;
  }
}

// send a Mail
int MailModule::sendMail(std::vector<std::string> *receivers,
                         const std::string &subject,
                         const std::string &body,
                         std::vector<std::string> *attachments ) {
  quickmail_initialize();
  quickmail mailobj = quickmail_create(smtp_settings_.from.c_str(), subject.c_str());

  // Add receivers to mail
  for (std::vector<std::string>::iterator it = receivers->begin(); it != receivers->end(); ++it) {
    quickmail_add_to(mailobj, it->c_str());
  }

  // Set the body of the mail
  quickmail_set_body(mailobj, body.c_str());

  // Add attachments to mail
  for (std::vector<std::string>::iterator it = attachments->begin(); it != attachments->end(); ++it) {
    quickmail_add_attachment_file(mailobj, it->c_str(), NULL);
  }

  // Try sending the mail. If the errormsg is NULL, everthing is fine, but else log a error message
  const char* errmsg;
  if ((errmsg = quickmail_send(mailobj, smtp_settings_.server.c_str(), smtp_settings_.port,
          smtp_settings_.user.c_str(), smtp_settings_.pass.c_str())) != NULL) {
    std::stringstream message;
    message << "Error sendin email: " << errmsg;
    log(lib_module::ERROR, message.str());

    // destroy the mailobj (libquickmail is a C library)
    quickmail_destroy(mailobj);
    return 0;
  }

  // destroy the mailobj (libquickmail is a C library)
  quickmail_destroy(mailobj);
  return 1;
}
