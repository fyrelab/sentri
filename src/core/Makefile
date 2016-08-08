include ../../Definitions.inc

MODULE_NAME=core
BIN=core

# Additional compiler flags
CFLAGS+=-DBOOST_LOG_DYN_LINK

UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
	 LIBS+=-lboost_log_setup-mt -lboost_log-mt
else
   LIBS+=-lboost_log_setup -lboost_log
endif

# Linking libs (-lmylib or -l:filename)
LIBS+= -lmodulesentri

# Flags for linker, define library paths (-L/usr/newhall/lib)
D_LFLAGS+= -L$(ROOT_DIR)/bin/debug/lib_module/
R_LFLAGS+= -L$(ROOT_DIR)/bin/release/lib_module/
T_LFLAGS+= -L$(ROOT_DIR)/bin/testing/lib_module/

.PHONY: all debug test check clean distclean run

# Add main.cpp injection for testing
T_SRC=$(shell find $(ROOT_DIR)/src/$(MODULE_NAME) -type f -name "*.cpp" -not -name "main.cpp")
T_OBJ=$(T_SRC:$(ROOT_DIR)/src/$(MODULE_NAME)/%.cpp=$(ROOT_DIR)/bin/testing/$(MODULE_NAME)/obj/%.o)
T_OBJ+=$(ROOT_DIR)/bin/testing/$(MODULE_NAME)/obj/injection.o

include ../../Targets.inc

run : debug
	@echo Executing "/bin/debug/$(MODULE_NAME)/$(BIN)"
	@export LD_LIBRARY_PATH=$(ROOT_DIR)/bin/debug/lib_module/ && cd $(ROOT_DIR)/bin/debug/$(MODULE_NAME)/ && $(ROOT_DIR)/bin/debug/$(MODULE_NAME)/$(BIN)


$(ROOT_DIR)/bin/testing/$(MODULE_NAME)/obj/injection.o : $(ROOT_DIR)/src/testing/watchdog/injection.cpp
	@mkdir -p $(dir $@)
	@echo Compiling main injection $(<:$(ROOT_DIR)/%=%)
	@$(CC) -c $< -o $@ $(T_CFLAGS) $(INCLUDES)
