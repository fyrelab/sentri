/*
  configloader.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_LIB_MODULE_CONFIGLOADER_H_
#define SRC_LIB_MODULE_CONFIGLOADER_H_

#include <stdlib.h>

#include <fstream>
#include <map>
#include <vector>
#include <string>

#include "lib_module/jsonxx.h"
#include "lib_module/defs.h"
#include "lib_module/logger.h"

using jsonxx::Array;
using jsonxx::String;
using jsonxx::Object;

namespace lib_module {

/*! \struct EventInfo
    Event struct. Uses string for the title of the event, the name of the module
    it belongs to, a description of the config files, and an event ID as key.
    Uses a map of strings for the variables which are sent by the event
    if it is triggered.
 */
struct EventInfo {
  /** Title of the Event */
  std::string title;
  /** Name of the Module to which the Event belongs to */
  std::string module;
  /** description from the conf.-file */
  std::string description;
  /** Event ID */
  std::string key;
  /** Variables sent with the Event by triggering Module */
  std::map<std::string, std::string> configuration;
  /** Attachments sent by the module */
  std::vector<std::string> attachments;
};

/*! \struct ActionInfo
    Action struct. Uses string for the title of the event, the name of the module
    it belongs to, a description of the config files, and an action ID as key.
    Uses a map of strings for the variables which are passed to an action.
 */
struct ActionInfo {
  /** Action title */
  std::string title;
  /** module name */
  std::string module;
  /** Description of the action */
  std::string description;
  /** Action ID */
  std::string key;
  /** Variables passed to an Action */
  std::map<std::string, std::string> configuration;
  /** Attachments that can be commited to the module */
  std::vector<std::string> attachments;
};

/*! \struct RuleInfo
    Rule struct. Uses string for the title. Uses RuleID for the id of a rule.
    Uses an EventInfo for the event and an ActionInfo for an action.
    Has a fire limit as double as first condition and a boolean to set if the
    rule is disabled.
 */
struct RuleInfo {
  /** title of the rule */
  std::string title;
  /** ID of the rule */
  RuleID id;
  /** event name */
  EventInfo event;
  // std::vector<ActionInfo> actions;
  /** action name */
  ActionInfo action;

  // Conditions
  double fire_limit;
  /** set to true if the rule shall be disabled */
  bool is_disabled;
};

/*!
    \brief loads configuration data out of files

    The configloader can get the configuration, events, action and rules from a
    module.
*/

class ConfigLoaderJ {
 public:
  /**
   * Expects the module .info file as relative path,
   * the module .config file as relative path, the module rule file as relative path and
   * the logging name. Relative paths need to be in json compatible format.
   * @param std::string module_info_file_ is the relative path to the module .info file
   * @param std::string module_config_file_ is the relative path to the module .config file
   * @param std::string module_rule_file_ is the relative path to the module .rule file
   * @param const Logger& logger contains the name of the logger as constant string reference
   */
  explicit ConfigLoaderJ(std::string module_info_file_, std::string module_config_file_, std::string module_rule_file_,
                         const Logger &logger);
  /**
   * Get the configuration and return a map of strings. The string contains
   * the configuration settings. First checks in the module config, if a value is set,
   * otherwise checks the default config file for a module.
   * @return a map of key-value og the config set in the info/config file for a module
   */
  std::map<std::string, std::string> *getConfig();

  /**
   * Function, that gives back a vector of Events that a module can send to
   * the core.
   * @param module_name e.g. "example_module"
   * @return a vector of events for the module which was given as argument
   */
  std::vector<EventInfo> *getEvents(const std::string &module_name);

  /**
   * Function, that gives back a vector of Actions that a module can perform.
   * @param module_name e.g. "example_module"
   * @return a vector of actions for the module which was given as argument
   */
  std::vector<ActionInfo> *getActions(const std::string &module_name);

  /**
   * Function that gives out a vector of all given rules a module takes place in
   * either as an Event-Module or an Action-Module
   * @param module_name e.g. "example_module", type "main" for
   * all available rules"
   * @return a vector of the rules for the module which was given as argument
   */
  std::vector<RuleInfo> *getRules(const std::string &module_name);

 private:
  /** path of the module info file */
  std::string module_info_file_;

  /** path of the module config file */
  std::string module_config_file_;

  /** path of the module rule file */
  std::string module_rule_file_;
  const Logger& logger_;
};

}  // namespace lib_module
#endif  // SRC_LIB_MODULE_CONFIGLOADER_H_
