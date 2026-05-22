/* Copyright (c) 2002, Alexander Popov (sasho@vip.bg)
   Copyright (c) 2002,2004,2005 Joerg Wunsch
   Copyright (c) 2005, Helmut Wallner
   Copyright (c) 2007, Dmitry Xmelkov
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
  POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Conversion: %f / %e / %g / %a and long-double variants
 *
 * Shared float-family dispatch block.
 *
 * Specifier-specific work is already delegated to:
 * - _printf_f.c / _printf_e.c / _printf_g.c / _printf_a.c
 */

{
    unsigned char case_convert;

    case_convert = TOLOWER(c) - c;
    c = TOLOWER(c);

    if (c == 'f') {
        if (__printf_entry_f(out, &stream_len, &flags, &prec, &width, case_convert, ap, &u.dtoa) < 0)
            goto fail;
    } else if (c == 'e') {
        if (__printf_entry_e(out, &stream_len, &flags, &prec, &width, case_convert, ap, &u.dtoa) < 0)
            goto fail;
    } else if (c == 'g') {
        if (__printf_entry_g(out, &stream_len, &flags, &prec, &width, case_convert, ap, &u.dtoa) < 0)
            goto fail;
#ifdef _NEED_IO_C99_FORMATS
    } else if (c == 'a') {
        if (__printf_entry_a(out, &stream_len, &flags, &prec, &width, case_convert, ap, &u.dtoa) < 0)
            goto fail;
#endif
    }
}
