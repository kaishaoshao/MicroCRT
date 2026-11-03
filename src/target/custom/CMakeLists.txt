# Custom / 3rd-Party Instruction Set Architecture Backend Example
#
# This demonstrates how any vendor can plug in a completely custom,
# proprietary instruction set into MicroCRT as a first-class Target.
#
# Usage:
#   cmake -B build -DMICROCRT_TARGETS_TO_BUILD="CustomISA" \
#                  -DMICROCRT_EXTRA_TARGET_DIRS="/path/to/custom_target"
# Or place this directory under src/target/<TargetName>

set(CUSTOM_ISA_NAME "CustomISA")

# 1. Register and define the custom Target
microcrt_add_target(${CUSTOM_ISA_NAME}
  DEFINITIONS "__CUSTOM_ISA__" "__MICROCRT_CUSTOM_BACKEND__"
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
)

# 2. Register Custom Subtargets (cores, profiles)
microcrt_register_subtarget(${CUSTOM_ISA_NAME} core-v1 DESCRIPTION "Version 1 Core Profile")
microcrt_register_subtarget(${CUSTOM_ISA_NAME} core-v2 DESCRIPTION "Version 2 Core Profile with Vector Accelerator")

if(NOT DEFINED MICROCRT_CUSTOMISA_SUBTARGET AND NOT DEFINED MICROCRT_SUBTARGET)
  set(MICROCRT_CUSTOMISA_SUBTARGET "core-v1" CACHE STRING "Selected CustomISA subtarget" FORCE)
endif()

microcrt_add_subtarget(${CUSTOM_ISA_NAME} core-v1
  DEFINITIONS "__CUSTOM_CORE_V1__"
)

microcrt_add_subtarget(${CUSTOM_ISA_NAME} core-v2
  DEFINITIONS "__CUSTOM_CORE_V2__" "__CUSTOM_VECTOR_ACCEL__"
)

# 3. Register Custom Vendor Extensions
microcrt_register_vendor_extension(${CUSTOM_ISA_NAME} Vendor CustomMatrix
  DESCRIPTION "Proprietary AI Matrix Multiply instruction set"
  DEFAULT OFF
)

microcrt_add_vendor_extension(${CUSTOM_ISA_NAME} Vendor CustomMatrix
  DEFINITIONS "__CUSTOM_VENDOR_MATRIX_ACCEL__"
)
