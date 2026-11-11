
option(ENABLE_SANITIZERS "Activate memory and behavior sanitizers if supported by compiler" ON)

if(ENABLE_SANITIZERS)
  include(CheckCXXCompilerFlag)

  set(SANITIZERS_FLAGS "")

  check_cxx_compiler_flag("-fsanitize=address" HAS_ASAN)
  if(HAS_ASAN)
    list(APPEND SANITIZERS_FLAGS "-fsanitize=address")
  endif()

  check_cxx_compiler_flag("-fsanitize=undefined" HAS_UBSAN)
  if(HAS_UBSAN)
    list(APPEND SANITIZERS_FLAGS "-fsanitize=undefined")
  endif()

  # Nota: Em muitas plataformas (como Linux), o ASan já inclui o LSan por padrão.
  # Mas checar explicitamente garante o funcionamento caso use apenas o LSan isolado.
  check_cxx_compiler_flag("-fsanitize=leak" HAS_LSAN)
  if(HAS_LSAN AND NOT HAS_ASAN)
    list(APPEND SANITIZERS_FLAGS "-fsanitize=leak")
  endif()
endif()
