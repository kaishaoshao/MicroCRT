# ==============================================================================
# MicroCRT Target & Subtarget & Vendor Extension Framework
#
# Modeled after LLVM's target backend hierarchy:
# 1. Target Architecture (Top-level ISA: e.g. RISCV, ARM, AArch64, X86, or Custom ISA)
# 2. Subtarget (Target core / CPU family / profile: e.g. RV32IMC, RV64GC, Cortex-M4)
# 3. Vendor / Custom Extensions (Proprietary & 3rd-party instruction sets:
#    e.g. T-Head XuanTie, Andes, custom accelerators, out-of-tree ASIPs)
# ==============================================================================

# Standard built-in target instruction sets (extensible at configure time)
set(MICROCRT_DEFAULT_KNOWN_TARGETS
  AArch64
  ARM
  RISCV
  X86
  LoongArch
  Mips
)

if(NOT DEFINED MICROCRT_ALL_TARGETS)
  set(MICROCRT_ALL_TARGETS ${MICROCRT_DEFAULT_KNOWN_TARGETS}
    CACHE STRING "List of all supported MicroCRT target architectures")
endif()

# Registry properties to store target, subtarget, and vendor metadata
set_property(GLOBAL PROPERTY MICROCRT_TARGET_DIRS)
set_property(GLOBAL PROPERTY MICROCRT_TARGET_DESCRIPTIONS)
set_property(GLOBAL PROPERTY MICROCRT_ACTIVE_SUBTARGETS)
set_property(GLOBAL PROPERTY MICROCRT_ACTIVE_VENDOR_EXTENSIONS)

# Helper to convert arbitrary string into a safe C identifier and CMake variable component
function(microcrt_make_c_identifier IN_STR OUT_VAR)
  string(TOUPPER "${IN_STR}" upper_str)
  string(REGEX REPLACE "[^A-Z0-9_]" "_" safe_str "${upper_str}")
  set(${OUT_VAR} "${safe_str}" PARENT_SCOPE)
endfunction()

# ------------------------------------------------------------------------------
# 1. Target (ISA) Architecture Management
# ------------------------------------------------------------------------------

# Detect host / native processor architecture
function(microcrt_detect_native_target OUT_VAR)
  set(proc "${CMAKE_SYSTEM_PROCESSOR}")
  if(NOT proc)
    set(proc "${CMAKE_HOST_SYSTEM_PROCESSOR}")
  endif()
  string(TOLOWER "${proc}" proc_lower)

  if(proc_lower MATCHES "^(x86_64|amd64|x86|i[3-6]86)$")
    set(${OUT_VAR} "X86" PARENT_SCOPE)
  elseif(proc_lower MATCHES "^(aarch64|arm64)$")
    set(${OUT_VAR} "AArch64" PARENT_SCOPE)
  elseif(proc_lower MATCHES "^arm")
    set(${OUT_VAR} "ARM" PARENT_SCOPE)
  elseif(proc_lower MATCHES "^riscv")
    set(${OUT_VAR} "RISCV" PARENT_SCOPE)
  elseif(proc_lower MATCHES "^(loongarch|la64|la32)")
    set(${OUT_VAR} "LoongArch" PARENT_SCOPE)
  elseif(proc_lower MATCHES "^mips")
    set(${OUT_VAR} "Mips" PARENT_SCOPE)
  else()
    set(${OUT_VAR} "X86" PARENT_SCOPE)
  endif()
endfunction()

# Canonicalize target name (maps aliases like armv7 -> ARM, rv64 -> RISCV, etc.)
function(microcrt_canonicalize_target_name IN_NAME OUT_VAR)
  string(TOLOWER "${IN_NAME}" name_lower)

  if(name_lower MATCHES "^(arm.*)$")
    set(${OUT_VAR} "ARM" PARENT_SCOPE)
  elseif(name_lower MATCHES "^(aarch64|arm64)$")
    set(${OUT_VAR} "AArch64" PARENT_SCOPE)
  elseif(name_lower MATCHES "^(riscv.*|rv32.*|rv64.*)$")
    set(${OUT_VAR} "RISCV" PARENT_SCOPE)
  elseif(name_lower MATCHES "^(x86_64|amd64|x86|i[3-6]86)$")
    set(${OUT_VAR} "X86" PARENT_SCOPE)
  elseif(name_lower MATCHES "^(loongarch.*|la64|la32)$")
    set(${OUT_VAR} "LoongArch" PARENT_SCOPE)
  elseif(name_lower MATCHES "^(mips.*)$")
    set(${OUT_VAR} "Mips" PARENT_SCOPE)
  else()
    # Check registered targets (including custom targets)
    get_property(all_targets GLOBAL PROPERTY MICROCRT_ALL_TARGETS_LIST)
    if(NOT all_targets)
      set(all_targets ${MICROCRT_ALL_TARGETS})
    endif()

    foreach(t IN LISTS all_targets)
      string(TOLOWER "${t}" t_lower)
      if(name_lower STREQUAL t_lower)
        set(${OUT_VAR} "${t}" PARENT_SCOPE)
        return()
      endif()
    endforeach()

    # Pass through custom name preserving user's case
    set(${OUT_VAR} "${IN_NAME}" PARENT_SCOPE)
  endif()
endfunction()

# Register a Target ISA (built-in or custom / 3rd-party)
function(microcrt_register_target TARGET_NAME)
  cmake_parse_arguments(ARG "" "DIR;DESCRIPTION" "" ${ARGN})

  microcrt_canonicalize_target_name("${TARGET_NAME}" canonical_name)

  # Add to global targets list if not already present
  get_property(all_targets GLOBAL PROPERTY MICROCRT_ALL_TARGETS_LIST)
  if(NOT canonical_name IN_LIST all_targets)
    list(APPEND all_targets "${canonical_name}")
    set_property(GLOBAL PROPERTY MICROCRT_ALL_TARGETS_LIST "${all_targets}")
    set(MICROCRT_ALL_TARGETS "${all_targets}" CACHE STRING "List of all supported MicroCRT target architectures" FORCE)
  endif()

  if(ARG_DIR)
    set_property(GLOBAL PROPERTY MICROCRT_TARGET_DIR_${canonical_name} "${ARG_DIR}")
  endif()
  if(ARG_DESCRIPTION)
    set_property(GLOBAL PROPERTY MICROCRT_TARGET_DESC_${canonical_name} "${ARG_DESCRIPTION}")
  endif()
endfunction()

# Initialize targets configuration
macro(microcrt_initialize_targets)
  # Pre-register default known targets
  foreach(_t IN LISTS MICROCRT_DEFAULT_KNOWN_TARGETS)
    microcrt_register_target("${_t}")
  endforeach()

  # Discover extra out-of-tree / 3rd-party custom target directories
  if(DEFINED MICROCRT_EXTRA_TARGET_DIRS)
    foreach(_ext_entry IN LISTS MICROCRT_EXTRA_TARGET_DIRS)
      if(_ext_entry MATCHES "^([^=]+)=(.+)$")
        set(_t_name "${CMAKE_MATCH_1}")
        set(_t_dir "${CMAKE_MATCH_2}")
      else()
        set(_t_dir "${_ext_entry}")
        get_filename_component(_t_name "${_t_dir}" NAME)
      endif()
      if(EXISTS "${_t_dir}")
        microcrt_register_target("${_t_name}" DIR "${_t_dir}" DESCRIPTION "Custom external target")
        message(STATUS "Registered external custom target: ${_t_name} from ${_t_dir}")
      endif()
    endforeach()
  endif()

  microcrt_detect_native_target(_MICROCRT_NATIVE_TARGET)

  # Support MICROCRT_TARGET_ARCH shortcut as well
  if(DEFINED MICROCRT_TARGET_ARCH AND NOT DEFINED MICROCRT_TARGETS_TO_BUILD)
    set(MICROCRT_TARGETS_TO_BUILD "${MICROCRT_TARGET_ARCH}" CACHE STRING
      "Semicolon-separated list of targets to build, or 'all', or 'Native'." FORCE)
  else()
    set(MICROCRT_TARGETS_TO_BUILD "Native" CACHE STRING
      "Semicolon-separated list of targets to build, or 'all', or 'Native'. Supported: ${MICROCRT_ALL_TARGETS}")
  endif()

  set(_EXPANDED_TARGETS "")
  if("${MICROCRT_TARGETS_TO_BUILD}" STREQUAL "all")
    get_property(_EXPANDED_TARGETS GLOBAL PROPERTY MICROCRT_ALL_TARGETS_LIST)
    if(NOT _EXPANDED_TARGETS)
      set(_EXPANDED_TARGETS ${MICROCRT_ALL_TARGETS})
    endif()
  elseif("${MICROCRT_TARGETS_TO_BUILD}" STREQUAL "Native" OR "${MICROCRT_TARGETS_TO_BUILD}" STREQUAL "Host")
    set(_EXPANDED_TARGETS ${_MICROCRT_NATIVE_TARGET})
  else()
    foreach(_raw_t IN LISTS MICROCRT_TARGETS_TO_BUILD)
      microcrt_canonicalize_target_name("${_raw_t}" _canonical_t)
      list(APPEND _EXPANDED_TARGETS "${_canonical_t}")
      # If not in MICROCRT_ALL_TARGETS, auto-register as custom target
      if(NOT _canonical_t IN_LIST MICROCRT_ALL_TARGETS)
        microcrt_register_target("${_canonical_t}" DESCRIPTION "User-specified custom target")
      endif()
    endforeach()
  endif()
  list(REMOVE_DUPLICATES _EXPANDED_TARGETS)

  set(MICROCRT_TARGETS_BUILT "${_EXPANDED_TARGETS}" CACHE INTERNAL "Active targets configured to build")

  foreach(_t IN LISTS MICROCRT_TARGETS_BUILT)
    string(TOUPPER "${_t}" _t_upper)
    set(MICROCRT_TARGET_${_t_upper}_ENABLED TRUE)
  endforeach()
endmacro()

# Declare target-level configuration
# Syntax:
# microcrt_add_target(TargetName
#   [TARGET_LIB <lib_name>]
#   [SOURCES <src1> ...]
#   [DEFINITIONS <def1> ...]
#   [COMPILE_OPTIONS <opt1> ...]
#   [INCLUDE_DIRECTORIES <inc1> ...]
#   [OVERRIDE]
# )
function(microcrt_add_target TARGET_NAME)
  cmake_parse_arguments(ARG "OVERRIDE" "TARGET_LIB" "SOURCES;DEFINITIONS;COMPILE_OPTIONS;INCLUDE_DIRECTORIES" ${ARGN})

  microcrt_canonicalize_target_name("${TARGET_NAME}" canonical_name)
  microcrt_make_c_identifier("${canonical_name}" target_ident)

  if(NOT canonical_name IN_LIST MICROCRT_TARGETS_BUILT)
    return()
  endif()

  set(lib_target "microcrt_c")
  if(ARG_TARGET_LIB)
    set(lib_target "${ARG_TARGET_LIB}")
  endif()

  if(TARGET ${lib_target})
    target_compile_definitions(${lib_target} PRIVATE
      "__MICROCRT_TARGET_${target_ident}__"
      ${ARG_DEFINITIONS}
    )

    if(ARG_INCLUDE_DIRECTORIES)
      target_include_directories(${lib_target} PRIVATE ${ARG_INCLUDE_DIRECTORIES})
    endif()

    if(ARG_COMPILE_OPTIONS)
      microcrt_detect_native_target(native_arch)
      if("${canonical_name}" STREQUAL "${native_arch}" OR CMAKE_CROSSCOMPILING)
        target_compile_options(${lib_target} PRIVATE ${ARG_COMPILE_OPTIONS})
      endif()
    endif()

    if(ARG_SOURCES)
      if(ARG_OVERRIDE)
        microcrt_sources(${lib_target} OVERRIDE ${ARG_SOURCES})
      else()
        microcrt_sources(${lib_target} ${ARG_SOURCES})
      endif()
    endif()
  endif()
endfunction()

# ------------------------------------------------------------------------------
# 2. Subtarget (子目标) Management
# ------------------------------------------------------------------------------

# Register a Subtarget under a Target ISA
# Example: microcrt_register_subtarget(RISCV rv64gc DESCRIPTION "64-bit RISC-V with GC extensions")
# Example: microcrt_register_subtarget(ARM cortex-m4 DESCRIPTION "ARM Cortex-M4 microcontroller")
function(microcrt_register_subtarget TARGET_NAME SUBTARGET_NAME)
  cmake_parse_arguments(ARG "" "DESCRIPTION;DIR" "" ${ARGN})

  microcrt_canonicalize_target_name("${TARGET_NAME}" canonical_name)
  string(TOUPPER "${canonical_name}" target_upper)

  set(subtarget_var "MICROCRT_${target_upper}_AVAILABLE_SUBTARGETS")
  get_property(avail_subtargets GLOBAL PROPERTY ${subtarget_var})
  if(NOT "${SUBTARGET_NAME}" IN_LIST avail_subtargets)
    list(APPEND avail_subtargets "${SUBTARGET_NAME}")
    set_property(GLOBAL PROPERTY ${subtarget_var} "${avail_subtargets}")
  endif()

  if(ARG_DIR)
    set_property(GLOBAL PROPERTY MICROCRT_SUBTARGET_DIR_${target_upper}_${SUBTARGET_NAME} "${ARG_DIR}")
  endif()
endfunction()

# Select or check active subtarget for a target
function(microcrt_get_active_subtarget TARGET_NAME OUT_VAR)
  microcrt_canonicalize_target_name("${TARGET_NAME}" canonical_name)
  string(TOUPPER "${canonical_name}" target_upper)

  # Check specific subtarget option or global MICROCRT_SUBTARGET
  set(chosen "${MICROCRT_${target_upper}_SUBTARGET}")
  if(NOT chosen AND DEFINED MICROCRT_SUBTARGET)
    set(chosen "${MICROCRT_SUBTARGET}")
  endif()
  if(NOT chosen)
    set(chosen "generic")
  endif()

  set(${OUT_VAR} "${chosen}" PARENT_SCOPE)
endfunction()

# Declare Subtarget-specific implementation and flags
# Syntax:
# microcrt_add_subtarget(TargetName SubtargetName
#   [TARGET_LIB <lib_name>]
#   [SOURCES <src1> ...]
#   [DEFINITIONS <def1> ...]
#   [COMPILE_OPTIONS <opt1> ...]
#   [INCLUDE_DIRECTORIES <inc1> ...]
#   [OVERRIDE]
# )
function(microcrt_add_subtarget TARGET_NAME SUBTARGET_NAME)
  cmake_parse_arguments(ARG "OVERRIDE" "TARGET_LIB" "SOURCES;DEFINITIONS;COMPILE_OPTIONS;INCLUDE_DIRECTORIES" ${ARGN})

  microcrt_canonicalize_target_name("${TARGET_NAME}" canonical_name)
  string(TOUPPER "${canonical_name}" target_upper)
  microcrt_make_c_identifier("${canonical_name}" target_ident)
  microcrt_make_c_identifier("${SUBTARGET_NAME}" subtarget_ident)

  if(NOT canonical_name IN_LIST MICROCRT_TARGETS_BUILT)
    return()
  endif()

  # Expose subtarget option to CMake cache
  if(NOT DEFINED CACHE{MICROCRT_${target_upper}_SUBTARGET})
    set(MICROCRT_${target_upper}_SUBTARGET "${SUBTARGET_NAME}" CACHE STRING
      "Selected subtarget for ${canonical_name}")
  endif()

  microcrt_get_active_subtarget("${canonical_name}" active_subtarget)

  # If this subtarget is currently selected (or all are being compiled)
  if("${active_subtarget}" STREQUAL "${SUBTARGET_NAME}" OR "${active_subtarget}" STREQUAL "all")
    set(lib_target "microcrt_c")
    if(ARG_TARGET_LIB)
      set(lib_target "${ARG_TARGET_LIB}")
    endif()

    if(TARGET ${lib_target})
      target_compile_definitions(${lib_target} PRIVATE
        "__MICROCRT_SUBTARGET_${target_ident}_${subtarget_ident}__"
        ${ARG_DEFINITIONS}
      )

      if(ARG_INCLUDE_DIRECTORIES)
        target_include_directories(${lib_target} PRIVATE ${ARG_INCLUDE_DIRECTORIES})
      endif()

      if(ARG_COMPILE_OPTIONS)
        microcrt_detect_native_target(native_arch)
        if("${canonical_name}" STREQUAL "${native_arch}" OR CMAKE_CROSSCOMPILING)
          target_compile_options(${lib_target} PRIVATE ${ARG_COMPILE_OPTIONS})
        endif()
      endif()

      if(ARG_SOURCES)
        # Subtargets override target generic sources by default
        microcrt_sources(${lib_target} OVERRIDE ${ARG_SOURCES})
      endif()
    endif()

    # Track active subtarget for status report
    set_property(GLOBAL PROPERTY MICROCRT_ACTIVE_SUBTARGET_${target_upper} "${SUBTARGET_NAME}")
    message(STATUS "  [${canonical_name}] Configured Active Subtarget: ${SUBTARGET_NAME}")
  endif()
endfunction()

# ------------------------------------------------------------------------------
# 3. Vendor / Custom Instruction Sets (别家自定义指令集 / 厂商扩展)
# ------------------------------------------------------------------------------

# Register a vendor custom instruction set extension
# Example: microcrt_register_vendor_extension(RISCV Thead XTheadV "T-Head XuanTie 0.7.1 Vector instructions" OFF)
# Example: microcrt_register_vendor_extension(RISCV Andes CustomDSP "Andes Custom DSP instructions" OFF)
function(microcrt_register_vendor_extension TARGET_NAME VENDOR_NAME EXTENSION_NAME)
  cmake_parse_arguments(ARG "" "DESCRIPTION;DEFAULT;DIR" "" ${ARGN})

  microcrt_canonicalize_target_name("${TARGET_NAME}" canonical_name)
  string(TOUPPER "${canonical_name}" target_upper)
  string(TOUPPER "${VENDOR_NAME}" vendor_upper)
  string(TOUPPER "${EXTENSION_NAME}" ext_upper)
  microcrt_make_c_identifier("${vendor_upper}" vendor_ident)
  microcrt_make_c_identifier("${ext_upper}" ext_ident)

  set(opt_name "MICROCRT_${target_upper}_VENDOR_${vendor_ident}_${ext_ident}")
  set(default_val OFF)
  if(DEFINED ARG_DEFAULT)
    set(default_val ${ARG_DEFAULT})
  endif()

  set(desc "Enable ${canonical_name} ${VENDOR_NAME} ${EXTENSION_NAME} custom instruction set")
  if(ARG_DESCRIPTION)
    set(desc "${ARG_DESCRIPTION}")
  endif()

  option(${opt_name} "${desc}" ${default_val})

  if(ARG_DIR)
    set_property(GLOBAL PROPERTY MICROCRT_EXT_DIR_${target_upper}_${vendor_ident}_${ext_ident} "${ARG_DIR}")
  endif()
endfunction()

# Declare Vendor Custom Instruction implementation and overrides
# Syntax:
# microcrt_add_vendor_extension(TargetName VendorName ExtensionName
#   [TARGET_LIB <lib_name>]
#   [SOURCES <src1> ...]
#   [DEFINITIONS <def1> ...]
#   [COMPILE_OPTIONS <opt1> ...]
#   [INCLUDE_DIRECTORIES <inc1> ...]
#   [OVERRIDE]
# )
function(microcrt_add_vendor_extension TARGET_NAME VENDOR_NAME EXTENSION_NAME)
  cmake_parse_arguments(ARG "OVERRIDE" "TARGET_LIB" "SOURCES;DEFINITIONS;COMPILE_OPTIONS;INCLUDE_DIRECTORIES" ${ARGN})

  microcrt_canonicalize_target_name("${TARGET_NAME}" canonical_name)
  string(TOUPPER "${canonical_name}" target_upper)
  string(TOUPPER "${VENDOR_NAME}" vendor_upper)
  string(TOUPPER "${EXTENSION_NAME}" ext_upper)
  microcrt_make_c_identifier("${canonical_name}" target_ident)
  microcrt_make_c_identifier("${vendor_upper}" vendor_ident)
  microcrt_make_c_identifier("${ext_upper}" ext_ident)

  if(NOT canonical_name IN_LIST MICROCRT_TARGETS_BUILT)
    return()
  endif()

  set(opt_name "MICROCRT_${target_upper}_VENDOR_${vendor_ident}_${ext_ident}")
  if(${opt_name})
    set(lib_target "microcrt_c")
    if(ARG_TARGET_LIB)
      set(lib_target "${ARG_TARGET_LIB}")
    endif()

    if(TARGET ${lib_target})
      target_compile_definitions(${lib_target} PRIVATE
        "__MICROCRT_${target_ident}_VENDOR_${vendor_ident}_${ext_ident}__"
        ${ARG_DEFINITIONS}
      )

      if(ARG_INCLUDE_DIRECTORIES)
        target_include_directories(${lib_target} PRIVATE ${ARG_INCLUDE_DIRECTORIES})
      endif()

      if(ARG_COMPILE_OPTIONS)
        microcrt_detect_native_target(native_arch)
        if("${canonical_name}" STREQUAL "${native_arch}" OR CMAKE_CROSSCOMPILING)
          target_compile_options(${lib_target} PRIVATE ${ARG_COMPILE_OPTIONS})
        endif()
      endif()

      if(ARG_SOURCES)
        # Vendor custom instructions have highest optimization priority and override subtarget/generic
        microcrt_sources(${lib_target} OVERRIDE ${ARG_SOURCES})
      endif()
    endif()

    # Track active vendor extension for summary
    get_property(active_exts GLOBAL PROPERTY MICROCRT_ACTIVE_VENDOR_EXTENSIONS_${target_upper})
    list(APPEND active_exts "${VENDOR_NAME}::${EXTENSION_NAME}")
    set_property(GLOBAL PROPERTY MICROCRT_ACTIVE_VENDOR_EXTENSIONS_${target_upper} "${active_exts}")
    message(STATUS "  [${canonical_name}] Enabled Custom Instruction Set: ${VENDOR_NAME}::${EXTENSION_NAME}")
  endif()
endfunction()

# Generic target feature flag helper (e.g. standard ISA extensions like NEON, AVX, RVV)
function(microcrt_target_feature TARGET_NAME FEATURE_NAME DESCRIPTION DEFAULT_VAL)
  microcrt_canonicalize_target_name("${TARGET_NAME}" canonical_name)
  string(TOUPPER "${canonical_name}" target_upper)
  microcrt_make_c_identifier("${FEATURE_NAME}" feature_ident)
  set(option_name "MICROCRT_${target_upper}_ENABLE_${feature_ident}")

  if(canonical_name IN_LIST MICROCRT_TARGETS_BUILT)
    option(${option_name} "${DESCRIPTION}" ${DEFAULT_VAL})
    if(${option_name})
      message(STATUS "  [${canonical_name}] ISA Feature enabled: ${FEATURE_NAME}")
    endif()
  endif()
endfunction()

# ------------------------------------------------------------------------------
# 4. Target Backend Loader & Recurser
# ------------------------------------------------------------------------------

# Recursively configure and load all enabled target backends
macro(microcrt_add_target_subdirectories DIR)
  foreach(_t IN LISTS MICROCRT_TARGETS_BUILT)
    # 1. Check custom registered directory
    get_property(_custom_dir GLOBAL PROPERTY MICROCRT_TARGET_DIR_${_t})
    set(_target_dir "")

    if(_custom_dir AND EXISTS "${_custom_dir}/CMakeLists.txt")
      set(_target_dir "${_custom_dir}")
    elseif(EXISTS "${DIR}/${_t}/CMakeLists.txt")
      set(_target_dir "${DIR}/${_t}")
    endif()

    if(_target_dir)
      message(STATUS "Configuring Target Backend: ${_t} (from ${_target_dir})")
      add_subdirectory("${_target_dir}" "${CMAKE_CURRENT_BINARY_DIR}/target_${_t}")
    else()
      message(STATUS "Target Backend ${_t} has no CMakeLists.txt in ${DIR}/${_t}, skipping")
    endif()
  endforeach()
endmacro()

# ------------------------------------------------------------------------------
# 5. Configuration Summary (LLVM-style Banner)
# ------------------------------------------------------------------------------

function(microcrt_print_configuration_summary)
  message(STATUS "=======================================================")
  message(STATUS "MicroCRT Instruction Set & Target Configuration Summary")
  message(STATUS "=======================================================")
  message(STATUS "  Version:                 ${PROJECT_VERSION}")
  message(STATUS "  C Compiler:              ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
  message(STATUS "  Build Type:              ${CMAKE_BUILD_TYPE}")
  message(STATUS "  Supported Targets:       ${MICROCRT_ALL_TARGETS}")
  message(STATUS "  Configured Targets:      ${MICROCRT_TARGETS_BUILT}")

  foreach(_t IN LISTS MICROCRT_TARGETS_BUILT)
    string(TOUPPER "${_t}" _t_upper)
    get_property(_active_subtarget GLOBAL PROPERTY MICROCRT_ACTIVE_SUBTARGET_${_t_upper})
    if(NOT _active_subtarget)
      set(_active_subtarget "generic")
    endif()

    get_property(_active_exts GLOBAL PROPERTY MICROCRT_ACTIVE_VENDOR_EXTENSIONS_${_t_upper})
    if(NOT _active_exts)
      set(_active_exts "none")
    endif()

    message(STATUS "  Target ${_t}:")
    message(STATUS "    - Subtarget:           ${_active_subtarget}")
    message(STATUS "    - Custom Extensions:   ${_active_exts}")
  endforeach()

  message(STATUS "  Build Legacy Stdio:      ${MICROCRT_BUILD_STDIO_LEGACY}")
  message(STATUS "  Build Tests:             ${MICROCRT_BUILD_TESTS}")
  message(STATUS "  Printf Auto Flush:       ${MICROCRT_PRINTF_ENABLE_AUTO_FLUSH}")
  message(STATUS "=======================================================")
endfunction()
