cmake_minimum_required(VERSION 3.25)
project(OTAv3 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(SRC_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")

include(options)
include(sanitizers)
include(libs_dependencies)

if(ENABLE_TESTS)
  # include(fixtures)
  include(libs_cpputest)
endif()

include(functions)
include(target_main)

if(ENABLE_TESTS)
  include(target_tests)
endif()
