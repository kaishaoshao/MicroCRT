//===-- include/libc/stdbool.h - MicroCRT Boolean Type Support --*- C -*-===//
//
// Part of the MicroCRT Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines standard boolean macros compatible with ISO C99.
///
//===----------------------------------------------------------------------===//

#ifndef __STDBOOL_H__
#define __STDBOOL_H__

#ifndef __cplusplus

/// Standard boolean data type.
#define bool _Bool
/// Boolean true constant value.
#define true 1
/// Boolean false constant value.
#define false 0

#endif // __cplusplus

/// Conformance macro indicating bool, true, and false are defined.
#define __bool_true_false_are_defined 1

#endif // __STDBOOL_H__
