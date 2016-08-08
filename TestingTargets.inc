runtest : staticlib test
	@echo Executing "/bin/testing/$(MODULE_NAME)/$(BIN)"
	@export LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(ROOT_DIR)/bin/testing/lib_module/ && cd $(ROOT_DIR)/bin/testing/$(MODULE_NAME) && ./$(BIN)

staticlib :
	@mkdir -p $(ROOT_DIR)/bin/testing/$(MODULE_NAME)
	@ar rcs $(ROOT_DIR)/bin/testing/$(MODULE_NAME)/lib$(MODULE_TO_TEST).a $(OBJ_TEST)
