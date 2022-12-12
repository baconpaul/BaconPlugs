cmake_minimum_required(VERSION 3.16)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
project(BaconMusic VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

set(PLUGIN_NAME ${PROJECT_NAME})
set(ADDITIONAL_PLUGIN_DISTRIBUTABLES patches README.md)
include(RackSDK.cmake)

message(STATUS "Installing into '${CMAKE_INSTALL_PREFIX}'")

file(GLOB SOURCES src/*.cpp libs/midifile/src/*.cpp libs/open303-code/Source/DSPCode/*.cpp)
list(REMOVE_ITEM SOURCES src/BaconPlugs.cpp)

add_library(${PLUGIN_NAME} STATIC ${SOURCES})

target_link_libraries(${PLUGIN_NAME} PUBLIC RackSDK)
target_include_directories(${PLUGIN_NAME} PRIVATE libs/midifile/include libs/open303-code/Source/DSPCode)

target_sources(${RACK_PLUGIN_LIB} PRIVATE src/BaconPlugs.cpp)
target_link_libraries(${RACK_PLUGIN_LIB} PRIVATE ${PLUGIN_NAME})

add_custom_target(${PROJECT_NAME}_quick_install
        COMMAND cmake -E copy ${CMAKE_BINARY_DIR}/${LIB_NAME}.* ${CMAKE_INSTALL_PREFIX}/${PLUGIN_NAME}
        COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/plugin.json ${CMAKE_INSTALL_PREFIX}/${PLUGIN_NAME}
        )
add_dependencies(${PROJECT_NAME}_quick_install ${PLUGIN_NAME})
add_dependencies(${PROJECT_NAME}_quick_install ${RACK_PLUGIN_LIB})
