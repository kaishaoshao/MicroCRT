//===-- include/libc/time.h - MicroCRT Time and Date Handling ---*- C -*-===//
//
// Part of the MicroCRT Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares date and time handling functions, clock operations,
/// and calendar formatting utilities.
///
//===----------------------------------------------------------------------===//

#ifndef __TIME_H__
#define __TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

//===----------------------------------------------------------------------===//
// Standard Types and Clock Constants
//===----------------------------------------------------------------------===//

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
/// Unsigned integer type resulting from the sizeof operator.
typedef unsigned long size_t;
#endif

/// Null pointer constant.
#undef NULL
#define NULL ((void *)0)

/// Type representing calendar time in seconds since the Epoch.
typedef long time_t;

/// Processor clock ticks count type.
typedef long clock_t;

/// Number of processor clock ticks per second returned by clock().
#define CLOCKS_PER_SEC 1000000L

/// Structure holding broken-down calendar time components.
struct tm {
  int tm_sec;   ///< Seconds [0, 60] (allowing 1 leap second).
  int tm_min;   ///< Minutes [0, 59].
  int tm_hour;  ///< Hours since midnight [0, 23].
  int tm_mday;  ///< Day of the month [1, 31].
  int tm_mon;   ///< Months since January [0, 11].
  int tm_year;  ///< Years since 1900.
  int tm_wday;  ///< Days since Sunday [0, 6].
  int tm_yday;  ///< Days since January 1 [0, 365].
  int tm_isdst; ///< Daylight Saving Time flag (>0 DST, 0 no, <0 unknown).
};

//===----------------------------------------------------------------------===//
// Time Manipulation Functions
//===----------------------------------------------------------------------===//

/// Returns the approximate processor time used by the process.
clock_t clock(void);

/// Returns the current calendar time in seconds since the Epoch.
time_t time(time_t *timer);

/// Computes the difference in seconds between two calendar times (time1 - time0).
double difftime(time_t time1, time_t time0);

/// Converts a broken-down time structure into a calendar time value.
time_t mktime(struct tm *timeptr);

//===----------------------------------------------------------------------===//
// Time Conversion and Formatting Functions
//===----------------------------------------------------------------------===//

/// Converts broken-down time into a 26-character textual string.
char *asctime(const struct tm *timeptr);

/// Converts calendar time into a textual string representing local time.
char *ctime(const time_t *timer);

/// Converts calendar time to UTC broken-down time format.
struct tm *gmtime(const time_t *timer);

/// Converts calendar time to local broken-down time format.
struct tm *localtime(const time_t *timer);

/// Formats broken-down time into string s according to format.
size_t strftime(char *s, size_t maxsize, const char *format,
                const struct tm *timeptr);

#ifdef __cplusplus
}
#endif

#endif // __TIME_H__
