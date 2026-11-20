file(GLOB SRC_FILES ${SRC_DIR}/*.cpp)
file(GLOB CPPUTEST_SRCS "${TEST_DIR}/*.cpp")

add_executable(tests ${CPPUTEST_SRCS} ${SRC_FILES})
target_link_libraries(tests PRIVATE LibOTA ArduinoJson CppUTest CppUTestExt)
target_include_directories(tests PUBLIC
  # ${TEST_DIR}/src
  #  ${TEST_DIR}
  ${TEST_DIR}/mocks
  ${SRC_DIR}
)

target_compile_options(tests PRIVATE
  -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -O0 -g3 -pipe
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-overloaded-virtual>
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-non-c-typedef-for-linkage>
  $<$<COMPILE_LANGUAGE:CXX>:-include;${libota_SOURCE_DIR}/tests/src/TestIncludes.h>
)

set(SPECIFIC_OBJS ${SRC_DIR}/Message.cpp ${SRC_DIR}/Publisher.cpp)
set_source_files_properties(${SPECIFIC_OBJS}
  PROPERTIES COMPILE_OPTIONS
  "$<$<COMPILE_LANGUAGE:CXX>:-include;${TEST_DIR}/mocks/ArduinoJsonCompat.h>"
)

target_compile_definitions(tests PRIVATE
  FIXTURES_DIR="${FIXTURES_DEST_DIR}"
  WWW_DIR="${CMAKE_BINARY_DIR}"
)

set_target_properties(tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
  OUTPUT_NAME "run_tests"
)

add_custom_target(run_tests
  COMMAND ${CMAKE_BINARY_DIR}/tests/run_tests
  DEPENDS tests
)

target_apply_sanitizers(tests)
