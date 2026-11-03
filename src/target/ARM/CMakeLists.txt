# ARM Target Architecture Backend

# 1. Base Target definition
microcrt_add_target(ARM
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
)

# 2. Subtargets
microcrt_register_subtarget(ARM cortex-m0  DESCRIPTION "ARM Cortex-M0/M0+ (ARMv6-M)")
microcrt_register_subtarget(ARM cortex-m3  DESCRIPTION "ARM Cortex-M3 (ARMv7-M)")
microcrt_register_subtarget(ARM cortex-m4  DESCRIPTION "ARM Cortex-M4 with DSP and FPU (ARMv7E-M)")
microcrt_register_subtarget(ARM cortex-m7  DESCRIPTION "ARM Cortex-M7 high-performance microcontroller (ARMv7E-M)")
microcrt_register_subtarget(ARM cortex-a7  DESCRIPTION "ARM Cortex-A7 (ARMv7-A)")

if(NOT DEFINED MICROCRT_ARM_SUBTARGET AND NOT DEFINED MICROCRT_SUBTARGET)
  set(MICROCRT_ARM_SUBTARGET "cortex-m4" CACHE STRING "Selected ARM subtarget" FORCE)
endif()

# Subtarget configurations
microcrt_add_subtarget(ARM cortex-m0
  DEFINITIONS "__ARM_ARCH_6M__"
  COMPILE_OPTIONS "-mcpu=cortex-m0" "-mthumb"
)

microcrt_add_subtarget(ARM cortex-m3
  DEFINITIONS "__ARM_ARCH_7M__"
  COMPILE_OPTIONS "-mcpu=cortex-m3" "-mthumb"
)

microcrt_add_subtarget(ARM cortex-m4
  DEFINITIONS "__ARM_ARCH_7EM__" "__ARM_FEATURE_DSP"
  COMPILE_OPTIONS "-mcpu=cortex-m4" "-mthumb"
)

microcrt_add_subtarget(ARM cortex-m7
  DEFINITIONS "__ARM_ARCH_7EM__" "__ARM_FEATURE_DSP"
  COMPILE_OPTIONS "-mcpu=cortex-m7" "-mthumb"
)

microcrt_add_subtarget(ARM cortex-a7
  DEFINITIONS "__ARM_ARCH_7A__"
  COMPILE_OPTIONS "-mcpu=cortex-a7"
)

# 3. Standard ISA features & Vendor Custom Extensions (别家自定义指令集 / 厂商扩展)
microcrt_target_feature(ARM NEON "Enable ARM NEON SIMD vector instructions" OFF)
microcrt_target_feature(ARM THUMB2 "Enable ARM Thumb-2 instructions" ON)

if(MICROCRT_ARM_ENABLE_NEON)
  set(ARM_NEON_INCLUDES "")
  if(EXISTS "${PROJECT_SOURCE_DIR}/src/libm/machine/arm/neon")
    set(ARM_NEON_INCLUDES "${PROJECT_SOURCE_DIR}/src/libm/machine/arm/neon")
  endif()
  microcrt_add_target(ARM
    DEFINITIONS "__ARM_NEON__" "__MICROCRT_ARM_NEON__"
    INCLUDE_DIRECTORIES ${ARM_NEON_INCLUDES}
  )
endif()

# Vendor custom coprocessor / accelerator instructions
microcrt_register_vendor_extension(ARM Vendor CustomDSP
  DESCRIPTION "Custom ARM Coprocessor / DSP extension"
  DEFAULT OFF
)

microcrt_add_vendor_extension(ARM Vendor CustomDSP
  DEFINITIONS "__ARM_VENDOR_CUSTOM_DSP__"
)
