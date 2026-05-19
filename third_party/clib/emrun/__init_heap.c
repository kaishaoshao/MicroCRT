// **********************************************************************
// *                    SEGGER Microcontroller GmbH                     *
// *                        The Embedded Experts                        *
// **********************************************************************
// *                                                                    *
// *            (c) 2014 - 2025 SEGGER Microcontroller GmbH             *
// *            (c) 2001 - 2025 Rowley Associates Limited               *
// *                                                                    *
// *           www.segger.com     Support: support@segger.com           *
// *                                                                    *
// **********************************************************************
// *                                                                    *
// * All rights reserved.                                               *
// *                                                                    *
// * Redistribution and use in source and binary forms, with or         *
// * without modification, are permitted provided that the following    *
// * condition is met:                                                  *
// *                                                                    *
// * - Redistributions of source code must retain the above copyright   *
// *   notice, this condition and the following disclaimer.             *
// *                                                                    *
// * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
// * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
// * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
// * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
// * DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
// * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
// * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
// * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
// * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
// * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
// * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
// * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
// * DAMAGE.                                                            *
// *                                                                    *
// **********************************************************************

#if defined(__SES_ARM) || defined(__SES_RISCV)
void __SEGGER_RTL_init_heap(void *, unsigned);
#endif

#ifdef WITH_END_ESTACK_MIN_HEAP_SIZE

__attribute__((__section__("._user_heap_stack"))) unsigned __heap_start__;
extern __attribute__((alias("__heap_start__"),__section__("._user_heap_stack"))) unsigned _end;
unsigned __heap_end__;
extern __attribute__((alias("__heap_end__"))) unsigned _estack;
__attribute__((__constructor__))
static void __init_heap(void)
{  
  extern unsigned _Min_Heap_Size;
#if defined(__SES_ARM) || defined(__SES_RISCV) 
  __SEGGER_RTL_init_heap(&__heap_start__, (unsigned)&_Min_Heap_Size);
#else 
  (&__heap_start__)[0] = 0;
  (&__heap_start__)[1] = (unsigned)&_Min_Heap_Size;
#endif  
}

#endif

#ifdef WITH_HEAPBASE_HEAPLIMIT

unsigned __heap_start__, __heap_end__;
extern __attribute__((alias("__heap_start__"))) unsigned __HeapBase;
extern __attribute__((alias("__heap_end__"))) unsigned __HeapLimit;
__attribute__((__constructor__))
static void __init_heap(void)
{
#if defined(__SES_ARM) || defined(__SES_RISCV)
  __SEGGER_RTL_init_heap(&__heap_start__, &__heap_end__-&__heap_start__);
#else
  (&__heap_start__)[0] = 0;
  (&__heap_start__)[1] = &__heap_end__-&__heap_start__;
#endif
}
#endif
