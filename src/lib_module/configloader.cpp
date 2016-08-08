/*
  configloader.cpp
  Copyright 2016 fyrelab
 */

#include "lib_module/configloader.h"

#include <utility>

#include "lib_module/jsonxx.h"

using jsonxx::Array;
using jsonxx::String;
using jsonxx::Object;
using jsonxx::Number;
using jsonxx::Boolean;

namespace lib_module {
ConfigLoaderJ::ConfigLoaderJ(std::string module_info_file, std::string module_config_file,
                             std::string module_rule_file, const Logger& logger)
                             : module_info_file_(module_info_file), module_config_file_(module_config_file),
                               module_rule_file_(module_rule_file), logger_(logger) {
}

std::map<std::string, std::string> *ConfigLoaderJ::getConfig() {
  std::ifstream file(module_config_file_);
  std::ifstream default_file(module_info_file_);

  Object parser;
  parser.parse(file);

  Object default_parser;
  default_parser.parse(default_file);

  std::map<std::string, std::string> *out_map = new std::map<std::string, std::string>();

  int i = 0;
  try {
    // Check whether the default file exists
    if (parser.has<Array>("configuration")) {
      while (parser.get<Array>("configuration").has<Object>(i)) {
        const Object &user_conf = parser.get<Array>("configuration").get<Object>(i);

        String key = user_conf.get<String>("key");
        out_map->insert(std::pair<std::string, std::string>(key,
        user_conf.get<String>("value")));
        i++;
      }
    } else {
      // std::cout << "Missing or corrupted .config-File" << std::endl;
      logger_.log(LogSeverity::INFO, "Missing or corrupted .config-File: " + module_config_file_);
    }
    if (default_parser.has<Array>("configuration")) {
      i = 0;
      while (default_parser.get<Array>("configuration").has<Object>(i)) {
        const Object &default_conf = default_parser.get<Array>("configuration").get<Object>(i);

        String default_key = default_conf.get<String>("key");
        if (out_map->count(default_key) == 1 && out_map->at(default_key) == "") {
          out_map->at(default_key) = default_conf.get<String>("default");
        } else if (out_map->count(default_key) == 0) {
          out_map->insert(std::pair<std::string, std::string>(default_key,
          default_conf.get<String>("default")));
        }
        i++;
      }
    } else {
      logger_.log(LogSeverity::ERROR, "Missing or corrupted .info-File: " + module_info_file_);
    }
  } catch (int e) {
    logger_.log(LogSeverity::FATAL, "It occured an error while reading a configuration.");
    exit(EXIT_FAILURE);
  }

  return out_map;
}

std::vector<EventInfo> *ConfigLoaderJ::getEvents(const std::string &module_name) {
  std::ifstream file(module_info_file_);

  Object parser;
  parser.parse(file);

  std::vector<EventInfo> *out_event = new std::vector<EventInfo>();

  int i = 0;
  int j = 0;

  try {
  while (parser.get<Array>("events").has<Object>(i)) {
    const Object &event = parser.get<Array>("events").get<Object>(i);

    EventInfo new_event;
    out_event->push_back(new_event);
    out_event->at(i).key = event.get<String>("key");
    out_event->at(i).title = event.get<String>("title");
    out_event->at(i).description = event.get<String>("description");
    j = 0;
    while (event.get<Array>("configuration").has<Object>(j)) {
      const Object &conf = event.get<Array>("configuration").get<Object>(j);

      std::string conf_id = conf.get<String>("key");
      out_event->at(i).configuration[conf_id] = conf.get<String>("default");
      j++;
    }
    out_event->at(i).module = module_name;
    i++;
  }
  } catch (int e) {
    // std::cout << "It occured an error while reading an event. Please check the .config-File" << std::endl;
    logger_.log(LogSeverity::FATAL,
               "It occured an error while reading an event configuration. Please check " + module_info_file_);
    exit(EXIT_FAILURE);
  }
  return out_event;
}

std::vector<ActionInfo> *ConfigLoaderJ::getActions(const std::string& module_name) {
  std::ifstream file(module_info_file_);

  Object parser;
  parser.parse(file);

  std::vector<ActionInfo> *out_action = new std::vector<ActionInfo>();

  int i = 0;
  int j = 0;

  try {
    while (parser.get<Array>("actions").has<Object>(i)) {
      const Object &action = parser.get<Array>("actions").get<Object>(i);

      ActionInfo new_action;
      out_action->push_back(new_action);
      out_action->at(i).key = action.get<String>("key");
      out_action->at(i).title = action.get<String>("title");
      out_action->at(i).description = action.get<String>("description");
      j = 0;
      while (action.get<Array>("configuration").has<Object>(j)) {
        const Object &conf = action.get<Array>("configuration").get<Object>(j);

        std::string conf_id = conf.get<String>("key");
        out_action->at(i).configuration[conf_id] = conf.get<String>("default");
        j++;
      }
      out_action->at(i).module = module_name;
      i++;
    }
  } catch (int e) {
    logger_.log(LogSeverity::FATAL,
                "It occured an error while reading an action configuration. Please check " + module_info_file_);
    exit(EXIT_FAILURE);
  }

  return out_action;
}

std::vector<RuleInfo> *ConfigLoaderJ::getRules(const std::string &module_name) {
  std::ifstream file(module_rule_file_);

  Object rule_parser;
  rule_parser.parse(file);

  std::vector<RuleInfo> *out_rule = new std::vector<RuleInfo>();

  int i = 0;  // Counting variabiable for rule parsing
  int j = 0;  // Counting variable for inner configuration parsing
  try {
    while (rule_parser.get<Array>("rules").has<Object>(i)) {
      const Object &rule = rule_parser.get<Array>("rules").get<Object>(i);
      const Object &event = rule.get<Object>("event");
      const Object &action = rule.get<Object>("action");

      if (module_name == "main" || event.get<String>("module") == module_name
                                || action.get<String>("module") == module_name) {
        RuleInfo newRule;

        newRule.id = i;
        newRule.title = rule.get<String>("title");

        newRule.fire_limit = rule.get<Object>("constraints").get<Number>("fire_limit", 0.0);
        newRule.is_disabled = rule.get<Object>("constraints").get<Boolean>("is_disabled", false);

        newRule.event.module = event.get<String>("module");
        newRule.event.key = event.get<String>("key");

        newRule.action.module = action.get<String>("module");
        newRule.action.key = action.get<String>("key");

        j = 0;
        while (event.get<Array>("configuration").has<Object>(j)) {
          const Object &conf = event.get<Array>("configuration").get<Object>(j);

          newRule.event.configuration[conf.get<String>("id")] = conf.get<String>("value");

          j++;
        }

        j = 0;
        while (action.get<Array>("configuration").has<Object>(j)) {
          const Object &conf = action.get<Array>("configuration").get<Object>(j);

          newRule.action.configuration[conf.get<String>("id")] = conf.get<String>("value");

          j++;
        }

        out_rule->push_back(newRule);
      }
      i++;
    }
  } catch (int e) {
    logger_.log(LogSeverity::FATAL,
                "It occured an error while reading a configuration. Please check " + module_rule_file_);
    exit(EXIT_FAILURE);
  }

  return out_rule;
}

}  // namespace lib_module
