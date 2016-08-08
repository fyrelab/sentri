/*
  message.h
  Copyright 2016 fyrelab
 */

 /*! \file message.h
     \brief sends messages of all types

     Every message which is being sent by the system is set together here. The
     messagetype and the messagebody are defined in this header.
 */

#ifndef SRC_LIB_MODULE_MESSAGE_H_
#define SRC_LIB_MODULE_MESSAGE_H_

#include <boost/log/trivial.hpp>
#include <cstdint>
#include <string>

#include "lib_module/defs.h"

using std::function;

namespace lib_module {

/*! \enum MessageType
    \brief An enum for MessageType.

    The enum contains all different types of messages which can be sent.
 */
enum MessageType {
  // Name (Description) [Message body]
  MSG_EVENT,  /**< Event (from event module to core) [event] */
  MSG_EVABT,  /**< Event abort (core tells event module event was not passed usually constraints not satisfied) */
  MSG_ACTIO,  /**< Action (from core to action module) [action] */
  MSG_HBEAT,  /**< Heartbeat (from module to core) [hbeat] */
  MSG_HBINT,  /**< Heartbeat interval (from module to core) [hbint] */
  MSG_ACKAC,  /**< Acknowledgement action (from action module to core) [ack] */
  MSG_ACKEV,  /**< Acknowledgement event (from core to event module) [action] */
  MSG_LOG,    /**< Log message */
  MSG_SYS     /**< System message */
};

// Magic numbers
#define MSG_VAR_SIZE 1016
#define MSG_MOD_NAME_SIZE 128
#define MSG_LOG_SIZE 2048

/*! \struct Event
    \brief Struct for an event.

    Contains a rule ID to get to know which rule is currently
    in use by the event. Action ID's, Event ID's and Rule ID's are unique. Also contains the size
    of the message which is sent as a character.
 */
struct Event {
  /** the rule ID is needed to get to know which rule is in use. The rule ID
      is unique. */
  RuleID rule_id;  //!< rule_id as a RuleID
  char vars[MSG_VAR_SIZE];  //!< message size as character
};

/*! \union MessageBody
    \brief union of the MessageBody which will build different types of messages

    The union for the messagebody contains a few structs, which all build different
    types of messages.
 */
union MessageBody {
  Event event;

  /*! \struct ack
      \brief Struct for acknowledgement.

      Contains the event ID to get to know from which
      module the event was from. The acknowledgement is being sent back from
      the action module to core.
   */
  struct {
    /** the event ID is needed to get to know which module will take the action.
        This event ID is unique */
    EventID event_id;  //!< event_id as EventID
  } ack;  //!< this is an acknowledgement action which is sent from module to core

  /*! \sruct hbeat
      Struct for heartbeat.
   */
  struct {
    /** module name size for the message which is sent. */
    char module[MSG_MOD_NAME_SIZE];  //!< module name size
  } hbeat;  //!< Heartbeats are being sent from module to core

  /*! \struct hbint
      \brief Struct for heartbeat interval.

      Heartbeats are being sent from the module
      to core. The are sent if the interval is reached (intervals are set in miliseconds)
      or if the speciific module takes an action.
   */
  struct {
    /** module name size for the message which is sent */
    char module[MSG_MOD_NAME_SIZE];  //!< module name size
    /** heartbeat interval. Heartbeats are given in miliseconds */
    unsigned int interval;  //!< heartbeat interval
  } hbint;  //!< the heartbeats are sent as interval from the specific module to core

  /*! \struct action
      \brief Struct for an action.

      An action is being sent if an event has been acknowledged
      by a module. The action message is sent from core to the event module. The struct
      contains the event ID of the event which is sent and the rule ID to get to know which
      rule is in use.
   */
  struct {
    /** the event ID is needed to get to know which module will take the action.
        The event ID is unique */
    EventID event_id;  //!< event_id as EventID
    /** the rule ID is needed to get to know which rule is in use. The rule ID
        is unique. */
    RuleID rule_id;  //!< rule_id as RuleID

    char vars[MSG_VAR_SIZE];  //!< size of the message
  } action;  //!< action message is sent back from core to event module if an event was acknowledged

  /*! \struct log
      \brief Struct for logging.

      The log message is being written into a logfile. A few
      log messages are also shown on the terminal.
   */
  struct {
    char module[MSG_MOD_NAME_SIZE];  //!< module name size
    /** LogSeverity is also an enum. It contains the variables LOCAL, TRACE, DEBUG,
        INFO, WARNING, ERROR and FATAL to clarify the severity of a logmessage.*/
    lib_module::LogSeverity severity;  //!< severity of a logmessage
    char log_message[MSG_LOG_SIZE];  //!< log message size
  } log;  //!< log message is written into a file and on the terminal

  /*! \struct sys
      \brief Struct for system message.

      The system message is sent by the system.
   */
  struct {
    /** Contains the size of the message which is sent by the system. */
    char message[MSG_VAR_SIZE];  //!< message size
  } sys;  //!< system message is sent by the system
};

/*! \struct Message
    \brief Struct for a Message.

    Contains a messagetype and a messagebody.
    @see MessageType
    @see MessageBody
 */
struct Message {
  /** Contains the messagetype. The messagetype is an enum which contains all different
      types of messages.*/
  MessageType type;  //!< type of the message
  /** Contains the messagebody. The messagebody is an union which contains structs of
      different messages. These messages are the body of the whole message which is written
      into the logfile or shown on the terminal.*/
  MessageBody body;  //!< body of the message
};
}  // namespace lib_module

#endif  // SRC_LIB_MODULE_MESSAGE_H_
