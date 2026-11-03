# RISC-V Target Architecture Backend

# 1. Base Target definition
microcrt_add_target(RISCV
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
)

# 2. Standard Subtargets (rv32imc, rv64gc, rv64gcv)
microcrt_register_subtarget(RISCV rv32imc DESCRIPTION "32-bit RISC-V with I, M, C extensions")
microcrt_register_subtarget(RISCV rv64gc  DESCRIPTION "64-bit RISC-V standard general-purpose profile (IMAFD_Zicsr_Zifencei_C)")
microcrt_register_subtarget(RISCV rv64gcv DESCRIPTION "64-bit RISC-V with Standard Vector extension (RVV)")

# Default subtarget if none explicitly requested
if(NOT DEFINED MICROCRT_RISCV_SUBTARGET AND NOT DEFINED MICROCRT_SUBTARGET)
  set(MICROCRT_RISCV_SUBTARGET "rv64gc" CACHE STRING "Selected RISC-V subtarget" FORCE)
endif()

# Subtarget configurations
microcrt_add_subtarget(RISCV rv32imc
  DEFINITIONS "__riscv_xlen=32" "__riscv_m" "__riscv_c"
)

microcrt_add_subtarget(RISCV rv64gc
  DEFINITIONS "__riscv_xlen=64" "__riscv_flen=64" "__riscv_c"
)

microcrt_add_subtarget(RISCV rv64gcv
  DEFINITIONS "__riscv_xlen=64" "__riscv_flen=64" "__riscv_c" "__riscv_vector"
)

# 3. Vendor Custom Instruction Sets (别家自定义指令集 / 厂商扩展)
# e.g., T-Head XuanTie Custom Instructions
microcrt_register_vendor_extension(RISCV Thead XTheadV
  DESCRIPTION "T-Head XuanTie Vector 0.7.1 custom instruction set"
  DEFAULT OFF
)

microcrt_register_vendor_extension(RISCV Thead XTheadMemPair
  DESCRIPTION "T-Head XuanTie Load/Store Pair custom instruction set"
  DEFAULT OFF
)

# e.g., Andes Custom Extensions
microcrt_register_vendor_extension(RISCV Andes CustomDSP
  DESCRIPTION "Andes Technology Custom DSP instruction set"
  DEFAULT OFF
)

# Generic Custom Vendor Extension hook
microcrt_register_vendor_extension(RISCV Vendor CustomISA
  DESCRIPTION "User-defined custom proprietary RISC-V instruction set"
  DEFAULT OFF
)

# Apply Vendor Extension configurations
microcrt_add_vendor_extension(RISCV Thead XTheadV
  DEFINITIONS "__XTHEAD_VECTOR__"
)

microcrt_add_vendor_extension(RISCV Thead XTheadMemPair
  DEFINITIONS "__XTHEAD_MEMPAIR__"
)

microcrt_add_vendor_extension(RISCV Andes CustomDSP
  DEFINITIONS "__ANDES_CUSTOM_DSP__"
)

microcrt_add_vendor_extension(RISCV Vendor CustomISA
  DEFINITIONS "__RISCV_CUSTOM_VENDOR_ISA__"
)

# Include optional vendor subdirectories
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/CMakeLists.txt")
  add_subdirectory(vendor)
endif()
