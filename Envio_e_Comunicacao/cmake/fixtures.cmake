
set(FIXTURES_DEST_DIR "${CMAKE_BINARY_DIR}/tests/fixtures")

set(FW_BIN        ${FIXTURES_DEST_DIR}/firmware.bin)
set(FW_MD5        ${FIXTURES_DEST_DIR}/firmware.bin.md5)
set(FW_MERGED_BIN ${FIXTURES_DEST_DIR}/firmware.merged.bin)

file(MAKE_DIRECTORY "${FIXTURES_DEST_DIR}")

if(NOT EXISTS "${FW_BIN}" OR NOT EXISTS "${FW_MD5}" OR NOT EXISTS "${FW_MERGED_BIN}")
  # 1. Create a base string block (1 MiB)
  string(REPEAT "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\n" 16384 BLOCK)

  # 2. Generate firmware.bin (1 MiB)
  file(WRITE "${FW_BIN}" "${BLOCK}")

  # 3. Compute MD5 checksum natively
  file(MD5 "${FW_BIN}" MD5_RESULT)
  file(WRITE "${FW_MD5}" "${MD5_RESULT}  firmware.bin\n")

  # 4. Generate firmware.merged.bin (65536 * 64 bytes 4 MiB)
  string(REPEAT "${BLOCK}" 4 BLOCKx4)

  file(WRITE "${FW_MERGED_BIN}" "${BLOCKx4}")
endif()
