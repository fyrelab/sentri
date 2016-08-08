BIN=test_$(MODULE_TO_TEST)
PATH_TO_TEST_OBJ=$(ROOT_DIR)/bin/testing/$(MODULE_TO_TEST)

# Linking libs (-lmylib or -l:filename)
LIBS+=-l$(MODULE_TO_TEST) -lmodulesentri -ltestlogger

# Flags for linker, define library paths (-L/usr/newhall/lib)
T_LFLAGS+= -L$(ROOT_DIR)/bin/testing/$(MODULE_NAME)/ -L$(ROOT_DIR)/bin/testing/lib_module/ -L$(ROOT_DIR)/bin/testing/testing/test_util/

OBJ_TEST=$(shell find $(PATH_TO_TEST_OBJ) -type f -name "*.o")
