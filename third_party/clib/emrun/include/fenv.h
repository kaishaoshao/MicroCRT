/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_FENV_H
#define __SEGGER_RTL_FENV_H

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define FE_TONEAREST       0
#define FE_TOWARDZERO      1
#define FE_DOWNWARD        2
#define FE_UPWARD          3

#define FE_DFL_ENV         &__SEGGER_RTL_default_fenv

#define FE_ALL_EXCEPT      0

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  int __control;
} fenv_t;

typedef struct {
  int __control;
} fexcept_t;

/*********************************************************************
*
*       External data
*
**********************************************************************
*/

extern const fenv_t __SEGGER_RTL_default_fenv;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

int feclearexcept   (int __excepts);
int feraiseexcept   (int __excepts);
int fetestexcept    (int __excepts);
int fegetexceptflag (fexcept_t *__flagp, int __excepts);
int fesetexceptflag (const fexcept_t *__flagp, int __excepts);
//
int fegetround      (void);
int fesetround      (int __round);
//
int fegetenv        (fenv_t *__envp);
int fesetenv        (const fenv_t *__envp);
int feholdexcept    (fenv_t *__envp);
int feupdateenv     (const fenv_t *__envp);

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
