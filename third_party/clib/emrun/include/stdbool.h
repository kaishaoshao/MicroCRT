/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_STDBOOL_H
#define __SEGGER_RTL_STDBOOL_H

#ifndef __cplusplus

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       bool
*
*  Description
*    Macros expanding to support the Boolean type.
*/
#define bool _Bool    // Underlying boolean type
#define true  1       // Boolean true value
#define false 0       // Boolean false value

/*********************************************************************
*
*       __bool_true_false_are_defined 
*
*  Description
*    This is required to be here for the C standard.
*/
#define __bool_true_false_are_defined 1

#endif

#endif

/*************************** End of file ****************************/
