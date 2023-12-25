# Directory
_LOCAL_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# Find all applications
_MK_FILES := $(shell find $(_LOCAL_DIR) -mindepth 2 -maxdepth 2 -name "Makefile.mk")

-include $(_MK_FILES)
