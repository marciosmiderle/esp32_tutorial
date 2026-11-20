include(FetchContent)

set(TESTS OFF CACHE BOOL "" FORCE)
set(CPPUTEST_BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  CppUTest
  GIT_REPOSITORY https://github.com/cpputest/cpputest.git
  GIT_TAG        v4.0
)
FetchContent_MakeAvailable(CppUTest)

if(MINGW AND TARGET CppUTest)
  get_target_property(CPPUTEST_LINK_LIBS CppUTest INTERFACE_LINK_LIBRARIES)
  string(REPLACE "winmm.lib" "winmm" CPPUTEST_LINK_LIBS "${CPPUTEST_LINK_LIBS}")
  set_property(TARGET CppUTest PROPERTY INTERFACE_LINK_LIBRARIES "${CPPUTEST_LINK_LIBS}")
endif()
