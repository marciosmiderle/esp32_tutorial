find_package(CppUTest REQUIRED)
if(MINGW AND TARGET CppUTest)
  get_target_property(CPPUTEST_LINK_LIBS
    CppUTest
    INTERFACE_LINK_LIBRARIES)

  string(REPLACE "winmm.lib" "winmm"
    CPPUTEST_LINK_LIBS
    "${CPPUTEST_LINK_LIBS}")

  set_property(TARGET CppUTest PROPERTY
    INTERFACE_LINK_LIBRARIES "${CPPUTEST_LINK_LIBS}")
endif()

include(FetchContent)
FetchContent_Declare(
  arduino_emulator
  GIT_REPOSITORY https://github.com/pschatzmann/Arduino-Emulator
  GIT_TAG        main
)
FetchContent_Declare(
  ArduinoJson
  GIT_REPOSITORY https://github.com/bblanchon/ArduinoJson.git
  GIT_TAG        v7.4.3
)
FetchContent_Declare(
  PubSubClient
  GIT_REPOSITORY https://github.com/knolleary/pubsubclient.git
  GIT_TAG        v2.8
)
FetchContent_MakeAvailable(arduino_emulator ArduinoJson PubSubClient)

add_library(PubSubClient STATIC "${pubsubclient_SOURCE_DIR}/src/PubSubClient.cpp")
target_include_directories(PubSubClient PUBLIC "${pubsubclient_SOURCE_DIR}/src")
target_link_libraries(PubSubClient PUBLIC arduino_emulator)
