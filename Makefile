# Directory
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# Compilation Configuratoin
CC:=gcc
IFLAGS:=-lpthread -lm
CFLAGS:=-mavx -mavx2 -g -O3

# File and directory names
BUILD_DIR := $(ROOT_DIR)/build
SRC_DIR := $(ROOT_DIR)/src

# Get all possible benchmarks
BENCHMARKS := $(notdir $(shell dirname $(shell find $(SRC_DIR)/ -mindepth 2 -maxdepth 2 -name "Makefile.mk")))

# Output
BINS_BM  := $(addprefix $(BUILD_DIR)/,$(BENCHMARKS))
CLEAN_BM := $(addprefix clean_,$(BENCHMARKS))

# Default
all: $(BINS_BM)

# Build directory
$(BUILD_DIR):
	mkdir -p $@

# Clean
clean: $(CLEAN_BM)
	rm -rf $(BUILD_DIR)

# Template
include template.mk

# All benchmarks/applications
-include $(SRC_DIR)/Makefile.mk

.PHONY: clean all
