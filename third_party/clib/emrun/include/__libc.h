/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/


#ifndef __libc_H
#define __libc_H

#ifdef __cplusplus
extern "C" {
#endif

void __heap_lock(void);
void __heap_unlock(void);

#ifdef __cplusplus
}
#endif

#endif

