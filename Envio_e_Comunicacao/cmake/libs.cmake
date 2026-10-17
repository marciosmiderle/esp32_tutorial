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
FetchContent_MakeAvailable(arduino_emulator)
include(${arduino_emulator_SOURCE_DIR}/Arduino.cmake)

arduino_library(ArduinoJson https://github.com/bblanchon/ArduinoJson.git v7.4.3)
arduino_library(PubSubClient https://github.com/knolleary/pubsubclient.git v2.8)
