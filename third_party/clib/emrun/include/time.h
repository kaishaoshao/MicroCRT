/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_TIME_H
#define __SEGGER_RTL_TIME_H

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL.h"
#include "sys/time.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#ifndef __SEGGER_RTL_SIZE_T_DEFINED
#define __SEGGER_RTL_SIZE_T_DEFINED
typedef __SEGGER_RTL_SIZE_T size_t;
#endif

#ifndef __SEGGER_RTL_LOCALE_T_DEFINED
#define __SEGGER_RTL_LOCALE_T_DEFINED
typedef struct __SEGGER_RTL_POSIX_locale_s *locale_t;
#endif

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TIME_UTC
#define TIME_UTC 1
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef long clock_t;
typedef long time_t;

struct tm {
  int tm_sec;     // seconds after the minute - [0, 59]
  int tm_min;     // minutes after the hour - [0, 59]
  int tm_hour;    // hours since midnight - [0, 23]
  int tm_mday;    // day of the month - [1, 31]
  int tm_mon;     // months since January - [0, 11]
  int tm_year;    // years since 1900
  int tm_wday;    // days since Sunday - [0, 6]
  int tm_yday;    // days since January 1 - [0, 365]
  int tm_isdst;   // daylight savings time flag
};

#ifndef __TIMESPEC_DEFINED
#define __TIMESPEC_DEFINED
struct timespec
{
  time_t tv_sec; // whole seconds
  long tv_nsec;  // nanoseconds
};
#endif

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
struct timezone;
int         gettimeofday (struct timeval *__tp, void *__tzp);
int         settimeofday (const struct timeval *__tp, const struct timezone *__tzp);
clock_t     clock        (void);
time_t      time         (time_t *__tp);
double      difftime     (time_t __time2, time_t __time1);
time_t      mktime       (struct tm *__tp);
char      * asctime      (const struct tm *__tp);
char      * asctime_r    (const struct tm *__tp, char *__buf);
char      * ctime        (const time_t *__tp);
char      * ctime_r      (const time_t *__tp, char *__buf);
struct tm * gmtime       (const time_t *__tp);
struct tm * gmtime_r     (const time_t *__tp, struct tm *__result);
struct tm * localtime    (const time_t *__tp);
struct tm * localtime_r  (const time_t *__tp, struct tm *__result);
size_t      strftime     (char *__s, size_t __smax, const char *__fmt, const struct tm *__tp);
size_t      strftime_l   (char *__s, size_t __smax, const char *__fmt, const struct tm *__tp, locale_t __loc);
int         timespec_get (struct timespec *__ts, int __base);

#define CLOCK_REALTIME ((clockid_t) 1)
#define CLOCK_PROCESS_CPUTIME_ID ((clockid_t) 2)
#define CLOCK_THREAD_CPUTIME_ID	((clockid_t) 3)
#define CLOCK_MONOTONIC	 ((clockid_t) 4)
#define TIMER_ABSTIME 4

struct itimerspec {
  struct timespec it_interval;
  struct timespec it_value;
};

struct sigevent;

typedef int pid_t;
typedef unsigned long clockid_t;
typedef unsigned long timer_t;

// These aren't implemented but are here to enable code to compile
int clock_getcpuclockid (pid_t pid, clockid_t *clock_id);
int clock_getres (clockid_t clock_id, struct timespec *res);
int clock_gettime (clockid_t clock_id, struct timespec *tp);
int clock_nanosleep (clockid_t clock_id, int flags, const struct timespec *rqtp, struct timespec *rmtp);
int clock_settime (clockid_t clock_id, const struct timespec *tp);
int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *timerid);
int timer_delete(timer_t timerid);
int timer_gettime(timer_t timerid, struct itimerspec *value);
int timer_getoverrun(timer_t timerid);
int timer_settime(timer_t timerid, int, const struct itimerspec *value, struct itimerspec *ovalue);

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
