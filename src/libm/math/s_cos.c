#include <math.h>
#include "sample_libm.h"

const double MY_PI = 3.14159265358979323846;
const double MY_PI_2 = 1.57079632679489661923; // Pi / 2

// 使用泰勒展开级数
// cos(x) = 1 - (x^2)/(2!) + (x^4)/(4!) - (x^6)/(6!) + ...
double my_cos_simple(double x) {
   const double C2  = -1.0 / 2.0;           //  -1/2!
   const double C4  =  1.0 / 24.0;          //   1/4!
   const double C6  = -1.0 / 720.0;         //  -1/6!
   const double C8  =  1.0 / 40320.0;       //   1/8!
   const double C10 = -1.0 / 3628800.0;     //   1/10!

   // 参数归约 : 根据cos函数特性将任意大小的输入x简化
   // 在一个在[0, 2*pi]，cos(x) 为偶函数
      //  warning : fmod 对于大数值的x将丢失精度
   x = fmod(fabs(x), 2.0 * MY_PI);

   // 归约到[-PI,PI]
   if (x > MY_PI)
     x -= 2.0 * MY_PI;

   double x_sq = x * x; // x^2
   double poly;

   // 极大程度减少乘法的次数
   // “笨”方法 1+C2*x^2+C4*x^4+C6*x^6+...
   // 需要 4 + 5 9次乘法 + 5次加法

   // 霍尔法则（秦九昭算法） 提取公因式
   // cos(x) = 1 + x^2(C2 + x^2(C4 + x^2(C6+x^2(C8 + x^2*C10)))
   // 5次乘法 + 5次加法
   poly = C10;
   poly = x_sq * poly + C8;
   poly = x_sq * poly + C6;
   poly = x_sq * poly + C4;
   poly = x_sq * poly + C2;
   poly = x_sq * poly + 1.0;

   return poly;
}

// --- 用于 Cody-Waite 参数归约的常量 ---
// Cody-Waite : 绝对不让两个大数想减，把一次危险的减法，拆分为两次安全的减法
// 把 PI 拆成两部分
const double MY_INV_PI = 0.31830988618379067154; // 1.0 / PI
const double PI_HI = 3.1415926535897931160E0;    // Pi 高位
const double PI_LO =
    1.2246467991473531772E-16; // Pi 低位即 PI - PI_HI 的误差

// libm : my_cos_pro (高精度归约 + Minimax 多项式)
// 使用一个三角恒等式，把 cos(x) 的计算转换成了 sin(r) 的计算
// r是极小的余数（规约产生的）r = |x| - (n - 0.5)PI
// n 是 floor((fabs(x)*PI)+0.5)
// cos(x) = cos((n-0.5)PI + r)
// cos(x) = cos(n*PI - PI/2 + r)
// cos(x - PI/2 + r) = sin(x)
// cos(x) = sin(n*PI + r)
// cos(|x|-(n+0.5)Pi)=sin(|x|-npi)=sin(r)
// n 是 偶数
// n 是 奇数
// 为什么要把cos换成sin
// 因为在r非常接近0时, sin(r)在数值计算上更精确
// cos(r)的泰勒级数是1-（r^2 / 2）+ ...
// 当r是一个极小的数（如1e-10）时，cos(r)的值非常非常接近1(比如0.999999)
// 在计算机中用double类型的数减去另一个几乎相等的数(1.0 - 0.000..1)会
// 损失大量精度。 这和fmod失败的原因一样，都属于“灾难性抵消”
// sin(r)的优势
// sin的泰勒级数时r-(r^3/6)+....
// 当r时一个极小的数(比如1e-10)时，sin(r)的值非常非常接近r(1e-10)
// 计算机在存储一个小数字(如1e-10)时，它的相对精度非常高
// 因此libm的专业算法(level2和3)会不惜一切代价避免取计算cos(r)，而是
// 想办法把他换成计算sin(r)

double my_cos_pro(double x) {
   if(isnan(x))
     return NAN;
   double r_abs = fabs(x);

   // 替换fmod
   // 原因： fmod 在处理余数时会发生精度丢失或者错误结果
   // floor是向下取整 不大于x的最大整数
   // +0.5 可以找到最接近x的整数
   // 巧妙地用“向下取整”实现了“四舍五入”
   int n = floor((r_abs * MY_INV_PI) + 0.5);
   double xn = (double)n;
   double xn_minus_half = xn - 0.5;

   // 高精度计算余数r
   double r = (r_abs - xn_minus_half * PI_HI) - (xn_minus_half * PI_LO);

   // Minimax 多项式逼近 sin(r)
   // sin(r) ≈ r + r * g * P(g),  其中 g = r*r
   const double S1 =
       -1.66666666666666324348e-01; // $-1/3! = -1/6 = -0.16666...$
   const double S2 =
       8.33333333332248946124e-03; // +1/5! = 1/120 = 0.008333...$
   const double S3 =
       -1.98412698298579493134e-04; // -1/7! = -1/5040 = -0.00019841...$
   const double S4 = 2.75573137064141112521e-06; //
   const double S5 = -2.50507602534068634195e-08;

   double g = r * r;
   double poly;

   // 秦九昭公式
   poly = S5;
   poly = g * poly + S4;
   poly = g * poly + S3;
   poly = g * poly + S2;
   poly = g * poly + S1;

   // Minimax逼近公式 sin(r) = r + r * g * P(g)
   double sin_r = r + r * g * poly;

   if((n & 1) == 0) // 偶数
      return sin_r;
   else
     return -sin_r;
}
