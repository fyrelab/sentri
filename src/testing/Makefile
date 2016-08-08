include ../../Definitions.inc

.PHONY: test runtest

test :
	@mkdir -p $(ROOT_DIR)/bin/testing
	@cd test_util && $(MAKE) test
	@cd watchdog && $(MAKE) test
	@cd lib && $(MAKE) staticlib && $(MAKE) test
	@cd core && $(MAKE) staticlib && $(MAKE) test

runtest :
	-@cd watchdog && $(MAKE) runtest
	-@export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(ROOT_DIR)/bin/testing/testing/test_util/ && cd lib && $(MAKE) runtest
	-@export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(ROOT_DIR)/bin/testing/testing/test_util/ && cd core && $(MAKE) runtest
