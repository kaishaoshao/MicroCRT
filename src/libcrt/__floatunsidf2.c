#include <stdint.h>

typedef union {
  uint64_t u64;
  double d;
} DoubleConverter;

/**
 * @brief 纯C语言实现的、等效于RV32软件路径汇编的函数。
 * 将一个32位无符号整数转换为double。
 * @param x 要转换的32位无符号整数。
 * @return 转换后的double值。
 */
double floatunsidf1(uint32_t x) {
  // 对应汇编中的: beqz a0, L(zero)
  // 如果输入为0，直接返回0.0。
  if (x == 0) {
    return 0.0;
  }

  // 对应汇编中的: li a2, 0x41D
  // 初始化指数。0x41D = 1053。
  // 计算方法: 1023 (double的指数偏移量) + 31 (32位无符号整数的最大幂次) - 1
  // (规格化调整)
  int exponent = 0x41D;

  // 对应汇编中的: NORM32D a0, a2, a3
  // --- 规格化过程开始 ---
  uint32_t mantissa = x;

  // 使用GCC/Clang内建函数__builtin_clz来高效计算前导零的个数。
  // 这等效于手写汇编中NORM32D宏通过循环实现的功能。
  int leading_zeros = __builtin_clz(mantissa);

  // 规格化尾数：将最高位的'1'移动到32位字的最左边。
  mantissa <<= leading_zeros;

  // 调整指数：每向左移动一位，指数就减一。
  exponent -= leading_zeros;
  // --- 规格化过程结束 ---
  // 此刻，mantissa是规格化后的32位尾数，exponent是最终的11位指数。

  // --- 对应汇编中 L(normalized) 之后的位操作 ---
  // 在RV32上，double类型通过 a1:a0 两个32位寄存器返回。
  // a1 = 高32位字, a0 = 低32位字。
  // double格式: [符号(1) 指数(11) 尾数高位(20)] [尾数低位(32)]

  // 对应汇编中的: sll a2, a2, 20
  // 构建高32位字中的指数部分：将11位指数左移20位。
  uint32_t high_word_exp_part = (uint32_t)exponent << 20;

  // 对应汇编中的: srl a3, a0, 11 (a3 = mantissa >> 11)
  // 获取规格化后尾数的高21位，用于填充高32位字中的尾数部分。
  // 注意：这里的位操作与汇编完全一致，即使看起来会多取一位。
  uint32_t high_word_mant_part = mantissa >> 11;

  // 对应汇编中的: sll a0, a0, 21
  // 获取规格化后尾数的低11位，并将其移动到32位字的最高位，形成低32位字。
  uint32_t low_word = mantissa << 21;

  // 对应汇编中的: add a1, a3, a2
  // 组合成高32位字
  uint32_t high_word = high_word_exp_part + high_word_mant_part;

  // 将高32位和低32位组合成一个完整的64位整数
  uint64_t final_bits = ((uint64_t)high_word << 32) | low_word;

  // 使用union进行最终的位转换并返回
  DoubleConverter converter;
  converter.u64 = final_bits;
  return converter.d;
}
