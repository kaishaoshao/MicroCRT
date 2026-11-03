cmake_minimum_required(VERSION 3.20.0)
project(MicroCRT VERSION 0.1.0 LANGUAGES C ASM)

include(cmake/microcrt_config.cmake)
include(cmake/MicroCRTTarget.cmake)

# Initialize LLVM-style target architecture backends
microcrt_initialize_targets()

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

if(MICROCRT_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
  enable_testing()
  add_subdirectory(tests)
endif()

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION include
)

if(TARGET microcrt_c)
  install(
    TARGETS microcrt_c
    ARCHIVE DESTINATION lib
  )
endif()

# Print LLVM-style configuration summary
microcrt_print_configuration_summary()
