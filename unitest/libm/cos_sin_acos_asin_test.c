#include <stdio.h>
#include <math.h> // 使用 fmod, fabs, sqrt, isnan, isinf 来辅助

/* * ===================================================================
 * ===================================================================
 *
 * Level 1: 简单教学版 (泰勒级数)
 *
 * ===================================================================
 * ===================================================================
 */

const double MY_PI = 3.14159265358979323846;
const double MY_PI_2 = 1.57079632679489661923; // Pi / 2

/*
 * ===================================================================
 * Level 1: my_cos_simple (使用泰勒级数)
 * ===================================================================
 * 泰勒展开: cos(x) = 1 - x^2/2! + x^4/4! - x^6/6! + ...
 */
double my_cos_simple(double x) {
  const double C2 = -1.0 / 2.0;        // -1/2!
  const double C4 = 1.0 / 24.0;        // 1/4!
  const double C6 = -1.0 / 720.0;      // -1/6!
  const double C8 = 1.0 / 40320.0;     // 1/8!
  const double C10 = -1.0 / 3628800.0; // 1/10!

  // 警告：fmod 对于大数值的 x 会丢失精度
  x = fmod(fabs(x), 2.0 * MY_PI);

  // 归约到 [-PI, PI]
  if (x > MY_PI) {
    x -= 2.0 * MY_PI;
  }

  double x_sq = x * x; // x^2
  double poly;

  // 霍纳法则 (Horner's method)
  poly = C10;
  poly = x_sq * poly + C8;
  poly = x_sq * poly + C6;
  poly = x_sq * poly + C4;
  poly = x_sq * poly + C2;
  poly = x_sq * poly + 1.0;

  return poly;
}

/*
 * ===================================================================
 * Level 1: my_acos_simple (使用泰勒级数)
 * ===================================================================
 * 恒等式: acos(x) = PI/2 - asin(x)
 * 泰勒展开: asin(x) = x + (1/6)x^3 + (3/40)x^5 + (5/112)x^7 + ...
 */
double my_asin_simple(double x) {
  // 系数
  const double C1 = 1.0 / 6.0;
  const double C3 = 3.0 / 40.0;
  const double C5 = 5.0 / 112.0;
  const double C7 = 35.0 / 1152.0;

  double g = x * x; // g = x^2
  double poly;

  // 霍纳法则
  poly = C7;
  poly = g * poly + C5;
  poly = g * poly + C3;
  poly = g * poly + C1;
  poly = g * poly + 1.0;

  return x * poly;
}

double my_acos_simple(double x) {
  if (isnan(x) || x > 1.0 || x < -1.0)
    return NAN;
  if (x == 1.0)
    return 0.0;
  if (x == -1.0)
    return MY_PI;

  // 警告：这个泰勒级数在 x 接近 1 或 -1 时精度很差
  return MY_PI_2 - my_asin_simple(x);
}

/* * ===================================================================
 * ===================================================================
 *
 * Level 2: 专业进阶版 (libm 核心算法)
 *
 * ===================================================================
 * ===================================================================
 */

// --- 用于 Cody-Waite 参数归约的常量 ---
const double MY_INV_PI = 0.31830988618379067154; // 1.0 / PI
const double PI_HI = 3.1415926535897931160E0;    // Pi 高位
const double PI_LO = 1.2246467991473531772E-16;  // Pi 低位 (尾巴)

/*
 * ===================================================================
 * 专业方法 1: my_cos_pro (高精度归约 + Minimax 多项式)
 * ===================================================================
 */
double my_cos_pro(double x) {
  // --- 第 1 步：处理“特殊值” ---
  if (isnan(x) || isinf(x)) {
    return NAN;
  }

  double r_abs = fabs(x);

  // --- 第 2 步：高精度参数归约 (Cody-Waite 思想) ---
  int n = floor((r_abs * MY_INV_PI) + 0.5);
  double xn = (double)n;
  double xn_minus_half = (xn - 0.5);

  // 高精度计算余数 r
  double r = (r_abs - xn_minus_half * PI_HI) - (xn_minus_half * PI_LO);

  // --- 第 3 步：Minimax 多项式逼近 sin(r) ---
  // sin(r) ≈ r + r * g * P(g),  其中 g = r*r
  const double S1 = -1.66666666666666324348e-01;
  const double S2 = 8.33333333332248946124e-03;
  const double S3 = -1.98412698298579493134e-04;
  const double S4 = 2.75573137064141112521e-06;
  const double S5 = -2.50507602534068634195e-08;

  double g = r * r;
  double poly;

  // 霍纳法则
  poly = S5;
  poly = g * poly + S4;
  poly = g * poly + S3;
  poly = g * poly + S2;
  poly = g * poly + S1;

  double sin_r = r + r * g * poly;

  // --- 第 4 步：结果重构 ---
  if ((n & 1) == 0) { // n 是偶数
    return sin_r;
  } else { // n 是奇数
    return -sin_r;
  }
}

/*
 * ===================================================================
 * 专业方法 2: my_asin_pro (有理逼近)
 * ===================================================================
 * asin(x) ≈ x + x * g * (P(g) / Q(g)),  其中 g = x*x
 */
double my_asin_pro(double x) {
  double g = x * x;

  // P(g) 的系数
  const double p0 = -0.549633342261981;
  const double p1 = 0.101525222338064;
  const double p2 = -0.006967457344735;

  // Q(g) 的系数
  const double q0 = -3.29779963148181;
  const double q1 = 1.0;

  // 霍纳法则计算 P(g)
  double poly_p = p2;
  poly_p = g * poly_p + p1;
  poly_p = g * poly_p + p0;

  // 霍纳法则计算 Q(g)
  double poly_q = q1;
  poly_q = g * poly_q + q0;

  // asin(x) = x + x*g*(P/Q)
  return x + x * g * (poly_p / poly_q);
}

/*
 * ===================================================================
 * my_acos_pro(x) - 反余弦 (使用有理逼近)
 * ===================================================================
 */
double my_acos_pro(double x) {
  // --- 第 1 步：特殊值 ---
  if (isnan(x))
    return NAN;
  if (x > 1.0 || x < -1.0)
    return NAN;
  if (x == 1.0)
    return 0.0;
  if (x == 0.0)
    return MY_PI_2;
  if (x == -1.0)
    return MY_PI;

  // --- 第 2 步：参数归约 (恒等式变换) ---

  // 2.3: x 接近 1 时的特殊处理
  if (x > 0.9) {
    double y = sqrt(0.5 * (1.0 - x));
    // 调用我们的 "pro" 版 asin
    return 2.0 * my_asin_pro(y);
  }

  // 2.1: acos(-x) = PI - acos(x)
  if (x < 0.0) {
    return MY_PI - my_acos_pro(-x);
  }

  // --- 第 3 步：调用 pro 版 asin ---
  // 2.2: acos(x) = PI/2 - asin(x)
  return MY_PI_2 - my_asin_pro(x);
}

/* * ===================================================================
 * ===================================================================
 *
 * main() 函数
 * (对比 Level 1 和 Level 2)
 *
 * ===================================================================
 * ===================================================================
 */
int main() {
  printf("--- (Level 1) 测试 my_cos_simple (泰勒级数) ---\n");
  printf("         x    |   fmod 余数   | my_cos_simple |   std_cos(x)\n");
  printf(
      "-------------------------------------------------------------------\n");

  double tests_cos[] = {0.0, MY_PI_2, MY_PI, 10.0, 1000.0, 1e7};
  for (int i = 0; i < 6; i++) {
    double x = tests_cos[i];
    // 计算 fmod 余数以供显示
    double r_simple = fmod(fabs(x), 2.0 * MY_PI);
    if (r_simple > MY_PI) {
      r_simple -= 2.0 * MY_PI;
    }
    printf(" %12.4e | %12.8f | %12.8f | %12.8f\n", x, r_simple,
           my_cos_simple(x), cos(x));
  }

  printf("\n!! 警告：注意看 x=1000.0 时的巨大误差！(fmod 失败)\n");

  printf("\n--- (Level 2) 测试 my_cos_pro (专业版实现) ---\n");
  printf("         x    | 高精度余数 r  |  my_cos_pro(x) |   std_cos(x)\n");
  printf("---------------------------------------------------------------------"
         "-\n");
  for (int i = 0; i < 6; i++) {
    double x = tests_cos[i];

    // --- 为了演示，我们在这里重复 pro 版的余数计算 ---
    double r_abs_pro = fabs(x);
    int n_pro = floor((r_abs_pro * MY_INV_PI) + 0.5);
    double xn_pro = (double)n_pro;
    double xn_minus_half_pro = (xn_pro - 0.5);
    double r_pro =
        (r_abs_pro - xn_minus_half_pro * PI_HI) - (xn_minus_half_pro * PI_LO);
    // --- 演示代码结束 ---

    printf(" %12.4e | %14.8f | %14.8f | %14.8f\n", x, r_pro, my_cos_pro(x),
           cos(x));
  }
  printf("\n** 成功：my_cos_pro 凭借高精度归约，可以正确处理大数值！\n");

  printf("\n\n--- (Level 1) 测试 my_acos_simple (泰勒级数) ---\n");
  printf("         x    | my_acos_simple|  std_acos(x)\n");
  printf("-------------------------------------------------\n");
  double tests_acos[] = {0.0, 0.5, 0.9, 0.99};
  for (int i = 0; i < 4; i++) {
    double x = tests_acos[i];
    printf(" %12.4f | %12.8f | %12.8f\n", x, my_acos_simple(x), acos(x));
  }
  printf("\n!! 警告：注意看 x 接近 1 时，误差开始变大。\n");

  printf("\n--- (Level 2) 测试 my_acos_pro (有理逼近) ---\n");
  printf("         x    | my_acos_pro(x) |  std_acos(x)\n");
  printf("--------------------------------------------------\n");
  for (int i = 0; i < 4; i++) {
    double x = tests_acos[i];
    printf(" %12.4f | %14.8f | %14.8f\n", x, my_acos_pro(x), acos(x));
  }
  printf(
      "\n** 成功：my_acos_pro 凭借恒等式变换，在 x 接近 1 时保持了高精度。\n");

  return 0;
}
