cmake_minimum_required(VERSION 3.20.0)
project(MicroCRT VERSION 0.1.0 LANGUAGES C ASM)

include(cmake/microcrt_config.cmake)

option(MICROCRT_BUILD_STDIO_LEGACY "Build the imported legacy stdio implementation" OFF)
option(MICROCRT_BUILD_TESTS "Build MicroCRT local tests" OFF)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
  set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

set(MICROCRT_COMPILE_OPTIONS
  "-ffreestanding"
  "-fno-builtin"
  "-fno-stack-protector"
  "-ffunction-sections"
  "-fdata-sections"
  "-Wall"
)

add_subdirectory(src)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION include
)

install(
  TARGETS microcrt_c
  ARCHIVE DESTINATION lib
)
