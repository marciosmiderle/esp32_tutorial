function(add_arduino_firmware_target INO_FULL_FILENAME TYPE IS_ALL_TARGET)
  cmake_path(GET INO_FULL_FILENAME FILENAME    INO_FILE)
  cmake_path(GET INO_FULL_FILENAME STEM        INO_NAME)
  cmake_path(GET INO_FULL_FILENAME PARENT_PATH INO_DIR)

  set(TARGET_NAME "${INO_NAME}_${TYPE}")
  # set(TARGET_NAME "${INO_NAME}")
  set(BUILD_DIR   "${CMAKE_BINARY_DIR}/${TARGET_NAME}")

  set(DEPEND_FILES ${ARGN})
  set(FIRMWARE_BIN  "${BUILD_DIR}/${INO_FILE}.bin")
  set(FIRMWARE_MAP  "${BUILD_DIR}/${INO_FILE}.map")
  set(FIRMWARE_ELF  "${BUILD_DIR}/${INO_FILE}.elf")
  set(FIRMWARE_MBIN "${BUILD_DIR}/${INO_FILE}.merged.bin")
  set(FIRMWARE_PART "${BUILD_DIR}/${INO_FILE}.partitions.bin")
  set(FIRMWARE_MD5  "${FIRMWARE_BIN}.md5")

  if(TYPE STREQUAL "debug")
    set(EXTRA_FLAGS "--optimize-for-debug")
  endif()

  add_custom_command(
    OUTPUT "${FIRMWARE_BIN}" "${FIRMWARE_MAP}" "${FIRMWARE_ELF}" "${FIRMWARE_MBIN}" "${FIRMWARE_PART}" "${FIRMWARE_MD5}"
    COMMAND "${ARDUINO_CLI_EXECUTABLE}" compile
    --build-path "${BUILD_DIR}"
    --warnings all
    --verbose
    ${EXTRA_FLAGS}
    COMMAND ${CMAKE_COMMAND} -E md5sum "${FIRMWARE_BIN}" > "${FIRMWARE_BIN}.md5"
    DEPENDS ${DEPEND_FILES}
    WORKING_DIRECTORY ${INO_DIR}
    USES_TERMINAL
  )

  if(IS_ALL_TARGET)
    add_custom_target(${TARGET_NAME} ALL DEPENDS "${FIRMWARE_BIN}")
  else()
    add_custom_target(${TARGET_NAME} DEPENDS "${FIRMWARE_BIN}")
  endif()

  set_property(TARGET ${TARGET_NAME} APPEND PROPERTY
    ADDITIONAL_CLEAN_FILES "${FIRMWARE_MD5}"
  )
endfunction()

function(find_arduino_cli_executable)
  find_program(ARDUINO_CLI_EXECUTABLE
    NAMES arduino-cli arduino-cli.exe
    HINTS
    "$ENV{LOCALAPPDATA}/Programs/Arduino CLI"
    "C:/Program Files/Arduino CLI"
    "/usr/local/bin"
    "/opt/homebrew/bin"
    "$ENV{HOME}/.local/bin"
  )
  if(ARDUINO_CLI_EXECUTABLE)
    message(STATUS "Found arduino-cli: ${ARDUINO_CLI_EXECUTABLE}")
  else()
    message(WARNING "arduino-cli not found! Please install it or add it to your PATH.")
  endif()
endfunction()
