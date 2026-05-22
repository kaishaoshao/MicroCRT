add_executable(microcrt-printf-smoke
  test_printf_smoke.c
  runtime_stubs.c
)

target_include_directories(microcrt-printf-smoke PRIVATE
  "${PROJECT_SOURCE_DIR}/include/libc"
)

target_link_libraries(microcrt-printf-smoke PRIVATE
  microcrt_c
)

add_test(NAME microcrt-printf-smoke COMMAND microcrt-printf-smoke)

add_executable(microcrt-printf-extended
  test_printf_extended.c
  runtime_stubs.c
)

target_include_directories(microcrt-printf-extended PRIVATE
  "${PROJECT_SOURCE_DIR}/include/libc"
)

target_link_libraries(microcrt-printf-extended PRIVATE
  microcrt_c
)

add_test(NAME microcrt-printf-extended COMMAND microcrt-printf-extended)
