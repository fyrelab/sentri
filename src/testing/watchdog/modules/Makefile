include ../../../../Definitions.inc

# Linking libs (-lmylib or -l:filename)
LIBS+=-lmodulesentri

# Add LFLAGS for all targets to target-specific flags
D_LFLAGS+=$(LFLAGS)
R_LFLAGS+=$(LFLAGS)
T_LFLAGS+=$(LFLAGS)

# Flags for linker, define library paths (-L/usr/newhall/lib)
D_LFLAGS+= -L$(ROOT_DIR)/bin/debug/lib_module/
R_LFLAGS+= -L$(ROOT_DIR)/bin/release/lib_module/
T_LFLAGS+= -L$(ROOT_DIR)/bin/testing/lib_module/

MOD_SRC=$(notdir $(shell find $(ROOT_DIR)/src/testing/watchdog/modules/ -type f -name "*.cpp"))
MOD=$(MOD_SRC:%.cpp=%)
MOD_BIN=$(foreach mod,$(MOD),$(ROOT_DIR)/bin/testing/modules/$(mod)/$(mod))

.PHONY: test

test : $(MOD_BIN)


$(ROOT_DIR)/bin/testing/modules/module_test1/module_test1 : $(ROOT_DIR)/bin/testing/modules/module_test1/obj/main.o $(HEADERS)
	@echo "Linking module_test1"
	@$(CC) -o $@ $(T_LFLAGS) $^ $(LIBS)

$(ROOT_DIR)/bin/testing/modules/module_test2/module_test2 : $(ROOT_DIR)/bin/testing/modules/module_test2/obj/main.o $(HEADERS)
	@echo "Linking module_test2"
	@$(CC) -o $@ $(T_LFLAGS) $^ $(LIBS)

$(ROOT_DIR)/bin/testing/modules/mod_test3/mod_test3 : $(ROOT_DIR)/bin/testing/modules/mod_test3/obj/main.o $(HEADERS)
	@echo "Linking mod_test3"
	@$(CC) -o $@ $(T_LFLAGS) $^ $(LIBS)

$(ROOT_DIR)/bin/testing/modules/module_test4/module_test4 : $(ROOT_DIR)/bin/testing/modules/module_test4/obj/main.o $(HEADERS)
	@echo "Linking module_test4"
	@$(CC) -o $@ $(T_LFLAGS) $^ $(LIBS)

$(ROOT_DIR)/bin/testing/modules/module_test5/module_test5 : $(ROOT_DIR)/bin/testing/modules/module_test5/obj/main.o $(HEADERS)
	@echo "Linking module_test5"
	@$(CC) -o $@ $(T_LFLAGS) $^ $(LIBS)

$(ROOT_DIR)/bin/testing/modules/module_test6/module_test6 : $(ROOT_DIR)/bin/testing/modules/module_test6/obj/main.o $(HEADERS)
	@echo "Linking module_test6"
	@$(CC) -o $@ $(T_LFLAGS) $^ $(LIBS)

$(ROOT_DIR)/bin/testing/modules/module_test7/module_test7 : $(ROOT_DIR)/bin/testing/modules/module_test7/obj/main.o $(HEADERS)
	@echo "Linking module_test7"
	@$(CC) -o $@ $(T_LFLAGS) $^ $(LIBS)

$(ROOT_DIR)/bin/testing/modules/%/obj/main.o : $(ROOT_DIR)/src/testing/watchdog/modules/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	@echo Compiling $(<:$(ROOT_DIR)/%=%)
	@$(CC) -c $< -o $@ $(T_CFLAGS) $(INCLUDES)
