static int __atomic_lock(void) 
{
#if defined(__aarch64__)
  int __daif;
  __asm__ __volatile__("mrs %[daif], daif\n"
                       "msr daifset, #2\n"
                       : [daif] "=r"(__daif));
  return (__daif & (1<<7)) == 0;
#elif defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_8M_BASELINE__) || defined(__ARM_ARCH_8M_MAINLINE__) || defined(__ARM_ARCH_81M_MAINLINE__)
  int __primask;
  __asm__ __volatile__("mrs %[__primask], primask\n"
                       "cpsid i\n"
                       : [__primask] "=r" (__primask));
  return __primask == 0;
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_8A__) || defined(__ARM_ARCH_8R__)
  int __cpsr;
  __asm__ __volatile__("mrs %[cpsr], cpsr\n"
                       "cpsid i\n"
                       : [cpsr] "=r"(__cpsr));
  return (__cpsr & 0x80) == 0;
#elif defined(__ARM_ARCH_4T__) || defined(__ARM_ARCH_5TE__)
  int __cpsr, __tmp;
  __asm__ __volatile__("mrs %[cpsr], cpsr\n"
                       "orr %[tmp], %[cpsr], #0x80\n"
                       "msr cpsr_c, %[tmp]\n"
                       : [cpsr] "=r"(__cpsr), [tmp] "=r"(__tmp));
  return (__cpsr & 0x80) == 0;
#else
  int __mstatus, __tmp;
  __asm__ __volatile__("csrr %[mstatus], mstatus\n" 
                       "andi %[tmp], %[mstatus], ~(1<<3)\n"
                       "csrw mstatus, %[tmp]\n"
                       : [mstatus] "=r" (__mstatus), [tmp] "=r"(__tmp));
  return (__mstatus & (1<<3));
#endif
}

static void __atomic_unlock(int en) 
{ 
  if (en)
    {
#if defined(__aarch64__)
  __asm__ __volatile__("msr daifclr, #2\n");
#elif defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_8M_BASELINE__) || defined(__ARM_ARCH_8M_MAINLINE__) || defined(__ARM_ARCH_81M_MAINLINE__) ||\
    defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_8A__) || defined(__ARM_ARCH_8R__)
      __asm__ __volatile__("cpsie i\n");
#elif defined(__ARM_ARCH_4T__) || defined(__ARM_ARCH_5TE__)
      int __cpsr;
      __asm__ __volatile__("mrs %[cpsr], cpsr\n"
                           "bic %[cpsr], %[cpsr], #0x80\n"
                           "msr cpsr_c, %[cpsr]\n"
                           : [cpsr] "=r"(__cpsr));
#else
       int __mstatus, __tmp;
       __asm__ __volatile__("csrr %[mstatus], mstatus\n" 
                            "ori %[tmp], %[mstatus], (1<<3)\n"
                            "csrw mstatus, %[tmp]\n"
                            : [mstatus] "=r" (__mstatus), [tmp] "=r"(__tmp));
#endif
    }
}
