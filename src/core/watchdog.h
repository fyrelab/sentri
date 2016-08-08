/*
  watchdog.h
  Copyright 2016 fyrelab
 */

#ifndef SRC_CORE_WATCHDOG_H_
#define SRC_CORE_WATCHDOG_H_

#include <boost/filesystem.hpp>
#include <boost/chrono.hpp>
#include <cstdint>
#include <map>
#include <string>

#include "core/messenger.h"
#include "lib_module/logger.h"

// watchdogs defs
#define DEFAULT_MORTALITY_RATE 0
#define MORTALITY_STEP 10  // increase mortality by this value if a module has to be resurrected
#define MORTALITY_DECREASE 1  // for every STABILITY_CHECK_INTERVAL a module is running decrease it's mortality rates
#define CRITICAL_MORTALITY_RATE 100  // if mortality reaches this level, bury the module
#define MAX_MISSED_HEARTBEATS 5  // if this amount of heartbeats didn't reach the watchdog, restart the module
#define CHECK_INTERVAL 10000  // in seconds, general watchdog check interval
#define STABILITY_CHECK_INTERVAL 60000  // in seconds, interval to evaluate module stability by mortality rate
#define KILL_TIMEOUT 1  // time in seconds we give the OS to kill a process (ENSURE THIS IS >= 1)

#define LOGGING_NAME "watchdog"

// Shorter timings for test environment
#ifdef TESTING
#undef CHECK_INTERVAL
#define CHECK_INTERVAL 500

#undef MAX_MISSED_HEARTBEATS
#define MAX_MISSED_HEARTBEATS 2

#undef CRITICAL_MORTALITY_RATE
#define CRITICAL_MORTALITY_RATE 30

#undef STABILITY_CHECK_INTERVAL
#define STABILITY_CHECK_INTERVAL 3000
#endif

using boost::filesystem::path;
using boost::chrono::time_point;

/**
 * Contains all necessary information about a given module
 */
struct ModuleData {
  bool running;
  bool buried;  ///< set when a module dies too often
  pid_t pid;
  path module_path;  ///< filesystem path to module, relative from WD
  std::string UQID;  ///< unique of message queue for this module
  unsigned int heartbeat_interval;  ///< in ms, modules can set this individually
  time_point<boost::chrono::system_clock> last_heartbeat;
  unsigned int missed_heartbeats;
  time_point<boost::chrono::system_clock> last_stability_check;  ///< timestamp of last stability check
  unsigned int mortality_rate;   ///< indicating how often a module died/became unresponsive
};

class Watchdog {
 public:
  /**
   * @param modulePath where the module binaries are located
   * @param messenger needed to create/remove queues
   * @param logger for logging
   */
  explicit Watchdog(const std::string &modulePath, Messenger &messenger, const lib_module::Logger &logger);
  ~Watchdog();

  /**
   * Fork all found modules.
   */
  void start();

  /**
   * Notify the watchdog, that a heartbeat from module with given name has arrived.
   * This will update the internal last heartbeat timestamp for that module.
   * @param module the name of the module that sent a hearbeat
   * @remark thread-safe
   */
  void heartbeat(const std::string &module);
  /**
   * Modules can set their heartbeat interval (i.e. the interval it has to send heartbeats).
   * @param module the module name
   * @param interval heartbeat interval in ms
   * @remark thread-safe
   */
  void setHeartbeatInterval(const std::string &module, unsigned int interval);


  #ifdef TESTING
  /**
   * Returns a reference to the watchdog internal module information map for testing purposes.
   * @return map containing module name and associated module data struct
   */
  inline std::map<std::string, ModuleData> &getModuleData() {
    return modules_;
  }

  /**
   * Returns the pointer to the watchdog thread
   * @return watchdog thread pointer, may be NULL
   */
  inline boost::thread *getWatchdogThread() {
    return watchdog_thread_;
  }

  /**
   * Returns a reference to the mutex that controls access to the modules map
   * @return modules map access mutex
   */
  inline boost::mutex &getMutex() {
    return modules_mtx_;
  }

  #endif

 private:
  /**
   * Starts async watchdog thread.
   * @see watchdogThread
   */
  void startAsyncGuarding();

  /**
   * Stops async watchdog thread.
   */
  void stopGuarding();

  /**
   * Main watchdog thread. It keeps an eye on the modules of the system, i.e. when they die, didn't send
   * heartbeats, become unresponsive, etc.
   */
  void watchdogThread();

  /**
   * Forks a single module specified by its name. It also writes associated information with that module in
   * the internal module information map.
   * Creates a process group for that module so all childs module will fork will also be in that group.
   * @param module a module's name
   */
  void spawnModule(const std::string &module);

  /**
   * Kills a single module specified by its name. Updates associated module information.
   * Makes sure all child-processes that module forked are killed by sending the SIGKILL to the whole
   * process group.
   */
  void killModule(const std::string &module);

  /**
   * Kills all modules that are present.
   */
  void killAll();

  /**
   * Re-spawns a module specified by its name and increases its mortality rate, i.e. a value to indicate how
   * often a module died/became unresponsive.
   * @param module the module's name
   */
  void resurrectModule(const std::string &module);

  /**
   * Permanently (until next restart of the whole core system) disables a module specified by its name from being re-spawned. That happens e.g. when a module
   * died too often.
   * @param module the module's name
   */
  void buryModule(const std::string &module);

  /**
   * A signal handler that is called whenever the core system receives a SIGINT signal (e.g. Ctrl + C).
   * This is mainly for debugging purposes to make sure all modules are killed properly.
   * @param param signal parameter from OS, not used in handler
   */
  static void staticSIGINTHandler(int param);;

  Messenger &messenger_;  ///< Messenger to handle adding/removing message queues
  const lib_module::Logger &logger_;  ///< Logger used for logging and system messages
  boost::thread *watchdog_thread_ = NULL;  ///< Main watchdog thread @see watchdogThread()
  boost::mutex modules_mtx_;  ///< Mutex that controls access to module information @see modules_
  std::map<std::string, ModuleData> modules_;   ///< Internal module information map @see ModuleData
};

#endif  // SRC_CORE_WATCHDOG_H_
