RACK_DIR ?= ../..
include $(RACK_DIR)/arch.mk

CMAKE_BUILD=dep/baconmusic-build
libbacon_music := $(CMAKE_BUILD)/libBaconMusic.a

OBJECTS += $(libbacon_music)

# Trigger the BaconPlugs CMake build when running `make dep`
DEPS += $(libbacon_music)

EXTRA_CMAKE :=
ifdef ARCH_MAC
ifdef ARCH_ARM64
    EXTRA_CMAKE += -DCMAKE_OSX_ARCHITECTURES="arm64"
else
    EXTRA_CMAKE += -DCMAKE_OSX_ARCHITECTURES="x86_64"
endif
endif

$(libbacon_music):
	$(CMAKE) -B$(CMAKE_BUILD) -DRACK_SDK_DIR=$(RACK_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(CMAKE_BUILD)/dist $(EXTRA_CMAKE)
	cmake --build $(CMAKE_BUILD) -- -j $(shell getconf _NPROCESSORS_ONLN)
	cmake --install $(CMAKE_BUILD)

# Add .cpp and .c files to the build
SOURCES += src/BaconPlugs.cpp

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res patches README.md

# Include the VCV plugin Makefile framework
RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk

CXXFLAGS := $(filter-out -std=c++11,$(CXXFLAGS))

COMMUNITY_ISSUE=https://github.com/VCVRack/community/issues/433

community:
	open $(COMMUNITY_ISSUE)

issue_blurb:	dist
	git diff --exit-code
	git diff --cached --exit-code
	@echo
	@echo "Paste this into github issue " $(COMMUNITY_ISSUE)
	@echo
	@echo "* Version: v$(VERSION)"
	@echo "* Transaction: " `git rev-parse HEAD`
	@echo "* Branch: " `git rev-parse --abbrev-ref HEAD`
