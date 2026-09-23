function(add_arduino_firmware_target TYPE OUTPUT_PREFIX EXTRA_FLAGS IS_ALL_TARGET)
  set(FIRMWARE_BIN  "${CMAKE_BINARY_DIR}/${TYPE}/${OUTPUT_PREFIX}.bin")
  set(FIRMWARE_MAP  "${CMAKE_BINARY_DIR}/${TYPE}/${OUTPUT_PREFIX}.map")
  set(FIRMWARE_ELF  "${CMAKE_BINARY_DIR}/${TYPE}/${OUTPUT_PREFIX}.elf")
  set(FIRMWARE_MBIN "${CMAKE_BINARY_DIR}/${TYPE}/${OUTPUT_PREFIX}.merged.bin")
  set(FIRMWARE_PART "${CMAKE_BINARY_DIR}/${TYPE}/${OUTPUT_PREFIX}.partitions.bin")
  set(FIRMWARE_MD5  "${FIRMWARE_BIN}.md5")
  add_custom_command(
    OUTPUT "${FIRMWARE_BIN}" "${FIRMWARE_MAP}" "${FIRMWARE_ELF}" "${FIRMWARE_MBIN}" "${FIRMWARE_PART}"
    COMMAND arduino-cli compile
    --build-path "${CMAKE_BINARY_DIR}/${TYPE}"
    --warnings all
    --verbose
    ${EXTRA_FLAGS}
    COMMAND ${CMAKE_COMMAND} -E md5sum "${FIRMWARE_BIN}" > "${FIRMWARE_BIN}.md5"
    DEPENDS ${SRC_FILES} ${INO_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    USES_TERMINAL
  )
  if(IS_ALL_TARGET)
    add_custom_target(${TYPE} ALL DEPENDS "${FIRMWARE_BIN}")
  else()
    add_custom_target(${TYPE} DEPENDS "${FIRMWARE_BIN}")
  endif()
  set_property(TARGET ${TYPE} APPEND PROPERTY
    ADDITIONAL_CLEAN_FILES "${FIRMWARE_MD5}"
  )
endfunction()
