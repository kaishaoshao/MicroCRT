add_library(microcrt_c STATIC)
set_target_properties(microcrt_c PROPERTIES OUTPUT_NAME c)

target_sources(microcrt_c PRIVATE
  microcrt_placeholder.c
)

target_include_directories(microcrt_c PUBLIC
  "${PROJECT_SOURCE_DIR}/include/libc"
)

target_include_directories(microcrt_c PRIVATE
  "${PROJECT_SOURCE_DIR}/src/libc/stdio"
  "${PROJECT_SOURCE_DIR}/src/libc/stdio/printf"
)

target_compile_options(microcrt_c PUBLIC ${MICROCRT_COMPILE_OPTIONS})
target_compile_definitions(microcrt_c PRIVATE
  __IO_PERCENT_N
)

if(MICROCRT_PRINTF_ENABLE_AUTO_FLUSH)
  target_compile_definitions(microcrt_c PRIVATE
    MICROCRT_PRINTF_ENABLE_AUTO_FLUSH=1
  )
endif()

add_subdirectory(stdio)
