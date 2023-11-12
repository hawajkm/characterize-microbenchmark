CC:=gcc
OPTS:=-O1 -lstdc++ -ffast-math

# Directories
BUILD_DIR := build
SRC_DIR := src

# Object files
C_FILES := $(shell find $(SRC_DIR) -name "*.c")
O_FILES := $(foreach x,$(C_FILES),$(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(x)))
O_FILES := $(foreach x,$(O_FILES),$(patsubst %.c,%.o,$(x)))

# Include directories
INCLUDE_DIR:=$(SRC_DIR) $(BUILD_DIR) $(BUILD_DIR)/gen

# Generate include directory argument
I_OPTS:=$(foreach x,$(INCLUDE_DIR),$(addprefix -I,$(x)))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(I_OPTS) $(OPTS) -c $< -o $@

build/main: $(O_FILES) | build
	$(CC) $(O_FILES) -o build/main

build:
	mkdir -p build

all: build/main

clean:
	rm -f $(O_FILES)
	rm -f build/main

.PHONY: clean all
