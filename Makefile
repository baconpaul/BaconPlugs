
SLUG = BaconMusic

VERSION = 0.6.0

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS += -Werror

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res doc patches README.md

# Include the VCV plugin Makefile framework
RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk

shadist:	dist
	openssl sha256 dist/$(SLUG)-$(VERSION)-$(ARCH).zip > dist/$(SLUG)-$(VERSION)-$(ARCH).zip.sha256
