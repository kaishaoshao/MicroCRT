if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libc/CMakeLists.txt")
  add_subdirectory(libc)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libm/CMakeLists.txt")
  add_subdirectory(libm)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libcrt/CMakeLists.txt")
  add_subdirectory(libcrt)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/target/CMakeLists.txt")
  add_subdirectory(target)
endif()
