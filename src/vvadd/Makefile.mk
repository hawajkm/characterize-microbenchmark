# Makefile directory
APP_NAME:=$(notdir $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))
$(APP_NAME)_name := $(APP_NAME)
$(APP_NAME)_dir  := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# Instantiate the template
$(eval $(call template_mk,$(APP_NAME),$($(APP_NAME)_dir)))
