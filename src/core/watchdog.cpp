/*
  watchdog.cpp
  Copyright 2016 fyrelab
 */

#include "core/watchdog.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <utility>

#include "core/defs.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using boost::chrono::system_clock;
using boost::chrono::milliseconds;
using boost::filesystem::path;
using boost::filesystem::directory_entry;
using boost::filesystem::directory_iterator;

static Watchdog *watchdog_signal_instance;  ///< pointer to watchdog for signal handler

Watchdog::Watchdog(const std::string &modulePath, Messenger &messenger, const lib_module::Logger &logger)
                   : messenger_(messenger), logger_(logger), modules_() {
  path modPath(modulePath);
  vector<path> availableModules;

  logger_.log(LOGGING_NAME, lib_module::INFO, "Watchdog initialising...");

  // iterate over module directory and get a list of all present modules
  if (exists(modPath) && is_directory(modPath)) {
    logger_.log(LOGGING_NAME, lib_module::INFO,
                "Modules located at: " + boost::filesystem::canonical(modPath).string());

    for (directory_entry &e : directory_iterator(modPath)) {
      string modName = e.path().filename().string();
      path modFile = path(e.path().string() + "/" + modName);

      if (boost::starts_with(modName, MODULE_PREFIX)) {
        if (exists(modFile)) {
          logger_.log(LOGGING_NAME, lib_module::INFO, "Found module: " + modName);

          // create message queue for this module
          string uqid = messenger_.addQueue(modName);

          // initialize module data
          ModuleData module_data;
          module_data.running = false;
          module_data.buried = false;
          module_data.pid = -1;
          module_data.module_path = modFile;
          module_data.UQID = uqid;
          module_data.heartbeat_interval = DEFAULT_HEARTBEAT_INTERVAL;
          module_data.last_heartbeat = system_clock::now();
          module_data.mortality_rate = DEFAULT_MORTALITY_RATE;
          module_data.missed_heartbeats = 0;

          modules_.insert(std::pair<string, ModuleData>(modName, module_data));
        } else {
            logger_.log(LOGGING_NAME, lib_module::ERROR, "Could not find module in " + e.path().string());
        }
      } else {
          logger_.log(LOGGING_NAME, lib_module::ERROR, "Invalid module folder in " + string(MODULE_PATH) + "!");
      }
    }
  } else {
    logger_.log(LOGGING_NAME, lib_module::ERROR, "Could not find module directory! "
                 + boost::filesystem::current_path().string());
  }

  watchdog_signal_instance = this;

  // install signal handler to catch SIGINT (Ctrl + C from shell)
  signal(SIGINT, Watchdog::staticSIGINTHandler);
}

Watchdog::~Watchdog() {
  stopGuarding();
  killAll();
}

void Watchdog::staticSIGINTHandler(int param) {
  // clean up all modules, make sure to stop watchdog thread first to prevent race conditions
  std::cout << std::endl << "Exiting..." << std::endl;
  watchdog_signal_instance->stopGuarding();
  watchdog_signal_instance->killAll();  // kill all modules
  kill(-getpgrp(), SIGKILL);  // kill whole remaining system
  exit(EXIT_FAILURE);
}

void Watchdog::start() {
  // fork off all modules as processes
  for (auto &m : modules_) {
    spawnModule(m.first);
  }

  startAsyncGuarding();
}

void Watchdog::spawnModule(const string &module) {
  if (modules_.find(module) != modules_.end()) {
      ModuleData &module_data = modules_[module];

      if (!module_data.running && !module_data.buried) {
        logger_.log(LOGGING_NAME, lib_module::INFO, "Forking: " + module);

        pid_t mod_pid = fork();

        if (mod_pid == 0) {
          // in child
          // create process group and session
          if (setsid() == -1) {
            logger_.log(LOGGING_NAME, lib_module::ERROR,
                        "Couldn't create process group for " + module + " with PID " + std::to_string(getpid()) + " !");
          }

          // exec module
          int exec_code = execl(module_data.module_path.string().c_str(),
                                module_data.module_path.filename().string().c_str(),
                                messenger_.getCoreQueueName().c_str(),
                                module_data.UQID.c_str(),
                                NULL);
          logger_.log(LOGGING_NAME, lib_module::ERROR,
                       "Error when executing " + module + ", execl returns " + std::to_string(exec_code));
          exit(EXIT_FAILURE);
        } else if (mod_pid > 0) {
          // in parent
          // add element to modules_
          module_data.running = true;
          module_data.pid = mod_pid;
          module_data.last_heartbeat = system_clock::now();
          module_data.missed_heartbeats = 0;
          module_data.last_stability_check = system_clock::now();
        } else {
          logger_.log(LOGGING_NAME, lib_module::ERROR, "Error when forking: " + module);
        }
      }
  }
}

void Watchdog::killModule(const string &module) {
  if (modules_.find(module) != modules_.end()) {
    ModuleData &data = modules_[module];
    int pid = data.pid;

    // check if module has its own process group
    if (data.pid == getpgid(data.pid))
      pid = -data.pid;

    kill(pid, SIGKILL);

    // save errno from SIGKILL
    int kill_errno = errno;

    // Make sure process(group) is really killed (without WNOHANG macOS blocks forever)
    // if process(group) wasn't killed, sleep a short time to make sure the OS can kill it
    time_t then = time(NULL);
    while (!waitpid(pid, NULL, WNOHANG) && then + KILL_TIMEOUT >= time(NULL)) {}

    if (waitpid(pid, NULL, WNOHANG)) {
      data.running = false;
      logger_.log(LOGGING_NAME, lib_module::INFO, "Killed " + module);
    } else {
      logger_.log(LOGGING_NAME, lib_module::ERROR,
                  "Couldn't kill " + module + ", ERRNO: " + std::string(strerror(kill_errno)));
    }
  }
}

void Watchdog::killAll() {
  for (auto &m : modules_) {
    killModule(m.first);
  }
}

void Watchdog::buryModule(const string &module) {
  if (modules_.find(module) != modules_.end()) {
    // only kill if module is still running, prevents double kill log messages when resurrecting
    if (!waitpid(modules_[module].pid, NULL, WNOHANG)) {
      killModule(module);
    }

    messenger_.removeQueue(module);  // clean up the message queue for this module
    modules_[module].buried = true;
    logger_.log(LOGGING_NAME, lib_module::ERROR, module + " died too often!");
    logger_.systemMessage(module + " died too often and has been disabled until the next restart of the system.");
  }
}

void Watchdog::resurrectModule(const string &module) {
  if (modules_.find(module) != modules_.end()) {
    killModule(module);

    // only restart the module if it didn't die too often
    if (modules_[module].mortality_rate < CRITICAL_MORTALITY_RATE) {
      spawnModule(module);
      modules_[module].mortality_rate += MORTALITY_STEP;
      logger_.log(LOGGING_NAME, lib_module::INFO, module + " has been resurrected");
    } else {
      buryModule(module);
    }
  }
}

void Watchdog::startAsyncGuarding() {
  // thread off watchdog loop
  watchdog_thread_ = new boost::thread(boost::bind(&Watchdog::watchdogThread, this));
}

void Watchdog::stopGuarding() {
  if (watchdog_thread_) {
    watchdog_thread_->interrupt();
    delete watchdog_thread_;
    watchdog_thread_ = NULL;
  }
}

void Watchdog::watchdogThread() {
  while (true) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(CHECK_INTERVAL));

    modules_mtx_.lock();
    for (auto &m : modules_) {
      if (!m.second.buried) {
        if (m.second.running && !waitpid(m.second.pid, NULL, WNOHANG)) {
          // check if there was a heartbeat in given HEARTBEAT_INTERVAL
          if (system_clock::now() - m.second.last_heartbeat > milliseconds(m.second.heartbeat_interval)) {
            m.second.missed_heartbeats++;
            logger_.log(LOGGING_NAME, lib_module::WARNING,
                        "Missed heartbeat (" + std::to_string(m.second.missed_heartbeats) + ") from " + m.first);

            // if we reached MAX_MISSED_HEARTBEATS, resurrect the module
            if (m.second.missed_heartbeats >= MAX_MISSED_HEARTBEATS) {
                logger_.log(LOGGING_NAME, lib_module::WARNING, m.first + " didn't send a heartbeat "
                            + std::to_string(MAX_MISSED_HEARTBEATS) + " times, resurrect.");
                resurrectModule(m.first);
            }
          }

          // decrease a module's mortality rate for every STABILITY_CHECK_INTERVAL it is running
          if (system_clock::now() - m.second.last_stability_check >= milliseconds(STABILITY_CHECK_INTERVAL)) {
              if (m.second.mortality_rate >= MORTALITY_DECREASE) {
                m.second.mortality_rate -= MORTALITY_DECREASE;
              } else {
                m.second.mortality_rate = 0;
              }
              m.second.last_stability_check = system_clock::now();

              logger_.log(LOGGING_NAME, lib_module::INFO, "Mortality rate was decreased for: " + m.first);
          }
        } else if (m.second.running) {
          // this module exited/is zombie but internal running state is not updated yet
          logger_.log(LOGGING_NAME, lib_module::INFO, "Somehow " + m.first + " got lost in time an space, resurrect.");
          resurrectModule(m.first);
        }
      }
    }
    modules_mtx_.unlock();
  }
}

void Watchdog::heartbeat(const string &module) {
  modules_mtx_.lock();
  if (modules_.find(module) != modules_.end()) {
    // save timestamp of current heartbeat arrival
    modules_[module].last_heartbeat = system_clock::now();

    // reset the missed heartbeats counter because we got a heartbeat
    if (modules_[module].missed_heartbeats) {
      modules_[module].missed_heartbeats = 0;
    }
  }
  modules_mtx_.unlock();
}

void Watchdog::setHeartbeatInterval(const string &module, unsigned int interval) {
  modules_mtx_.lock();
  if (modules_.find(module) != modules_.end()) {
    modules_[module].heartbeat_interval = interval;
    logger_.log(LOGGING_NAME, lib_module::DEBUG,
                "Heartbeat interval set for " + module + " : " + std::to_string(interval));
  }
  modules_mtx_.unlock();
}
