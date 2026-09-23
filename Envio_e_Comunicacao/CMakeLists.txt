cmake_minimum_required(VERSION 3.14)
project(OTAv2 LANGUAGES CXX)

# project modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# libs
include(functions)

set(SRC_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB INO_FILES  "${CMAKE_CURRENT_SOURCE_DIR}/*.ino")
file(GLOB SRC_FILES  "${SRC_DIR}/*.cpp")
file(GLOB HPP_FILES  "${SRC_DIR}/*.hpp")

# firmware debug and release
add_arduino_firmware_target(debug "Envio_e_Comunicacao.ino" "--optimize-for-debug" TRUE)
add_arduino_firmware_target(release "Envio_e_Comunicacao.ino" "" FALSE)

# tests
include(tests)

set_property(DIRECTORY PROPERTY ADDITIONAL_CLEAN_FILES "arduino-cli cache clean")
