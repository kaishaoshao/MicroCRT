#include "sample_libm.h"

// 泰勒展开：asin(x) = x + (1/6)x^3 + (3/40)x^5 + (5/112)x^7 + ...
double my_asin_simple(double x) {
  const double C1 = 1.0 / 6.0;
  const double C3 = 3.0 / 40.0;
  const double C5 = 5.0 / 112.0;
  const double C7 = 35.0 / 1152.0;

  double g = x * x;
  double poly;

  // 秦九昭算法
  poly = C7;
  poly = g * poly + C5;
  poly = g * poly + C3;
  poly = g * poly + C1;
  poly = g * poly + 1.0;

  return x * poly;
}

// my_asin_pro (有理逼近)
// asin(x) = X + x * g * (P(g)/ Q(g))，其中g=x * x
double my_asin_pro(double x) {
  double g = x * x;
  // P(g) 的系数
  const double p0 = -0.549633342261981;
  const double p1 = 0.101525222338064;
  const double p2 = -0.006967457344735;

  // Q(g) 的系数
  const double q0 = -3.29779963148181;
  const double q1 = 1.0;

	// 秦九昭公式 P(g)
  double poly_p = p2;
  poly_p = g * poly_p + p1;
  poly_p = g * poly_p + p0;

  // 秦九昭公式 Q(g)
  double poly_q = q1;
  poly_p = g * poly_p + p0;

	// asin(x) = x + x * g * (P/Q)
  return x + x * g * (poly_p / poly_q);
}
