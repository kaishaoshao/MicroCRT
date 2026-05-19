/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL_Int.h"
#include "time.h"
#include "stdlib.h"
#include "wchar.h"
#include "limits.h"
#include "locale.h"
#include "string.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define SECSPERMIN	60L
#define MINSPERHOUR	60L
#define HOURSPERDAY	24L
#define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY	(SECSPERHOUR * HOURSPERDAY)
#define DAYSPERWEEK	7
#define MONSPERYEAR	12

#define EPOCH_YEAR      1970
#define EPOCH_WDAY      4

#define ISLEAP(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

#define CHECKVALUE(N, U, V) \
  if (tp->U < 0 || tp->U > N-1) \
    { \
      div_t l = div(tp->U, N); \
      tp->U = l.rem; \
      tp->V += l.quot; \
      if (tp->U < 0) \
        { \
          tp->U += N; \
          tp->V--; \
        } \
    }

#if __SEGGER_RTL_MINIMAL_LOCALE
  #define SELECT(M, F) M
#else
  #define SELECT(M, F) F
#endif

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/

static const unsigned char __SEGGER_RTL_mon_lengths[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static const short __SEGGER_RTL_year_lengths[2] = {
  365,
  366
};

static const char __SEGGER_RTL_unknown_string[] = "<?>";

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static char      __SEGGER_RTL_asctime_buf[28];
static struct tm __SEGGER_RTL_tm;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_checktm()
*
*/
static void __SEGGER_RTL_checktm(struct tm *tp) {
  const unsigned char *ip;
  long year;
  CHECKVALUE(60, tm_sec, tm_min);
  CHECKVALUE(60, tm_min, tm_hour);
  CHECKVALUE(24, tm_hour, tm_mday);
  CHECKVALUE(12, tm_mon, tm_year);
  year = tp->tm_year + 1900;
  ip = __SEGGER_RTL_mon_lengths[ISLEAP(year)];
  if (tp->tm_mday <= 0)
  {
    while (tp->tm_mday <= 0)
    {
      if (tp->tm_mon == 0)
      {
        tp->tm_year--;
        year--;
        ip = __SEGGER_RTL_mon_lengths[ISLEAP(year)];
        tp->tm_mon = 11;
      }
      else
        tp->tm_mon--;
      tp->tm_mday += ip[tp->tm_mon];
    }
  }
  else
  {
    while (tp->tm_mday > ip[tp->tm_mon])
    {
      tp->tm_mday -= ip[tp->tm_mon];
      if (tp->tm_mon == 11)
      {
        tp->tm_year++;
        year++;
        ip = __SEGGER_RTL_mon_lengths[ISLEAP(year)];
        tp->tm_mon = 0;
      }
      else
        tp->tm_mon++;
    }
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_put_ch()
*
*/
static void __SEGGER_RTL_put_ch(__SEGGER_RTL_time_state_t *state, int i) {
  if (state->smax > 1) {
    state->smax--;
    state->buf[0] = i;
    state->buf++;
  } else {
    state->smax = 0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_put_str()
*
*/
static void __SEGGER_RTL_put_str(__SEGGER_RTL_time_state_t *state, const char *str) {
  while (*str) {
    __SEGGER_RTL_put_ch(state, *str++);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_put_digit()
*
*/
static void __SEGGER_RTL_put_digit(__SEGGER_RTL_time_state_t *state, unsigned i) {
  __SEGGER_RTL_put_ch(state, i%10 + '0');
}

/*********************************************************************
*
*       __SEGGER_RTL_put_twodigit()
*
*/
static void __SEGGER_RTL_put_twodigit(__SEGGER_RTL_time_state_t *state, unsigned i) {
  __SEGGER_RTL_put_ch(state, i/10 + '0');
  __SEGGER_RTL_put_ch(state, i%10 + '0');
}

/*********************************************************************
*
*       __SEGGER_RTL_put_twodigits_leading_blank()
*
*/
static void __SEGGER_RTL_put_twodigits_leading_blank(__SEGGER_RTL_time_state_t *state, unsigned i) {
  if (i < 10) {
    __SEGGER_RTL_put_ch(state, ' ');
    __SEGGER_RTL_put_ch(state, i + '0');
  } else {
    __SEGGER_RTL_put_twodigit(state, i);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_put_twodigit_optional_zero_suppress()
*
*/
static void __SEGGER_RTL_put_twodigit_optional_zero_suppress(__SEGGER_RTL_time_state_t *state, int hash, unsigned i) {
  if (hash && i < 10) {
    __SEGGER_RTL_put_ch(state, i + '0');
  } else {
    __SEGGER_RTL_put_twodigit(state, i);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_put_threedigit()
*
*/
static void __SEGGER_RTL_put_threedigit(__SEGGER_RTL_time_state_t *state, int hash, unsigned i) {
  if (hash && i < 10) {
    __SEGGER_RTL_put_ch(state, i + '0');
  } else if (hash && i < 100) {
    __SEGGER_RTL_put_twodigit(state, i);
  } else {
    __SEGGER_RTL_put_digit(state, i/100);
    __SEGGER_RTL_put_twodigit(state, i % 100);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_put_wide_char()
*
*/
static int __SEGGER_RTL_put_wide_char(__SEGGER_RTL_time_state_t *state, int wc) {
  char buf[MB_LEN_MAX];
  int  n;
  //
  // Convert to locale's encoding.
  //
  n = wctomb(buf, wc);
  //
  // If we can't encode this or there's not enough space,
  // we're through with it.
  //
  if (n < 0 || (size_t)n > state->smax) {
    return 0;
  }
  //
  // Copy over converted character.
  //
  (memcpy)(state->buf, buf, n);
  //
  // Advance.
  //
  state->buf += n;
  state->smax -= n;
  //
  // And done.
  //
  return n;
}

/*********************************************************************
*
*       __SEGGER_RTL_put_fourdigit()
*
*/
static void __SEGGER_RTL_put_fourdigit(__SEGGER_RTL_time_state_t *state, unsigned i) {
  __SEGGER_RTL_put_twodigit(state, i / 100);
  __SEGGER_RTL_put_twodigit(state, i % 100);
}

/*********************************************************************
*
*       __SEGGER_RTL_put_utf8_str()
*
*/
static void __SEGGER_RTL_put_utf8_str(__SEGGER_RTL_time_state_t *state, const char *str) {
  mbstate_t utf8_mbstate;
  wchar_t   wc;
  size_t    len;
  //
  // Append the UTF-8-encoded string 'str'.
  //
  // Initialize states.
  //
  __SEGGER_RTL_init_mbstate(&utf8_mbstate);
  len = (strlen)(str);
  //
  // Copy characters across.
  //
  while (len > 0) {
    //
    // Get one UTF-8-encoded source character.
    //
    int n = __SEGGER_RTL_utf8_mbtowc(&wc, str, len, &utf8_mbstate);
    //
    // Source conversion error?  Not good, either we screwed up in our library
    // or the locale provided by the user is duff.
    //
    if (n < 0) {
      return;
    }
    //
    // Append character.
    //
    if (wc == 0) {
      return;
    }
    if (__SEGGER_RTL_put_wide_char(state, wc) == 0) {
      return;
    }
    //
    // Move along.
    //
    str += n;
    len -= n;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_put_formatted()
*
*/
static void __SEGGER_RTL_put_formatted(__SEGGER_RTL_time_state_t *state, const char *fmt, const struct tm *tp, const __SEGGER_RTL_locale_t *locale) {
  //
  int          hash;
  wchar_t      wc;
  int          n;
  const char * end;
  //
  // Need string length when decoding.
  //
  end = fmt + (strlen)(fmt);
  //
  while (state->smax && fmt < end) {
    //
    // Grab a multi-byte character.
    //
#if __SEGGER_RTL_MINIMAL_LOCALE
    wc = *fmt;
    n = wc != 0;
#else
    n = locale->codeset->__mbtowc(&wc, fmt, end-fmt, &state->mbstate);
    if (n < 0) {
      //
      // Multi-byte decode error.  ISO Standard doesn't really
      // say what to do here, so just skip one character--the alternative
      // is to immediately return.  Perhaps we should do that.
      // ENHANCEMENT: Check what glibc does about it?
      //
      n = 1; wc = 0;
    }
#endif
    //
    // Advance over character.
    //
    fmt += n;
    //
    // Take care of non-meta character.
    //
    if (wc != '%') {
      if (__SEGGER_RTL_put_wide_char(state, wc) == 0) {
        return;
      }
    } else {
      hash = 0;
      if (*fmt == 'E' || *fmt == 'O') {
        //
        // Ignore 'E' and 'O' modifiers in all locales.
        // We just don't have enough available for this, I think.
        //
        ++fmt;
      } else if (*fmt == '#') {
        hash = 1;
        ++fmt;
      }
      //
      switch (*fmt++) {
      case 0:
        --fmt;
        break;
        //
      case '%': // Percent sign
        __SEGGER_RTL_put_ch(state, '%');
        break;
        //
      case 'a': // Abbreviated weekday name
        __SEGGER_RTL_put_str(state, __SEGGER_RTL_string_list_decode(SELECT(__SEGGER_RTL_c_locale_abbrev_day_names, locale->data->abbrev_day_names), tp->tm_wday));
        break;
        //
      case 'A': // Full weekday name
        __SEGGER_RTL_put_str(state, __SEGGER_RTL_string_list_decode(SELECT(__SEGGER_RTL_c_locale_day_names, locale->data->day_names), tp->tm_wday));
        break;
        //
      case 'b': // Abbreviated month name
      case 'h': // As 'b'
        __SEGGER_RTL_put_utf8_str(state, __SEGGER_RTL_string_list_decode(SELECT(__SEGGER_RTL_c_locale_abbrev_month_names, locale->data->abbrev_month_names), tp->tm_mon));
        break;
        //
      case 'B': // Full month name
        __SEGGER_RTL_put_str(state, __SEGGER_RTL_string_list_decode(SELECT(__SEGGER_RTL_c_locale_month_names, locale->data->month_names), tp->tm_mon));
        break;
        //
      case 'c': // Date and time representation appropriate for locale
        if (hash) { // Long date and time
          // Microsoft-style long time
          __SEGGER_RTL_put_formatted(state, "%A, %B %#d, %Y, %H:%M:%S", tp, locale);
        } else {
          __SEGGER_RTL_put_formatted(state, SELECT(__SEGGER_RTL_c_locale_date_time_format, locale->data->date_time_format), tp, locale);
        }
        break;
        //
      case 'C': // Century number
        __SEGGER_RTL_put_twodigit_optional_zero_suppress(state, hash, (tp->tm_year+1900) / 1000);
        break;
        //
      case 'd': // Day of month as decimal number [01,31]
        __SEGGER_RTL_put_twodigit_optional_zero_suppress(state, hash, tp->tm_mday);
        break;
        //
      case 'D': // Date as %m/%d/%y - POSIX.2
        __SEGGER_RTL_put_formatted(state, "%m/%d/%y", tp, &__SEGGER_RTL_c_locale);
        break;
        //
      case 'e': // Day of month [ 1,31], single digit preceded by space
        __SEGGER_RTL_put_twodigits_leading_blank(state, tp->tm_mday);
        break;
        //
      case 'F': // %Y-%m-%d
        __SEGGER_RTL_put_formatted(state, "%Y-%m-%d", tp, locale);
        break;
        //
#if 0
      case 'g': // Week-based year within century [00,99]
        break;
      case 'G': // Week-based year, including the century [0000,9999]
        break;
#endif
        //
      case 'H': // Hour in 24-hour format [00,23]
        __SEGGER_RTL_put_twodigit_optional_zero_suppress(state, hash, tp->tm_hour);
        break;
        //
      case 'I': // Hour in 12-hour format [01,12]
        {
          int h = tp->tm_hour;
          h %= 12;
          if (h == 0) {
            h = 12;
          }
          __SEGGER_RTL_put_twodigit_optional_zero_suppress(state, hash, h);
        }
        break;

      case 'j': // Day of year as decimal number [001-366]
        __SEGGER_RTL_put_threedigit(state, hash, tp->tm_yday+1);
        break;
        //
      case 'k': // Hour (24-hour clock) [ 0,23] - POSIX.2
        __SEGGER_RTL_put_twodigits_leading_blank(state, tp->tm_hour);
        break;
        //
      case 'l': // Hour (12-hour clock) [ 0,12]
        if (tp->tm_hour <= 12) {
          __SEGGER_RTL_put_twodigits_leading_blank(state, tp->tm_hour);
        } else {
          __SEGGER_RTL_put_twodigits_leading_blank(state, tp->tm_hour-12);
        }
        break;
        //
      case 'm': // Month as decimal number [01,12]
        __SEGGER_RTL_put_twodigit_optional_zero_suppress(state, hash, tp->tm_mon+1);
        break;
        //
      case 'M': // Minute as decimal number [00,59]
        __SEGGER_RTL_put_twodigit_optional_zero_suppress(state, hash, tp->tm_min);
        break;
        //
      case 'n': // Insert a newline - POSIX.2
        __SEGGER_RTL_put_ch(state, '\n');
        break;
        //
      case 'p': // Current locale's A.M./P.M. indicator for 12-hour clock
        __SEGGER_RTL_put_str(state, __SEGGER_RTL_string_list_decode(SELECT(__SEGGER_RTL_c_locale_am_pm_indicator, locale->data->am_pm_indicator), tp->tm_hour >= 12));
        break;
        //
      case 'r': // Equivalent to %I:%M:%s %p - POSIX.2
        __SEGGER_RTL_put_formatted(state, "%I:%M:%s %p", tp, locale);
        break;
        //
      case 'R': // Time as %H:%M - POSIX.2
        __SEGGER_RTL_put_formatted(state, "%H:%M", tp, locale);
        break;
        //
      case 'S': // Second as decimal number (00-59)
        __SEGGER_RTL_put_twodigit_optional_zero_suppress(state, hash, tp->tm_sec);
        break;
        //
      case 't': // Insert Tab - POSIX.2
        __SEGGER_RTL_put_ch(state, '\t');
        break;
        //
      case 'T': // Time as %H:%M:%S
        __SEGGER_RTL_put_formatted(state, "%H:%M:%S", tp, locale);
        break;
        //
      case 'U': // Week of year as decimal number, with Sunday as first day of week (00-53)
        __SEGGER_RTL_put_twodigit_optional_zero_suppress(state, hash, (tp->tm_yday + 7 - tp->tm_wday) / 7);
        break;
        //
#if 0
      case 'V': // The ISO 8601 week number as a decimal number [01,53]
        break;
#endif
        //
      case 'w': // Weekday as decimal number (0-6; Sunday is 0)
        __SEGGER_RTL_put_digit(state, tp->tm_wday);
        break;
        //
      case 'W': // Week of year as decimal number, with Monday as first day of week (00-53)
        {
          int w = (tp->tm_wday) ? tp->tm_wday - 1 : 6;                
          __SEGGER_RTL_put_twodigit_optional_zero_suppress(state, hash, (tp->tm_yday + 7 - w) / 7);
        }
        break;
        //
      case 'x': // Date representation for current locale
        if (hash) { // Long date representation
          __SEGGER_RTL_put_formatted(state, "%A, %B %#d, %Y", tp, locale);
        } else {
          __SEGGER_RTL_put_formatted(state, SELECT(__SEGGER_RTL_c_locale_date_format, locale->data->date_format), tp, locale);
        }
        break;
        //
      case 'X': // Time representation for current locale, equivalent to %T in C locale
        __SEGGER_RTL_put_formatted(state, SELECT(__SEGGER_RTL_c_locale_time_format, locale->data->time_format), tp, locale);
        break;
        //
      case 'y': // Year without century, as decimal number (00-99)
        __SEGGER_RTL_put_twodigit_optional_zero_suppress(state, hash, tp->tm_year % 100);
        break;
        //
      case 'Y': // Year with century, as decimal number
        __SEGGER_RTL_put_fourdigit(state, 1900 + tp->tm_year);
        break;
        //
      case 'z': 
      case 'Z': // Either the time-zone name or time zone abbreviation; no characters if time zone is unknown
        break;
      }
    }
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_twodigit()
*
*/
static char * __SEGGER_RTL_twodigit(char *b, unsigned i) {
  *b++ = (i/10)+'0';
  *b++ = (i%10)+'0';
  return b;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_string_list_encode()
*
*/
int __SEGGER_RTL_string_list_encode(const char *list, const char *str) {
  int index;
  //
  index = 0;
  for (;;) {
    size_t n;
    //
    // Matched this entry?
    //
    if ((strcmp)(list, str) == 0) {
      return index;
    }
    //
    // If final entry, didn't find it so return -1.
    //
    n = (strlen)(str);
    if (n == 0) {
      return -1;
    }
    //
    // Skip to next string.
    //
    str += n + 1;
    ++index;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_string_list_decode()
*
*/
const char * __SEGGER_RTL_string_list_decode(const char *str, int index) {
  size_t n;
  //
  for (;;) {
    if (index == 0) {
      return str;
    }
    n = (strlen)(str);
    if (n == 0 || index < 0) {
      return __SEGGER_RTL_unknown_string;
    }
    str += n + 1;
    --index;
  }
}

/*********************************************************************
*
*       time()
*
*/
time_t __SEGGER_RTL_PUBLIC_API time(time_t *timer) {
  struct timeval tv;
  //
  // Request time of day.
  //
  if (gettimeofday(&tv, 0) < 0) {
    //
    // Didn't get one.
    //
    tv.tv_sec = (time_t)(-1);
  }
  //
  if (timer) {
    *timer = tv.tv_sec;
  }
  //
  return tv.tv_sec;
}

/*********************************************************************
*
*       mktime()
*
*  Function description
*    Convert a struct tm to time_t.
*
*  Parameters
*    tp - Pointer to time object.
*
*  Return value
*    Number of seconds since UTC 1 January 1970 of the validated object.
*
*  Additional information
*    Validates (and updates) the object pointed to by tp to ensure that the
*    tm_sec, tm_min, tm_hour, and tm_mon fields are within the supported
*    integer ranges and the tm_mday, tm_mon and tm_year fields are
*    consistent. The validated object is converted to the number of seconds
*    since UTC 1 January 1970 and returned.
*
*  Thread safety
*    Safe.
*/
time_t __SEGGER_RTL_PUBLIC_API mktime(struct tm *tp) {
  time_t tim;
  long days, year;
  int i;
  const unsigned char *ip;
  //
  __SEGGER_RTL_checktm(tp);
  //
  year = tp->tm_year + 1900;
  ip = __SEGGER_RTL_mon_lengths[ISLEAP(year)];
  days = tp->tm_mday - 1;
  for (i = 0; i < tp->tm_mon; i++)
    days += ip[i];        
  tp->tm_yday = days;
  //
  if (year > EPOCH_YEAR) {
    for (i = EPOCH_YEAR; i < year; i++) {
      days += __SEGGER_RTL_year_lengths[ISLEAP(i)];
    }
  } else {
    for (i = EPOCH_YEAR-1; i >= year; i--) {
      days -= __SEGGER_RTL_year_lengths[ISLEAP(i)];
    }
  }
  //
  tp->tm_wday = (EPOCH_WDAY + days) % DAYSPERWEEK;
  if (tp->tm_wday < 0) {
    tp->tm_wday += DAYSPERWEEK;
  }
  //
  tim = tp->tm_sec + (tp->tm_min * SECSPERMIN) + (tp->tm_hour * SECSPERHOUR) + (days * SECSPERDAY);
  //
  return tim;
}

/*********************************************************************
*
*       difftime()
*
*  Function description
*    Calculate difference between two times.
*
*  Parameters
*    time2 - End time.
*    time1 - Start time.
*
*  Return value
*    returns time2-time1 as a double precision number.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API difftime(time_t time2, time_t time1) {
  return time2-time1;
}

/*********************************************************************
*
*       localtime_r()
*
*  Function description
*    Convert time to local time, reentrant.
*
*  Parameters
*    tp - Pointer to time to convert.
*    tm - Pointer to object that receives the converted local time.
*
*  Return value
*    Returns tm.
*
*  Additional information
*    Converts the time pointed to by tp to local time format and
*    writes it to the object pointed to by tm.
*
*  Thread safety
*    Safe.
*/
struct tm * __SEGGER_RTL_PUBLIC_API localtime_r(const time_t *tp, struct tm *tm) {
  const unsigned char  * ip;
  ldiv_t                 l;  
  long                   days, rem, year;
  int                    isleap;
  //
  l = ldiv(*tp, SECSPERDAY);
  days = l.quot;
  rem = l.rem;
  if (rem < 0) {
    rem += SECSPERDAY;
    --days;
  }
  //
  l = ldiv(rem, SECSPERHOUR);
  tm->tm_hour = l.quot;
  rem = l.rem;
  //
  l = ldiv(rem, SECSPERMIN);
  tm->tm_min = l.quot;
  tm->tm_sec = l.rem;
  //
  tm->tm_wday = (EPOCH_WDAY + days) % DAYSPERWEEK;
  if (tm->tm_wday < 0) {
    tm->tm_wday += DAYSPERWEEK;
  }
  //
  year = EPOCH_YEAR;
  if (days >=0) {
    for (;;) {     
      isleap = ISLEAP(year);
      if (days < __SEGGER_RTL_year_lengths[isleap])
        break;      
      days -= __SEGGER_RTL_year_lengths[isleap];
      year++;
    }
  } else {
    do {
      year--;
      isleap = ISLEAP(year);
      days += __SEGGER_RTL_year_lengths[isleap];        
    }  while (days < 0);
  }
  tm->tm_year = year-1900;
  tm->tm_yday = days;  
  //
  ip = __SEGGER_RTL_mon_lengths[isleap];
  for (tm->tm_mon = 0; days >= ip[tm->tm_mon]; tm->tm_mon++) {
    days -= ip[tm->tm_mon];
  }
  //
  tm->tm_mday = days + 1;
  return tm;
}

/*********************************************************************
*
*       localtime()
*
*  Function description
*    Convert time to local time.
*
*  Parameters
*    tp - Pointer to time to convert.
*
*  Return value
*    Pointer to a statically-allocated object holding the
*    local time.
*
*  Additional information
*    Converts the time pointed to by tp to local time format.
*
*  Notes
*    The returned pointer points to a static object: this function
*    is not thread safe.
*
*  Thread safety
*    Unsafe.
*/
struct tm * __SEGGER_RTL_PUBLIC_API localtime(const time_t *tp) {  
  localtime_r(tp, &__SEGGER_RTL_tm);
  return &__SEGGER_RTL_tm;
}

/*********************************************************************
*
*       gmtime_r()
*
*  Function description
*    Convert time_t to struct tm, reentrant.
*
*  Parameters
*    tp - Pointer to time to convert.
*    tm - Pointer to object that receives the converted time.
*
*  Return value
*    Returns tm.
*
*  Additional information
*    Converts the time pointed to by tp to a struct tm.
*
*  Thread safety
*    Safe.
*/
struct tm * __SEGGER_RTL_PUBLIC_API gmtime_r(const time_t *tp, struct tm *tm) {
  localtime_r(tp, tm);
  return tm;
}

/*********************************************************************
*
*       gmtime()
*
*  Function description
*    Convert time_t to struct tm.
*
*  Parameters
*    tp - Pointer to time to convert.
*
*  Return value
*    Pointer to converted time.
*
*  Additional information
*    Converts the time pointed to by tp to a struct tm.
*
*  Notes
*    The returned pointer points to a static buffer: this function
*    is not thread safe.
*
*  Thread safety
*    Unsafe.
*/
struct tm * __SEGGER_RTL_PUBLIC_API gmtime(const time_t *tp) {
  localtime_r(tp, &__SEGGER_RTL_tm);
  return &__SEGGER_RTL_tm;
}

/*********************************************************************
*
*       ctime_r()
*
*  Function description
*    Convert time_t to a string, reentrant.
*
*  Parameters
*    tp  - Pointer to time to convert.
*    buf - Pointer to array of characters that receives the
*          zero-terminated string; the array must be at least
*          26 characters in length.
*
*  Return value
*    Returns the value of buf.
*
*  Additional information
*    Converts the time pointed to by tp to a null-terminated string.
*
*  Notes
*    The returned string is held in a static buffer: this function
*    is not thread safe.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API ctime_r(const time_t *tp, char *buf) {
  struct tm _tm;
  //
  localtime_r(tp, &_tm);
  return asctime_r(&_tm, buf);
}

/*********************************************************************
*
*       ctime()
*
*  Function description
*    Convert time_t to a string.
*
*  Parameters
*    tp - Pointer to time to convert.
*
*  Return value
*    Pointer to zero-terminated converted string.
*
*  Additional information
*    Converts the time pointed to by tp to a null-terminated string.
*
*  Notes
*    The returned string is held in a static buffer: this function
*    is not thread safe.
*
*  Thread safety
*    Unsafe.
*/
char * __SEGGER_RTL_PUBLIC_API ctime(const time_t *tp) {
  localtime_r(tp, &__SEGGER_RTL_tm);
  return asctime(&__SEGGER_RTL_tm);
}

/*********************************************************************
*
*       asctime_r()
*
*  Function description
*    Convert time_t to a string, reentrant.
*
*  Parameters
*    tp  - Pointer to time to convert.
*    buf - Pointer to array of characters that receives the
*          zero-terminated string; the array must be at least
*          26 characters in length.
*
*  Return value
*    Returns the value of buf.
*
*  Additional information
*    Converts the time pointed to by tp to a null-terminated string
*    of the \tt{Sun Sep 16 01:03:52 1973}. The converted string is
*    written into the array pointed to by buf.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API asctime_r(const struct tm *tp, char *buf) {  
  int year;
  char *b = buf;
  //
  b = mempcpy(b, __SEGGER_RTL_string_list_decode(__SEGGER_RTL_c_locale_abbrev_day_names, tp->tm_wday), 3);
  *b++ = ' ';
  b = mempcpy(b, __SEGGER_RTL_string_list_decode(__SEGGER_RTL_c_locale_abbrev_month_names, tp->tm_mon), 3);
  *b++ = ' ';
  b = __SEGGER_RTL_twodigit(b, tp->tm_mday);
  *b++ = ' ';
  b = __SEGGER_RTL_twodigit(b, tp->tm_hour);
  *b++ = ':';
  b = __SEGGER_RTL_twodigit(b, tp->tm_min);
  *b++ = ':';
  b = __SEGGER_RTL_twodigit(b, tp->tm_sec);
  *b++ = ' ';
  year = 1900 + tp->tm_year;
  b = __SEGGER_RTL_twodigit(b, year / 100);
  b = __SEGGER_RTL_twodigit(b, year % 100);
  *b++ = '\n';
  *b = 0;
  //
  return buf;
}

/*********************************************************************
*
*       asctime()
*
*  Function description
*    Convert time_t to a string.
*
*  Parameters
*    tp - Pointer to time to convert.
*
*  Return value
*    Pointer to zero-terminated converted string.
*
*  Additional information
*    Converts the time pointed to by tp to a null-terminated string
*    of the form \tt{Sun Sep 16 01:03:52 1973}. The returned string is
*    held in a static buffer.
*
*  Notes
*    The returned string is held in a static buffer: this function
*    is not thread safe.
*
*  Thread safety
*    Unsafe.
*/
char * __SEGGER_RTL_PUBLIC_API asctime(const struct tm *tp) {
  return asctime_r(tp, __SEGGER_RTL_asctime_buf);
}

/*********************************************************************
*
*       strftime_l()
*
*  Function description
*    Convert time to a string.
*
*  Parameters
*    s    - Pointer to object that receives the converted string.
*    smax - Maximum number of characters written to the array pointed to by s.
*    fmt  - Pointer to zero-terminated format control string.
*    tp   - Pointer to time to convert.
*    loc  - Locale to use for conversion.
*
*  Return value
*    Returns the name of the current locale.
*
*  Additional information
*    Formats the time pointed to by tp to a null-terminated string of maximum
*    size smax-1 into the pointed to by *s based on the fmt format string and
*    using the locale loc.
*
*    The format string consists of conversion specifications and ordinary
*    characters. Conversion specifications start with a "%" character followed
*    by an optional "#" character.
*
*    See strftime() for a description of the format conversion specifications.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API strftime_l(char *s, size_t smax, const char *fmt, const struct tm *tp, locale_t loc) {
  __SEGGER_RTL_time_state_t state;
  //
#if __SEGGER_RTL_MINIMAL_LOCALE
  __SEGGER_RTL_USE_PARA(loc);
#endif
  //
  state.smax = smax;
  state.buf = s;
  __SEGGER_RTL_init_mbstate(&state.mbstate);
  __SEGGER_RTL_put_formatted(&state, fmt, tp, SELECT(0, loc->__category[LC_TIME]));
  if (state.smax) {
    state.buf[0] = 0; 
    return state.buf - s;
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       strftime()
*
*  Function description
*    Convert time to a string.
*
*  Parameters
*    s    - Pointer to object that receives the converted string.
*    smax - Maximum number of characters written to the array pointed to by s.
*    fmt  - Pointer to zero-terminated format control string.
*    tp   - Pointer to time to convert.
*
*  Return value
*    Returns the name of the current locale.
*
*  Additional information
*    Formats the time pointed to by tp to a null-terminated string of maximum
*    size smax-1 into the pointed to by *s based on the fmt format string.
*    The format string consists of conversion specifications and ordinary
*    characters. Conversion specifications start with a "%" character followed
*    by an optional "#" character.
*
*    The following conversion specifications are supported:
*    
*    +---------------+------------------------------------------------------------------------------------------------+
*    | Specification | Description                                                                                    |
*    +---------------+------------------------------------------------------------------------------------------------+
*    | %a            | Abbreviated weekday name                                                                       |
*    | %A            | Full weekday name                                                                              |
*    | %b            | Abbreviated month name                                                                         |
*    | %B            | Full month name                                                                                |
*    | %c            | Date and time representation appropriate for locale                                            |
*    | %#c           | Date and time formatted as "%A, %B %#d, %Y, %H:%M:%S" (Microsoft extension)                    |
*    | %C            | Century number                                                                                 |
*    | %d            | Day of month as a decimal number [01,31]                                                       |
*    | %#d           | Day of month without leading zero [1,31]                                                       |
*    | %D            | Date in the form %m/%d/%y (POSIX.1-2008 extension)                                             |
*    | %e            | Day of month [ 1,31], single digit preceded by space                                           |
*    | %F            | Date in the format %Y-%m-%d                                                                    |
*    | %h            | Abbreviated month name as %b                                                                   |
*    | %H            | Hour in 24-hour format [00,23]                                                                 |
*    | %#H           | Hour in 24-hour format without leading zeros [0,23]                                            |
*    | %I            | Hour in 12-hour format [01,12]                                                                 |
*    | %#I           | Hour in 12-hour format without leading zeros [1,12]                                            |
*    | %j            | Day of year as a decimal number [001,366]                                                      |
*    | %#j           | Day of year as a decimal number without leading zeros [1,366]                                  |
*    | %k            | Hour in 24-hour clock format [ 0,23] (POSIX.1-2008 extension)                                  |
*    | %l            | Hour in 12-hour clock format [ 0,12] (POSIX.1-2008 extension)                                  |
*    | %m            | Month as a decimal number [01,12]                                                              |
*    | %#m           | Month as a decimal number without leading zeros [1,12]                                         |
*    | %M            | Minute as a decimal number [00,59]                                                             |
*    | %#M           | Minute as a decimal number without leading zeros [0,59]                                        |
*    | %n            | Insert newline character (POSIX.1-2008 extension)                                              |
*    | %p            | Locale's a.m or p.m indicator for 12-hour clock                                                |
*    | %r            | Time as %I:%M:%s %p (POSIX.1-2008 extension)                                                   |
*    | %R            | Time as %H:%M (POSIX.1-2008 extension)                                                         |
*    | %S            | Second as a decimal number [00,59]                                                             |
*    | %t            | Insert tab character (POSIX.1-2008 extension)                                                  |
*    | %T            | Time as %H:%M:%S                                                                               |
*    | %#S           | Second as a decimal number without leading zeros [0,59]                                        |
*    | %U            | Week of year as a decimal number [00,53], Sunday is first day of the week                      |
*    | %#U           | Week of year as a decimal number without leading zeros [0,53], Sunday is first day of the week |
*    | %w            | Weekday as a decimal number [0,6], Sunday is 0                                                 |
*    | %W            | Week number as a decimal number [00,53], Monday is first day of the week                       |
*    | %#W           | Week number as a decimal number without leading zeros [0,53], Monday is first day of the week  |
*    | %x            | Locale's date representation                                                                   |
*    | %#x           | Locale's long date representation                                                              |
*    | %X            | Locale's time representation                                                                   |
*    | %y            | Year without century, as a decimal number [00,99]                                              |
*    | %#y           | Year without century, as a decimal number without leading zeros [0,99]                         |
*    | %Y            | Year with century, as decimal number                                                           |
*    | %z,%Z         | Timezone name or abbreviation                                                                  |
*    | %%            | %                                                                                              |
*    +---------------+------------------------------------------------------------------------------------------------+
*
*  Thread safety
*    Safe [if configured].
*/
size_t __SEGGER_RTL_PUBLIC_API strftime(char *s, size_t smax, const char *fmt, const struct tm *tp) {
  return strftime_l(s, smax, fmt, tp, SELECT(NULL, __SEGGER_RTL_current_locale()));
}

/*********************************************************************
*
*       gettimeofday()
*
*/
int __SEGGER_RTL_PUBLIC_API gettimeofday(struct timeval *tp, void *tzp) {
  __SEGGER_RTL_USE_PARA(tzp);
  //
  return __SEGGER_RTL_X_get_time_of_day(tp);
}

/*********************************************************************
*
*       settimeofday()
*
*/
int __SEGGER_RTL_PUBLIC_API settimeofday(const struct timeval *tp, const struct timezone *tzp) {
  __SEGGER_RTL_USE_PARA(tzp);
  //
  return __SEGGER_RTL_X_set_time_of_day(tp);
}

/*************************** End of file ****************************/
