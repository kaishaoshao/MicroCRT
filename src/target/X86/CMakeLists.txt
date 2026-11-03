# X86 Target Architecture Backend

# 1. Base Target definition
microcrt_add_target(X86
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
)

# 2. Subtargets
microcrt_register_subtarget(X86 i386    DESCRIPTION "Legacy 32-bit x86")
microcrt_register_subtarget(X86 x86_64  DESCRIPTION "Standard 64-bit x86-64")
microcrt_register_subtarget(X86 haswell DESCRIPTION "Intel Haswell profile (AVX2/FMA)")

if(NOT DEFINED MICROCRT_X86_SUBTARGET AND NOT DEFINED MICROCRT_SUBTARGET)
  set(MICROCRT_X86_SUBTARGET "x86_64" CACHE STRING "Selected X86 subtarget" FORCE)
endif()

microcrt_add_subtarget(X86 i386
  DEFINITIONS "__i386__"
)

microcrt_add_subtarget(X86 x86_64
  DEFINITIONS "__x86_64__"
)

microcrt_add_subtarget(X86 haswell
  DEFINITIONS "__x86_64__" "__AVX2__"
)

# 3. Features & Vendor Custom Extensions
microcrt_target_feature(X86 AVX "Enable AVX SIMD instructions" OFF)

if(MICROCRT_X86_ENABLE_AVX)
  microcrt_add_target(X86
    DEFINITIONS "__AVX__"
  )
endif()
