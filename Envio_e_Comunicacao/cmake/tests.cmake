include(libs)

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

file(GLOB TEST_FILES "${TEST_DIR}/*.cpp" "${TEST_DIR}/mocks/*.cpp")

set(TEST_PROJECT_SOURCES ${SRC_FILES})

add_executable(tests ${TEST_PROJECT_SOURCES} ${TEST_FILES})
target_link_libraries(tests PRIVATE CppUTest CppUTestExt arduino_emulator ArduinoJson PubSubClient)
target_include_directories(tests PRIVATE
  ${TEST_DIR}/mocks
  ${TEST_DIR}
  ${SRC_DIR}
)

target_compile_options(tests PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-overloaded-virtual -O0 -g3 -pipe)

# Injeta mocks globais e compatibilidade (ex: printf, ArduinoJson String write)
target_compile_options(tests PRIVATE -include "${TEST_DIR}/TestIncludes.h")

set_target_properties(tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
  OUTPUT_NAME "run_tests"
)

add_custom_target(run_tests
  COMMAND ${CMAKE_BINARY_DIR}/tests/run_tests
  DEPENDS tests
)
