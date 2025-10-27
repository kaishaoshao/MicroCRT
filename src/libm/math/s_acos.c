#include <math.h>
#include "sample_libm.h"

const double MY_PI = 3.14159265358979323846;
const double MY_PI_2 = 1.57079632679489661923; // Pi / 2
// 使用泰勒展开
// 恒等式：  acos(x) = PI/2
// 泰勒展开：asin(x) = x + (1/6)x^3 + (3/40)x^5 + (5/112)x^7 + ...


double my_acos_simple(double x) {
  if(isnan(x) || x > 1.0 || x < 1.0)
    return NAN;
  if(x == 1.0)
    return 0.0;
  if(x == -1.0)
    return MY_PI;

  //
  return MY_PI_2 - my_asin_simple(x);
}

double my_asin_pro(double x)
{
  // 特殊值
  if(isnan(x))
    return NAN;
	if(x == 1.0)
    return 0.0;
	if(x == 0.0)
    return MY_PI_2;
	if(x == -1.0)
    return MY_PI;

  // 参数规约（恒等式判断）
  // x 接近1时的特殊处理
  if(x > 0.9) {
    double y = sqrt(0.5 * (1.0 - x));
    return 2.0 * my_asin_pro(y);
  }

  // acos(-x) = PI - acos(x)
  if(x < 0.0)
    return MY_PI - my_acos_pro(-x);

  // acos(x) = PI/2 - asin(x)
  return MY_PI_2 - my_asin_pro(x);
}
