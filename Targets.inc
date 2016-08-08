# Add CFLAGS for all targets to target-specific flags
D_CFLAGS+=$(CFLAGS)
R_CFLAGS+=$(CFLAGS)
T_CFLAGS+=$(D_CFLAGS)

# Add LFLAGS for all targets to target-specific flags
D_LFLAGS+=$(LFLAGS)
R_LFLAGS+=$(LFLAGS)
T_LFLAGS+=$(LFLAGS)

# Find source file and gen object file paths
SRC=$(shell find $(ROOT_DIR)/src/$(MODULE_NAME) -type f -name "*.cpp")
D_OBJ=$(SRC:$(ROOT_DIR)/src/$(MODULE_NAME)/%.cpp=$(ROOT_DIR)/bin/debug/$(MODULE_NAME)/obj/%.o)
R_OBJ=$(SRC:$(ROOT_DIR)/src/$(MODULE_NAME)/%.cpp=$(ROOT_DIR)/bin/release/$(MODULE_NAME)/obj/%.o)

ifeq ($(T_OBJ),)
	T_SRC=$(shell find $(ROOT_DIR)/src/$(MODULE_NAME) -type f -name "*.cpp")
	T_OBJ=$(T_SRC:$(ROOT_DIR)/src/$(MODULE_NAME)/%.cpp=$(ROOT_DIR)/bin/testing/$(MODULE_NAME)/obj/%.o)
endif


ifeq ($(MODULE_NAME),lib_module)
	HEADERS=$(shell find $(ROOT_DIR)/src/lib_module -type f -name "*.h")
else
	HEADERS=$(shell find $(ROOT_DIR)/src/$(MODULE_NAME) $(ROOT_DIR)/src/lib_module -type f -name "*.h")
endif

all : $(ROOT_DIR)/bin/release/$(MODULE_NAME)/$(BIN)
debug : $(ROOT_DIR)/bin/debug/$(MODULE_NAME)/$(BIN)
test : $(ROOT_DIR)/bin/testing/$(MODULE_NAME)/$(BIN)

### Link object files
# Debug build
$(ROOT_DIR)/bin/debug/$(MODULE_NAME)/$(BIN) : $(D_OBJ) $(HEADERS)
	@mkdir -p $(dir $@)
	@$(PYTHON) $(ROOT_DIR)/dep/styleguide/cpplint/cpplint.py --linelength=120 $(HEADERS) > /dev/null
	@echo "Linking debug/$(MODULE_NAME)/$(BIN)"
	@$(CC) -o $@ $(D_LFLAGS) $(D_OBJ) $(LIBS)

# Release build
$(ROOT_DIR)/bin/release/$(MODULE_NAME)/$(BIN) : $(R_OBJ) $(HEADERS)
	@mkdir -p $(dir $@)
	@$(PYTHON) $(ROOT_DIR)/dep/styleguide/cpplint/cpplint.py --linelength=120 $(HEADERS) > /dev/null
	@echo "Linking release/$(MODULE_NAME)/$(BIN)"
	@$(CC) -o $@ $(R_LFLAGS) $(R_OBJ) $(LIBS)

# Testing build
$(ROOT_DIR)/bin/testing/$(MODULE_NAME)/$(BIN) : $(T_OBJ) $(HEADERS)
	@mkdir -p $(dir $@)
	@echo "Linking testing/$(MODULE_NAME)/$(BIN)"
	@$(CC) -o $@ $(T_LFLAGS) $(T_OBJ) $(LIBS)


### Compile object files with build specific flags
# Debug
$(ROOT_DIR)/bin/debug/$(MODULE_NAME)/obj/%.o : $(ROOT_DIR)/src/$(MODULE_NAME)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	@$(PYTHON) $(ROOT_DIR)/dep/styleguide/cpplint/cpplint.py --linelength=120 $< > /dev/null
	@echo Compiling $(<:$(ROOT_DIR)/%=%)
	@$(CC) -c $< -o $@ $(D_CFLAGS) $(CFLAGS) $(INCLUDES)

# Relese
$(ROOT_DIR)/bin/release/$(MODULE_NAME)/obj/%.o : $(ROOT_DIR)/src/$(MODULE_NAME)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	@$(PYTHON) $(ROOT_DIR)/dep/styleguide/cpplint/cpplint.py --linelength=120 $< > /dev/null
	@echo Compiling $(<:$(ROOT_DIR)/%=%)
	@$(CC) -c $< -o $@ $(R_CFLAGS) $(INCLUDES)

# Testing
$(ROOT_DIR)/bin/testing/$(MODULE_NAME)/obj/%.o : $(ROOT_DIR)/src/$(MODULE_NAME)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	@echo Compiling $(<:$(ROOT_DIR)/%=%)
	@$(CC) -c $< -o $@ $(T_CFLAGS) $(INCLUDES)

### Clean
clean :
	@rm -rf $(ROOT_DIR)/bin/debug/$(MODULE_NAME)/obj $(ROOT_DIR)/bin/release/$(MODULE_NAME)/obj
	@echo "Object files cleaned"

distclean :
	@rm -rf $(ROOT_DIR)/bin/release/$(MODULE_NAME)
	@rm -rf $(ROOT_DIR)/bin/debug/$(MODULE_NAME)
	@echo "All clean"
