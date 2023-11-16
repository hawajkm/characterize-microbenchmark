CC:=gcc
IFLAGS:=-lm
CFLAGS:=-O3

# Output name
BIN := main

# File and directory names
BUILD_DIR := build
SRC_DIR := src

# Object files
C_FILES := $(shell find $(SRC_DIR) -name "*.c")
O_FILES := $(foreach x,$(C_FILES),$(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(x)))
O_FILES := $(foreach x,$(O_FILES),$(patsubst %.c,%.o,$(x)))
D_FILES := $(O_FILES:%.o=%.d)

# Default
all: $(BUILD_DIR)/$(BIN)

-include $(D_FILES)

# Include directories
INCLUDE_DIR:=$(SRC_DIR) $(BUILD_DIR) $(BUILD_DIR)/gen

# Generate include directory argument
INCLUDES:=$(foreach x,$(INCLUDE_DIR),$(addprefix -I,$(x)))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/$(BIN): $(O_FILES) | build
	$(CC) $(O_FILES) $(IFLAGS) -o $@

build:
	mkdir -p build

clean:
	rm -f $(O_FILES)
	rm -f $(D_FILES)
	rm -f $(BUILD_DIR)/$(BIN)

.PHONY: clean all
