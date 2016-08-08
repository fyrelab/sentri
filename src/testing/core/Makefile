include ../../../Definitions.inc

# Example: modules/module_video, core, lib_module
MODULE_TO_TEST=core
MODULE_NAME=testing/core

include ../../../TestingDefinitions.inc

# Additional compiler flags
CFLAGS+=-DBOOST_LOG_DYN_LINK

UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
	 LIBS+=-lboost_log_setup-mt -lboost_log-mt
else
   LIBS+=-lboost_log_setup -lboost_log
endif

.PHONY: test runtest

include ../../../Targets.inc
include ../../../TestingTargets.inc
