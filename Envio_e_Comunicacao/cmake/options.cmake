if(NOT DEFINED ENABLE_TESTS)
  option(ENABLE_TESTS "Activate tests" OFF)
endif()

if(NOT DEFINED ENABLE_SANITIZERS)
  option(ENABLE_SANITIZERS "Activate memory and behavior sanitizers" ON)
endif()

# show status
if(ENABLE_TESTS)
  message(STATUS "Activate tests ON")
endif()
if(ENABLE_SANITIZERS)
  message(STATUS "Activate memory and behavior sanitizers ON")
endif()
