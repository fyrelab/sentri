/*
  main.cpp
  Copyright 2016 fyrelab
 */

#include <boost/filesystem.hpp>
#include <string>

#include "core/sentri.h"
#include "core/defs.h"
#include "lib_module/defs.h"

using boost::filesystem::exists;
using boost::filesystem::path;

int main() {
  // delete lock file from webtool
  boost::filesystem::remove(boost::filesystem::path(lib_module::getModulesConfPath() + "/lock"));

#ifndef DEV
  // check if default conf exists, if not copy a default set from static data files
  if (!exists(path(lib_module::getModulesConfPath()))) {
    std::cout << "Creating default configuration files..." << std::endl;
    std::string cmd = "mkdir -p " + lib_module::getModulesConfPath() + " && cp -r " + lib_module::getModulesInfoPath()
                      + "/default/* " + lib_module::getModulesConfPath();
    system(cmd.c_str());
  }
#endif

  Sentri *sentri = new Sentri(lib_module::getRulesConfPath(), MODULE_PATH);
  sentri->start();

  delete sentri;

  return 0;
}
