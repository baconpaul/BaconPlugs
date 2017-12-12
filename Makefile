DIST_NAME=BaconPlugs
SOURCES = $(wildcard src/*.cpp)


include ../../plugin.mk


dist: all
	mkdir -p dist/$(DIST_NAME)
	cp LICENSE* dist/$(DIST_NAME)/
	cp $(TARGET) dist/$(DIST_NAME)/
	cp -R res dist/$(DIST_NAME)/
