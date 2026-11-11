cmake_minimum_required(VERSION 3.25)
project(OTAv2 LANGUAGES C CXX)

# project modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# libs
include(functions)

file(GLOB INO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.ino)
set(SRC_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB SRC_FILES ${SRC_DIR}/*.cpp)
file(GLOB HPP_FILES ${SRC_DIR}/*.hpp)
list(APPEND DEP_FILES ${HPP_FILES} ${SRC_FILES} ${INO_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/sketch.yaml)

find_arduino_cli_executable()
if(ARDUINO_CLI_EXECUTABLE)
  # firmware debug and release
  add_arduino_firmware_target(debug "Envio_e_Comunicacao.ino" "--optimize-for-debug"
    TRUE  ${DEP_FILES})
  add_arduino_firmware_target(release "Envio_e_Comunicacao.ino" ""
    FALSE ${DEP_FILES})
else(ARDUINO_CLI_EXECUTABLE)
  message(WARNING "It is not possible to compile firmware images without arduino-cli.")
endif(ARDUINO_CLI_EXECUTABLE)

option(ENABLE_TESTS "Activate tests" ON)
# tests
if(ENABLE_TESTS)
  include(tests)
endif(ENABLE_TESTS)

set_property(DIRECTORY PROPERTY ADDITIONAL_CLEAN_FILES "arduino-cli cache clean")
