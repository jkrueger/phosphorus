# Core build system definitions

TOP := $(shell pwd)

all:

ifndef OS
OS := $(shell uname)
endif

ifndef ARCH
ARCH := x86_64
endif

ifndef VERBOSE
VERBOSE := @
else
VERBOSE :=
endif


ifndef SO_SUFFIX
  ifeq ($(OS),Darwin)
    SO_SUFFIX := dylib 
  else
    SO_SUFFIX := so
  endif
endif

BUILD_DIR = target/$(OS)-$(ARCH)

all_targets :=
all_headers :=
all_objects :=
all_deps    :=

define SUBDIR
dir := $(strip $1)
include $(strip $1)/build.mk
endef

.SECONDARY: $(all_headers)

# include builtin build rules

include $(TOP)/build/rules.mk


$(BUILD_DIR):
	$(VERBOSE)mkdir -p $@
