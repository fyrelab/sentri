include ../../../Definitions.inc

MODULE_NAME=modules/module_firebase
BIN=module_firebase

# Linking libs (-lmylib or -l:filename)
LIBS+=-lmodulesentri
CFLAGS+=-std=c++1y

# Flags for linker, define library paths (-L/usr/newhall/lib)
INCLUDES+=-isystem $(ROOT_DIR)/dep/embeddedRest/

D_LFLAGS+= -L$(ROOT_DIR)/bin/debug/lib_module/
R_LFLAGS+= -L$(ROOT_DIR)/bin/release/lib_module/
T_LFLAGS+= -L$(ROOT_DIR)/bin/testing/lib_module/

include ../../../Targets.inc
