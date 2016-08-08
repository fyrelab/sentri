include ../../../Definitions.inc

# Example: modules/module_video, core, lib_module
MODULE_TO_TEST=lib_module
MODULE_NAME=testing/lib

include ../../../TestingDefinitions.inc

T_LFLAGS+= -L$(ROOT_DIR)/bin/testing/testing/test_util/
LIBS+=-ltestlogger

.PHONY: test runtest

include ../../../Targets.inc
include ../../../TestingTargets.inc
