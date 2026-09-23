include(libs)

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

file(GLOB TEST_FILES "${TEST_DIR}/*.cpp" "${TEST_DIR}/mocks/*.cpp")

# Lista explícita de fontes do projeto para evitar conflitos com mocks
set(TEST_PROJECT_SOURCES
  ${SRC_DIR}/CheckSum.cpp
  ${SRC_DIR}/Console.cpp
  ${SRC_DIR}/DHTSensor.cpp
  ${SRC_DIR}/DHTView.cpp
  ${SRC_DIR}/Estacao.cpp
  ${SRC_DIR}/EstacaoView.cpp
  ${SRC_DIR}/HttpClientLocal.cpp
  ${SRC_DIR}/Logger.cpp
  ${SRC_DIR}/LoggerBase.cpp
  ${SRC_DIR}/LoggerMqtt.cpp
  ${SRC_DIR}/LoggerSerial.cpp
  ${SRC_DIR}/Message.cpp
  ${SRC_DIR}/MqttClient.cpp
  ${SRC_DIR}/NtcSensor.cpp
  ${SRC_DIR}/NtcView.cpp
  ${SRC_DIR}/PirSensor.cpp
  ${SRC_DIR}/RetryLogic.cpp
  ${SRC_DIR}/WatchDog.cpp
  ${SRC_DIR}/WiFiManager.cpp
)

file(GLOB LIB_SRC_FILES
  "${arduinojson_SOURCE_DIR}/src/*.cpp"
  "${pubsubclient_SOURCE_DIR}/src/*.cpp"
)

add_executable(tests
  ${TEST_PROJECT_SOURCES}
  ${TEST_FILES}
  ${LIB_SRC_FILES}
)
target_link_libraries(tests PRIVATE CppUTest CppUTestExt arduino_emulator)
target_include_directories(tests PRIVATE
  ${TEST_DIR}/mocks
  ${SRC_DIR}
  ${TEST_DIR}
  ${arduinojson_SOURCE_DIR}/src
  ${pubsubclient_SOURCE_DIR}/src
)

target_compile_options(tests PRIVATE -Wall -Wextra -Og -g3)

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
