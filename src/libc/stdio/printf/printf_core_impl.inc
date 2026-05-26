/*
 * Shared printf core implementation template.
 *
 * This include assembles one complete core variant in three stages:
 * 1. variant feature selection
 * 2. shared helper/support assembly
 * 3. common core execution template
 *
 * Historical notice:
 * The implementation is derived from the AVR-LibC vfprintf family and
 * therefore retains the original upstream copyright notice below.
 */

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

/* From: Id: printf_p_new.c,v 1.1.1.9 2002/10/15 20:10:28 joerg_wunsch Exp */
/* $Id: vfprintf.c 2191 2010-11-05 13:45:57Z arcanum $ */

/* Stage 1: select variant capabilities, flags and scratch sizing. */
#include "printf_config.h"

/* Stage 2: assemble shared support helpers for this variant. */
#include "ultoa_invert.c"
#if PRINTF_CAP_POSITIONAL
#include "_printf_positional.c"
#endif
#include "_printf_text_core.c"
#include "_printf_parser.c"
#include "_printf_char_core.c"
#include "_printf_string_core.c"
#include "_printf_int_dec.c"
#include "_printf_int_base.c"
#include "_printf_pointer.c"
#include "_printf_c.c"
#include "_printf_s.c"
#include "_printf_n.c"
#include "_printf_d.c"
#include "_printf_u.c"
#include "_printf_o.c"
#include "_printf_x.c"
#include "_printf_p.c"
#if PRINTF_CAP_BINARY
#include "_printf_b.c"
#endif
#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
#include "_printf_fp_dec.c"
#include "_printf_fp_hex.c"
#include "_printf_f.c"
#include "_printf_e.c"
#include "_printf_g.c"
#if PRINTF_CAP_C99_FORMATS
#include "_printf_a.c"
#endif
#endif
#include "_printf_conversion_dispatch.c"

/* Stage 3: compile the common core template with the chosen support set. */
#include "printf_core.inc"
