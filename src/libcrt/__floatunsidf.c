#include <stdint.h>


#define significandBits 52
#define exponentBias 1023
#define implicitBit (1ULL << significandBits) // 0x10000000000000ULL

static __inline double fromRep(uint64_t x) {
  const union {
    double f;
    uint64_t i;
  } rep = {.i = x};
  return rep.f;
}
#define unlikely(X) __builtin_expect(!!(X), 0)
double floatunsidf1(uint32_t a)
{

    const int aWidth = sizeof a * 8;

    // Handle zero as a special case to protect clz
    if (a == 0)
      return fromRep(0);

    // Exponent of (fp_t)a is the width of abs(a).
    const int exponent = (aWidth - 1) - __builtin_clz(a);
    uint64_t result;

    // Shift a into the significand field and clear the implicit bit.
    const int shift = significandBits - exponent;
    result = (uint64_t)a << shift ^ implicitBit;

    // Insert the exponent
    result += (uint64_t)(exponent + exponentBias) << significandBits;
    return fromRep(result);
}