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

# firmware debug and release
add_arduino_firmware_target(debug "Envio_e_Comunicacao.ino" "--optimize-for-debug"
  TRUE  ${DEP_FILES})
add_arduino_firmware_target(release "Envio_e_Comunicacao.ino" ""
  FALSE ${DEP_FILES})

# tests
include(tests)

set_property(DIRECTORY PROPERTY ADDITIONAL_CLEAN_FILES "arduino-cli cache clean")
