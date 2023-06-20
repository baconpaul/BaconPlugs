cmake_minimum_required(VERSION 3.16)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
project(BaconMusic VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PLUGIN_NAME ${PROJECT_NAME})
set(ADDITIONAL_PLUGIN_DISTRIBUTABLES res patches LICENSE README.md)
include(RackSDK.cmake)

message(STATUS "BaconMusic for Rack Build Process" )
message(STATUS "Installing into '${CMAKE_INSTALL_PREFIX}'")

file(GLOB SOURCES src/*.cpp libs/midifile/src/*.cpp libs/open303-code/Source/DSPCode/*.cpp libs/mda-copies/*.cpp)
add_compile_options(-fvisibility=hidden -fvisibility-inlines-hidden)

target_include_directories(${RACK_PLUGIN_LIB} PRIVATE src libs/midifile/include libs/mda-copies libs/open303-code/Source/DSPCode)
target_sources(${RACK_PLUGIN_LIB} PRIVATE
        ${SOURCES})

