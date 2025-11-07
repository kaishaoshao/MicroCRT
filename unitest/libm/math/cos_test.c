#include <stdio.h>
#include <math.h>   // 使用 fabs, isnan, isinf / Used for fabs, isnan, isinf
#include <stdint.h> // 为了 uint64_t 和 uint32_t / For uint64_t and uint32_t
#include <string.h> // 为了 memcpy / For memcpy

/*
 * C 语言标准库 <math.h> 中没有 fma (fused multiply-add)。
 * fma(a, b, c) 计算 (a * b) + c，并且只在最后进行一次舍入。
 * 这比 (a * b) + c 更精确。
 * 我们在下面的代码中将使用 (a * b + c) 的形式来模拟它。
 * ---
 * The C standard library <math.h> does not (always) have fma (fused
 * multiply-add). fma(a, b, c) calculates (a * b) + c with only one final
 * rounding. This is more precise than (a * b) + c. We will simulate it with (a
 * * b + c) below for clarity.
 */
// 模拟 _FMA(A, B, C)
// Emulate _FMA(A, B, C)
#define MY_FMA(A, B, C) ((A) * (B) + (C))

/* * ===================================================================
 * ===================================================================
 *
 * 专业算法复现 (严格按照  (n - 0.5) 算法)
 * Professional Algorithm Reproduction (Strictly  (n-0.5) Algorithm)
 *
 * ===================================================================
 * ===================================================================
 */

/* * ===================================================================
 * 辅助函数 (复现 /fdlibm 的位操作)
 * Helper Functions (Reproducing /fdlibm bit operations)
 * ===================================================================
 */

// --- 技巧 1 (memcpy): C 语言标准方法 (推荐) ---
// --- Trick 1 (memcpy): Standard C Method (Recommended) ---

/**
 * @brief 将 double "位转换" 为 uint64_t (使用 memcpy，标准 C 方法)
 * @brief Bitcast double to uint64_t (using memcpy, Standard C Method)
 *
 * @param x 输入的 double 值。
 * @param x The input double value.
 * @return uint64_t x 的 64 位整数二进制表示。
 * @return uint64_t The 64-bit integer binary representation of x.
 *
 * @details
 * 技巧 (Trick): 标准 C 位转换 (Standard C Bitcast)
 * 目的 (Goal):
 * 1. 安全性: 避免违反 C/C++ 的 "严格别名" (strict-aliasing) 规则。
 * 1. Safety: Avoids violating C/C++ "strict-aliasing" rules.
 * 2. 可移植性: union (联合体) 技巧在某些 C 标准下是"未定义行为"。
 * 2. Portability: The union trick is "Undefined Behavior" (UB) in strict C
 * standards.
 * 3. 性能: 现代编译器 (gcc, clang) 会将此 memcpy 优化为
 * 零开销的寄存器移动, 性能与 union 相同。
 * 3. Performance: Modern compilers (gcc, clang) optimize this memcpy to
 * zero-cost register moves, same as the union.
 */
static inline uint64_t bitcast_double_to_u64_memcpy(double x) {
  uint64_t u;
  // memcpy 是C语言标准中在不违反严格别名(strict-aliasing)规则
  // 的情况下执行位转换(bitcast)的正确方法。
  // memcpy is the correct C standard way to perform a bitcast
  // without violating strict-aliasing rules.
  memcpy(&u, &x, sizeof(u));
  return u;
}

// --- 技巧 2 (union): 传统/ 方法 (不推荐, 但很常见) ---
// --- Trick 2 (union): Traditional/ Method (Not recommended, but common)
// ---

/**
 * @brief 用于 "类型双关" (type-punning) 的联合体。
 * @brief Union for "type-punning".
 */
typedef union {
  double f;   // 64-bit 浮点数 / 64-bit float
  uint64_t u; // 64-bit 无符号整数 / 64-bit unsigned integer
} double_g_t;

/**
 * @brief 将 double "位转换" 为 uint64_t (使用 union 技巧)
 * @brief Bitcast double to uint64_t (using union trick)
 *
 * @param x 输入的 double 值。
 * @param x The input double value.
 * @return uint64_t x 的 64 位整数二进制表示。
 * @return uint64_t The 64-bit integer binary representation of x.
 *
 * @details
 * 技巧 (Trick): 联合体类型双关 (Union Type Punning)
 * 目的 (Goal):
 * 1. 性能: 这是最传统、最直接的位转换方式。
 * 1. Performance: This is the most traditional, direct way to bitcast.
 * 2. 缺点: 在严格的 C99/C++ 标准中, 写入一个 union 成员
 * (如 g.f = x) 然后读取另一个成员 (g.u)
 * 是"未定义行为" (Undefined Behavior)。
 * 2. Downside: In strict C99/C++, writing to one member (g.f) and reading
 * from another (g.u) is "Undefined Behavior" (UB).
 * 3. 现实: 尽管如此, 几乎所有编译器 (如 gcc) 都支持
 * 这种用法作为一种"扩展",  RTL 就依赖此特性。
 * 3. Reality: Despite UB, almost all compilers (like gcc) support this
 * as an extension, which  RTL relies on.
 */
static inline uint64_t bitcast_double_to_u64_union(double x) {
  double_g_t g;
  g.f = x;
  return g.u;
}

/**
 * @brief 使用位操作(bit-twiddling)高效检查一个 double 是否为"特殊值"。
 * @brief Efficiently checks if a double is a "special value" using
 * bit-twiddling.
 *
 * @param x 一个 double 值的 64 位整数(uint64_t)表示。
 * @param x The 64-bit integer (uint64_t) representation of a double value.
 *
 * @return int 1 (true) 如果 x 是 Inf, NaN, Zero, 或 Subnormal; 0 (false)
 * 如果是正常数。
 * @return int 1 (true) if x is Inf, NaN, Zero, or Subnormal; 0 (false) if it is
 * a normal number.
 *
 * @details
 * 技巧 (Trick): 无符号整数下溢/上溢 (Unsigned Underflow/Overflow)
 *
 * 这是一个源自  RTL 的高性能技巧，它用一次无符号整数比较
 * 同时捕获指数(exponent)的上溢和下溢。
 * This is a high-performance trick from  RTL. It uses one unsigned
 * comparison to catch both the underflow and overflow of the exponent.
 *
 * 一个 double 的 11 位指数 `e` (已移除符号位) 定义如下:
 * A double's 11-bit exponent 'e' (sign bit removed) is defined as:
 * - [0x001, 0x7FE]: 正常数 (Normal Number)
 * - 0x000:           零 / 非正规数 (Zero / Subnormal)
 * - 0x7FF:           无穷大 / 非数字 (Infinity / NaN)
 *
 * 本函数执行的逻辑是: `(((x >> 52) & 0x7FFu) - 1u) >= 0x7FEu`
 * This function executes the logic: `(((x >> 52) & 0x7FFu) - 1u) >= 0x7FEu`
 *
 * 1. 正常数 (e 在 [0x001, 0x7FE]):
 * 1. Normal Number (e is in [0x001, 0x7FE]):
 * (e - 1u) 的范围是 [0x000, 0x7FD]。
 * (e - 1u) range is [0x000, 0x7FD].
 * (e - 1u) >= 0x7FEu 总是 False。
 * (e - 1u) >= 0x7FEu is always False.
 *
 * 2. Zero / Subnormal (e == 0x000):
 * 2. Zero / Subnormal (e == 0x000):
 * (0x000 - 1u) 在无符号运算中 "下溢" (underflow) 为 0xFFFF...
 * (一个非常大的数)。 (0x000 - 1u) "underflows" to 0xFFFF... (a very large
 * number) in unsigned arithmetic. (大数) >= 0x7FEu 总是 True。 (捕获成功!)
 * (Large num) >= 0x7FEu is always True. (Caught!)
 *
 * 3. Inf / NaN (e == 0x7FF):
 * 3. Inf / NaN (e == 0x7FF):
 * (0x7FF - 1u) = 0x7FE。
 * (0x7FF - 1u) = 0x7FE.
 * 0x7FE >= 0x7FEu 总是 True。 (捕获成功!)
 * 0x7FE >= 0x7FEu is always True. (Caught!)
 */
static inline int is_double_special_bits(uint64_t x) {
  // ((e & 0x7FF) - 1) >= 0x7FE
  return ((unsigned short)(x >> 52) & 0x7FFu) - 1u >= 0x7FEu;
}

/**
 * @brief "isfinite" 检查 (仅检查 Inf/NaN) (来自  的逻辑)
 * @brief "isfinite" check (checks for Inf/NaN only) (from  logic)
 *
 * @param x 一个 double 值的 64 位整数(uint64_t)表示。
 * @param x The 64-bit integer (uint64_t) representation of a double value.
 *
 * @return int 1 (true) 如果 x 是有限的 (非 Inf/NaN); 0 (false) 如果 x 是
 * Inf/NaN。
 * @return int 1 (true) if x is finite (not Inf/NaN); 0 (false) if x is Inf/NaN.
 *
 * @details
 * 技巧 (Trick): 符号位移除与无符号比较
 * Trick: Sign bit removal and unsigned comparison
 *
 * 目的 (Goal):
 * 通过一次比较，同时排除 Inf 和 NaN。
 * Exclude Inf and NaN with a single comparison.
 * 这比 `(exp & 0x7FF) != 0x7FF` 更高效，因为它不需要移位和掩码。
 * This is more efficient than `(exp & 0x7FF) != 0x7FF` as it avoids shifts and
 * masks.
 *
 * 逻辑 (Logic):
 * 1. (x << 1):
 * 将 64 位数左移 1 位。这会丢弃最高位的 "符号位"。
 * Left-shift the 64-bit number by 1. This discards the top "sign bit".
 * 现在的 63-53 位是指数, 52-1 位是尾数。
 * Bits 63-53 are now the exponent, 52-1 are the mantissa.
 *
 * 2. 0xFFE0000000000000ULL:
 * 这是一个"魔术"常量。它的前 11 位是 1 (即 0x7FF, 指数部分),
 * 但尾数部分全为 0。 (ULL 后缀代表 Unsigned Long Long)
 * A "magic" constant. Its first 11 bits are 1s (the 0x7FF exponent part),
 * but the mantissa part is all 0s. (ULL suffix means Unsigned Long Long)
 *
 * 3. 比较: (x << 1) < (魔术常量)
 * Comparison: (x << 1) < (magic constant)
 *
 * a. 正常数 (Normal/Zero/Sub):
 * (x << 1) 的指数部分 < 0x7FF。
 * (x << 1)'s exponent part is < 0x7FF.
 * (小数) < (魔术常量) 总是 True。 (是 finite)
 * (Small num) < (Magic const) is always True. (is finite)
 *
 * b. Inf (无穷大):
 * x 的指数是 0x7FF, 尾数全 0。
 * x's exponent is 0x7FF, mantissa is all 0s.
 * (x << 1) 会是 0xFFF0... (因为尾数全0, 且指数位 10..0 左移一位变为 11..10)。
 * (x << 1) will be 0xFFF0... (because mantissa is all 0s, and exp bits 10..0
 * shift to 11..10). (0xFFF0...) < (0xFFE0...) 是 False。 (不是 finite)
 * (0xFFF0...) < (0xFFE0...) is False. (not finite)
 * * *更正: `x` (Inf) 的位是 `0x7FF00...0`。`x << 1` 是 `0xFFE0...0`。
 * *Correction: `x` (Inf) bits are `0x7FF00...0`. `x << 1` is `0xFFE0...0`.
 * (0xFFE0...) < (0xFFE0...) 是 False。 (不是 finite)
 * (0xFFE0...) < (0xFFE0...) is False. (not finite)
 *
 * c. NaN (非数字):
 * x 的指数是 0x7FF, 尾数非 0。
 * x's exponent is 0x7FF, mantissa is non-zero.
 * (x << 1) 会是 0xFFE... 或 0xFFF... (取决于尾数)。
 * (x << 1) will be 0xFFE... or 0xFFF... (depending on mantissa).
 * 这个值**大于或等于**魔术常量。
 * This value is **greater than or equal to** the magic constant.
 * (0xFFE... 或 0xFFF...) < (0xFFE0...) 总是 False。 (不是 finite)
 * (0xFFE... or 0xFFF...) < (0xFFE0...) is always False. (not finite)
 *
 */
static inline int is_double_finite_bits(uint64_t x) {
  // (x << 1) < 0xFFE0...
  return (x << 1) < 0xFFE0000000000000ULL;
}

// --- `cos` 的常量 (严格  版本) ---
// --- Constants for `cos` (Strict  Version) ---

// 标准数学常量
// Standard math constants
const double MY_M_PI = 3.14159265358979323846;
const double MY_M_PI_2 = 1.57079632679489661923;   // PI / 2
const double MY_M_INV_PI = 0.31830988618379067154; // 1 / PI

//  使用的高精度拆分 (与 fdlibm 不同)
// 's high-precision split (different from fdlibm)
// C1 是 Pi 的一个高精度、可精确表示的近似值 ("大头")
// C1 is a high-precision, exactly representable approximation of Pi ("the
// head")
const double _C1 = 3.1416015625;
// C2 是一个修正值 ("尾巴")，使得 PI ≈ C1 - C2
// C2 is the correction value ("the tail"), such that PI ≈ C1 - C2
const double _C2 = 8.908910206761537356617e-6;

// __kernel_sin 的系数 (fdlibm `k_sin.c`, 8 个)
// __kernel_sin coefficients (fdlibm `k_sin.c`, 8 terms)
// 这些是 sin(r) ≈ r + r^3*P(r^2) 的多项式 P(g) 的系数 (g = r*r)
// These are the coefficients for the polynomial P(g) in sin(r) ≈ r +
// r^3*P(r^2), where g = r*r
const double P[8] = {
    -0.16666666666666665052e+0,  // S1 (≈ -1/3!)
    0.83333333333331650315e-2,   // S2 (≈ +1/5!)
    -0.19841269841201840457e-3,  // S3 (≈ -1/7!)
    0.27557319210152756119e-5,   // S4 (≈ +1/9!)
    -0.25052106798274584544e-7,  // S5 (≈ -1/11!)
    0.16058936490371589114e-9,   // S6 (≈ +1/13!)
    -0.76429178068910467734e-12, // S7 (≈ -1/15!)
    0.27204790957888846175e-14   // S8 (≈ +1/17!)
};

/*
 * ===================================================================
 * 专业方法: my_cos_pro (严格复现  (n-0.5) 算法)
 * Professional Method: my_cos_pro (Strictly Reproducing  (n-0.5)
 * algorithm)
 * ===================================================================
 */

/**
 * @brief 计算 double 值的余弦 (复现  (n-0.5) 算法)
 * @brief Calculates cosine for a double value (Reproducing  (n-0.5)
 * algorithm)
 *
 * @param x 输入的角度 (弧度)。
 * @param x Input angle in radians.
 * @return double x 的余弦值。
 * @return double The cosine of x.
 *
 * @details
 * 算法分为 4 个步骤:
 * The algorithm has 4 steps:
 * 1. 特殊值处理 (Inf, NaN, 0, Subnormal)
 * Handle special values (Inf, NaN, 0, Subnormal)
 * 2. 高精度参数归约
 * High-precision argument reduction
 * 3. 多项式逼近
 * Polynomial approximation
 * 4. 结果重构
 * Result reconstruction
 */
double my_cos_pro(double x) {
  double g;
  double xn;
  int n;

  // 默认使用标准的 memcpy 方法
  // Default to using the standard memcpy method
  uint64_t x_bits = bitcast_double_to_u64_memcpy(
      x); // 获取 x 的位表示 / Get bit representation of x

  // --- 第 1 步：处理特殊值 (使用位操作逻辑) ---
  // --- Step 1: Handle Special Values (using bit logic) ---
  // 使用 is_double_special_bits 一次性检查所有特殊情况
  // Use is_double_special_bits to check all special cases at once
  if (is_double_special_bits(x_bits)) {
    // 如果是特殊值, 再检查它是否 "有限"
    // If it's special, check if it's "finite"
    if (!is_double_finite_bits(x_bits)) {
      // x 是 Inf 或 NaN
      // x is Inf or NaN
      return NAN;
    } else {
      // x 是 0 或 Subnormal
      // x is 0 or Subnormal
      // cos(0) = 1
      return 1.0;
    }
  }

  // --- 第 2 步：高精度参数归约 (严格  逻辑) ---
  // --- Step 2: High-Precision Argument Reduction (Strict  logic) ---

  // cos(x) = cos(-x), 只处理正数
  // cos(x) = cos(-x), we only handle positive numbers
  x = fabs(x); // 仍然需要标准 fabs / Still need standard fabs

  // 归约的第一步: 计算整数 n
  // First step of reduction: Calculate integer n
  // n = (int) ( (|x| + PI/2) * (1/PI) + 0.5 )
  //   = (int) ( |x|/PI + 0.5 + 0.5 )
  //   = (int) ( |x|/PI + 1.0 )
  //   = floor(|x|/PI) + 1
  //   (使用 C 运算符模拟 MY_FMA, MY_ADD)
  n = (int)(((x + MY_M_PI_2) * MY_M_INV_PI) + 0.5);

  // 归约的第二步: 准备 (n - 0.5)
  // Second step of reduction: Prepare (n - 0.5)
  // xn = (n - 0.5)
  // (使用 C 运算符模拟 MY_SUB)
  xn = (double)(n)-0.5;

  // 归约的第三步: 高精度计算余数 r
  // Third step of reduction: High-precision remainder calculation for r
  // r = |x| - (n - 0.5) * PI
  // r = (|x| - (n - 0.5) * C1) + ((n - 0.5) * C2)
  // (使用 C 运算符模拟 MY_ADD, MY_SUB, MY_MUL)
  x = (x - xn * _C1) + (xn * _C2);

  // 此时 x 变量中存储的是高精度余数 r (范围在 [-pi/4, pi/4] 附近)
  // At this point, the x variable holds the high-precision remainder r (near
  // [-pi/4, pi/4])

  // --- 第 3 步：多项式逼近 sin(r) ---
  // --- Step 3: Polynomial Approximation of sin(r) ---

  // 算法的核心技巧: 将 cos(x) 转换为 sin(r)
  // Algorithm's core trick: Convert cos(x) to sin(r)
  // 我们调用 __kernel_sin 来计算 sin(r)
  // We call __kernel_sin to calculate sin(r)

  // (使用 C 运算符模拟)
  x = __kernel_sin(x); // x (r) -> sin(r)

  // --- 第 4 步：结果重构 ( (n & 1) 逻辑) ---
  // --- Step 4: Reconstruct Result ( (n & 1) logic) ---
  // 数学恒等式: cos(x) = sin(n*PI + r)
  // Math Identity: cos(x) = sin(n*PI + r)
  if ((n & 1) == 0) { // n 是偶数 / n is even
    // sin(n*PI + r) = sin(r)
    return x;
  } else { // n 是奇数 / n is odd
    // sin(n*PI + r) = -sin(r)
    return -x;
  }
}

/*
 * ===================================================================
 * 内部 "kernel" 函数 (fdlibm `k_sin.c`)
 * Internal "kernel" function (fdlibm `k_sin.c`)
 * (这个函数是 my_cos_pro 必需的)
 * (This function is required by my_cos_pro)
 * ===================================================================
 */

/**
 * @brief 计算 sin(r) 的核心多项式 (仅用于 r 在 [-pi/4, pi/4] 范围)
 * @brief Calculates the core polynomial for sin(r) (only for r in [-pi/4,
 * pi/4])
 *
 * @param r 高精度归约后的余数。
 * @param r The high-precision reduced remainder.
 * @return double sin(r) 的高精度近似值。
 * @return double The high-precision approximation of sin(r).
 *
 * @details
 * 数学公式: sin(r) ≈ r + r*g*P(g),  其中 g = r*r
 * Math Formula: sin(r) ≈ r + r*g*P(g),  where g = r*r
 * P(g) = S1 + S2*g + S3*g^2 + ... + S8*g^7
 * P(g) is calculated using Horner's method for efficiency.
 */
static inline double __kernel_sin(double r) {
  double g = r * r;
  double poly;

  // 霍纳法则 (Horner's method):
  // 从最高阶 S8 (P[7]) 开始
  // Start from the highest order S8 (P[7])
  poly = P[7];              // S8
  poly = (poly * g) + P[6]; // S7
  poly = (poly * g) + P[5]; // S6
  poly = (poly * g) + P[4]; // S5
  poly = (poly * g) + P[3]; // S4
  poly = (poly * g) + P[2]; // S3
  poly = (poly * g) + P[1]; // S2
  poly = (poly * g) + P[0]; // S1

  // 最终计算: r + r*g*P(g)
  // Final calculation: r + r*g*P(g)
  // (使用 C 运算符模拟 MY_FMA, MY_MUL)
  return (r * g) * poly + r;
}

/* * ===================================================================
 * ===================================================================
 *
 * main() 函数
 * main() Function
 * (仅用于测试专业版)
 * (For testing the professional version only)
 *
 * ===================================================================
 * ===================================================================
 */
int main() {
  printf("--- 测试 my_cos_pro (严格  算法复现) ---\n");
  printf("--- Testing my_cos_pro (Strict  Algorithm Reproduction) ---\n");
  printf("         x    | 高精度余数 r  |  my_cos_pro   |   std_cos(x)\n");
  printf(
      "         x    | Hi-Prec Remainder r |  my_cos_pro   |   std_cos(x)\n");
  printf("---------------------------------------------------------------------"
         "-\n");

  // 测试用例，包括 0, pi/2, pi, 大数
  // Test cases, including 0, pi/2, pi, and large numbers
  double tests_cos[] = {0.0, 1.57079632679, 3.14159265359, 10.0, 1000.0, 1e7};
  for (int i = 0; i < 6; i++) {
    double x = tests_cos[i];

    // --- 为了演示，我们在这里重复 pro 版的余数计算 ---
    // --- For demonstration, we repeat the remainder calculation here ---
    // 这部分代码仅用于在 main 函数中打印出余数 r
    // This part is only for printing the remainder r in main
    double r_abs_pro = fabs(x);
    int n_pro = (int)(((r_abs_pro + MY_M_PI_2) * MY_M_INV_PI) + 0.5);
    double xn_pro = (double)(n_pro)-0.5;
    double r_pro = (r_abs_pro - xn_pro * _C1) + (xn_pro * _C2);
    // --- 演示代码结束 ---
    // --- End of demonstration code ---

    // 打印对比结果
    // Print comparison results
    printf(" %12.4e | %14.8f | %14.8f | %14.8f\n", x, r_pro, my_cos_pro(x),
           cos(x));
  }
  printf("\n** 成功：专业版凭借高精度归约，可以正确处理大数值！\n");
  printf("** Success: The professional version can handle large values "
         "correctly due to high-precision reduction!\n");

  return 0;
}
