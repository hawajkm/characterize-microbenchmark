define template_mk

# Name and directories
$(1)_BIN := $(1)
$(1)_DIR := $(2)
$(1)_BUILD_DIR := $$(BUILD_DIR)/$(1)_build

# Object files
$(1)_C_FILES := $$(shell find $$($(1)_DIR) -name "*.c")
$(1)_O_FILES := $$(foreach x,$$($(1)_C_FILES),$$(patsubst $$($(1)_DIR)/%,$$($(1)_BUILD_DIR)/%,$$(x)))
$(1)_O_FILES := $$(foreach x,$$($(1)_O_FILES),$$(patsubst %.c,%.o,$$(x)))
$(1)_D_FILES := $$($(1)_O_FILES:%.o=%.d)

# Include directories
$(1)_INCLUDE_DIR := $$($(1)_DIR) $$(SRC_DIR) $$(BUILD_DIR) $$($(1)_BUILD_DIR)

# Generate include directory argument
$(1)_INCLUDES := $$(foreach x,$$($(1)_INCLUDE_DIR),$$(addprefix -I,$$(x)))

# GCC -MMD glory!
-include $$($(1)_D_FILES)

$$($(1)_BUILD_DIR)/%.o: $$($(1)_DIR)/%.c | $$($(1)_BUILD_DIR)
	mkdir -p $$(dir $$@)
	$$(CC) $$($(1)_INCLUDES) $$(CFLAGS) -MMD -c $$< -o $$@

$$(BUILD_DIR)/$$($(1)_BIN): $$($(1)_O_FILES) | $$($(1)_BUILD_DIR)
	$$(CC) $$($(1)_O_FILES) $$(IFLAGS) -o $$@

$$($(1)_BUILD_DIR): | $$(BUILD_DIR)
	mkdir -p $$@

$(1): $$(BUILD_DIR)/$$($(1)_BIN)

clean_$(1):
	rm -f  $$($(1)_O_FILES)
	rm -f  $$($(1)_D_FILES)
	rm -f  $$(BUILD_DIR)/$$($(1)_BIN)
	rm -rf $$($(1)_BUILD_DIR)

.PHONY: clean_$(1) $(1)

endef
