cmake_policy(SET CMP0169 OLD)
include(FetchContent)

FetchContent_Declare(
  LibOTA
  GIT_REPOSITORY https://github.com/marciosmiderle/libota
  GIT_TAG        main
  #SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libraries/libota"
)
set(LIBOTA_ENABLE_EXAMPLES false)

if(ENABLE_TESTS)
  
  FetchContent_MakeAvailable(LibOTA)
  arduino_library(ArduinoJson https://github.com/bblanchon/ArduinoJson.git v7.4.3)

else()

  set(LIBOTA_ENABLE_TESTS false)
  FetchContent_GetProperties(LibOTA)
  if(NOT libota_POPULATED)
    FetchContent_Populate(LibOTA)
  endif()

endif()
