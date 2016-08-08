include ../../../Definitions.inc

.PHONY: test runtest

test :
	@cd modules && $(MAKE) test

runtest : test
	@echo Executing "/bin/testing/core/core"
	@export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(ROOT_DIR)/bin/testing/lib_module/ && cd $(ROOT_DIR)/bin/testing/core/ && $(ROOT_DIR)/bin/testing/core/core
