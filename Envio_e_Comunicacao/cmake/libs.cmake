find_package(CppUTest REQUIRED)
if(CPPUTEST_INCLUDE_DIRS)
  set(CPPUTEST_CORRECT_INCLUDE "${CPPUTEST_INCLUDE_DIRS}")
elseif(CppUTest_INCLUDE_DIRS)
  set(CPPUTEST_CORRECT_INCLUDE "${CppUTest_INCLUDE_DIRS}")
else()
  set(CPPUTEST_CORRECT_INCLUDE "/ucrt64/include") # fallback seguro para o MSYS2
endif()
if(TARGET CppUTest)
  set_target_properties(CppUTest PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CPPUTEST_CORRECT_INCLUDE}/CppUTest")
endif()
if(TARGET CppUTestExt)
  set_target_properties(CppUTestExt PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CPPUTEST_CORRECT_INCLUDE}/CppUTestExt")
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
