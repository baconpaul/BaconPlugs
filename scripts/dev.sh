pushd surge/ignore/rack-build && CFLAGS= && make -j surge-common
popd
RACK_DIR=../Rack-SDK make -j dist && cp dist/BaconMusic-2.0.DEVELOPMENT-mac.vcvplugin ~/Documents/Rack2/plugins && /Applications/VCV\ Rack\ 2\ Free.app/Contents/MacOS/Rack

