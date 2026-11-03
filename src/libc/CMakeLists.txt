add_library(microcrt_c STATIC)
set_target_properties(microcrt_c PROPERTIES OUTPUT_NAME c)

target_sources(microcrt_c PRIVATE
  microcrt_placeholder.c
)

target_include_directories(microcrt_c PUBLIC
  "${PROJECT_SOURCE_DIR}/include/libc"
)

target_include_directories(microcrt_c PRIVATE
  "${PROJECT_SOURCE_DIR}/include"
)

target_compile_options(microcrt_c PUBLIC ${MICROCRT_COMPILE_OPTIONS})

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/stdio/CMakeLists.txt")
  add_subdirectory(stdio)
endif()
