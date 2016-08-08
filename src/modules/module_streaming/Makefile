include ../../../Definitions.inc

MODULE_NAME=modules/module_streaming
BIN=module_streaming

# Linking libs (-lmylib or -l:filename)
LIBS+=-lmodulesentri

# Flags for linker, define library paths (-L/usr/newhall/lib)
LFLAGS+=
D_LFLAGS+= -L$(ROOT_DIR)/bin/debug/lib_module/
R_LFLAGS+= -L$(ROOT_DIR)/bin/release/lib_module/
T_LFLAGS+= -L$(ROOT_DIR)/bin/testing/lib_module/

include ../../../Targets.inc
