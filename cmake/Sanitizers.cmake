# SPDX-License-Identifier: MIT
function(t27_enable_sanitizers target)
  if(MSVC OR NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "T27_ENABLE_SANITIZERS requires GCC or Clang with ASan/UBSan")
  endif()
  target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
  target_link_options(${target} PUBLIC -fsanitize=address,undefined -fno-sanitize-recover=all)
endfunction()
