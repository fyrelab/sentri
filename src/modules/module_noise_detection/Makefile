include ../../../Definitions.inc

MODULE_NAME=modules/module_noise_detection
BIN=module_noise_detection

# Linking libs (-lmylib or -l:filename)
LIBS+= -lmodulesentri -lportaudio

# Flags for linker, define library paths (-L/usr/newhall/lib)
LFLAGS+=
D_LFLAGS+= -L$(ROOT_DIR)/bin/debug/lib_module/
R_LFLAGS+= -L$(ROOT_DIR)/bin/release/lib_module/
T_LFLAGS+= -L$(ROOT_DIR)/bin/testing/lib_module/

INCLUDES+=-I/usr/local/Cellar/portaudio/19.20140130/include
LFLAGS+=-L/usr/local/Cellar/portaudio/19.20140130/lib

include ../../../Targets.inc
