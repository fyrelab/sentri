include ../../../Definitions.inc

MODULE_NAME=modules/module_mail
BIN=module_mail

# Linking libs (-lmylib or -l:filename)
LIBS+=-lquickmail -lcurl -lmodulesentri

# Flags for linker, define library paths (-L/usr/newhall/lib)
LFLAGS+= -L$(ROOT_DIR)/dep/libquickmail-0.1.22/
D_LFLAGS+= -L$(ROOT_DIR)/bin/debug/lib_module/
R_LFLAGS+= -L$(ROOT_DIR)/bin/release/lib_module/
T_LFLAGS+= -L$(ROOT_DIR)/bin/testing/lib_module/

include ../../../Targets.inc
