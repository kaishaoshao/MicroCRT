/* Copyright (c) 2017 Keith Packard
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

#if !IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE) && defined(__IO_SMALL_ULTOA)

/*
 * Enable fancy divmod for targets where we don't expect either
 * hardware division support or that software will commonly be using
 * the soft division code. That means platforms where 'long' is not at
 * least 64-bits get the fancy code for 64-bit values and platforms
 * where 'int' is not at least 32-bits also get the fancy code for
 * 32-bit values
 */

#if SIZEOF_ULTOA > __SIZEOF_LONG__

#define FANCY_DIVMOD

static inline ultoa_unsigned_t
udivmod10(ultoa_unsigned_t n, char *rp)
{
    ultoa_unsigned_t q;
    char             r;

    q = (n >> 1) + (n >> 2);
    q = q + (q >> 4);
    q = q + (q >> 8);
    q = q + (q >> 16);

#if SIZEOF_ULTOA > 4
    q = q + (q >> 32);
#endif

    q = q >> 3;
    r = (char)(n - (((q << 2) + q) << 1));

    if (r > 9) {
        q++;
        r -= 10;
    }
    *rp = r;
    return q;
}

static inline ultoa_unsigned_t
udivmod(ultoa_unsigned_t val, int base, char *dig)
{
    switch (base) {
#ifdef __IO_PERCENT_B
    case 2:
        *dig = val & 1;
        return val >> 1;
#endif
    case 8:
        *dig = val & 7;
        return val >> 3;
    case 16:
        *dig = val & 15;
        return val >> 4;
    }
    return udivmod10(val, dig);
}

#endif
#endif

static char *
__ultoa_invert(ultoa_unsigned_t val, char *str, int base)
{
    char hex = ('a' - '0' - 10 + 16) - base;

    base &= 31;

    do {
        char v;

#ifdef FANCY_DIVMOD
        val = udivmod(val, base, &v);
#else
        v = val % base;
        val /= base;
#endif
        if (v > 9)
            v += hex;
        v += '0';
        *str++ = v;
    } while (val);
    return str;
}
