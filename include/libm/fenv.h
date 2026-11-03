//===-- include/libm/fenv.h - MicroCRT Floating-Point Environment -*- C -*-===//
//
// Part of the MicroCRT Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares floating-point status flags, rounding modes, exception
/// handling, and environment control functions according to ISO C99.
///
//===----------------------------------------------------------------------===//

#ifndef _FENV_H
#define _FENV_H

#ifdef __cplusplus
extern "C" {
#endif

//===----------------------------------------------------------------------===//
// Floating-Point Exception Flags
//===----------------------------------------------------------------------===//

#define FE_INVALID      0x01  ///< Invalid floating-point operation.
#define FE_DIVBYZERO    0x02  ///< Division by zero.
#define FE_OVERFLOW     0x04  ///< Floating-point overflow.
#define FE_UNDERFLOW    0x08  ///< Floating-point underflow.
#define FE_INEXACT      0x10  ///< Inexact result.
#define FE_ALL_EXCEPT   (FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT)

//===----------------------------------------------------------------------===//
// Floating-Point Rounding Modes
//===----------------------------------------------------------------------===//

#define FE_TONEAREST    0x0000  ///< Round to nearest representable value.
#define FE_UPWARD       0x0400  ///< Round toward positive infinity (+inf).
#define FE_DOWNWARD     0x0800  ///< Round toward negative infinity (-inf).
#define FE_TOWARDZERO   0x0C00  ///< Round toward zero (truncation).

/// Type representing floating-point exception status flags collection.
typedef unsigned short fexcept_t;

/// Type representing the entire floating-point status/control environment.
typedef struct {
  unsigned int __control_word;
  unsigned int __status_word;
} fenv_t;

/// Pointer to default floating-point environment.
#define FE_DFL_ENV      ((const fenv_t *)-1)

//===----------------------------------------------------------------------===//
// Floating-Point Exception Handling Functions
//===----------------------------------------------------------------------===//

/// Clears floating-point exceptions represented by excepts.
int feclearexcept(int excepts);

/// Stores implementation-defined representation of exception flags in flagp.
int fegetexceptflag(fexcept_t *flagp, int excepts);

/// Raises the floating-point exceptions represented by excepts.
int feraiseexcept(int excepts);

/// Sets floating-point exception flags from representation flagp.
int fesetexceptflag(const fexcept_t *flagp, int excepts);

/// Determines which of the specified exception flags are currently set.
int fetestexcept(int excepts);

//===----------------------------------------------------------------------===//
// Floating-Point Rounding Control Functions
//===----------------------------------------------------------------------===//

/// Gets current floating-point rounding direction macro.
int fegetround(void);

/// Sets current floating-point rounding direction.
int fesetround(int round);

//===----------------------------------------------------------------------===//
// Floating-Point Environment Control Functions
//===----------------------------------------------------------------------===//

/// Stores current floating-point environment in object envp.
int fegetenv(fenv_t *envp);

/// Saves current environment, clears all exceptions, and enters nonstop mode.
int feholdexcept(fenv_t *envp);

/// Establishes floating-point environment represented by object envp.
int fesetenv(const fenv_t *envp);

/// Restores previously saved environment and raises saved exceptions.
int feupdateenv(const fenv_t *envp);

#ifdef __cplusplus
}
#endif

#endif // _FENV_H
