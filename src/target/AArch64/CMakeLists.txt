# AArch64 Target Architecture Backend

# 1. Base Target definition
microcrt_add_target(AArch64
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
)

# 2. Subtargets
microcrt_register_subtarget(AArch64 generic     DESCRIPTION "Generic AArch64 baseline")
microcrt_register_subtarget(AArch64 cortex-a53  DESCRIPTION "ARM Cortex-A53 (ARMv8-A)")
microcrt_register_subtarget(AArch64 neoverse-n1 DESCRIPTION "ARM Neoverse-N1 server core (ARMv8.2-A)")

if(NOT DEFINED MICROCRT_AARCH64_SUBTARGET AND NOT DEFINED MICROCRT_SUBTARGET)
  set(MICROCRT_AARCH64_SUBTARGET "generic" CACHE STRING "Selected AArch64 subtarget" FORCE)
endif()

microcrt_add_subtarget(AArch64 generic
  DEFINITIONS "__aarch64__"
)

microcrt_add_subtarget(AArch64 cortex-a53
  DEFINITIONS "__aarch64__" "__ARM_ARCH_8A__"
  COMPILE_OPTIONS "-mcpu=cortex-a53"
)

microcrt_add_subtarget(AArch64 neoverse-n1
  DEFINITIONS "__aarch64__" "__ARM_ARCH_8A__"
  COMPILE_OPTIONS "-mcpu=neoverse-n1"
)

# 3. Features & Vendor Custom Extensions
microcrt_target_feature(AArch64 SVE "Enable AArch64 Scalable Vector Extension (SVE)" OFF)

if(MICROCRT_AARCH64_ENABLE_SVE)
  microcrt_add_target(AArch64
    DEFINITIONS "__ARM_FEATURE_SVE"
  )
endif()

# Vendor custom matrix / NPU instructions
microcrt_register_vendor_extension(AArch64 Vendor MatrixExt
  DESCRIPTION "Vendor proprietary Matrix/Tensor extension"
  DEFAULT OFF
)

microcrt_add_vendor_extension(AArch64 Vendor MatrixExt
  DEFINITIONS "__AARCH64_VENDOR_MATRIX_EXT__"
)
