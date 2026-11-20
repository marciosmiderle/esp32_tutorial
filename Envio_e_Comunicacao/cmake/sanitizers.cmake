include(CheckCXXCompilerFlag)

set(SANITIZERS_FLAGS "")

if(ENABLE_SANITIZERS)
  check_cxx_compiler_flag("-fsanitize=address" HAS_ASAN)
  if(HAS_ASAN)
    list(APPEND SANITIZERS_FLAGS "-fsanitize=address")
  endif()

  check_cxx_compiler_flag("-fsanitize=undefined" HAS_UBSAN)
  if(HAS_UBSAN)
    list(APPEND SANITIZERS_FLAGS "-fsanitize=undefined")
  endif()

  check_cxx_compiler_flag("-fsanitize=leak" HAS_LSAN)
  if(HAS_LSAN AND NOT HAS_ASAN)
    list(APPEND SANITIZERS_FLAGS "-fsanitize=leak")
  endif()
endif()

function(target_apply_sanitizers TGT)
  if(SANITIZERS_FLAGS)
    target_compile_options(${TGT} PRIVATE ${SANITIZERS_FLAGS})
    target_link_options(${TGT} PUBLIC ${SANITIZERS_FLAGS})
  endif()
endfunction()
