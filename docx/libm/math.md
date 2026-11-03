# MicroCRT Math 模块函数规范与实现清单

`math.h` 是 MicroCRT 数学库的核心公共头文件，定义在 [include/libm/math.h](file:///include/libm/math.h)。全库函数覆盖三角、反三角、双曲、指数对数、幂运算、取整截断与特殊浮点操作。

---

## 一、数学常数定义 (Constants)

| 宏名称 | 数学含义 | 近似常量值 |
| :--- | :--- | :--- |
| `M_E` | 自然常数 $e$ | `2.71828182845904523536` |
| `M_LOG2E` | $\log_2(e)$ | `1.44269504088896340736` |
| `M_LOG10E` | $\log_{10}(e)$ | `0.43429448190325182765` |
| `M_LN2` | $\ln(2)$ | `0.69314718055994530942` |
| `M_LN10` | $\ln(10)$ | `2.30258509299404568402` |
| `M_PI` | 圆周率 $\pi$ | `3.14159265358979323846` |
| `M_PI_2` | $\pi / 2$ | `1.57079632679489661923` |
| `M_PI_4` | $\pi / 4$ | `0.78539816339744830962` |
| `M_1_PI` | $1 / \pi$ | `0.31830988618379067154` |
| `M_2_PI` | $2 / \pi$ | `0.63661977236758134308` |
| `M_2_SQRTPI` | $2 / \sqrt{\pi}$ | `1.12837916709551257390` |
| `M_SQRT2` | $\sqrt{2}$ | `1.41421356237309504880` |
| `M_SQRT1_2` | $1 / \sqrt{2}$ | `0.70710678118654752440` |

---

## 二、标准数学函数分类清单

每个函数均提供三种精度签名：
- 单精度：`function_name + f`（如 `sinf`）
- 双精度：`function_name`（如 `sin`）
- 扩展精度：`function_name + l`（如 `sinl`）

### 1. 三角函数 (Trigonometric Functions)
| 函数签名 (Double) | 单精度签名 (Float) | 数学定义 | 定义域 / 值域 | 特殊输入处理 |
| :--- | :--- | :--- | :--- | :--- |
| `double cos(double x);` | `float cosf(float x);` | $\cos(x)$ | $(-\infty, +\infty) \to [-1, 1]$ | $\pm\infty \to \text{NaN}$, 触发 `FE_INVALID` |
| `double sin(double x);` | `float sinf(float x);` | $\sin(x)$ | $(-\infty, +\infty) \to [-1, 1]$ | $\pm\infty \to \text{NaN}$ |
| `double tan(double x);` | `float tanf(float x);` | $\tan(x)$ | $x \neq \pi/2 + k\pi \to (-\infty, +\infty)$ | 极点处返回 $\pm\infty$ |
| `void sincos(double x, double *s, double *c);` | `void sincosf(float x, float *s, float *c);` | 同时计算 $\sin$ 与 $\cos$ | 同上 | 共享一次区间归约，效率翻倍 |

### 2. 反三角函数 (Inverse Trigonometric Functions)
| 函数签名 (Double) | 单精度签名 (Float) | 数学定义 | 定义域 / 值域 | 特殊输入处理 |
| :--- | :--- | :--- | :--- | :--- |
| `double acos(double x);` | `float acosf(float x);` | $\arccos(x)$ | $[-1, 1] \to [0, \pi]$ | $|x| > 1 \to \text{NaN}$, `errno = EDOM` |
| `double asin(double x);` | `float asinf(float x);` | $\arcsin(x)$ | $[-1, 1] \to [-\pi/2, \pi/2]$ | $|x| > 1 \to \text{NaN}$, `errno = EDOM` |
| `double atan(double x);` | `float atanf(float x);` | $\arctan(x)$ | $(-\infty, +\infty) \to [-\pi/2, \pi/2]$ | $x = \pm\infty \to \pm\pi/2$ |
| `double atan2(double y, double x);` | `float atan2f(float y, float x);` | 四象限 $\arctan(y/x)$ | 全平面 $\to [-\pi, \pi]$ | 正确处理符号确定象限，$(0,0) \to 0$ |

### 3. 双曲与反双曲函数 (Hyperbolic Functions)
| 函数签名 (Double) | 单精度签名 (Float) | 数学定义 | 描述 |
| :--- | :--- | :--- | :--- |
| `double cosh(double x);` | `float coshf(float x);` | $\frac{e^x + e^{-x}}{2}$ | 双曲余弦 |
| `double sinh(double x);` | `float sinhf(float x);` | $\frac{e^x - e^{-x}}{2}$ | 双曲正弦 |
| `double tanh(double x);` | `float tanhf(float x);` | $\frac{\sinh(x)}{\cosh(x)}$ | 双曲正切 |
| `double acosh(double x);` | `float acoshf(float x);` | $\ln(x + \sqrt{x^2 - 1})$ | 反双曲余弦（$x \ge 1$） |
| `double asinh(double x);` | `float asinhf(float x);` | $\ln(x + \sqrt{x^2 + 1})$ | 反双曲正弦 |
| `double atanh(double x);` | `float atanhf(float x);` | $\frac{1}{2}\ln\left(\frac{1+x}{1-x}\right)$ | 反双曲正切（$|x| < 1$） |

### 4. 指数与对数函数 (Exponential and Logarithmic)
| 函数签名 (Double) | 单精度签名 (Float) | 数学定义 | 描述与优化策略 |
| :--- | :--- | :--- | :--- |
| `double exp(double x);` | `float expf(float x);` | $e^x$ | 自然指数 |
| `double exp2(double x);` | `float exp2f(float x);` | $2^x$ | 以 2 为底指数（直接映射浮点阶码） |
| `double expm1(double x);` | `float expm1f(float x);` | $e^x - 1$ | 当 $|x| \ll 1$ 时避免精度严重损失 |
| `double log(double x);` | `float logf(float x);` | $\ln(x)$ | 自然对数（$x > 0$） |
| `double log10(double x);` | `float log10f(float x);` | $\log_{10}(x)$ | 常用对数 |
| `double log2(double x);` | `float log2f(float x);` | $\log_2(x)$ | 二进制对数 |
| `double log1p(double x);` | `float log1pf(float x);` | $\ln(1 + x)$ | 当 $|x| \ll 1$ 时的高精度对数 |
| `double logb(double x);` | `float logbf(float x);` | 浮点数阶码 | 返回浮点有偏阶码的浮点值 |
| `int ilogb(double x);` | `int ilogbf(float x);` | 浮点数阶码 | 返回整数阶码 |
| `double frexp(double x, int *exp);` | `float frexpf(float x, int *exp);` | 尾数分解 | 将 $x$ 分解为尾数 $[0.5, 1)$ 和 $2^{exp}$ |
| `double ldexp(double x, int exp);` | `float ldexpf(float x, int exp);` | $x \times 2^{exp}$ | 浮点数快速位运算缩放 |
| `double scalbn(double x, int n);` | `float scalbnf(float x, int n);` | $x \times b^n$ | 浮点基数缩放（等价于 ldexp） |
| `double modf(double x, double *iptr);` | `float modff(float x, float *iptr);` | 整数与小数分离 | 将浮点数无损失拆分为整数部分与小数部分 |

### 5. 幂函数、平方根与多维距离 (Power, Root & Distance)
| 函数签名 (Double) | 单精度签名 (Float) | 数学定义 | 描述 |
| :--- | :--- | :--- | :--- |
| `double pow(double x, double y);` | `float powf(float x, float y);` | $x^y$ | 通用幂运算 |
| `double sqrt(double x);` | `float sqrtf(float x);` | $\sqrt{x}$ | 平方根（优先硬件 FPU 指令加速，如 `VSQRT.F32` / `fsqrt.s`） |
| `double cbrt(double x);` | `float cbrtf(float x);` | $\sqrt[3]{x}$ | 立方根 |
| `double hypot(double x, double y);`| `float hypotf(float x, float y);` | $\sqrt{x^2 + y^2}$ | 斜边长度（内部防下溢与溢出放大缩减算法） |
| `double fabs(double x);` | `float fabsf(float x);` | $\|x\|$ | 绝对值（硬件清除符号位） |

### 6. 误差与伽马函数 (Error and Gamma)
| 函数签名 (Double) | 单精度签名 (Float) | 数学定义 | 描述 |
| :--- | :--- | :--- | :--- |
| `double erf(double x);` | `float erff(float x);` | $\frac{2}{\sqrt{\pi}}\int_0^x e^{-t^2}dt$ | 高斯误差函数 |
| `double erfc(double x);` | `float erfcf(float x);` | $1 - \text{erf}(x)$ | 互补误差函数（高精计算大 $x$ 尾部） |
| `double tgamma(double x);` | `float tgammaf(float x);` | $\Gamma(x) = \int_0^\infty t^{x-1}e^{-t}dt$ | 真实伽马函数（阶乘推广） |
| `double lgamma(double x);` | `float lgammaf(float x);` | $\ln\|\Gamma(x)\|$ | 伽马函数绝对值的自然对数 |

### 7. 取整与截断函数 (Nearest Integer & Rounding)
| 函数签名 (Double) | 单精度签名 (Float) | 舍入方向 | 示例 ($x = 2.7, -2.7$) |
| :--- | :--- | :--- | :--- |
| `double ceil(double x);` | `float ceilf(float x);` | 向上取整（正无穷方向） | `3.0, -2.0` |
| `double floor(double x);` | `float floorf(float x);` | 向下取整（负无穷方向） | `2.0, -3.0` |
| `double trunc(double x);` | `float truncf(float x);` | 向零截断（舍弃小数部分） | `2.0, -2.0` |
| `double round(double x);` | `float roundf(float x);` | 四舍五入（远离零方向） | `3.0, -3.0` |
| `long lround(double x);` | `long lroundf(float x);` | 四舍五入转换为 `long` 整数 | `3, -3` |
| `long long llround(double x);`| `long long llroundf(float x);`| 四舍五入转换为 `long long` 整数 | `3, -3` |
| `double rint(double x);` | `float rintf(float x);` | 依当前 FPU 舍入模式取整 | 依 `fegetround()` |
| `long lrint(double x);` | `long lrintf(float x);` | 依当前 FPU 舍入模式转为 `long` | 依 `fegetround()` |
| `double nearbyint(double x);` | `float nearbyintf(float x);`| 依当前模式取整但不报 `FE_INEXACT` | 依 `fegetround()` |

### 8. 差值、极值与融合乘加 (Extremum, Difference & FMA)
| 函数签名 (Double) | 单精度签名 (Float) | 功能与数学含义 |
| :--- | :--- | :--- |
| `double fdim(double x, double y);` | `float fdimf(float x, float y);` | 正差值：$\max(x - y, 0)$ |
| `double fmax(double x, double y);` | `float fmaxf(float x, float y);` | 较大值（若一个为 NaN 则返回非 NaN 那个） |
| `double fmin(double x, double y);` | `float fminf(float x, float y);` | 较小值（自动过滤非实质 NaN） |
| `double fma(double x, double y, double z);` | `float fmaf(float x, float y, float z);` | 融合乘加 $x \times y + z$，中间不发生精度截断 |
| `double fmod(double x, double y);` | `float fmodf(float x, float y);` | 浮点余数（商向零截断：$x - n \times y$） |
| `double remainder(double x, double y);`| `float remainderf(float x, float y);`| IEEE 浮点余数（商四舍五入到最近整数） |
| `double remquo(double x, double y, int *quo);` | `float remquof(float x, float y, int *quo);` | 返回 IEEE 余数并保留商的低位到 `quo` |

### 9. 浮点位操作与步进 (Floating-point Stepping)
| 函数签名 (Double) | 单精度签名 (Float) | 功能说明 |
| :--- | :--- | :--- |
| `double copysign(double x, double y);` | `float copysignf(float x, float y);` | 组合 $x$ 的数值大小与 $y$ 的正负符号 |
| `double nan(const char *tagp);` | `float nanf(const char *tagp);` | 生成带标识的安静非数（Quiet NaN） |
| `double nextafter(double x, double y);`| `float nextafterf(float x, float y);`| 返回浮点数 $x$ 朝 $y$ 方向邻接的下一个机器可表示浮点数 |
| `double nexttoward(double x, long double y);` | `float nexttowardf(float x, long double y);` | 精度拓展方向邻接步进 |
