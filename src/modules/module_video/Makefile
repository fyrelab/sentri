include ../../../Definitions.inc

MODULE_NAME=modules/module_video
BIN=module_video

# Linking libs (-lmylib or -l:filename)
LIBS+=-lmodulesentri $(filter-out -lippicv, $(shell pkg-config --libs opencv))

INCLUDES+= $(shell pkg-config --cflags opencv)

# Flags for linker, define library paths (-L/usr/newhall/lib)
LFLAGS+=
D_LFLAGS+= -L$(ROOT_DIR)/bin/debug/lib_module/
R_LFLAGS+= -L$(ROOT_DIR)/bin/release/lib_module/
T_LFLAGS+= -L$(ROOT_DIR)/bin/testing/lib_module/

include ../../../Targets.inc

run : all
	@echo Executing "/bin/debug/$(MODULE_NAME)/$(BIN)"
	@export DYLD_LIBRARY_PATH=$(ROOT_DIR)/dep/opencv-2.4.13/release/lib/ && $(ROOT_DIR)/bin/debug/$(MODULE_NAME)/$(BIN)
