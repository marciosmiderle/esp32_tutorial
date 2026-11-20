set(SKETCH_YAML_IN "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sketch.yaml.in")
set(SKETCH_YAML    "${CMAKE_CURRENT_SOURCE_DIR}/sketch.yaml")
configure_file("${SKETCH_YAML_IN}" "${SKETCH_YAML}")

file(GLOB INO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.ino)

file(GLOB SRC_FILES ${SRC_DIR}/*.cpp)
file(GLOB HPP_FILES ${SRC_DIR}/*.hpp)
list(APPEND DEP_FILES ${HPP_FILES} ${SRC_FILES} ${INO_FILES} "${SKETCH_YAML}" "${SKETCH_YAML_IN}")

find_arduino_cli_executable()
if(ARDUINO_CLI_EXECUTABLE)
  foreach(INO_FILENAME ${INO_FILES})
    add_arduino_firmware_target(${INO_FILENAME} debug   TRUE  ${DEP_FILES})
    add_arduino_firmware_target(${INO_FILENAME} release FALSE ${DEP_FILES})
  endforeach()
else()
  message(WARNING "It is not possible to compile firmware images without arduino-cli.")
endif()

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "arduino-cli cache clean")
