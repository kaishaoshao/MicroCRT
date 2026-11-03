# 自定义/第三方独立指令集架构接入指南

MicroCRT 采用类似 LLVM Target 后端的分层模型，支持第三方厂商接入完全自研的指令集架构（如自研 DSP、NPU、专用 ASIP 等）。

## 架构层级

```text
1. Target (顶级指令集，如 CustomISA)
    └── 2. Subtarget (子目标，如 core-v1, core-v2)
          └── 3. Vendor Extensions (厂商自定义指令集/专用指令扩展)
```

## 接入方式

### 方式 1：树内直接放置
直接在 `src/target/` 目录下创建新架构目录，如 `src/target/MyNPU/CMakeLists.txt`。

### 方式 2：树外外部目录引入（Out-of-Tree）
第三方代码仓库完全独立于 MicroCRT 源码树，通过 CMake 参数接入：
```bash
cmake -B build \
  -DMICROCRT_TARGETS_TO_BUILD="MyNPU" \
  -DMICROCRT_EXTRA_TARGET_DIRS="/opt/vendor/mynpu-microcrt-backend"
```

## 编写 `CMakeLists.txt`

```cmake
# 1. 注册与声明 Target
microcrt_add_target(MyNPU
  DEFINITIONS "__MY_NPU__"
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
  SOURCES src/crt0.S src/memcpy.S
)

# 2. 注册与声明子目标 (Subtarget)
microcrt_register_subtarget(MyNPU v1 DESCRIPTION "MyNPU Generation 1")
microcrt_register_subtarget(MyNPU v2 DESCRIPTION "MyNPU Generation 2")

microcrt_add_subtarget(MyNPU v1
  DEFINITIONS "__MY_NPU_V1__"
  SOURCES src/v1/math.c
)

# 3. 注册与声明厂商扩展指令集 (Vendor Custom Extension)
microcrt_register_vendor_extension(MyNPU Vendor MatrixExt
  DESCRIPTION "Matrix acceleration instructions"
  DEFAULT ON
)

microcrt_add_vendor_extension(MyNPU Vendor MatrixExt
  DEFINITIONS "__MY_NPU_MATRIX_EXT__"
  SOURCES src/vendor/matrix_ops.S
)
```
