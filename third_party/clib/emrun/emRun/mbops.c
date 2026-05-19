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
#include "wctype.h"
#include "wchar.h"
#include "locale.h"
#include "xlocale.h"
#include "stdlib.h"
#include "errno.h"
#include "limits.h"
#include "stdio.h"
#include "string.h"

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

// The global locale is writable.
__SEGGER_RTL_LOCALE_THREAD struct __SEGGER_RTL_POSIX_locale_s   __SEGGER_RTL_global_locale = {
  { &__SEGGER_RTL_c_locale,
    &__SEGGER_RTL_c_locale,
    &__SEGGER_RTL_c_locale,
    &__SEGGER_RTL_c_locale,
    &__SEGGER_RTL_c_locale
  }
};
__SEGGER_RTL_LOCALE_THREAD struct __SEGGER_RTL_POSIX_locale_s * __SEGGER_RTL_locale_ptr;
__SEGGER_RTL_LOCALE_THREAD char                               * __SEGGER_RTL_locale_name_buffer;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static __SEGGER_RTL_STATE_THREAD mbstate_t __SEGGER_RTL_wcsrtombs_mbstate;
static __SEGGER_RTL_STATE_THREAD mbstate_t __SEGGER_RTL_wctomb_mbstate;
static __SEGGER_RTL_STATE_THREAD mbstate_t __SEGGER_RTL_mbrlen_mbstate;
static __SEGGER_RTL_STATE_THREAD mbstate_t __SEGGER_RTL_mbrtowc_mbstate;
static __SEGGER_RTL_STATE_THREAD mbstate_t __SEGGER_RTL_mbtowc_mbstate;
static __SEGGER_RTL_STATE_THREAD mbstate_t __SEGGER_RTL_mbsrtowcs_mbstate;
#if !__SEGGER_RTL_MINIMAL_LOCALE
static __SEGGER_RTL_STATE_THREAD mbstate_t __SEGGER_RTL_wcrtomb_mbstate;
#endif

static struct lconv __SEGGER_RTL_lconv_data;

/*********************************************************************
*
*       Const data
*
**********************************************************************
*/

const char __SEGGER_RTL_c_locale_day_names[] = {
  "Sunday\0"
  "Monday\0"
  "Tuesday\0"
  "Wednesday\0"
  "Thursday\0"
  "Friday\0"
  "Saturday\0"
};

const char __SEGGER_RTL_c_locale_month_names[] = {
  "January\0"
  "February\0"
  "March\0"
  "April\0"
  "May\0"
  "June\0"
  "July\0"
  "August\0"
  "September\0"
  "October\0"
  "November\0"
  "December\0"
};

const char __SEGGER_RTL_c_locale_am_pm_indicator[] = { 
  "AM\0"
  "PM\0"
};

// Format for %x in strftime
const char __SEGGER_RTL_c_locale_date_format[] = {
  "%m/%d/%y"
};

// Format for %X in strftime
const char __SEGGER_RTL_c_locale_time_format[] = {
  "%H:%M:%S"
};

// Format for %c in strftime
const char __SEGGER_RTL_c_locale_date_time_format[] = {
  "%a %b %e %T %Y"
};

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/

// Must match __SEGGER_RTL_WT_TOLOWER and __SEGGER_RTL_WT_TOUPPER in __SEGGER_RTL_Int.h
static const char __SEGGER_RTL_wctrans_name_list[] = {
  "tolower\0"
  "toupper\0"
};

// Matches encoding above.
static const char __SEGGER_RTL_wctype_class_name_list[] = {
  "alnum\0"
  "alpha\0"
  "cntrl\0"
  "digit\0"
  "graph\0"
  "lower\0"
  "upper\0"
  "space\0"
  "print\0"
  "punct\0"
  "blank\0"
  "xdigit\0"
};

static const __SEGGER_RTL_unicode_map_bmp_range_t __SEGGER_RTL_unicode_bmp_toupper_range2_map[35] = {
  { 0x00F6, 0x00F8, 0x00D6 },
  { 0x0101, 0x012F, 0x0100 },
  { 0x0133, 0x0137, 0x0132 },
  { 0x013A, 0x0148, 0x0139 },
  { 0x014B, 0x0177, 0x014A },
  { 0x017A, 0x017E, 0x0179 },
  { 0x0183, 0x0185, 0x0182 },
  { 0x01A1, 0x01A5, 0x01A0 },
  { 0x01B4, 0x01B6, 0x01B3 },
  { 0x01CE, 0x01DC, 0x01CD },
  { 0x01DF, 0x01EF, 0x01DE },
  { 0x01F9, 0x021F, 0x01F8 },
  { 0x0223, 0x0233, 0x0222 },
  { 0x0247, 0x024F, 0x0246 },
  { 0x0371, 0x0373, 0x0370 },
  { 0x03D9, 0x03EF, 0x03D8 },
  { 0x0461, 0x0481, 0x0460 },
  { 0x048B, 0x04BF, 0x048A },
  { 0x04C2, 0x04CE, 0x04C1 },
  { 0x04D1, 0x0527, 0x04D0 },
  { 0x1E01, 0x1E95, 0x1E00 },
  { 0x1EA1, 0x1EFF, 0x1EA0 },
  { 0x1F51, 0x1F57, 0x1F59 },
  { 0x2C68, 0x2C6C, 0x2C67 },
  { 0x2C81, 0x2CE3, 0x2C80 },
  { 0x2CEC, 0x2CEE, 0x2CEB },
  { 0x2D25, 0x2D27, 0x10C5 },
  { 0xA641, 0xA66D, 0xA640 },
  { 0xA681, 0xA697, 0xA680 },
  { 0xA723, 0xA72F, 0xA722 },
  { 0xA733, 0xA76F, 0xA732 },
  { 0xA77A, 0xA77C, 0xA779 },
  { 0xA77F, 0xA787, 0xA77E },
  { 0xA791, 0xA793, 0xA790 },
  { 0xA7A1, 0xA7A9, 0xA7A0 },
};

static const __SEGGER_RTL_unicode_map_bmp_range_t __SEGGER_RTL_unicode_bmp_toupper_range1_map[37] = {
  { 0x0061, 0x007A, 0x0041 },
  { 0x00E0, 0x00F5, 0x00C0 },
  { 0x00F9, 0x00FE, 0x00D9 },
  { 0x023F, 0x0240, 0x2C7E },
  { 0x0256, 0x0257, 0x0189 },
  { 0x028A, 0x028B, 0x01B1 },
  { 0x037B, 0x037D, 0x03FD },
  { 0x03AD, 0x03AF, 0x0388 },
  { 0x03B1, 0x03C1, 0x0391 },
  { 0x03C3, 0x03CB, 0x03A3 },
  { 0x03CD, 0x03CE, 0x038E },
  { 0x0430, 0x044F, 0x0410 },
  { 0x0450, 0x045F, 0x0400 },
  { 0x0561, 0x0586, 0x0531 },
  { 0x1F00, 0x1F07, 0x1F08 },
  { 0x1F10, 0x1F15, 0x1F18 },
  { 0x1F20, 0x1F27, 0x1F28 },
  { 0x1F30, 0x1F37, 0x1F38 },
  { 0x1F40, 0x1F45, 0x1F48 },
  { 0x1F60, 0x1F67, 0x1F68 },
  { 0x1F70, 0x1F71, 0x1FBA },
  { 0x1F72, 0x1F75, 0x1FC8 },
  { 0x1F76, 0x1F77, 0x1FDA },
  { 0x1F78, 0x1F79, 0x1FF8 },
  { 0x1F7A, 0x1F7B, 0x1FEA },
  { 0x1F7C, 0x1F7D, 0x1FFA },
  { 0x1F80, 0x1F87, 0x1F88 },
  { 0x1F90, 0x1F97, 0x1F98 },
  { 0x1FA0, 0x1FA7, 0x1FA8 },
  { 0x1FB0, 0x1FB1, 0x1FB8 },
  { 0x1FD0, 0x1FD1, 0x1FD8 },
  { 0x1FE0, 0x1FE1, 0x1FE8 },
  { 0x2170, 0x217F, 0x2160 },
  { 0x24D0, 0x24E9, 0x24B6 },
  { 0x2C30, 0x2C5E, 0x2C00 },
  { 0x2D00, 0x2D24, 0x10A0 },
  { 0xFF41, 0xFF5A, 0xFF21 },
};

static const __SEGGER_RTL_unicode_map_bmp_singleton_t __SEGGER_RTL_unicode_bmp_toupper_singleton_map[90] = {
  { 0x00B5, 0x039C },
  { 0x00FF, 0x0178 },
  { 0x0131, 0x0049 },
  { 0x017F, 0x0053 },
  { 0x0180, 0x0243 },
  { 0x0188, 0x0187 },
  { 0x018C, 0x018B },
  { 0x0192, 0x0191 },
  { 0x0195, 0x01F6 },
  { 0x0199, 0x0198 },
  { 0x019A, 0x023D },
  { 0x019E, 0x0220 },
  { 0x01A8, 0x01A7 },
  { 0x01AD, 0x01AC },
  { 0x01B0, 0x01AF },
  { 0x01B9, 0x01B8 },
  { 0x01BD, 0x01BC },
  { 0x01BF, 0x01F7 },
  { 0x01C5, 0x01C4 },
  { 0x01C6, 0x01C4 },
  { 0x01C8, 0x01C7 },
  { 0x01C9, 0x01C7 },
  { 0x01CB, 0x01CA },
  { 0x01CC, 0x01CA },
  { 0x01DD, 0x018E },
  { 0x01F2, 0x01F1 },
  { 0x01F3, 0x01F1 },
  { 0x01F5, 0x01F4 },
  { 0x023C, 0x023B },
  { 0x0242, 0x0241 },
  { 0x0250, 0x2C6F },
  { 0x0251, 0x2C6D },
  { 0x0252, 0x2C70 },
  { 0x0253, 0x0181 },
  { 0x0254, 0x0186 },
  { 0x0259, 0x018F },
  { 0x025B, 0x0190 },
  { 0x0260, 0x0193 },
  { 0x0263, 0x0194 },
  { 0x0265, 0xA78D },
  { 0x0266, 0xA7AA },
  { 0x0268, 0x0197 },
  { 0x0269, 0x0196 },
  { 0x026B, 0x2C62 },
  { 0x026F, 0x019C },
  { 0x0271, 0x2C6E },
  { 0x0272, 0x019D },
  { 0x0275, 0x019F },
  { 0x027D, 0x2C64 },
  { 0x0280, 0x01A6 },
  { 0x0283, 0x01A9 },
  { 0x0288, 0x01AE },
  { 0x0289, 0x0244 },
  { 0x028C, 0x0245 },
  { 0x0292, 0x01B7 },
  { 0x0345, 0x0399 },
  { 0x0377, 0x0376 },
  { 0x03AC, 0x0386 },
  { 0x03C2, 0x03A3 },
  { 0x03CC, 0x038C },
  { 0x03D0, 0x0392 },
  { 0x03D1, 0x0398 },
  { 0x03D5, 0x03A6 },
  { 0x03D6, 0x03A0 },
  { 0x03D7, 0x03CF },
  { 0x03F0, 0x039A },
  { 0x03F1, 0x03A1 },
  { 0x03F2, 0x03F9 },
  { 0x03F5, 0x0395 },
  { 0x03F8, 0x03F7 },
  { 0x03FB, 0x03FA },
  { 0x04CF, 0x04C0 },
  { 0x1D79, 0xA77D },
  { 0x1D7D, 0x2C63 },
  { 0x1E9B, 0x1E60 },
  { 0x1FB3, 0x1FBC },
  { 0x1FBE, 0x0399 },
  { 0x1FC3, 0x1FCC },
  { 0x1FE5, 0x1FEC },
  { 0x1FF3, 0x1FFC },
  { 0x214E, 0x2132 },
  { 0x2184, 0x2183 },
  { 0x2C61, 0x2C60 },
  { 0x2C65, 0x023A },
  { 0x2C66, 0x023E },
  { 0x2C73, 0x2C72 },
  { 0x2C76, 0x2C75 },
  { 0x2CF3, 0x2CF2 },
  { 0x2D2D, 0x10CD },
  { 0xA78C, 0xA78B },
};

static const __SEGGER_RTL_unicode_map_bmp_range_t __SEGGER_RTL_unicode_bmp_tolower_range2_map[37] = {
  { 0x00D6, 0x00D8, 0x00F6 },
  { 0x0100, 0x012E, 0x0101 },
  { 0x0132, 0x0136, 0x0133 },
  { 0x0139, 0x0147, 0x013A },
  { 0x014A, 0x0176, 0x014B },
  { 0x0179, 0x017D, 0x017A },
  { 0x0182, 0x0184, 0x0183 },
  { 0x01A0, 0x01A4, 0x01A1 },
  { 0x01B3, 0x01B5, 0x01B4 },
  { 0x01CB, 0x01DB, 0x01CC },
  { 0x01DE, 0x01EE, 0x01DF },
  { 0x01F2, 0x01F4, 0x01F3 },
  { 0x01F8, 0x021E, 0x01F9 },
  { 0x0222, 0x0232, 0x0223 },
  { 0x0246, 0x024E, 0x0247 },
  { 0x0370, 0x0372, 0x0371 },
  { 0x03A1, 0x03A3, 0x03C1 },
  { 0x03D8, 0x03EE, 0x03D9 },
  { 0x0460, 0x0480, 0x0461 },
  { 0x048A, 0x04BE, 0x048B },
  { 0x04C1, 0x04CD, 0x04C2 },
  { 0x04D0, 0x0526, 0x04D1 },
  { 0x10C5, 0x10C7, 0x2D25 },
  { 0x1E00, 0x1E94, 0x1E01 },
  { 0x1EA0, 0x1EFE, 0x1EA1 },
  { 0x1F59, 0x1F5F, 0x1F51 },
  { 0x2C67, 0x2C6B, 0x2C68 },
  { 0x2C80, 0x2CE2, 0x2C81 },
  { 0x2CEB, 0x2CED, 0x2CEC },
  { 0xA640, 0xA66C, 0xA641 },
  { 0xA680, 0xA696, 0xA681 },
  { 0xA722, 0xA72E, 0xA723 },
  { 0xA732, 0xA76E, 0xA733 },
  { 0xA779, 0xA77B, 0xA77A },
  { 0xA77E, 0xA786, 0xA77F },
  { 0xA790, 0xA792, 0xA791 },
  { 0xA7A0, 0xA7A8, 0xA7A1 },
};

static const __SEGGER_RTL_unicode_map_bmp_range_t __SEGGER_RTL_unicode_bmp_tolower_range1_map[37] = {
  { 0x0041, 0x005A, 0x0061 },
  { 0x00C0, 0x00D5, 0x00E0 },
  { 0x00D9, 0x00DE, 0x00F9 },
  { 0x0189, 0x018A, 0x0256 },
  { 0x01B1, 0x01B2, 0x028A },
  { 0x0388, 0x038A, 0x03AD },
  { 0x038E, 0x038F, 0x03CD },
  { 0x0391, 0x03A0, 0x03B1 },
  { 0x03A4, 0x03AB, 0x03C4 },
  { 0x03FD, 0x03FF, 0x037B },
  { 0x0400, 0x040F, 0x0450 },
  { 0x0410, 0x042F, 0x0430 },
  { 0x0531, 0x0556, 0x0561 },
  { 0x10A0, 0x10C4, 0x2D00 },
  { 0x1F08, 0x1F0F, 0x1F00 },
  { 0x1F18, 0x1F1D, 0x1F10 },
  { 0x1F28, 0x1F2F, 0x1F20 },
  { 0x1F38, 0x1F3F, 0x1F30 },
  { 0x1F48, 0x1F4D, 0x1F40 },
  { 0x1F68, 0x1F6F, 0x1F60 },
  { 0x1F88, 0x1F8F, 0x1F80 },
  { 0x1F98, 0x1F9F, 0x1F90 },
  { 0x1FA8, 0x1FAF, 0x1FA0 },
  { 0x1FB8, 0x1FB9, 0x1FB0 },
  { 0x1FBA, 0x1FBB, 0x1F70 },
  { 0x1FC8, 0x1FCB, 0x1F72 },
  { 0x1FD8, 0x1FD9, 0x1FD0 },
  { 0x1FDA, 0x1FDB, 0x1F76 },
  { 0x1FE8, 0x1FE9, 0x1FE0 },
  { 0x1FEA, 0x1FEB, 0x1F7A },
  { 0x1FF8, 0x1FF9, 0x1F78 },
  { 0x1FFA, 0x1FFB, 0x1F7C },
  { 0x2160, 0x216F, 0x2170 },
  { 0x24B6, 0x24CF, 0x24D0 },
  { 0x2C00, 0x2C2E, 0x2C30 },
  { 0x2C7E, 0x2C7F, 0x023F },
  { 0xFF21, 0xFF3A, 0xFF41 },
};

static const __SEGGER_RTL_unicode_map_bmp_singleton_t __SEGGER_RTL_unicode_bmp_tolower_singleton_map[79] = {
  { 0x0130, 0x0069 },
  { 0x0178, 0x00FF },
  { 0x0181, 0x0253 },
  { 0x0186, 0x0254 },
  { 0x0187, 0x0188 },
  { 0x018B, 0x018C },
  { 0x018E, 0x01DD },
  { 0x018F, 0x0259 },
  { 0x0190, 0x025B },
  { 0x0191, 0x0192 },
  { 0x0193, 0x0260 },
  { 0x0194, 0x0263 },
  { 0x0196, 0x0269 },
  { 0x0197, 0x0268 },
  { 0x0198, 0x0199 },
  { 0x019C, 0x026F },
  { 0x019D, 0x0272 },
  { 0x019F, 0x0275 },
  { 0x01A6, 0x0280 },
  { 0x01A7, 0x01A8 },
  { 0x01A9, 0x0283 },
  { 0x01AC, 0x01AD },
  { 0x01AE, 0x0288 },
  { 0x01AF, 0x01B0 },
  { 0x01B7, 0x0292 },
  { 0x01B8, 0x01B9 },
  { 0x01BC, 0x01BD },
  { 0x01C4, 0x01C6 },
  { 0x01C5, 0x01C6 },
  { 0x01C7, 0x01C9 },
  { 0x01C8, 0x01C9 },
  { 0x01CA, 0x01CC },
  { 0x01F1, 0x01F3 },
  { 0x01F6, 0x0195 },
  { 0x01F7, 0x01BF },
  { 0x0220, 0x019E },
  { 0x023A, 0x2C65 },
  { 0x023B, 0x023C },
  { 0x023D, 0x019A },
  { 0x023E, 0x2C66 },
  { 0x0241, 0x0242 },
  { 0x0243, 0x0180 },
  { 0x0244, 0x0289 },
  { 0x0245, 0x028C },
  { 0x0376, 0x0377 },
  { 0x0386, 0x03AC },
  { 0x038C, 0x03CC },
  { 0x03CF, 0x03D7 },
  { 0x03F4, 0x03B8 },
  { 0x03F7, 0x03F8 },
  { 0x03F9, 0x03F2 },
  { 0x03FA, 0x03FB },
  { 0x04C0, 0x04CF },
  { 0x10CD, 0x2D2D },
  { 0x1E9E, 0x00DF },
  { 0x1FBC, 0x1FB3 },
  { 0x1FCC, 0x1FC3 },
  { 0x1FEC, 0x1FE5 },
  { 0x1FFC, 0x1FF3 },
  { 0x2126, 0x03C9 },
  { 0x212A, 0x006B },
  { 0x212B, 0x00E5 },
  { 0x2132, 0x214E },
  { 0x2183, 0x2184 },
  { 0x2C60, 0x2C61 },
  { 0x2C62, 0x026B },
  { 0x2C63, 0x1D7D },
  { 0x2C64, 0x027D },
  { 0x2C6D, 0x0251 },
  { 0x2C6E, 0x0271 },
  { 0x2C6F, 0x0250 },
  { 0x2C70, 0x0252 },
  { 0x2C72, 0x2C73 },
  { 0x2C75, 0x2C76 },
  { 0x2CF2, 0x2CF3 },
  { 0xA77D, 0x1D79 },
  { 0xA78B, 0xA78C },
  { 0xA78D, 0x0265 },
  { 0xA7AA, 0x0266 },
};

static const unsigned short __SEGGER_RTL_unicode_bmp_isalpha_singleton[86] = {
  0x00AA,
  0x00B5,
  0x00BA,
  0x02C6,
  0x02EC,
  0x02EE,
  0x0386,
  0x038C,
  0x0559,
  0x06D5,
  0x06FF,
  0x0710,
  0x07B1,
  0x081A,
  0x0824,
  0x0828,
  0x08A0,
  0x093D,
  0x0950,
  0x09B2,
  0x09BD,
  0x09CE,
  0x0A5E,
  0x0ABD,
  0x0AD0,
  0x0B3D,
  0x0B71,
  0x0B83,
  0x0B9C,
  0x0BD0,
  0x0C3D,
  0x0CBD,
  0x0CDE,
  0x0D3D,
  0x0D4E,
  0x0DBD,
  0x0E84,
  0x0E8A,
  0x0E8D,
  0x0EA5,
  0x0EA7,
  0x0EBD,
  0x0F00,
  0x103F,
  0x1061,
  0x108E,
  0x10C7,
  0x10CD,
  0x1258,
  0x12C0,
  0x17DC,
  0x18AA,
  0x1F59,
  0x1F5B,
  0x1F5D,
  0x1FBE,
  0x2071,
  0x207F,
  0x2102,
  0x2107,
  0x2115,
  0x2124,
  0x2126,
  0x2128,
  0x214E,
  0x2D27,
  0x2D2D,
  0x2D6F,
  0x3006,
  0x303C,
  0x309F,
  0x30FF,
  0x3400,
  0x4DB5,
  0x4E00,
  0x9FCC,
  0xA8FB,
  0xAA7A,
  0xAAB1,
  0xAAC0,
  0xAAC2,
  0xAAF2,
  0xAC00,
  0xD7A3,
  0xFB1D,
  0xFB3E,
};

#if __SEGGER_RTL_SIZEOF_WCHAR_T != 2
static const long __SEGGER_RTL_unicode_nonbmp_isalpha_singleton[29] = {
  0x010808,
  0x01083C,
  0x010A00,
  0x016F50,
  0x01D4A2,
  0x01D4BB,
  0x01D546,
  0x01EE24,
  0x01EE27,
  0x01EE39,
  0x01EE3B,
  0x01EE42,
  0x01EE47,
  0x01EE49,
  0x01EE4B,
  0x01EE54,
  0x01EE57,
  0x01EE59,
  0x01EE5B,
  0x01EE5D,
  0x01EE5F,
  0x01EE64,
  0x01EE7E,
  0x020000,
  0x02A6D6,
  0x02A700,
  0x02B734,
  0x02B740,
  0x02B81D,
};
#endif

static const __SEGGER_RTL_unicode_set_bmp_range_t __SEGGER_RTL_unicode_bmp_isalpha_range[283] = {
  { 0x0041, 0x005A },
  { 0x0061, 0x007A },
  { 0x00C0, 0x00D6 },
  { 0x00D8, 0x00F6 },
  { 0x00F8, 0x02C1 },
  { 0x02C8, 0x02D1 },
  { 0x02E0, 0x02E4 },
  { 0x0370, 0x0373 },
  { 0x0376, 0x0377 },
  { 0x037B, 0x037D },
  { 0x0388, 0x038A },
  { 0x038E, 0x03A1 },
  { 0x03A3, 0x03F5 },
  { 0x03F7, 0x0481 },
  { 0x048A, 0x0527 },
  { 0x0531, 0x0556 },
  { 0x0561, 0x0587 },
  { 0x05D0, 0x05EA },
  { 0x05F0, 0x05F2 },
  { 0x0620, 0x063F },
  { 0x0641, 0x064A },
  { 0x066E, 0x066F },
  { 0x0671, 0x06D3 },
  { 0x06EE, 0x06EF },
  { 0x06FA, 0x06FC },
  { 0x0712, 0x072F },
  { 0x074D, 0x07A5 },
  { 0x07CA, 0x07EA },
  { 0x0800, 0x0815 },
  { 0x0840, 0x0858 },
  { 0x08A2, 0x08AC },
  { 0x0904, 0x0939 },
  { 0x0958, 0x0961 },
  { 0x0972, 0x0977 },
  { 0x0979, 0x097F },
  { 0x0985, 0x098C },
  { 0x098F, 0x0990 },
  { 0x0993, 0x09A8 },
  { 0x09AA, 0x09B0 },
  { 0x09B6, 0x09B9 },
  { 0x09DC, 0x09DD },
  { 0x09DF, 0x09E1 },
  { 0x09F0, 0x09F1 },
  { 0x0A05, 0x0A0A },
  { 0x0A0F, 0x0A10 },
  { 0x0A13, 0x0A28 },
  { 0x0A2A, 0x0A30 },
  { 0x0A32, 0x0A33 },
  { 0x0A35, 0x0A36 },
  { 0x0A38, 0x0A39 },
  { 0x0A59, 0x0A5C },
  { 0x0A72, 0x0A74 },
  { 0x0A85, 0x0A8D },
  { 0x0A8F, 0x0A91 },
  { 0x0A93, 0x0AA8 },
  { 0x0AAA, 0x0AB0 },
  { 0x0AB2, 0x0AB3 },
  { 0x0AB5, 0x0AB9 },
  { 0x0AE0, 0x0AE1 },
  { 0x0B05, 0x0B0C },
  { 0x0B0F, 0x0B10 },
  { 0x0B13, 0x0B28 },
  { 0x0B2A, 0x0B30 },
  { 0x0B32, 0x0B33 },
  { 0x0B35, 0x0B39 },
  { 0x0B5C, 0x0B5D },
  { 0x0B5F, 0x0B61 },
  { 0x0B85, 0x0B8A },
  { 0x0B8E, 0x0B90 },
  { 0x0B92, 0x0B95 },
  { 0x0B99, 0x0B9A },
  { 0x0B9E, 0x0B9F },
  { 0x0BA3, 0x0BA4 },
  { 0x0BA8, 0x0BAA },
  { 0x0BAE, 0x0BB9 },
  { 0x0C05, 0x0C0C },
  { 0x0C0E, 0x0C10 },
  { 0x0C12, 0x0C28 },
  { 0x0C2A, 0x0C33 },
  { 0x0C35, 0x0C39 },
  { 0x0C58, 0x0C59 },
  { 0x0C60, 0x0C61 },
  { 0x0C85, 0x0C8C },
  { 0x0C8E, 0x0C90 },
  { 0x0C92, 0x0CA8 },
  { 0x0CAA, 0x0CB3 },
  { 0x0CB5, 0x0CB9 },
  { 0x0CE0, 0x0CE1 },
  { 0x0CF1, 0x0CF2 },
  { 0x0D05, 0x0D0C },
  { 0x0D0E, 0x0D10 },
  { 0x0D12, 0x0D3A },
  { 0x0D60, 0x0D61 },
  { 0x0D7A, 0x0D7F },
  { 0x0D85, 0x0D96 },
  { 0x0D9A, 0x0DB1 },
  { 0x0DB3, 0x0DBB },
  { 0x0DC0, 0x0DC6 },
  { 0x0E01, 0x0E30 },
  { 0x0E32, 0x0E33 },
  { 0x0E40, 0x0E45 },
  { 0x0E81, 0x0E82 },
  { 0x0E87, 0x0E88 },
  { 0x0E94, 0x0E97 },
  { 0x0E99, 0x0E9F },
  { 0x0EA1, 0x0EA3 },
  { 0x0EAA, 0x0EAB },
  { 0x0EAD, 0x0EB0 },
  { 0x0EB2, 0x0EB3 },
  { 0x0EC0, 0x0EC4 },
  { 0x0EDC, 0x0EDF },
  { 0x0F40, 0x0F47 },
  { 0x0F49, 0x0F6C },
  { 0x0F88, 0x0F8C },
  { 0x1000, 0x102A },
  { 0x1050, 0x1055 },
  { 0x105A, 0x105D },
  { 0x1065, 0x1066 },
  { 0x106E, 0x1070 },
  { 0x1075, 0x1081 },
  { 0x10A0, 0x10C5 },
  { 0x10D0, 0x10FA },
  { 0x10FC, 0x1248 },
  { 0x124A, 0x124D },
  { 0x1250, 0x1256 },
  { 0x125A, 0x125D },
  { 0x1260, 0x1288 },
  { 0x128A, 0x128D },
  { 0x1290, 0x12B0 },
  { 0x12B2, 0x12B5 },
  { 0x12B8, 0x12BE },
  { 0x12C2, 0x12C5 },
  { 0x12C8, 0x12D6 },
  { 0x12D8, 0x1310 },
  { 0x1312, 0x1315 },
  { 0x1318, 0x135A },
  { 0x1380, 0x138F },
  { 0x13A0, 0x13F4 },
  { 0x1401, 0x166C },
  { 0x166F, 0x167F },
  { 0x1681, 0x169A },
  { 0x16A0, 0x16EA },
  { 0x1700, 0x170C },
  { 0x170E, 0x1711 },
  { 0x1720, 0x1731 },
  { 0x1740, 0x1751 },
  { 0x1760, 0x176C },
  { 0x176E, 0x1770 },
  { 0x1780, 0x17B3 },
  { 0x1820, 0x1877 },
  { 0x1880, 0x18A8 },
  { 0x18B0, 0x18F5 },
  { 0x1900, 0x191C },
  { 0x1950, 0x196D },
  { 0x1970, 0x1974 },
  { 0x1980, 0x19AB },
  { 0x19C1, 0x19C7 },
  { 0x1A00, 0x1A16 },
  { 0x1A20, 0x1A54 },
  { 0x1B05, 0x1B33 },
  { 0x1B45, 0x1B4B },
  { 0x1B83, 0x1BA0 },
  { 0x1BAE, 0x1BAF },
  { 0x1BBA, 0x1BE5 },
  { 0x1C00, 0x1C23 },
  { 0x1C4D, 0x1C4F },
  { 0x1C5A, 0x1C77 },
  { 0x1CE9, 0x1CEC },
  { 0x1CEE, 0x1CF1 },
  { 0x1CF5, 0x1CF6 },
  { 0x1D00, 0x1DBF },
  { 0x1E00, 0x1F15 },
  { 0x1F18, 0x1F1D },
  { 0x1F20, 0x1F45 },
  { 0x1F48, 0x1F4D },
  { 0x1F50, 0x1F57 },
  { 0x1F5F, 0x1F7D },
  { 0x1F80, 0x1FB4 },
  { 0x1FB6, 0x1FBC },
  { 0x1FC2, 0x1FC4 },
  { 0x1FC6, 0x1FCC },
  { 0x1FD0, 0x1FD3 },
  { 0x1FD6, 0x1FDB },
  { 0x1FE0, 0x1FEC },
  { 0x1FF2, 0x1FF4 },
  { 0x1FF6, 0x1FFC },
  { 0x2090, 0x209C },
  { 0x210A, 0x2113 },
  { 0x2119, 0x211D },
  { 0x212A, 0x212D },
  { 0x212F, 0x2139 },
  { 0x213C, 0x213F },
  { 0x2145, 0x2149 },
  { 0x2183, 0x2184 },
  { 0x2C00, 0x2C2E },
  { 0x2C30, 0x2C5E },
  { 0x2C60, 0x2CE4 },
  { 0x2CEB, 0x2CEE },
  { 0x2CF2, 0x2CF3 },
  { 0x2D00, 0x2D25 },
  { 0x2D30, 0x2D67 },
  { 0x2D80, 0x2D96 },
  { 0x2DA0, 0x2DA6 },
  { 0x2DA8, 0x2DAE },
  { 0x2DB0, 0x2DB6 },
  { 0x2DB8, 0x2DBE },
  { 0x2DC0, 0x2DC6 },
  { 0x2DC8, 0x2DCE },
  { 0x2DD0, 0x2DD6 },
  { 0x2DD8, 0x2DDE },
  { 0x3041, 0x3096 },
  { 0x30A1, 0x30FA },
  { 0x3105, 0x312D },
  { 0x3131, 0x318E },
  { 0x31A0, 0x31BA },
  { 0x31F0, 0x31FF },
  { 0xA000, 0xA014 },
  { 0xA016, 0xA48C },
  { 0xA4D0, 0xA4FD },
  { 0xA500, 0xA60B },
  { 0xA610, 0xA61F },
  { 0xA62A, 0xA62B },
  { 0xA640, 0xA66E },
  { 0xA680, 0xA697 },
  { 0xA6A0, 0xA6E5 },
  { 0xA717, 0xA71F },
  { 0xA722, 0xA788 },
  { 0xA78B, 0xA78E },
  { 0xA790, 0xA793 },
  { 0xA7A0, 0xA7AA },
  { 0xA7F8, 0xA801 },
  { 0xA803, 0xA805 },
  { 0xA807, 0xA80A },
  { 0xA80C, 0xA822 },
  { 0xA840, 0xA873 },
  { 0xA882, 0xA8B3 },
  { 0xA8F2, 0xA8F7 },
  { 0xA90A, 0xA925 },
  { 0xA930, 0xA946 },
  { 0xA960, 0xA97C },
  { 0xA984, 0xA9B2 },
  { 0xAA00, 0xAA28 },
  { 0xAA40, 0xAA42 },
  { 0xAA44, 0xAA4B },
  { 0xAA60, 0xAA76 },
  { 0xAA80, 0xAAAF },
  { 0xAAB5, 0xAAB6 },
  { 0xAAB9, 0xAABD },
  { 0xAADB, 0xAADC },
  { 0xAAE0, 0xAAEA },
  { 0xAB01, 0xAB06 },
  { 0xAB09, 0xAB0E },
  { 0xAB11, 0xAB16 },
  { 0xAB20, 0xAB26 },
  { 0xAB28, 0xAB2E },
  { 0xABC0, 0xABE2 },
  { 0xD7B0, 0xD7C6 },
  { 0xD7CB, 0xD7FB },
  { 0xF900, 0xFA6D },
  { 0xFA70, 0xFAD9 },
  { 0xFB00, 0xFB06 },
  { 0xFB13, 0xFB17 },
  { 0xFB1F, 0xFB28 },
  { 0xFB2A, 0xFB36 },
  { 0xFB38, 0xFB3C },
  { 0xFB40, 0xFB41 },
  { 0xFB43, 0xFB44 },
  { 0xFB46, 0xFBB1 },
  { 0xFBD3, 0xFD3D },
  { 0xFD50, 0xFD8F },
  { 0xFD92, 0xFDC7 },
  { 0xFDF0, 0xFDFB },
  { 0xFE70, 0xFE74 },
  { 0xFE76, 0xFEFC },
  { 0xFF21, 0xFF3A },
  { 0xFF41, 0xFF5A },
  { 0xFF66, 0xFF6F },
  { 0xFF71, 0xFF9D },
  { 0xFFA0, 0xFFBE },
  { 0xFFC2, 0xFFC7 },
  { 0xFFCA, 0xFFCF },
  { 0xFFD2, 0xFFD7 },
  { 0xFFDA, 0xFFDC },
};

#if __SEGGER_RTL_SIZEOF_WCHAR_T != 2
static const __SEGGER_RTL_unicode_set_nonbmp_range_t __SEGGER_RTL_unicode_nonbmp_isalpha_range[90] = {
  { 0x010000, 0x01000B },
  { 0x01000D, 0x010026 },
  { 0x010028, 0x01003A },
  { 0x01003C, 0x01003D },
  { 0x01003F, 0x01004D },
  { 0x010050, 0x01005D },
  { 0x010080, 0x0100FA },
  { 0x010280, 0x01029C },
  { 0x0102A0, 0x0102D0 },
  { 0x010300, 0x01031E },
  { 0x010330, 0x010340 },
  { 0x010342, 0x010349 },
  { 0x010380, 0x01039D },
  { 0x0103A0, 0x0103C3 },
  { 0x0103C8, 0x0103CF },
  { 0x010400, 0x01049D },
  { 0x010800, 0x010805 },
  { 0x01080A, 0x010835 },
  { 0x010837, 0x010838 },
  { 0x01083F, 0x010855 },
  { 0x010900, 0x010915 },
  { 0x010920, 0x010939 },
  { 0x010980, 0x0109B7 },
  { 0x0109BE, 0x0109BF },
  { 0x010A10, 0x010A13 },
  { 0x010A15, 0x010A17 },
  { 0x010A19, 0x010A33 },
  { 0x010A60, 0x010A7C },
  { 0x010B00, 0x010B35 },
  { 0x010B40, 0x010B55 },
  { 0x010B60, 0x010B72 },
  { 0x010C00, 0x010C48 },
  { 0x011003, 0x011037 },
  { 0x011083, 0x0110AF },
  { 0x0110D0, 0x0110E8 },
  { 0x011103, 0x011126 },
  { 0x011183, 0x0111B2 },
  { 0x0111C1, 0x0111C4 },
  { 0x011680, 0x0116AA },
  { 0x012000, 0x01236E },
  { 0x013000, 0x01342E },
  { 0x016800, 0x016A38 },
  { 0x016F00, 0x016F44 },
  { 0x016F93, 0x016F9F },
  { 0x01B000, 0x01B001 },
  { 0x01D400, 0x01D454 },
  { 0x01D456, 0x01D49C },
  { 0x01D49E, 0x01D49F },
  { 0x01D4A5, 0x01D4A6 },
  { 0x01D4A9, 0x01D4AC },
  { 0x01D4AE, 0x01D4B9 },
  { 0x01D4BD, 0x01D4C3 },
  { 0x01D4C5, 0x01D505 },
  { 0x01D507, 0x01D50A },
  { 0x01D50D, 0x01D514 },
  { 0x01D516, 0x01D51C },
  { 0x01D51E, 0x01D539 },
  { 0x01D53B, 0x01D53E },
  { 0x01D540, 0x01D544 },
  { 0x01D54A, 0x01D550 },
  { 0x01D552, 0x01D6A5 },
  { 0x01D6A8, 0x01D6C0 },
  { 0x01D6C2, 0x01D6DA },
  { 0x01D6DC, 0x01D6FA },
  { 0x01D6FC, 0x01D714 },
  { 0x01D716, 0x01D734 },
  { 0x01D736, 0x01D74E },
  { 0x01D750, 0x01D76E },
  { 0x01D770, 0x01D788 },
  { 0x01D78A, 0x01D7A8 },
  { 0x01D7AA, 0x01D7C2 },
  { 0x01D7C4, 0x01D7CB },
  { 0x01EE00, 0x01EE03 },
  { 0x01EE05, 0x01EE1F },
  { 0x01EE21, 0x01EE22 },
  { 0x01EE29, 0x01EE32 },
  { 0x01EE34, 0x01EE37 },
  { 0x01EE4D, 0x01EE4F },
  { 0x01EE51, 0x01EE52 },
  { 0x01EE61, 0x01EE62 },
  { 0x01EE67, 0x01EE6A },
  { 0x01EE6C, 0x01EE72 },
  { 0x01EE74, 0x01EE77 },
  { 0x01EE79, 0x01EE7C },
  { 0x01EE80, 0x01EE89 },
  { 0x01EE8B, 0x01EE9B },
  { 0x01EEA1, 0x01EEA3 },
  { 0x01EEA5, 0x01EEA9 },
  { 0x01EEAB, 0x01EEBB },
  { 0x02F800, 0x02FA1D },
};
#endif

static const unsigned short __SEGGER_RTL_unicode_bmp_isprint_singleton[41] = {
  0x038C,
  0x058F,
  0x085E,
  0x08A0,
  0x09B2,
  0x09D7,
  0x0A3C,
  0x0A51,
  0x0A5E,
  0x0AD0,
  0x0B9C,
  0x0BD0,
  0x0BD7,
  0x0CDE,
  0x0D57,
  0x0DBD,
  0x0DCA,
  0x0DD6,
  0x0E84,
  0x0E8A,
  0x0E8D,
  0x0EA5,
  0x0EA7,
  0x0EC6,
  0x10C7,
  0x10CD,
  0x1258,
  0x12C0,
  0x1940,
  0x1F59,
  0x1F5B,
  0x1F5D,
  0x2D27,
  0x2D2D,
  0x4DB5,
  0x9FCC,
  0xAC00,
  0xD7A3,
  0xE000,
  0xFB3E,
  0xFEFF,
};

#if __SEGGER_RTL_SIZEOF_WCHAR_T != 2
static const long __SEGGER_RTL_unicode_nonbmp_isprint_singleton[34] = {
  0x010808,
  0x01083C,
  0x01093F,
  0x01D4A2,
  0x01D4BB,
  0x01D546,
  0x01EE24,
  0x01EE27,
  0x01EE39,
  0x01EE3B,
  0x01EE42,
  0x01EE47,
  0x01EE49,
  0x01EE4B,
  0x01EE54,
  0x01EE57,
  0x01EE59,
  0x01EE5B,
  0x01EE5D,
  0x01EE5F,
  0x01EE64,
  0x01EE7E,
  0x01F440,
  0x020000,
  0x02A6D6,
  0x02A700,
  0x02B734,
  0x02B740,
  0x02B81D,
  0x0E0001,
  0x0F0000,
  0x0FFFFD,
  0x100000,
  0x10FFFD,
};
#endif

static const __SEGGER_RTL_unicode_set_bmp_range_t __SEGGER_RTL_unicode_bmp_isprint_range[333] = {
  { 0x0020, 0x007E },
  { 0x00A0, 0x0377 },
  { 0x037A, 0x037E },
  { 0x0384, 0x038A },
  { 0x038E, 0x03A1 },
  { 0x03A3, 0x0527 },
  { 0x0531, 0x0556 },
  { 0x0559, 0x055F },
  { 0x0561, 0x0587 },
  { 0x0589, 0x058A },
  { 0x0591, 0x05C7 },
  { 0x05D0, 0x05EA },
  { 0x05F0, 0x05F4 },
  { 0x0600, 0x0604 },
  { 0x0606, 0x061B },
  { 0x061E, 0x070D },
  { 0x070F, 0x074A },
  { 0x074D, 0x07B1 },
  { 0x07C0, 0x07FA },
  { 0x0800, 0x082D },
  { 0x0830, 0x083E },
  { 0x0840, 0x085B },
  { 0x08A2, 0x08AC },
  { 0x08E4, 0x08FE },
  { 0x0900, 0x0977 },
  { 0x0979, 0x097F },
  { 0x0981, 0x0983 },
  { 0x0985, 0x098C },
  { 0x098F, 0x0990 },
  { 0x0993, 0x09A8 },
  { 0x09AA, 0x09B0 },
  { 0x09B6, 0x09B9 },
  { 0x09BC, 0x09C4 },
  { 0x09C7, 0x09C8 },
  { 0x09CB, 0x09CE },
  { 0x09DC, 0x09DD },
  { 0x09DF, 0x09E3 },
  { 0x09E6, 0x09FB },
  { 0x0A01, 0x0A03 },
  { 0x0A05, 0x0A0A },
  { 0x0A0F, 0x0A10 },
  { 0x0A13, 0x0A28 },
  { 0x0A2A, 0x0A30 },
  { 0x0A32, 0x0A33 },
  { 0x0A35, 0x0A36 },
  { 0x0A38, 0x0A39 },
  { 0x0A3E, 0x0A42 },
  { 0x0A47, 0x0A48 },
  { 0x0A4B, 0x0A4D },
  { 0x0A59, 0x0A5C },
  { 0x0A66, 0x0A75 },
  { 0x0A81, 0x0A83 },
  { 0x0A85, 0x0A8D },
  { 0x0A8F, 0x0A91 },
  { 0x0A93, 0x0AA8 },
  { 0x0AAA, 0x0AB0 },
  { 0x0AB2, 0x0AB3 },
  { 0x0AB5, 0x0AB9 },
  { 0x0ABC, 0x0AC5 },
  { 0x0AC7, 0x0AC9 },
  { 0x0ACB, 0x0ACD },
  { 0x0AE0, 0x0AE3 },
  { 0x0AE6, 0x0AF1 },
  { 0x0B01, 0x0B03 },
  { 0x0B05, 0x0B0C },
  { 0x0B0F, 0x0B10 },
  { 0x0B13, 0x0B28 },
  { 0x0B2A, 0x0B30 },
  { 0x0B32, 0x0B33 },
  { 0x0B35, 0x0B39 },
  { 0x0B3C, 0x0B44 },
  { 0x0B47, 0x0B48 },
  { 0x0B4B, 0x0B4D },
  { 0x0B56, 0x0B57 },
  { 0x0B5C, 0x0B5D },
  { 0x0B5F, 0x0B63 },
  { 0x0B66, 0x0B77 },
  { 0x0B82, 0x0B83 },
  { 0x0B85, 0x0B8A },
  { 0x0B8E, 0x0B90 },
  { 0x0B92, 0x0B95 },
  { 0x0B99, 0x0B9A },
  { 0x0B9E, 0x0B9F },
  { 0x0BA3, 0x0BA4 },
  { 0x0BA8, 0x0BAA },
  { 0x0BAE, 0x0BB9 },
  { 0x0BBE, 0x0BC2 },
  { 0x0BC6, 0x0BC8 },
  { 0x0BCA, 0x0BCD },
  { 0x0BE6, 0x0BFA },
  { 0x0C01, 0x0C03 },
  { 0x0C05, 0x0C0C },
  { 0x0C0E, 0x0C10 },
  { 0x0C12, 0x0C28 },
  { 0x0C2A, 0x0C33 },
  { 0x0C35, 0x0C39 },
  { 0x0C3D, 0x0C44 },
  { 0x0C46, 0x0C48 },
  { 0x0C4A, 0x0C4D },
  { 0x0C55, 0x0C56 },
  { 0x0C58, 0x0C59 },
  { 0x0C60, 0x0C63 },
  { 0x0C66, 0x0C6F },
  { 0x0C78, 0x0C7F },
  { 0x0C82, 0x0C83 },
  { 0x0C85, 0x0C8C },
  { 0x0C8E, 0x0C90 },
  { 0x0C92, 0x0CA8 },
  { 0x0CAA, 0x0CB3 },
  { 0x0CB5, 0x0CB9 },
  { 0x0CBC, 0x0CC4 },
  { 0x0CC6, 0x0CC8 },
  { 0x0CCA, 0x0CCD },
  { 0x0CD5, 0x0CD6 },
  { 0x0CE0, 0x0CE3 },
  { 0x0CE6, 0x0CEF },
  { 0x0CF1, 0x0CF2 },
  { 0x0D02, 0x0D03 },
  { 0x0D05, 0x0D0C },
  { 0x0D0E, 0x0D10 },
  { 0x0D12, 0x0D3A },
  { 0x0D3D, 0x0D44 },
  { 0x0D46, 0x0D48 },
  { 0x0D4A, 0x0D4E },
  { 0x0D60, 0x0D63 },
  { 0x0D66, 0x0D75 },
  { 0x0D79, 0x0D7F },
  { 0x0D82, 0x0D83 },
  { 0x0D85, 0x0D96 },
  { 0x0D9A, 0x0DB1 },
  { 0x0DB3, 0x0DBB },
  { 0x0DC0, 0x0DC6 },
  { 0x0DCF, 0x0DD4 },
  { 0x0DD8, 0x0DDF },
  { 0x0DF2, 0x0DF4 },
  { 0x0E01, 0x0E3A },
  { 0x0E3F, 0x0E5B },
  { 0x0E81, 0x0E82 },
  { 0x0E87, 0x0E88 },
  { 0x0E94, 0x0E97 },
  { 0x0E99, 0x0E9F },
  { 0x0EA1, 0x0EA3 },
  { 0x0EAA, 0x0EAB },
  { 0x0EAD, 0x0EB9 },
  { 0x0EBB, 0x0EBD },
  { 0x0EC0, 0x0EC4 },
  { 0x0EC8, 0x0ECD },
  { 0x0ED0, 0x0ED9 },
  { 0x0EDC, 0x0EDF },
  { 0x0F00, 0x0F47 },
  { 0x0F49, 0x0F6C },
  { 0x0F71, 0x0F97 },
  { 0x0F99, 0x0FBC },
  { 0x0FBE, 0x0FCC },
  { 0x0FCE, 0x0FDA },
  { 0x1000, 0x10C5 },
  { 0x10D0, 0x1248 },
  { 0x124A, 0x124D },
  { 0x1250, 0x1256 },
  { 0x125A, 0x125D },
  { 0x1260, 0x1288 },
  { 0x128A, 0x128D },
  { 0x1290, 0x12B0 },
  { 0x12B2, 0x12B5 },
  { 0x12B8, 0x12BE },
  { 0x12C2, 0x12C5 },
  { 0x12C8, 0x12D6 },
  { 0x12D8, 0x1310 },
  { 0x1312, 0x1315 },
  { 0x1318, 0x135A },
  { 0x135D, 0x137C },
  { 0x1380, 0x1399 },
  { 0x13A0, 0x13F4 },
  { 0x1400, 0x169C },
  { 0x16A0, 0x16F0 },
  { 0x1700, 0x170C },
  { 0x170E, 0x1714 },
  { 0x1720, 0x1736 },
  { 0x1740, 0x1753 },
  { 0x1760, 0x176C },
  { 0x176E, 0x1770 },
  { 0x1772, 0x1773 },
  { 0x1780, 0x17DD },
  { 0x17E0, 0x17E9 },
  { 0x17F0, 0x17F9 },
  { 0x1800, 0x180E },
  { 0x1810, 0x1819 },
  { 0x1820, 0x1877 },
  { 0x1880, 0x18AA },
  { 0x18B0, 0x18F5 },
  { 0x1900, 0x191C },
  { 0x1920, 0x192B },
  { 0x1930, 0x193B },
  { 0x1944, 0x196D },
  { 0x1970, 0x1974 },
  { 0x1980, 0x19AB },
  { 0x19B0, 0x19C9 },
  { 0x19D0, 0x19DA },
  { 0x19DE, 0x1A1B },
  { 0x1A1E, 0x1A5E },
  { 0x1A60, 0x1A7C },
  { 0x1A7F, 0x1A89 },
  { 0x1A90, 0x1A99 },
  { 0x1AA0, 0x1AAD },
  { 0x1B00, 0x1B4B },
  { 0x1B50, 0x1B7C },
  { 0x1B80, 0x1BF3 },
  { 0x1BFC, 0x1C37 },
  { 0x1C3B, 0x1C49 },
  { 0x1C4D, 0x1C7F },
  { 0x1CC0, 0x1CC7 },
  { 0x1CD0, 0x1CF6 },
  { 0x1D00, 0x1DE6 },
  { 0x1DFC, 0x1F15 },
  { 0x1F18, 0x1F1D },
  { 0x1F20, 0x1F45 },
  { 0x1F48, 0x1F4D },
  { 0x1F50, 0x1F57 },
  { 0x1F5F, 0x1F7D },
  { 0x1F80, 0x1FB4 },
  { 0x1FB6, 0x1FC4 },
  { 0x1FC6, 0x1FD3 },
  { 0x1FD6, 0x1FDB },
  { 0x1FDD, 0x1FEF },
  { 0x1FF2, 0x1FF4 },
  { 0x1FF6, 0x1FFE },
  { 0x2000, 0x2027 },
  { 0x202A, 0x2064 },
  { 0x206A, 0x2071 },
  { 0x2074, 0x208E },
  { 0x2090, 0x209C },
  { 0x20A0, 0x20BA },
  { 0x20D0, 0x20F0 },
  { 0x2100, 0x2189 },
  { 0x2190, 0x23F3 },
  { 0x2400, 0x2426 },
  { 0x2440, 0x244A },
  { 0x2460, 0x26FF },
  { 0x2701, 0x2B4C },
  { 0x2B50, 0x2B59 },
  { 0x2C00, 0x2C2E },
  { 0x2C30, 0x2C5E },
  { 0x2C60, 0x2CF3 },
  { 0x2CF9, 0x2D25 },
  { 0x2D30, 0x2D67 },
  { 0x2D6F, 0x2D70 },
  { 0x2D7F, 0x2D96 },
  { 0x2DA0, 0x2DA6 },
  { 0x2DA8, 0x2DAE },
  { 0x2DB0, 0x2DB6 },
  { 0x2DB8, 0x2DBE },
  { 0x2DC0, 0x2DC6 },
  { 0x2DC8, 0x2DCE },
  { 0x2DD0, 0x2DD6 },
  { 0x2DD8, 0x2DDE },
  { 0x2DE0, 0x2E3B },
  { 0x2E80, 0x2E99 },
  { 0x2E9B, 0x2EF3 },
  { 0x2F00, 0x2FD5 },
  { 0x2FF0, 0x2FFB },
  { 0x3000, 0x303F },
  { 0x3041, 0x3096 },
  { 0x3099, 0x30FF },
  { 0x3105, 0x312D },
  { 0x3131, 0x318E },
  { 0x3190, 0x31BA },
  { 0x31C0, 0x31E3 },
  { 0x31F0, 0x321E },
  { 0x3220, 0x32FE },
  { 0x3300, 0x3400 },
  { 0x4DC0, 0x4E00 },
  { 0xA000, 0xA48C },
  { 0xA490, 0xA4C6 },
  { 0xA4D0, 0xA62B },
  { 0xA640, 0xA697 },
  { 0xA69F, 0xA6F7 },
  { 0xA700, 0xA78E },
  { 0xA790, 0xA793 },
  { 0xA7A0, 0xA7AA },
  { 0xA7F8, 0xA82B },
  { 0xA830, 0xA839 },
  { 0xA840, 0xA877 },
  { 0xA880, 0xA8C4 },
  { 0xA8CE, 0xA8D9 },
  { 0xA8E0, 0xA8FB },
  { 0xA900, 0xA953 },
  { 0xA95F, 0xA97C },
  { 0xA980, 0xA9CD },
  { 0xA9CF, 0xA9D9 },
  { 0xA9DE, 0xA9DF },
  { 0xAA00, 0xAA36 },
  { 0xAA40, 0xAA4D },
  { 0xAA50, 0xAA59 },
  { 0xAA5C, 0xAA7B },
  { 0xAA80, 0xAAC2 },
  { 0xAADB, 0xAAF6 },
  { 0xAB01, 0xAB06 },
  { 0xAB09, 0xAB0E },
  { 0xAB11, 0xAB16 },
  { 0xAB20, 0xAB26 },
  { 0xAB28, 0xAB2E },
  { 0xABC0, 0xABED },
  { 0xABF0, 0xABF9 },
  { 0xD7B0, 0xD7C6 },
  { 0xD7CB, 0xD7FB },
  { 0xF8FF, 0xFA6D },
  { 0xFA70, 0xFAD9 },
  { 0xFB00, 0xFB06 },
  { 0xFB13, 0xFB17 },
  { 0xFB1D, 0xFB36 },
  { 0xFB38, 0xFB3C },
  { 0xFB40, 0xFB41 },
  { 0xFB43, 0xFB44 },
  { 0xFB46, 0xFBC1 },
  { 0xFBD3, 0xFD3F },
  { 0xFD50, 0xFD8F },
  { 0xFD92, 0xFDC7 },
  { 0xFDF0, 0xFDFD },
  { 0xFE00, 0xFE19 },
  { 0xFE20, 0xFE26 },
  { 0xFE30, 0xFE52 },
  { 0xFE54, 0xFE66 },
  { 0xFE68, 0xFE6B },
  { 0xFE70, 0xFE74 },
  { 0xFE76, 0xFEFC },
  { 0xFF01, 0xFFBE },
  { 0xFFC2, 0xFFC7 },
  { 0xFFCA, 0xFFCF },
  { 0xFFD2, 0xFFD7 },
  { 0xFFDA, 0xFFDC },
  { 0xFFE0, 0xFFE6 },
  { 0xFFE8, 0xFFEE },
  { 0xFFF9, 0xFFFD },
};

#if __SEGGER_RTL_SIZEOF_WCHAR_T != 2
static const __SEGGER_RTL_unicode_set_nonbmp_range_t __SEGGER_RTL_unicode_nonbmp_isprint_range[142] = {
  { 0x010000, 0x01000B },
  { 0x01000D, 0x010026 },
  { 0x010028, 0x01003A },
  { 0x01003C, 0x01003D },
  { 0x01003F, 0x01004D },
  { 0x010050, 0x01005D },
  { 0x010080, 0x0100FA },
  { 0x010100, 0x010102 },
  { 0x010107, 0x010133 },
  { 0x010137, 0x01018A },
  { 0x010190, 0x01019B },
  { 0x0101D0, 0x0101FD },
  { 0x010280, 0x01029C },
  { 0x0102A0, 0x0102D0 },
  { 0x010300, 0x01031E },
  { 0x010320, 0x010323 },
  { 0x010330, 0x01034A },
  { 0x010380, 0x01039D },
  { 0x01039F, 0x0103C3 },
  { 0x0103C8, 0x0103D5 },
  { 0x010400, 0x01049D },
  { 0x0104A0, 0x0104A9 },
  { 0x010800, 0x010805 },
  { 0x01080A, 0x010835 },
  { 0x010837, 0x010838 },
  { 0x01083F, 0x010855 },
  { 0x010857, 0x01085F },
  { 0x010900, 0x01091B },
  { 0x01091F, 0x010939 },
  { 0x010980, 0x0109B7 },
  { 0x0109BE, 0x0109BF },
  { 0x010A00, 0x010A03 },
  { 0x010A05, 0x010A06 },
  { 0x010A0C, 0x010A13 },
  { 0x010A15, 0x010A17 },
  { 0x010A19, 0x010A33 },
  { 0x010A38, 0x010A3A },
  { 0x010A3F, 0x010A47 },
  { 0x010A50, 0x010A58 },
  { 0x010A60, 0x010A7F },
  { 0x010B00, 0x010B35 },
  { 0x010B39, 0x010B55 },
  { 0x010B58, 0x010B72 },
  { 0x010B78, 0x010B7F },
  { 0x010C00, 0x010C48 },
  { 0x010E60, 0x010E7E },
  { 0x011000, 0x01104D },
  { 0x011052, 0x01106F },
  { 0x011080, 0x0110C1 },
  { 0x0110D0, 0x0110E8 },
  { 0x0110F0, 0x0110F9 },
  { 0x011100, 0x011134 },
  { 0x011136, 0x011143 },
  { 0x011180, 0x0111C8 },
  { 0x0111D0, 0x0111D9 },
  { 0x011680, 0x0116B7 },
  { 0x0116C0, 0x0116C9 },
  { 0x012000, 0x01236E },
  { 0x012400, 0x012462 },
  { 0x012470, 0x012473 },
  { 0x013000, 0x01342E },
  { 0x016800, 0x016A38 },
  { 0x016F00, 0x016F44 },
  { 0x016F50, 0x016F7E },
  { 0x016F8F, 0x016F9F },
  { 0x01B000, 0x01B001 },
  { 0x01D000, 0x01D0F5 },
  { 0x01D100, 0x01D126 },
  { 0x01D129, 0x01D1DD },
  { 0x01D200, 0x01D245 },
  { 0x01D300, 0x01D356 },
  { 0x01D360, 0x01D371 },
  { 0x01D400, 0x01D454 },
  { 0x01D456, 0x01D49C },
  { 0x01D49E, 0x01D49F },
  { 0x01D4A5, 0x01D4A6 },
  { 0x01D4A9, 0x01D4AC },
  { 0x01D4AE, 0x01D4B9 },
  { 0x01D4BD, 0x01D4C3 },
  { 0x01D4C5, 0x01D505 },
  { 0x01D507, 0x01D50A },
  { 0x01D50D, 0x01D514 },
  { 0x01D516, 0x01D51C },
  { 0x01D51E, 0x01D539 },
  { 0x01D53B, 0x01D53E },
  { 0x01D540, 0x01D544 },
  { 0x01D54A, 0x01D550 },
  { 0x01D552, 0x01D6A5 },
  { 0x01D6A8, 0x01D7CB },
  { 0x01D7CE, 0x01D7FF },
  { 0x01EE00, 0x01EE03 },
  { 0x01EE05, 0x01EE1F },
  { 0x01EE21, 0x01EE22 },
  { 0x01EE29, 0x01EE32 },
  { 0x01EE34, 0x01EE37 },
  { 0x01EE4D, 0x01EE4F },
  { 0x01EE51, 0x01EE52 },
  { 0x01EE61, 0x01EE62 },
  { 0x01EE67, 0x01EE6A },
  { 0x01EE6C, 0x01EE72 },
  { 0x01EE74, 0x01EE77 },
  { 0x01EE79, 0x01EE7C },
  { 0x01EE80, 0x01EE89 },
  { 0x01EE8B, 0x01EE9B },
  { 0x01EEA1, 0x01EEA3 },
  { 0x01EEA5, 0x01EEA9 },
  { 0x01EEAB, 0x01EEBB },
  { 0x01EEF0, 0x01EEF1 },
  { 0x01F000, 0x01F02B },
  { 0x01F030, 0x01F093 },
  { 0x01F0A0, 0x01F0AE },
  { 0x01F0B1, 0x01F0BE },
  { 0x01F0C1, 0x01F0CF },
  { 0x01F0D1, 0x01F0DF },
  { 0x01F100, 0x01F10A },
  { 0x01F110, 0x01F12E },
  { 0x01F130, 0x01F16B },
  { 0x01F170, 0x01F19A },
  { 0x01F1E6, 0x01F202 },
  { 0x01F210, 0x01F23A },
  { 0x01F240, 0x01F248 },
  { 0x01F250, 0x01F251 },
  { 0x01F300, 0x01F320 },
  { 0x01F330, 0x01F335 },
  { 0x01F337, 0x01F37C },
  { 0x01F380, 0x01F393 },
  { 0x01F3A0, 0x01F3C4 },
  { 0x01F3C6, 0x01F3CA },
  { 0x01F3E0, 0x01F3F0 },
  { 0x01F400, 0x01F43E },
  { 0x01F442, 0x01F4F7 },
  { 0x01F4F9, 0x01F4FC },
  { 0x01F500, 0x01F53D },
  { 0x01F540, 0x01F543 },
  { 0x01F550, 0x01F567 },
  { 0x01F5FB, 0x01F640 },
  { 0x01F645, 0x01F64F },
  { 0x01F680, 0x01F6C5 },
  { 0x01F700, 0x01F773 },
  { 0x02F800, 0x02FA1D },
  { 0x0E0020, 0x0E007F },
  { 0x0E0100, 0x0E01EF },
};
#endif

static const unsigned char __SEGGER_RTL_ascii_ctype_map[128] = {
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 00 (NUL)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 01 (SOH)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 02 (STX)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 03 (ETX)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 04 (EOT)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 05 (ENQ)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 06 (ACK)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 07 (BEL)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 08 (BS)      
  __SEGGER_RTL_CTYPE_CNTRL | __SEGGER_RTL_CTYPE_SPACE | __SEGGER_RTL_CTYPE_BLANK, // 09 (HT)      
  __SEGGER_RTL_CTYPE_CNTRL | __SEGGER_RTL_CTYPE_SPACE,                            // 0A (LF)      
  __SEGGER_RTL_CTYPE_CNTRL | __SEGGER_RTL_CTYPE_SPACE,                            // 0B (VT)      
  __SEGGER_RTL_CTYPE_CNTRL | __SEGGER_RTL_CTYPE_SPACE,                            // 0C (FF)      
  __SEGGER_RTL_CTYPE_CNTRL | __SEGGER_RTL_CTYPE_SPACE,                            // 0D (CR)      
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 0E (SI)      
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 0F (SO)      
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 10 (DLE)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 11 (DC1)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 12 (DC2)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 13 (DC3)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 14 (DC4)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 15 (NAK)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 16 (SYN)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 17 (ETB)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 18 (CAN)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 19 (EM)      
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 1A (SUB)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 1B (ESC)     
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 1C (FS)      
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 1D (GS)      
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 1E (RS)      
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 1F (US)      
  __SEGGER_RTL_CTYPE_SPACE | __SEGGER_RTL_CTYPE_BLANK,                            // 20 SPACE -- blank is distinguished from whitespace by isprint
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 21 !         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 22 "         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 23 #         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 24 $         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 25 %         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 26 &         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 27 '         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 28 (         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 29 )         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 2A *         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 2B +         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 2C ,         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 2D -         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 2E .         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 2F /         
  __SEGGER_RTL_CTYPE_DIGIT | __SEGGER_RTL_CTYPE_XDIGIT,                           // 30 0         
  __SEGGER_RTL_CTYPE_DIGIT | __SEGGER_RTL_CTYPE_XDIGIT,                           // 31 1         
  __SEGGER_RTL_CTYPE_DIGIT | __SEGGER_RTL_CTYPE_XDIGIT,                           // 32 2         
  __SEGGER_RTL_CTYPE_DIGIT | __SEGGER_RTL_CTYPE_XDIGIT,                           // 33 3         
  __SEGGER_RTL_CTYPE_DIGIT | __SEGGER_RTL_CTYPE_XDIGIT,                           // 34 4         
  __SEGGER_RTL_CTYPE_DIGIT | __SEGGER_RTL_CTYPE_XDIGIT,                           // 35 5         
  __SEGGER_RTL_CTYPE_DIGIT | __SEGGER_RTL_CTYPE_XDIGIT,                           // 36 6         
  __SEGGER_RTL_CTYPE_DIGIT | __SEGGER_RTL_CTYPE_XDIGIT,                           // 37 7         
  __SEGGER_RTL_CTYPE_DIGIT | __SEGGER_RTL_CTYPE_XDIGIT,                           // 38 8         
  __SEGGER_RTL_CTYPE_DIGIT | __SEGGER_RTL_CTYPE_XDIGIT,                           // 39 9         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 3A :         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 3B semicolon         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 3C <         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 3D =         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 3E >         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 3F ?         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 40 @         
  __SEGGER_RTL_CTYPE_UPPER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 41 A         
  __SEGGER_RTL_CTYPE_UPPER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 42 B         
  __SEGGER_RTL_CTYPE_UPPER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 43 C         
  __SEGGER_RTL_CTYPE_UPPER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 44 D         
  __SEGGER_RTL_CTYPE_UPPER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 45 E         
  __SEGGER_RTL_CTYPE_UPPER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 46 F         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 47 G         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 48 H         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 49 I         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 4A J         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 4B K         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 4C L         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 4D M         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 4E N         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 4F O         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 50 P         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 51 Q         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 52 R         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 53 S         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 54 T         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 55 U         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 56 V         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 57 W         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 58 X         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 59 Y         
  __SEGGER_RTL_CTYPE_UPPER,                                                       // 5A Z         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 5B [         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 5C (backslash)
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 5D ]         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 5E ^         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 5F _         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 60 `         
  __SEGGER_RTL_CTYPE_LOWER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 61 a         
  __SEGGER_RTL_CTYPE_LOWER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 62 b         
  __SEGGER_RTL_CTYPE_LOWER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 63 c         
  __SEGGER_RTL_CTYPE_LOWER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 64 d         
  __SEGGER_RTL_CTYPE_LOWER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 65 e         
  __SEGGER_RTL_CTYPE_LOWER | __SEGGER_RTL_CTYPE_XDIGIT,                           // 66 f         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 67 g         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 68 h         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 69 i         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 6A j         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 6B k         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 6C l         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 6D m         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 6E n         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 6F o         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 70 p         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 71 q         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 72 r         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 73 s         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 74 t         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 75 u         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 76 v         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 77 w         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 78 x         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 79 y         
  __SEGGER_RTL_CTYPE_LOWER,                                                       // 7A z         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 7B {         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 7C |         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 7D }         
  __SEGGER_RTL_CTYPE_PUNCT,                                                       // 7E ~         
  __SEGGER_RTL_CTYPE_CNTRL,                                                       // 7F (DEL)     
};

// Must match enumeration in __SEGGER_RTL_Int.h
static const unsigned char __SEGGER_RTL_ascii_ctype_mask[__SEGGER_RTL_WC_XDIGIT+1] = {
  0,
  __SEGGER_RTL_CTYPE_ALNUM,
  __SEGGER_RTL_CTYPE_ALPHA,
  __SEGGER_RTL_CTYPE_CNTRL,
  __SEGGER_RTL_CTYPE_DIGIT,
  __SEGGER_RTL_CTYPE_GRAPH,
  __SEGGER_RTL_CTYPE_LOWER,
  __SEGGER_RTL_CTYPE_UPPER,
  __SEGGER_RTL_CTYPE_SPACE,
  __SEGGER_RTL_CTYPE_PRINT,
  __SEGGER_RTL_CTYPE_PUNCT,
  __SEGGER_RTL_CTYPE_BLANK,
  __SEGGER_RTL_CTYPE_XDIGIT,
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_ascii_isctype()
*
*/
static int __SEGGER_RTL_ascii_isctype(int ch, int type) {
  if (0 <= ch && ch <= 0x7F) {
    return __SEGGER_RTL_ascii_ctype_map[ch] & __SEGGER_RTL_ascii_ctype_mask[type];
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_ascii_toupper()
*
*/
static int __SEGGER_RTL_ascii_toupper(int ch) {
  if ('a' <= ch && ch <= 'z') {
    return ch - 'a' + 'A';
  } else {
    return ch;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_ascii_tolower()
*
*/
static int __SEGGER_RTL_ascii_tolower(int ch) {
  if ('A' <= ch && ch <= 'Z') {
    return ch - 'A' + 'a';
  } else {
    return ch;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_ascii_iswctype()
*
*/
static int __SEGGER_RTL_ascii_iswctype(__SEGGER_RTL_WINT_T ch, int type) {
  if (0 <= ch && ch <= 0x7F) {
    return __SEGGER_RTL_ascii_ctype_map[ch] & __SEGGER_RTL_ascii_ctype_mask[type];
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_ascii_towupper()
*
*/
static __SEGGER_RTL_WINT_T __SEGGER_RTL_ascii_towupper(__SEGGER_RTL_WINT_T ch) {
  if ('a' <= ch && ch <= 'z') {
    return ch - 'a' + 'A';
  } else {
    return ch;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_ascii_towlower()
*
*/
static __SEGGER_RTL_WINT_T __SEGGER_RTL_ascii_towlower(__SEGGER_RTL_WINT_T ch) {
  if ('A' <= ch && ch <= 'Z') {
    return ch - 'A' + 'a';
  } else {
    return ch;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_set_locale()
*
*/
static char * __SEGGER_RTL_set_locale(int category, const char *locale, int set) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  //
  __SEGGER_RTL_USE_PARA(category);
  __SEGGER_RTL_USE_PARA(locale);
  __SEGGER_RTL_USE_PARA(set);
  //
  return NULL;
  //
#else
  //
  int i, j;
  const __SEGGER_RTL_locale_t *loc;
  //
  // Found the the locale, set whatever we need.
  //
  if (category == LC_ALL) {
    //
    // Special category to set all of them.
    //
    for (i = 0; i < __SEGGER_RTL_MAX_CATEGORY; ++i) {
      //
      // Need to skip down the locale list to find the locale.
      //
      if ((strchr)(locale, ';') != NULL) {
        //
        // This is a locale list, so skip down the list of locales...
        //
        for (j = 0; j < i; ++j, ++locale) {
          //
          // If there is no separator, we have a malformed locale
          // list that does not contain enough categories in it.
          //
          if ((locale = (strchr)(locale, ';')) == 0) {
            return 0;
          }
        }
      }
      //
      // Find the locale associated with this name.  If we can't find
      // the locale, we fail.
      //
      if ((loc = __SEGGER_RTL_find_locale(locale)) == NULL) {
        return NULL;
      }
      //
      // Set the locale.
      //
      if (set) {
        __SEGGER_RTL_current_locale()->__category[i] = loc;
      }
    }
  } else if (0 <= category && category < __SEGGER_RTL_MAX_CATEGORY) {
    //
    // Find the locale associated with this name.  If we can't find
    // the locale, we fail.
    //
    if ((loc = __SEGGER_RTL_find_locale(locale)) == 0) {
      return 0;
    }
    //
    // Set the single category specified.
    //
    if (set) {
      __SEGGER_RTL_current_locale()->__category[category] = loc;
    }
  } else {
    //
    // Invalid category, so failure.
    //
    return 0;
  }
  //
  // Return previous locale.
  //
  return set ? __SEGGER_RTL_locale_name_buffer : "";
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_isctype()
*
*/
static int __SEGGER_RTL_unicode_isctype(int ch, int type) {
  // 1:1 correspondence between narrow chars and wide chars in UTF-8 (ISO-8859-1)
  // apart from the wrinkle translating EOF to WEOF.
  return __SEGGER_RTL_unicode_iswctype(ch, type);
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswspace()
*
*/
static int __SEGGER_RTL_unicode_iswspace(wint_t ch) {
  // Based on Unicode 6.2.  HT (U+0009), LF (U+000a), VT (U+000b),
  // FF (U+000c), and CR (U+000d), with all characters from general
  // category Zs which are not marked as <noBreak>.
  return ((0x0009 <= ch && ch <= 0x000D) ||
          ch == 0x0020 ||
          ch == 0x1680 ||
          ch == 0x180E ||
          (0x2000 <= ch && ch <= 0x200A && ch != 0x2007) ||
          ch == 0x205F ||
          ch == 0x3000);
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswxdigit()
*
*/
static int __SEGGER_RTL_unicode_iswxdigit(wint_t ch) {
  return (ch <= 0x0030 && ch <= 0x0039) ||
         (ch <= 0x0041 && ch <= 0x0046) ||
         (ch <= 0x0061 && ch <= 0x0066);
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswdigit()
*
*/
static int __SEGGER_RTL_unicode_iswdigit(wint_t ch) {
  return 0x0030 <= ch && ch <= 0x0039;
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswalpha()
*
*/
static int __SEGGER_RTL_unicode_iswalpha(wint_t ch) {
  void *p;

  // Search BMP singletons.
  p = bsearch(&ch,
              __SEGGER_RTL_unicode_bmp_isalpha_singleton,
              sizeof(__SEGGER_RTL_unicode_bmp_isalpha_singleton) / sizeof(__SEGGER_RTL_unicode_bmp_isalpha_singleton[0]),
              sizeof(__SEGGER_RTL_unicode_bmp_isalpha_singleton[0]),
              __SEGGER_RTL_unicode_set_bmp_singleton_search);
  if (p)
    return 1;

  // Search BMP ranges.
  p = bsearch(&ch,
              __SEGGER_RTL_unicode_bmp_isalpha_range,
              sizeof(__SEGGER_RTL_unicode_bmp_isalpha_range) / sizeof(__SEGGER_RTL_unicode_bmp_isalpha_range[0]),
              sizeof(__SEGGER_RTL_unicode_bmp_isalpha_range[0]),
              __SEGGER_RTL_unicode_set_bmp_range_search);
  if (p)
    return 1;

#if __SEGGER_RTL_SIZEOF_WCHAR_T != 2
  // Search non-BMP singletons.
  p = bsearch(&ch,
              __SEGGER_RTL_unicode_nonbmp_isalpha_singleton,
              sizeof(__SEGGER_RTL_unicode_nonbmp_isalpha_singleton) / sizeof(__SEGGER_RTL_unicode_nonbmp_isalpha_singleton[0]),
              sizeof(__SEGGER_RTL_unicode_nonbmp_isalpha_singleton[0]),
              __SEGGER_RTL_unicode_set_nonbmp_singleton_search);
  if (p)
    return 1;

  // Search non-BMP ranges.
  p = bsearch(&ch,
              __SEGGER_RTL_unicode_nonbmp_isalpha_range,
              sizeof(__SEGGER_RTL_unicode_nonbmp_isalpha_range) / sizeof(__SEGGER_RTL_unicode_nonbmp_isalpha_range[0]),
              sizeof(__SEGGER_RTL_unicode_nonbmp_isalpha_range[0]),
              __SEGGER_RTL_unicode_set_nonbmp_range_search);
  if (p)
    return 1;
#endif

  // No mapping.
  return 0;
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswalnum()
*
*/
static int __SEGGER_RTL_unicode_iswalnum(wint_t ch) {
  return __SEGGER_RTL_unicode_iswdigit(ch) || __SEGGER_RTL_unicode_iswalpha(ch);
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswlower()
*
*/
static int __SEGGER_RTL_unicode_iswlower(wint_t ch) {
  return __SEGGER_RTL_unicode_towupper(ch) != ch;
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswupper()
*
*/
static int __SEGGER_RTL_unicode_iswupper(wint_t ch) {
  return __SEGGER_RTL_unicode_towlower(ch) != ch;
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswprint()
*
*/
static int __SEGGER_RTL_unicode_iswprint(wint_t ch) {
  void *p;

  // Search BMP singletons.
  p = bsearch(&ch,
              __SEGGER_RTL_unicode_bmp_isprint_singleton,
              sizeof(__SEGGER_RTL_unicode_bmp_isprint_singleton) / sizeof(__SEGGER_RTL_unicode_bmp_isprint_singleton[0]),
              sizeof(__SEGGER_RTL_unicode_bmp_isprint_singleton[0]),
              __SEGGER_RTL_unicode_set_bmp_singleton_search);
  if (p)
    return 1;

  // Search BMP ranges.
  p = bsearch(&ch,
              __SEGGER_RTL_unicode_bmp_isprint_range,
              sizeof(__SEGGER_RTL_unicode_bmp_isprint_range) / sizeof(__SEGGER_RTL_unicode_bmp_isprint_range[0]),
              sizeof(__SEGGER_RTL_unicode_bmp_isprint_range[0]),
              __SEGGER_RTL_unicode_set_bmp_range_search);
  if (p)
    return 1;

#if __SEGGER_RTL_SIZEOF_WCHAR_T != 2
  // Search non-BMP singletons.
  p = bsearch(&ch,
              __SEGGER_RTL_unicode_nonbmp_isprint_singleton,
              sizeof(__SEGGER_RTL_unicode_nonbmp_isprint_singleton) / sizeof(__SEGGER_RTL_unicode_nonbmp_isprint_singleton[0]),
              sizeof(__SEGGER_RTL_unicode_nonbmp_isprint_singleton[0]),
              __SEGGER_RTL_unicode_set_nonbmp_singleton_search);
  if (p)
    return 1;

  // Search non-BMP ranges.
  p = bsearch(&ch,
              __SEGGER_RTL_unicode_nonbmp_isprint_range,
              sizeof(__SEGGER_RTL_unicode_nonbmp_isprint_range) / sizeof(__SEGGER_RTL_unicode_nonbmp_isprint_range[0]),
              sizeof(__SEGGER_RTL_unicode_nonbmp_isprint_range[0]),
              __SEGGER_RTL_unicode_set_nonbmp_range_search);
  if (p)
    return 1;
#endif

  // No mapping.
  return 0;
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswgraph()
*
*/
static int __SEGGER_RTL_unicode_iswgraph(wint_t ch) {
  return __SEGGER_RTL_unicode_iswprint(ch) && !__SEGGER_RTL_unicode_iswspace(ch);
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswpunct()
*
*/
static int __SEGGER_RTL_unicode_iswpunct(wint_t ch) {
  return __SEGGER_RTL_unicode_iswgraph(ch) && !__SEGGER_RTL_unicode_iswalnum(ch);
}

/*********************************************************************
*
*       __SEGGER_RTL_iswctype_l()
*
*/
static wctype_t __SEGGER_RTL_iswctype_l(int ch, int type, locale_t loc) {
  return loc->__category[LC_CTYPE]->codeset->__iswctype(ch, type);
}

/*********************************************************************
*
*       __SEGGER_RTL_isctype()
*
*/
static wctype_t __SEGGER_RTL_isctype(int ch, int type) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return __SEGGER_RTL_ascii_isctype(ch, type);
#else
  return __SEGGER_RTL_global_locale_category(LC_CTYPE)->codeset->__isctype(ch, type);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_iswctype()
*
*/
static wctype_t __SEGGER_RTL_iswctype(int ch, int type) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return __SEGGER_RTL_isctype(ch, type);
#else
  return __SEGGER_RTL_global_locale_category(LC_CTYPE)->codeset->__iswctype(ch, type);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_toupper()
*
*/
static int __SEGGER_RTL_unicode_toupper(int ch) {
  long cp = __SEGGER_RTL_unicode_towupper(ch);
  return cp == WEOF ? ch : cp;
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_tolower()
*
*/
static int __SEGGER_RTL_unicode_tolower(int ch) {
  long cp = __SEGGER_RTL_unicode_towlower(ch);
  return cp == WEOF ? ch : cp;
}

/*********************************************************************
*
*       __SEGGER_RTL_isctype_l()
*
*/
static wctype_t __SEGGER_RTL_isctype_l(int ch, int type, locale_t loc) {
  return loc->__category[LC_CTYPE]->codeset->__iswctype(ch, type);
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswblank()
*
*/
static int __SEGGER_RTL_unicode_iswblank(wint_t ch) {
  // Based on Unicode 6.2.  Tab (U+0009) with all characters
  // from general category Zs which are not marked as <noBreak>.
  return (ch == 0x0009 ||
          ch == 0x0020 ||
          ch == 0x1680 ||
          ch == 0x180E ||
          (0x2000 <= ch && ch <= 0x200A && ch != 0x2007) ||
          ch == 0x205F ||
          ch == 0x3000);
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswcntrl()
*
*/
static int __SEGGER_RTL_unicode_iswcntrl(wint_t ch) {
  // Unicode 6.2, includes general categories Cc, Zl, and Zp.
  return ((0x0000 <= ch && ch <= 0x001F) ||
          (0x007F <= ch && ch <= 0x009F) ||
          (0x2028 <= ch && ch <= 0x2029));
}

/*********************************************************************
*
*       __SEGGER_RTL_encode_locale()
*
*/
static char * __SEGGER_RTL_encode_locale(char *buffer, locale_t locale) {
  int i;
  //
  // Zap buffer.
  //
  buffer[0] = 0;
  //
  // Append all locale names.
  //
  for (i = 0; i < __SEGGER_RTL_MAX_CATEGORY; ++i) {
    strcat(buffer, locale->__category[i]->name);
    if (i != __SEGGER_RTL_MAX_CATEGORY-1) {
      strcat(buffer, ";");
    }
  }
  //
  // Return converted name.
  //
  return buffer;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_current_locale()
*
*/
struct __SEGGER_RTL_POSIX_locale_s * __SEGGER_RTL_current_locale(void) {
#if !__SEGGER_RTL_MINIMAL_LOCALE
  //
  if (__SEGGER_RTL_locale_ptr == NULL) {
    //
    // No per-thread locale currently set, default to thread-local "global" locale.
    //
    return &__SEGGER_RTL_global_locale;
  }
  //
  return __SEGGER_RTL_locale_ptr;
#else
  return NULL;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_iswctype()
*
*/
int __SEGGER_RTL_unicode_iswctype(wint_t ch, wctype_t ty) {
  switch (ty) {
  case __SEGGER_RTL_WC_ALNUM:  return __SEGGER_RTL_unicode_iswalnum(ch);
  case __SEGGER_RTL_WC_ALPHA:  return __SEGGER_RTL_unicode_iswalpha(ch);
  case __SEGGER_RTL_WC_BLANK:  return __SEGGER_RTL_unicode_iswblank(ch);
  case __SEGGER_RTL_WC_CNTRL:  return __SEGGER_RTL_unicode_iswcntrl(ch);
  case __SEGGER_RTL_WC_DIGIT:  return __SEGGER_RTL_unicode_iswdigit(ch);
  case __SEGGER_RTL_WC_GRAPH:  return __SEGGER_RTL_unicode_iswgraph(ch);
  case __SEGGER_RTL_WC_LOWER:  return __SEGGER_RTL_unicode_iswlower(ch);
  case __SEGGER_RTL_WC_PRINT:  return __SEGGER_RTL_unicode_iswprint(ch);
  case __SEGGER_RTL_WC_PUNCT:  return __SEGGER_RTL_unicode_iswpunct(ch);
  case __SEGGER_RTL_WC_SPACE:  return __SEGGER_RTL_unicode_iswspace(ch);
  case __SEGGER_RTL_WC_UPPER:  return __SEGGER_RTL_unicode_iswupper(ch);
  case __SEGGER_RTL_WC_XDIGIT: return __SEGGER_RTL_unicode_iswxdigit(ch);
  default:                     return 0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_find_locale()
*
*/
const __SEGGER_RTL_locale_t * __SEGGER_RTL_find_locale(const char *locale) {
  //
  // C and POSIX locales are identical.
  //
  if (!locale)
    return 0;
  if (*locale == 0 ||
      __SEGGER_RTL_compare_locale_name(locale, "C") == 0 ||
      __SEGGER_RTL_compare_locale_name(locale, "POSIX") == 0) {
    return &__SEGGER_RTL_c_locale;
  } else {
    // User may supply a locale lookup function.
    return __SEGGER_RTL_X_find_locale(locale);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_global_locale_category()
*
*/
#if !__SEGGER_RTL_MINIMAL_LOCALE
const __SEGGER_RTL_locale_t * __SEGGER_RTL_global_locale_category(int __category) {
  //
  // Category can't be LC_ALL!
  //
  return __SEGGER_RTL_current_locale()->__category[__category];
}
#endif

/*********************************************************************
*
*       __SEGGER_RTL_locale_category()
*
*/
const __SEGGER_RTL_locale_t * __SEGGER_RTL_locale_category(struct __SEGGER_RTL_POSIX_locale_s *__locale, int __category) {
  //
  // Category can't be LC_ALL!
  //
  return __locale->__category[__category];
}

/*********************************************************************
*
*       __SEGGER_RTL_compare_locale_name()
*
*/
int __SEGGER_RTL_compare_locale_name(const char *str0, const char *str1) {
  for (;;) {
    //
    // Get candidates to match.
    //
    char c0 = *str0++;
    char c1 = *str1++;
    //
    // Semicolons separate locale names with a constructed
    // locale from setlocale().
    //
    if (c0 == ';') { c0 = 0; }
    if (c1 == ';') { c1 = 0; }
    //
    // If different, no match.
    //
    if (c0 != c1) {
      return 1;
    }
    //
    // Both the same.  If end of string, matched.
    //
    if (c0 == 0) {
      return 0;
    }
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_map_range_search()
*
*/
int __SEGGER_RTL_unicode_map_range_search(const void *k0, const void *k1) {
  unsigned cp = *(const unsigned *)k0;
  const __SEGGER_RTL_unicode_map_bmp_range_t *cursor = k1;

  if (cp < cursor->min) {
    return -1;
  } else if (cursor->min <= cp && cp <= cursor->max) {
    return 0;
  } else {
    return +1;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_map_singleton_search()
*
*/
int __SEGGER_RTL_unicode_map_singleton_search(const void *k0, const void *k1) {
  unsigned cp = *(const unsigned *)k0;
  const __SEGGER_RTL_unicode_map_bmp_singleton_t *cursor = k1;
  //
  if (cp < cursor->cp) {
    return -1;
  } else if (cursor->cp == cp) {
    return 0;
  } else {
    return +1;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_towupper()
*
*/
wint_t __SEGGER_RTL_unicode_towupper(wint_t ch) {
  const __SEGGER_RTL_unicode_map_bmp_singleton_t *cursor_0;
  const __SEGGER_RTL_unicode_map_bmp_range_t *cursor_1;
  //
  // Search first range map; likely to hit ASCII range, in general, for Latin alphabets.
  //
  cursor_1 = bsearch(&ch,
                     __SEGGER_RTL_unicode_bmp_toupper_range1_map,
                     sizeof(__SEGGER_RTL_unicode_bmp_toupper_range1_map) / sizeof(__SEGGER_RTL_unicode_bmp_toupper_range1_map[0]),
                     sizeof(__SEGGER_RTL_unicode_bmp_toupper_range1_map[0]),
                     __SEGGER_RTL_unicode_map_range_search);
  if (cursor_1) {
    return cursor_1->map + ch-cursor_1->min;
  }
  //
  // Search isolated mappings.
  //
  cursor_0 = bsearch(&ch,
                     __SEGGER_RTL_unicode_bmp_toupper_singleton_map,
                     sizeof(__SEGGER_RTL_unicode_bmp_toupper_singleton_map) / sizeof(__SEGGER_RTL_unicode_bmp_toupper_singleton_map[0]),
                     sizeof(__SEGGER_RTL_unicode_bmp_toupper_singleton_map[0]),
                     __SEGGER_RTL_unicode_map_singleton_search);
  if (cursor_0) {
    return cursor_0->map;
  }
  //
  // Still no luck, try second range map.
  //
  cursor_1 = bsearch(&ch,
                     __SEGGER_RTL_unicode_bmp_toupper_range2_map,
                     sizeof(__SEGGER_RTL_unicode_bmp_toupper_range2_map) / sizeof(__SEGGER_RTL_unicode_bmp_toupper_range2_map[0]),
                     sizeof(__SEGGER_RTL_unicode_bmp_toupper_range2_map[0]),
                     __SEGGER_RTL_unicode_map_range_search);
  //
  // Narrow match on found record.
  //
  if (cursor_1 && (cursor_1->min&1) == (ch&1)) {
    return cursor_1->map + ch-cursor_1->min;
  }
  //
  // No mapping, return unchanged.
  //
  return ch;
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_towlower()
*
*/
wint_t __SEGGER_RTL_unicode_towlower(wint_t ch) {
  const __SEGGER_RTL_unicode_map_bmp_singleton_t *cursor_0;
  const __SEGGER_RTL_unicode_map_bmp_range_t *cursor_1;
  //
  // Search first range map; likely to hit ASCII range, in general, for Latin alphabets.
  //
  cursor_1 = bsearch(&ch,
                     __SEGGER_RTL_unicode_bmp_tolower_range1_map,
                     sizeof(__SEGGER_RTL_unicode_bmp_tolower_range1_map) / sizeof(__SEGGER_RTL_unicode_bmp_tolower_range1_map[0]),
                     sizeof(__SEGGER_RTL_unicode_bmp_tolower_range1_map[0]),
                     __SEGGER_RTL_unicode_map_range_search);
  if (cursor_1) {
    return cursor_1->map + ch-cursor_1->min;
  }
  //
  // Search isolated mappings.
  //
  cursor_0 = bsearch(&ch,
                     __SEGGER_RTL_unicode_bmp_tolower_singleton_map,
                     sizeof(__SEGGER_RTL_unicode_bmp_tolower_singleton_map) / sizeof(__SEGGER_RTL_unicode_bmp_tolower_singleton_map[0]),
                     sizeof(__SEGGER_RTL_unicode_bmp_tolower_singleton_map[0]),
                     __SEGGER_RTL_unicode_map_singleton_search);
  if (cursor_0) {
    return cursor_0->map;
  }
  //
  // Still no luck, try second range map.
  //
  cursor_1 = bsearch(&ch,
                     __SEGGER_RTL_unicode_bmp_tolower_range2_map,
                     sizeof(__SEGGER_RTL_unicode_bmp_tolower_range2_map) / sizeof(__SEGGER_RTL_unicode_bmp_tolower_range2_map[0]),
                     sizeof(__SEGGER_RTL_unicode_bmp_tolower_range2_map[0]),
                     __SEGGER_RTL_unicode_map_range_search);
  //
  // Narrow match on found record.
  //
  if (cursor_1 && (cursor_1->min&1) == (ch&1)) {
    return cursor_1->map + ch-cursor_1->min;
  }
  //
  // No mapping, return unchanged.
  //
  return ch;
}

/*********************************************************************
*
*       __SEGGER_RTL_mb_cur_max()
*
*/
int __SEGGER_RTL_mb_cur_max(void) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return 1;
#else
  return __SEGGER_RTL_mb_max(__SEGGER_RTL_current_locale());
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_mb_max()
*
*/
int __SEGGER_RTL_mb_max(const struct __SEGGER_RTL_POSIX_locale_s *loc) {
  __SEGGER_RTL_USE_PARA(loc);
  //
  return 4;  // see __SEGGER_RTL_utf8_wctomb()
}

/*********************************************************************
*
*       __SEGGER_RTL_init_mbstate()
*
*/
void __SEGGER_RTL_init_mbstate(struct __mbstate_s *state) {
  state->__state = 0;
  state->__wchar = 0;
}

/*********************************************************************
*
*       __SEGGER_RTL_utf8_mbtowc()
*
*/
int __SEGGER_RTL_utf8_mbtowc(wchar_t *pwc, const char *s, size_t len, struct __mbstate_s *mbstate) {
  int state = mbstate->__state;
  wchar_t wc = mbstate->__wchar;
  unsigned char *su = (unsigned char *)s;
  unsigned u;
  //
  for (;;) {
    if (len == 0) {
      //
      // Incomplete conversion.
      //
      mbstate->__wchar = wc;
      mbstate->__state = state;
      return -2;
    }
    //
    // Grab next byte from input.
    //
    u = *su++;
    --len;
    //
    if (state > 0) {
      // Expecting a continuation byte, but it has to be in the
      // appropriate form.
      if ((u & 0xC0) != 0x80) {
        // All continuation bytes are 10xxxxxx; if not, bad sequence.
        return -EILSEQ;
      }
      //
      // Shift byte into character.
      //
      wc = (wc << 6) | (u & 0x3F);
      --state;
    } else if ((u & 0x80) == 0) {
      //
      // 0xxxxxxx is a single-byte sequence.
      //
      wc = u;
    } else if ((u & 0xE0) == 0xC0) {
      //
      // 110xxxxx is a 2-byte sequence.
      //
      wc = u & 0x1F;
      state = 1;
#if __SEGGER_RTL_SIZEOF_WCHAR_T == 4
    } else if ((u & 0xF0) == 0xE0) {
      //
      // 1110xxxx is a 3-byte sequence.
      //
      wc = u & 0x0F;
      state = 2;
    } else if ((u & 0xF8) == 0xF0) {
      //
      // 11110xxx is a 4-byte sequence.
      //
      wc = u & 0x07;
      state = 3;
#endif
    } else {
      //
      // We don't support other encodings (5 and 6-byte sequences).
      //
      return -EILSEQ;
    }
    //
    if (state == 0) {
      //
      // All continuation bytes present and correct, generate an output!
      //
      if (pwc) {
        *pwc++ = wc;
      }
      //
      // Return to initial state.
      //
      __SEGGER_RTL_init_mbstate(mbstate);
      //
      // If the generated output is zero, return a zero-length value.
      //
      return wc == 0 ? 0 : (const char *)su - s;
    }
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_utf8_wctomb()
*
*/
int __SEGGER_RTL_utf8_wctomb(char *s, wchar_t wc, struct __mbstate_s *mbstate) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  //
  __SEGGER_RTL_USE_PARA(mbstate);
  //
  if (0 <= wc && wc <= 0x7F) {
    s[0] = (char)wc;
    return 1;
  } else {
    // Negative value indicates an error; client may set errno from this.
    return -EILSEQ;
  }
  //
#else
  //
  __SEGGER_RTL_USE_PARA(mbstate);
  //
  // Need to deal with unsigned characters.
  //
  unsigned long u;
  //
  // Reduce to unsigned data.  Can't encode EOF.
  //
  u = wc;
  //
  // Only convert what we know...
  //
  if (u < 0x80) {
    s[0] = (char)u;
    return 1;
  } else if (u < 0x800) {
    s[0] = (char)(0xC0 | (u >> 6));
    s[1] = (char)(0x80 | (u & 0x3F));
    return 2;
  } else if (__SEGGER_RTL_SIZEOF_WCHAR_T == 2 || u < 0x10000) {
    s[0] = (char)(0xE0 | (u >> 12));
    s[1] = (char)(0x80 | ((u >> 6) & 0x3F));
    s[2] = (char)(0x80 | (u & 0x3F));
    return 3;
#if __SEGGER_RTL_SIZEOF_WCHAR_T == 4
  } else if (u <= 0x10FFFF) {
    s[0] = (char)(0xF0 | (u >> 18));
    s[1] = (char)(0x80 | ((u >> 12) & 0x3F));
    s[2] = (char)(0x80 | ((u >> 6) & 0x3F));
    s[3] = (char)(0x80 | (u & 0x3F));
    return 4;
#endif
  } else {
    // Negative value indicates an error; client may set errno from this.
    return -EILSEQ;
  }
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_ascii_mbtowc()
*
*/
int __SEGGER_RTL_ascii_mbtowc(wchar_t *pwc, const char *s, size_t len, struct __mbstate_s *mbstate) {
  if (!s)
    return 0;
  unsigned char *su = (unsigned char *)s;
  unsigned u;
  //
  for (;;) {
    //
    // All done?
    //
    if (len == 0) {
      return 0;
    }
    //
    // Grab next byte from input.
    //
    u = *su++;
    --len;
    //
    // Only characters 0-127 are considered.
    //
    if (u > 0x7F) {
      // Invalid encoding.
      return -EILSEQ;
    }
    //
    // Generate an output.
    //
    if (pwc) {
      *pwc = u;
    }
    //
    // Return to initial state.
    //
    __SEGGER_RTL_init_mbstate(mbstate);
    //
    // If the generated output is zero, return a zero-length value.
    //
    return u == 0 ? 0 : (const char *)su - s;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_ascii_wctomb()
*
*/
int __SEGGER_RTL_ascii_wctomb(char *s, wchar_t wc, struct __mbstate_s *mbstate) {
  unsigned long u;
  //
  __SEGGER_RTL_USE_PARA(mbstate);
  //
  // Reduce to unsigned data.  Can't encode EOF.
  //
  u = wc;
  //
  // Discard unsupported wide characters....
  if (u > 0x7F) {
    //
    // Negative value indicates an error; client may set errno from this.
    //
    return -EILSEQ;
  }
  //
  // Only convert what we know...
  //
  s[0] = (char)u;
  return 1;
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_set_bmp_singleton_search()
*
*/
int __SEGGER_RTL_unicode_set_bmp_singleton_search(const void *k0, const void *k1) {
  wint_t key       = *(wint_t *)k0;
  wint_t candidate = *(unsigned short *)k1;
  //
  if (key < candidate) {
    return -1;
  } else if (key == candidate) {
    return 0;
  } else {
    return +1;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_set_nonbmp_singleton_search()
*
*/
int __SEGGER_RTL_unicode_set_nonbmp_singleton_search(const void *k0, const void *k1) {
  wint_t key = *(wint_t *)k0;
  wint_t candidate = *(long *)k1;
  //
  if (key < candidate) {
    return -1;
  } else if (key == candidate) {
    return 0;
  } else {
    return +1;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_set_bmp_range_search()
*
*/
int __SEGGER_RTL_unicode_set_bmp_range_search(const void *k0, const void *k1) {
  wint_t key = *(wint_t *)k0;
  const __SEGGER_RTL_unicode_set_bmp_range_t *candidate = k1;
  //
  if (key < candidate->min) {
    return -1;
  } else if (candidate->min <= key && key <= candidate->max) {
    return 0;
  } else {
    return +1;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_unicode_set_nonbmp_range_search()
*
*/
int __SEGGER_RTL_unicode_set_nonbmp_range_search(const void *k0, const void *k1) {
  wint_t key = *(wint_t *)k0;
  const __SEGGER_RTL_unicode_set_nonbmp_range_t *candidate = k1;
  //
  if (key < candidate->min) {
    return -1;
  } else if (candidate->min <= key && key <= candidate->max) {
    return 0;
  } else {
    return +1;
  }
}

/*********************************************************************
*
*       isalpha()
*
*  Function description
*    Is character alphabetic?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns true if the character c is alphabetic in the current locale.
*    That is, any character for which isupper() or islower() returns true
*    is considered alphabetic in addition to any of the locale-specific set of
*    alphabetic characters for which none of iscntrl(), isdigit(), ispunct(),
*    or isspace() is true.
*    
*    In the C locale, isalpha() returns nonzero (true) if and only if
*    isupper() or islower() return true for value of the argument c. 
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API isalpha(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_ALPHA);
#endif
}

/*********************************************************************
*
*       isalpha_l()
*
*  Function description
*    Is character alphabetic, per locale? (POSIX.1).
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns true if the character c is alphabetic in the locale loc.
*    That is, any character for which isupper() or islower() returns true
*    is considered alphabetic in addition to any of the locale-specific set of
*    alphabetic characters for which none of iscntrl_l(), isdigit_l(),
*    ispunct_l(), or isspace_l() is true in the locale loc.
*    
*    In the C locale, isalpha_l() returns nonzero (true) if and only if
*    isupper_l() or islower_l() return true for value of the argument c. 
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API isalpha_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return isalpha((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_ALPHA, loc);
#endif
}

/*********************************************************************
*
*       isupper()
*
*  Function description
*    Is character an uppercase letter?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is an uppercase letter in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API isupper(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return 'A' <= c && c <= 'Z';
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_UPPER);
#endif
}

/*********************************************************************
*
*       isupper_l()
*
*  Function description
*    Is character an uppercase letter, per locale? (POSIX.1).
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is an uppercase letter in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API isupper_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return isupper((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_UPPER, loc);
#endif
}

/*********************************************************************
*
*       islower()
*
*  Function description
*    Is character a lowercase letter?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a lowercase letter in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API islower(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return 'a' <= c && c <= 'z';
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_LOWER);
#endif
}

/*********************************************************************
*
*       islower_l()
*
*  Function description
*    Is character a lowercase letter, per locale? (POSIX.1).
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a lowercase letter in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API islower_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return islower((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_LOWER, loc);
#endif
}

/*********************************************************************
*
*       isdigit()
*
*  Function description
*    Is character a decimal digit?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a digit in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API isdigit(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return '0' <= (unsigned)c && (unsigned)c <= '9';
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_DIGIT);
#endif
}

/*********************************************************************
*
*       isdigit_l()
*
*  Function description
*    Is character a decimal digit, per locale? (POSIX.1)
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a digit in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API isdigit_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return isdigit((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_DIGIT, loc);
#endif
}

/*********************************************************************
*
*       isxdigit()
*
*  Function description
*    Is character a hexadecimal digit?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a hexadecimal digit in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API isxdigit(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return ('0' <= c && c <= '9') ||
         ('a' <= c && c <= 'f') ||
         ('A' <= c && c <= 'F');
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_XDIGIT);
#endif
}

/*********************************************************************
*
*       isxdigit_l()
*
*  Function description
*    Is character a hexadecimal digit, per locale? (POSIX.1).
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a hexadecimal digit in the current locale.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API isxdigit_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return isxdigit((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_XDIGIT, loc);
#endif
}

/*********************************************************************
*
*       isspace()
*
*  Function description
*    Is character a whitespace character?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a standard white-space character in the current locale.
*    The standard white-space characters  are space, form feed,
*    new-line, carriage return, horizontal tab, and vertical tab. 
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API isspace(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return (0x09 <= c && c <= 0x0D) || (c == ' ');
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_SPACE);
#endif
}

/*********************************************************************
*
*       isspace_l()
*
*  Function description
*    Is character a whitespace character, per locale? (POSIX.1).
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a standard white-space character in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int isspace_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return isspace((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_SPACE, loc);
#endif
}

/*********************************************************************
*
*       ispunct()
*
*  Function description
*    Is character a punctuation mark?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) for every printing character for which 
*    neither isspace() nor isalnum() is true in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API ispunct(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return ('!' <= c && c <= '/') ||
         (':' <= c && c <= '@') ||
         ('[' <= c && c <= '`') ||
         ('{' <= c && c <= '~');
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_PUNCT);
#endif
}

/*********************************************************************
*
*       ispunct_l()
*
*  Function description
*    Is character a punctuation mark, per locale? (POSIX.1).
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) for every printing character for which 
*    neither isspace() nor isalnum() is true in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API ispunct_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return ispunct((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_PUNCT, loc);
#endif
}

/*********************************************************************
*
*       isalnum()
*
*  Function description
*    Is character alphanumeric?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is an alphabetic or numeric character in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API isalnum(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return ('A' <= c && c <= 'Z') ||
         ('a' <= c && c <= 'z') ||
         ('0' <= c && c <= '9');
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_ALNUM);
#endif
}

/*********************************************************************
*
*       isalnum_l()
*
*  Function description
*    Is character alphanumeric, per locale? (POSIX.1).
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is an alphabetic or numeric character in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API isalnum_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return isalnum((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_ALNUM, loc);
#endif
}

/*********************************************************************
*
*       isprint()
*
*  Function description
*    Is character printable?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is any printing character including space in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API isprint(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return 0x20 <= c && c < 0x7F;
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_PRINT);
#endif
}

/*********************************************************************
*
*       isprint_l()
*
*  Function description
*    Is character printable, per locale? (POSIX.1).
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is any printing character including space in the locale loc. 
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API isprint_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return isprint((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_PRINT, loc);
#endif
}

/*********************************************************************
*
*       isgraph()
*
*  Function description
*    Is character any printing character?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is any printing character except space in the current locale. 
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API isgraph(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return (unsigned)c > 0x20 && c != 0x7F;
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_GRAPH);
#endif
}

/*********************************************************************
*
*       isgraph_l()
*
*  Function description
*    Is character any printing character, per locale? (POSIX.1).
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is any printing character except space in the locale loc. 
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API isgraph_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return isgraph((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_GRAPH, loc);
#endif
}

/*********************************************************************
*
*       iscntrl()
*
*  Function description
*    Is character a control?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a control character in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iscntrl(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return (unsigned)c < 0x20 || c == 0x7F;
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_CNTRL);
#endif
}

/*********************************************************************
*
*       iscntrl_l()
*
*  Function description
*    Is character a control, per locale? (POSIX.1).
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a control character in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iscntrl_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return iscntrl((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_CNTRL, loc);
#endif
}

/*********************************************************************
*
*       isblank()
*
*  Function description
*    Is character a blank?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is either a space character or tab character in the current
*    locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API isblank(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return c == ' ' || c == '\t';
#else
  return __SEGGER_RTL_isctype(c, __SEGGER_RTL_WC_BLANK);
#endif
}

/*********************************************************************
*
*       isblank_l()
*
*  Function description
*    Is character a blank, per locale? (POSIX.1).
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is either a space character or the tab character in locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API isblank_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return isblank((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return __SEGGER_RTL_isctype_l(c, __SEGGER_RTL_WC_BLANK, loc);
#endif
}

/*********************************************************************
*
*       isascii()
*
*  Function description
*    Is character a 7-bit ASCII code?
*
*  Parameters
*    c - Character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    has an ASCII code between 0 and 127 in the current locale.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API isascii(int c) {
  return 0 <= c && c <= 127;
}

/*********************************************************************
*
*       isascii_l()
*
*  Function description
*    Is character a 7-bit ASCII code, per locale (POSIX.1)?
*
*  Parameters
*    c   - Character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    has an ASCII code between 0 and 127 in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API isascii_l(int c, locale_t loc) {
  __SEGGER_RTL_USE_PARA(loc);
  //
  return 0 <= c && c <= 127;
}

/*********************************************************************
*
*       tolower()
*
*  Function description
*    Convert uppercase character to lowercase.
*
*  Parameters
*    c - Character to convert.
*
*  Return value
*    Converted character.
*
*  Additional information
*    Converts an uppercase letter to a corresponding lowercase letter. 
*    
*    If the argument c is a character for which isupper() is true and there
*    are one or more corresponding characters, as specified by the current locale,
*    for which islower() is true, the tolower() function returns one of the
*    corresponding characters (always the same one for any given locale);
*    otherwise, the argument is returned unchanged.
*    
*  Notes
*    Even though isupper() can return true for some characters, tolower()
*    may return that uppercase character unchanged as there are no corresponding
*    lowercase characters in the locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API tolower(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return __SEGGER_RTL_ascii_tolower(c);
#else
  return __SEGGER_RTL_global_locale_category(LC_CTYPE)->codeset->__tolower(c);
#endif
}

/*********************************************************************
*
*       tolower_l()
*
*  Function description
*    Convert uppercase character to lowercase, per locale (POSIX.1).
*
*  Parameters
*    c   - Character to convert.
*    loc - Locale used to convert c.
*
*  Return value
*    Converted character.
*
*  Additional information
*    Converts an uppercase letter to a corresponding lowercase letter
*    in locale loc.  If the argument is a character for which isupper_l() is
*    true in locale loc, tolower_l() returns the corresponding lowercase
*    letter; otherwise, the argument is returned unchanged. 
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API tolower_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return tolower((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return loc->__category[LC_CTYPE]->codeset->__tolower(c);
#endif
}

/*********************************************************************
*
*       toupper()
*
*  Function description
*    Convert lowercase character to uppercase.
*
*  Parameters
*    c - Character to convert.
*
*  Return value
*    Converted character.
*
*  Additional information
*    Converts a lowercase letter to a corresponding uppercase letter.
*    
*    If the argument c is a character for which islower() is true and there are
*    one or more corresponding characters, as specified by the current locale, for
*    which isupper() is true, toupper() returns one of the corresponding
*    characters (always the same one for any given locale); otherwise, the argument
*    is returned unchanged.
*    
*  Notes
*    Even though islower() can return true for some characters, toupper()
*    may return that lowercase character unchanged as there are no corresponding
*    uppercase characters in the locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API toupper(int c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return __SEGGER_RTL_ascii_toupper(c);
#else
  return __SEGGER_RTL_global_locale_category(LC_CTYPE)->codeset->__toupper(c);
#endif
}

/*********************************************************************
*
*       toupper_l()
*
*  Function description
*    Convert lowercase character to uppercase, per locale (POSIX.1).
*
*  Parameters
*    c   - Character to convert.
*    loc - Locale used to convert c.
*
*  Return value
*    Converted character.
*
*  Additional information
*    Converts a lowercase letter to a corresponding uppercase letter
*    in locale loc. If the argument c is a character for which islower_l() is
*    true in locale loc, tolower_l() returns the corresponding uppercase letter;
*    otherwise, the argument is returned unchanged.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API toupper_l(int c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return toupper((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return loc->__category[LC_CTYPE]->codeset->__toupper(c);
#endif
}

/*********************************************************************
*
*       towlower()
*
*  Function description
*    Convert uppercase character to lowercase.
*
*  Parameters
*    c - Wide character to convert.
*
*  Return value
*    Converted wide character.
*
*  Additional information
*    Converts an uppercase letter to a corresponding lowercase letter. 
*    
*    If the argument c is a wide character for which iswupper() is true and there
*    are one or more corresponding wide characters, in the current locale, for
*    which iswlower() is true, towlower() returns one (and always the same one
*    for any given locale) of the corresponding wide characters; otherwise, c is
*    returned unchanged.
*
*  Thread safety
*    Safe [if configured].
*/
wint_t __SEGGER_RTL_PUBLIC_API towlower(wint_t c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return tolower(c);
#else
  return __SEGGER_RTL_global_locale_category(LC_CTYPE)->codeset->__towlower(c);
#endif
}

/*********************************************************************
*
*       towlower_l()
*
*  Function description
*    Convert uppercase character to lowercase, per locale (POSIX.1).
*
*  Parameters
*    c   - Wide character to convert.
*    loc - Locale used to convert c.
*
*  Return value
*    Converted wide character.
*
*  Additional information
*    Converts an uppercase letter to a corresponding lowercase letter. 
*    
*    If the argument c is a wide character for which iswupper_l() is true
*    and there are one or more corresponding wide characters, in the locale
*    loc, for which iswlower_l() is true, towlower_l() returns one (and
*    always the same one for any given locale) of the corresponding wide
*    characters; otherwise, c is returned unchanged.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
wint_t __SEGGER_RTL_PUBLIC_API towlower_l(wint_t c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return towlower((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return loc->__category[LC_CTYPE]->codeset->__towlower(c);
#endif
}

/*********************************************************************
*
*       towupper()
*
*  Function description
*    Convert uppercase character to lowercase.
*
*  Parameters
*    c - Wide character to convert.
*
*  Return value
*    Converted wide character.
*
*  Additional information
*    Converts a lowercase letter to a corresponding uppercase letter. 
*    
*    If the argument c is a wide character for which iswlower() is true
*    and there are one or more corresponding wide characters, in the
*    current current locale, for which iswupper() is true, towupper()
*    returns one (and always the same one for any given locale) of the
*    corresponding wide characters; otherwise, c is returned unchanged.
*
*  Thread safety
*    Safe [if configured].
*/
wint_t __SEGGER_RTL_PUBLIC_API towupper(wint_t c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return toupper(c);
#else
  return __SEGGER_RTL_global_locale_category(LC_CTYPE)->codeset->__towupper(c);
#endif
}

/*********************************************************************
*
*       towupper_l()
*
*  Function description
*    Convert uppercase character to lowercase, per locale (POSIX.1).
*
*  Parameters
*    c   - Wide character to convert.
*    loc - Locale used to convert c.
*
*  Return value
*    Converted wide character.
*
*  Additional information
*    Converts a lowercase letter to a corresponding uppercase letter. 
*    
*    If the argument c is a wide character for which iswlower_l() is true
*    and there are one or more corresponding wide characters, in the
*    current locale loc, for which iswupper_l() is true, towupper_l()
*    returns one (and always the same one for any given locale) of the
*    corresponding wide characters; otherwise, c is returned unchanged.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
wint_t __SEGGER_RTL_PUBLIC_API towupper_l(wint_t c, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return towupper((__SEGGER_RTL_USE_PARA(loc), c));
#else
  return loc->__category[LC_CTYPE]->codeset->__towupper(c);
#endif
}

/*********************************************************************
*
*       towctrans()
*
*  Function description
*    Translate character.
*
*  Parameters
*    c - Wide character to convert.
*    t - Mapping to use for conversion.
*
*  Return value
*    Converted wide character.
*
*  Additional information
*    Maps the wide character c using the mapping described by
*    t in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
wint_t __SEGGER_RTL_PUBLIC_API towctrans(wint_t c, wctrans_t t) {
  if (t == __SEGGER_RTL_WT_TOLOWER) {
    return towlower(c);
  } else if (t == __SEGGER_RTL_WT_TOUPPER) {
    return towupper(c);
  } else {
    return c;
  }
}

/*********************************************************************
*
*       towctrans_l()
*
*  Function description
*    Translate character, per locale (POSIX.1).
*
*  Parameters
*    c   - Wide character to convert.
*    t   - Mapping to use for conversion.
*    loc - Locale used for conversion.
*
*  Return value
*    Converted wide character.
*
*  Additional information
*    Maps the wide character c using the mapping described by
*    t in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
wint_t __SEGGER_RTL_PUBLIC_API towctrans_l(wint_t c, wctrans_t t, locale_t loc) {
  if (t == __SEGGER_RTL_WT_TOLOWER) {
    return towlower_l(c, loc);
  } else if (t == __SEGGER_RTL_WT_TOUPPER) {
    return towupper_l(c, loc);
  } else {
    return c;
  }
}

/*********************************************************************
*
*       wctrans()
*
*  Function description
*    Construct character mapping.
*
*  Parameters
*    name - Name of mapping.
*
*  Return value
*    Transformation mapping; zero if mapping unrecognized.
*
*  Additional information
*    Constructs a value of type wctrans_t that describes a mapping
*    between wide characters identified by the string argument property.
*    
*    If property identifies a valid mapping of wide characters in the
*    current locale, wctrans_l() returns a nonzero value that is valid as
*    the second argument to towctrans(); otherwise, it returns zero.
*
*    The only mappings supported are "tolower" and "toupper".
*
*  Thread safety
*    Safe.
*/
wctrans_t __SEGGER_RTL_PUBLIC_API wctrans(const char *name) {
  return __SEGGER_RTL_string_list_encode(__SEGGER_RTL_wctrans_name_list, name) + 1;
}

/*********************************************************************
*
*       wctrans_l()
*
*  Function description
*    Construct character mapping, per locale (POSIX.1).
*
*  Parameters
*    name - Name of mapping.
*    loc  - Locale used for mapping.
*
*  Return value
*    Transformation mapping; zero if mapping unrecognized.
*
*  Additional information
*    Constructs a value of type wctrans_t that describes a mapping
*    between wide characters identified by the string argument property.
*    
*    If property identifies a valid mapping of wide characters in the
*    locale loc, wctrans_l() returns a nonzero value that is valid as
*    the second argument to towctrans(); otherwise, it returns zero.
*
*    The only mappings supported are "tolower" and "toupper".
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
wctrans_t __SEGGER_RTL_PUBLIC_API wctrans_l(const char *name, locale_t loc) {
  __SEGGER_RTL_USE_PARA(loc);
  //
  return __SEGGER_RTL_string_list_encode(__SEGGER_RTL_wctrans_name_list, name) + 1;
}

/*********************************************************************
*
*       wctype()
*
*  Function description
*    Construct character class.
*
*  Parameters
*    name - Name of mapping.
*
*  Return value
*    Character class; zero if class unrecognized.
*
*  Additional information
*    Constructs a value of type wctype_t that describes a class
*    of wide characters identified by the string argument property.
*
*    If property identifies a valid class of wide characters in
*    the current locale, returns a nonzero value that is valid
*    as the second argument to iswctype(); otherwise, it returns
*    zero.
*
*  Notes
*    The only mappings supported are:
*
*    * "alnum"
*    * "alpha",
*    * "blank"
*    * "cntrl"
*    * "digit"
*    * "graph"
*    * "lower"
*    * "print"
*    * "punct"
*    * "space"
*    * "upper"
*    * "xdigit"
*
*  Thread safety
*    Safe [if configured].
*/
wctype_t __SEGGER_RTL_PUBLIC_API wctype(char const *name) {
  return 1 + __SEGGER_RTL_string_list_encode(__SEGGER_RTL_wctype_class_name_list, name);
}

/*********************************************************************
*
*       iswctype()
*
*  Function description
*    Construct character mapping.
*
*  Parameters
*    c - Wide character to test.
*    t - Property to test, typically delivered by calling wctype().
*
*  Return value
*    Returns nonzero (true) if and only if the wide character c
*    has the property t in the current locale.
*
*  Additional information
*    Determines whether the wide character c has the
*    property described by t in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswctype(wint_t c, wctype_t t) {
  return __SEGGER_RTL_iswctype(c, t);
}

/*********************************************************************
*
*       iswctype_l()
*
*  Function description
*    Construct character mapping, per locale (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    t   - Property to test, typically delivered by calling wctype_l().
*    loc - Locale used for mapping.
*
*  Return value
*    Returns nonzero (true) if and only if the wide character c
*    has the property t in the locale loc.
*
*  Additional information
*    Determines whether the wide character c has the
*    property described by t in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswctype_l(wint_t c, wctype_t t, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, t, loc);
}

/*********************************************************************
*
*       iswalpha()
*
*  Function description
*    Is character alphabetic?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns true if the character c is alphabetic in the current locale.
*    That is, any character for which iswupper() or iswlower() returns true
*    is considered alphabetic in addition to any of the locale-specific set of
*    alphabetic characters for which none of iswcntrl(), iswdigit(), iswpunct(),
*    or isspace() is true.
*    
*    In the C locale, isalpha() returns nonzero (true) if and only if
*    isupper() or islower() return true for value of the argument c. 
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswalpha(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_ALPHA);
}

/*********************************************************************
*
*       iswalpha_l()
*
*  Function description
*    Is character alphabetic, per locale? (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns true if the wide character c is alphabetic in the locale loc.
*    That is, any character for which iswupper() or iswlower() returns true
*    is considered alphabetic in addition to any of the locale-specific set of
*    alphabetic characters for which none of iswcntrl_l(), iswdigit_l(),
*    iswpunct_l(), or iswspace_l() is true in the locale loc.
*    
*    In the C locale, iswalpha_l() returns nonzero (true) if and only if
*    iswupper_l() or iswlower_l() return true for value of the argument c. 
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswalpha_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_ALPHA, loc);
}

/*********************************************************************
*
*       iswupper()
*
*  Function description
*    Is character an uppercase letter?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is an uppercase letter in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswupper(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_UPPER);
}

/*********************************************************************
*
*       iswupper_l()
*
*  Function description
*    Is character an uppercase letter, per locale? (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is an uppercase letter in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswupper_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_UPPER, loc);
}

/*********************************************************************
*
*       iswlower()
*
*  Function description
*    Is character a lowercase letter?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a lowercase letter in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswlower(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_LOWER);
}

/*********************************************************************
*
*       iswlower_l()
*
*  Function description
*    Is character a lowercase letter, per locale? (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a lowercase letter in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswlower_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_LOWER, loc);
}

/*********************************************************************
*
*       iswdigit()
*
*  Function description
*    Is character a decimal digit?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a digit in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswdigit(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_DIGIT);
}

/*********************************************************************
*
*       iswdigit_l()
*
*  Function description
*    Is character a decimal digit, per locale? (POSIX.1)
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a digit in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswdigit_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_DIGIT, loc);
}

/*********************************************************************
*
*       iswxdigit()
*
*  Function description
*    Is character a hexadecimal digit?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a hexadecimal digit in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswxdigit(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_XDIGIT);
}

/*********************************************************************
*
*       iswxdigit_l()
*
*  Function description
*    Is character a hexadecimal digit, per locale? (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a hexadecimal digit in the current locale.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswxdigit_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_XDIGIT, loc);
}

/*********************************************************************
*
*       iswspace()
*
*  Function description
*    Is character a whitespace character?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a standard white-space character in the current locale.
*    The standard white-space characters  are space, form feed,
*    new-line, carriage return, horizontal tab, and vertical tab. 
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswspace(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_SPACE);
}

/*********************************************************************
*
*       iswspace_l()
*
*  Function description
*    Is character a whitespace character, per locale? (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a standard white-space character in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswspace_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_SPACE, loc);
}

/*********************************************************************
*
*       iswpunct()
*
*  Function description
*    Is character a punctuation mark?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns nonzero (true) for every printing character for which 
*    neither isspace() nor isalnum() is true in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswpunct(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_PUNCT);
}

/*********************************************************************
*
*       iswpunct_l()
*
*  Function description
*    Is character a punctuation mark, per locale? (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) for every printing character for which 
*    neither isspace() nor isalnum() is true in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswpunct_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_PUNCT, loc);
}

/*********************************************************************
*
*       iswalnum()
*
*  Function description
*    Is character alphanumeric?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is an alphabetic or numeric character in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswalnum(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_ALNUM);
}

/*********************************************************************
*
*       iswalnum_l()
*
*  Function description
*    Is character alphanumeric, per locale? (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is an alphabetic or numeric character in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswalnum_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_ALNUM, loc);
}

/*********************************************************************
*
*       iswprint()
*
*  Function description
*    Is character printable?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is any printing character including space in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswprint(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_PRINT);
}

/*********************************************************************
*
*       iswprint_l()
*
*  Function description
*    Is character printable, per locale? (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is any printing character including space in the locale loc. 
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswprint_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_PRINT, loc);
}

/*********************************************************************
*
*       iswgraph()
*
*  Function description
*    Is character any printing character?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is any printing character except space in the current locale. 
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswgraph(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_GRAPH);
}

/*********************************************************************
*
*       iswgraph_l()
*
*  Function description
*    Is character any printing character, per locale? (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is any printing character except space in the locale loc. 
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswgraph_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_GRAPH, loc);
}

/*********************************************************************
*
*       iswcntrl()
*
*  Function description
*    Is character a control?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a control character in the current locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswcntrl(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_CNTRL);
}

/*********************************************************************
*
*       iswcntrl_l()
*
*  Function description
*    Is character a control, per locale? (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is a control character in the locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswcntrl_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_CNTRL, loc);
}

/*********************************************************************
*
*       iswblank()
*
*  Function description
*    Is character a blank?
*
*  Parameters
*    c - Wide character to test.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is either a space character or tab character in the current
*    locale.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API iswblank(wint_t c) {
  return __SEGGER_RTL_iswctype(c, __SEGGER_RTL_WC_BLANK);
}

/*********************************************************************
*
*       iswblank_l()
*
*  Function description
*    Is character a blank, per locale? (POSIX.1).
*
*  Parameters
*    c   - Wide character to test.
*    loc - Locale used to test c.
*
*  Return value
*    Returns nonzero (true) if and only if the value of the argument 
*    c is either a space character or the tab character in locale loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API iswblank_l(wint_t c, locale_t loc) {
  return __SEGGER_RTL_iswctype_l(c, __SEGGER_RTL_WC_BLANK, loc);
}

/*********************************************************************
*
*       wctob()
*
*  Function description
*    Convert wide character to single-byte character.
*
*  Parameters
*    c - Character to convert.
*
*  Return value
*    Returns EOF if c does not correspond to a multi-byte character
*    with length one in the initial shift state in the current locale.
*    Otherwise, it returns the single-byte representation
*    of that character as an unsigned char converted to an int.
*
*  Additional information
*    Determines whether c corresponds to a member of the extended
*    character set whose multi-byte character representation is a
*    single byte in the current locale when in the initial shift state.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API wctob(wint_t c) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return c & 0xFF;
#else
  return wctob_l(c, __SEGGER_RTL_current_locale());
#endif
}

/*********************************************************************
*
*       wctob_l()
*
*  Function description
*    Convert wide character to single-byte character, per locale (POSIX.1).
*
*  Parameters
*    c   - Character to convert.
*    loc - Locale used for conversion.
*
*  Return value
*    Returns EOF if c does not correspond to a multi-byte character
*    with length one in the initial shift state in the locale loc.
*    Otherwise, it returns the single-byte representation
*    of that character as an unsigned char converted to an int.
*
*  Additional information
*    Determines whether c corresponds to a member of the extended
*    character set whose multi-byte character representation is a
*    single byte in the locale loc when in the initial shift state.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API wctob_l(wint_t c, locale_t loc) {
  mbstate_t mbstate;
  char      nc[MB_LEN_MAX];
  //
  // Handle WEOF specially, don't ask the codec to do it.
  //
  if (c == WEOF) {
    return EOF;
  }
  //
  // Codec to initial state.
  //
  __SEGGER_RTL_init_mbstate(&mbstate);
  //
  // Try to convert a single character successfully in the initial state.
  //
  if (loc->__category[LC_CTYPE]->codeset->__wctomb(nc, c, &mbstate) == 1) {
    return nc[0] & 0xFF;
  } else {
    return EOF;
  }
}

/*********************************************************************
*
*       wctomb()
*
*  Function description
*    Convert wide character to multi-byte character.
*
*  Parameters
*    s  - Pointer to array that receives the multi-byte character.
*    wc - Wide character to convert.
*
*  Return value
*    Returns the number of bytes stored in the array object. When wc is
*    not a valid wide character, an encoding error occurs: wctomb() stores
*    the value of the macro EILSEQ in errno and returns (size_t)(-1); the
*    conversion state is unspecified.
*
*  Additional information
*    If s is a null pointer, wctomb() is equivalent to the call
*    wcrtomb(buf, 0, ps) where buf is an internal buffer.
*    
*    If s is not a null pointer, wctomb() determines the number of bytes needed
*    to represent the multi-byte character that corresponds to the wide character given
*    by wc in the current locale, and stores the multi-byte character representation
*    in the array whose first element is pointed to by s. At most MB_CUR_MAX bytes
*    are stored.  If wc is a null wide character, a null byte is stored; the resulting
*    state described is the initial conversion state.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API wctomb(char *s, wchar_t wc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return __SEGGER_RTL_ascii_wctomb(s, wc, NULL);
#else
  return wctomb_l(s, wc, __SEGGER_RTL_current_locale());
#endif
}

/*********************************************************************
*
*       wctomb_l()
*
*  Function description
*    Convert wide character to multi-byte character, per locale (POSIX.1).
*
*  Parameters
*    s   - Pointer to array that receives the multi-byte character.
*    wc  - Wide character to convert.
*    loc - Locale used for conversion.
*
*  Return value
*    Returns the number of bytes stored in the array object. When wc is
*    not a valid wide character, an encoding error occurs: wctomb_l() stores
*    the value of the macro EILSEQ in errno and returns (size_t)(-1); the
*    conversion state is unspecified.
*
*  Additional information
*    If s is a null pointer, wctomb_l() is equivalent to the call
*    wcrtomb_l(buf, 0, ps, loc) where buf is an internal buffer.
*    
*    If s is not a null pointer, wctomb_l() determines the number of bytes needed
*    to represent the multi-byte character that corresponds to the wide character given
*    by wc in the locale loc, and stores the multi-byte character representation
*    in the array whose first element is pointed to by s. At most MB_CUR_MAX bytes
*    are stored.  If wc is a null wide character, a null byte is stored; the resulting
*    state described is the initial conversion state.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API wctomb_l(char *s, wchar_t wc, locale_t loc) {
  //
#if __SEGGER_RTL_MINIMAL_LOCALE
  //
  __SEGGER_RTL_USE_PARA(loc);
  //
  return __SEGGER_RTL_ascii_wctomb(s, wc, &__SEGGER_RTL_wctomb_mbstate);
  //
#else
  //
  // Null pointer resets to initial state and answers whether encodings
  // are state-dependent or not.
  //
  if (s == 0) {
    __SEGGER_RTL_init_mbstate(&__SEGGER_RTL_wctomb_mbstate);
    //
    // We don't support state-dependent encodings.
    //
    return 0;
  }
  //
  // Try to convert a single character successfully in the initial state.
  //
  return loc->__category[LC_CTYPE]->codeset->__wctomb(s, wc, &__SEGGER_RTL_wctomb_mbstate);
  //
#endif
}

/*********************************************************************
*
*       wcrtomb()
*
*  Function description
*    Convert wide character to multi-byte character, restartable.
*
*  Parameters
*    s  - Pointer to array that receives the multi-byte character.
*    wc - Wide character to convert.
*    ps - Pointer to multi-byte conversion state.
*
*  Return value
*    Returns the number of bytes stored in the array object. When wc is
*    not a valid wide character, an encoding error occurs: wcrtomb() stores
*    the value of the macro EILSEQ in errno and returns (size_t)(-1); the
*    conversion state is unspecified.
*
*  Additional information
*    If s is a null pointer, wcrtomb() is equivalent to the call
*    wcrtomb(buf, 0, ps) where buf is an internal buffer.
*    
*    If s is not a null pointer, wcrtomb() determines the number of bytes needed
*    to represent the multi-byte character that corresponds to the wide character given
*    by wc in the locale loc, and stores the multi-byte character representation
*    in the array whose first element is pointed to by s. At most MB_CUR_MAX bytes are
*    stored.  If wc is a null wide character, a null byte is stored; the resulting
*    state described is the initial conversion state.
*
*  Thread safety
*    Safe [if configured].
*/
size_t __SEGGER_RTL_PUBLIC_API wcrtomb(char *s, wchar_t wc, mbstate_t *ps) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return wcrtomb_l(s, wc, ps, NULL);
#else
  return wcrtomb_l(s, wc, ps, __SEGGER_RTL_current_locale());
#endif
}

/*********************************************************************
*
*       wcrtomb_l()
*
*  Function description
*    Convert wide character to multi-byte character, restartable,
*    per locale (POSIX.1).
*
*  Parameters
*    s   - Pointer to array that receives the multi-byte character.
*    wc  - Wide character to convert.
*    ps  - Pointer to multi-byte conversion state.
*    loc - Locale used for conversion.
*
*  Return value
*    Returns the number of bytes stored in the array object. When wc is
*    not a valid wide character, an encoding error occurs: wcrtomb_l() stores
*    the value of the macro EILSEQ in errno and returns (size_t)(-1); the
*    conversion state is unspecified.
*
*  Additional information
*    If s is a null pointer, wcrtomb() is equivalent to the call
*    wcrtomb(buf, 0, ps) where buf is an internal buffer.
*    
*    If s is not a null pointer, wcrtomb() determines the number of bytes needed
*    to represent the multi-byte character that corresponds to the wide character given
*    by wc in the current locale, and stores the multi-byte character representation
*    in the array whose first element is pointed to by s. At most MB_CUR_MAX bytes are
*    stored.  If wc is a null wide character, a null byte is stored; the resulting
*    state described is the initial conversion state.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API wcrtomb_l(char *s, wchar_t wc, mbstate_t *ps, locale_t loc) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  //
  __SEGGER_RTL_USE_PARA(ps);
  __SEGGER_RTL_USE_PARA(loc);
  //
  s[0] = wc & 0xFF;
  return 1;
  //
#else
  //
  char buf[MB_LEN_MAX];
  //
  if (ps == 0) {
    ps = &__SEGGER_RTL_wcrtomb_mbstate;
  }
  //
  if (s) {
    return loc->__category[LC_CTYPE]->codeset->__wctomb(s, wc, ps);
  } else {
    return loc->__category[LC_CTYPE]->codeset->__wctomb(buf, 0, ps);
  }
#endif
}

/*********************************************************************
*
*       wcstombs()
*
*  Function description
*    Convert wide string to multi-byte string.
*
*  Parameters
*    s    - Pointer to array that receives the multi-byte string.
*    pwcs - Pointer to wide character string to convert.
*    n    - Maximum number of bytes to write into s.
*
*  Return value
*    If a wide character is encountered that does not correspond
*    to a valid multibyte character in the current locale, returns
*    (size_t)(-1). Otherwise, returns the number of bytes written,
*    not including a terminating null character (if any).
*
*  Additional information
*    Converts a sequence of wide characters in the current locale from
*    the array pointed to by pwcs into a sequence of corresponding
*    multi-byte characters that begins in the initial shift state,
*    and stores these multi-byte characters into the array pointed
*    to by s, stopping if a multi-byte character would exceed the
*    limit of n total bytes or if a null character is stored. Each
*    wide character is converted as if by a call to wctomb(), except
*    that the conversion state of wctomb() is not affected.
*
*  Thread safety
*    Safe [if configured].
*/
size_t __SEGGER_RTL_PUBLIC_API wcstombs(char *s, const wchar_t *pwcs, size_t n) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return wcstombs_l(s, pwcs, n, NULL);
#else
  return wcstombs_l(s, pwcs, n, __SEGGER_RTL_current_locale());
#endif
}

/*********************************************************************
*
*       wcstombs_l()
*
*  Function description
*    Convert wide string to multi-byte string.
*
*  Parameters
*    s    - Pointer to array that receives the multi-byte string.
*    pwcs - Pointer to wide character string to convert.
*    n    - Maximum number of bytes to write into s.
*    loc  - Locale used for conversion.
*
*  Return value
*    If a wide character is encountered that does not correspond
*    to a valid multibyte character in the locale loc, returns
*    (size_t)(-1). Otherwise, returns the number of bytes written,
*    not including a terminating null character (if any).
*
*  Additional information
*    Converts a sequence of wide characters in the locale loc from
*    the array pointed to by pwcs into a sequence of corresponding
*    multi-byte characters that begins in the initial shift state,
*    and stores these multi-byte characters into the array pointed
*    to by s, stopping if a multi-byte character would exceed the
*    limit of n total bytes or if a null character is stored. Each
*    wide character is converted as if by a call to wctomb(), except
*    that the conversion state of wctomb() is not affected.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API wcstombs_l(char *s, const wchar_t *pwcs, size_t n, locale_t loc) {
  size_t     nc;
  size_t     mb_max;
  mbstate_t  mbstate;
  char       buf[MB_LEN_MAX];
  int        i;
  //
  // Initialize conversion state.
  //
  __SEGGER_RTL_init_mbstate(&mbstate);
  //
  // Compute mb_max for this locale once.
  //
  mb_max = __SEGGER_RTL_mb_max(loc);
  //
  // Generate at maximum n bytes.
  //
  for (nc = 0; nc < n; ++pwcs) {
    //
    // Translate one wide character.
    //
    if (s && mb_max <= n - nc) {
      //
      // Copy directly.
      //
      if ((i = loc->__category[LC_CTYPE]->codeset->__wctomb(s + nc, *pwcs, &mbstate)) < 0) {
        return (size_t)(-1);
      }
      nc += i;
      if (0 < i && s[nc-1] == 0) {
        return nc - 1;
      }
    } else {
      //
      // Use a local buffer.
      //
      if ((i = loc->__category[LC_CTYPE]->codeset->__wctomb(buf, *pwcs, &mbstate)) < 0)
        {
          return (size_t)(-1);
        }
      else if (i <= (int)(n - nc)) {
        //
        // This conversion fits, copy and try next character.
        //
        if (s != 0) {
          (memcpy)(s + nc, buf, i);
        }
        nc += i;
        if (0 < i && buf[i-1] == 0)
          return nc - 1;
      } else {
        //
        // This won't all fit, copy part and give up.
        //
        if (s) {
          (memcpy)(s + nc, buf, n - nc);
        }
        return n;
      }
    }
  }

  return nc;
}

/*********************************************************************
*
*       wcsrtombs()
*
*  Function description
*    Convert wide string to multi-byte string, restartable.
*
*  Parameters
*    dst - Pointer to array that receives the multi-byte string.
*    src - Indirect pointer to wide character string being converted.
*    len - Maximum number of bytes to write into the array pointed to by dst.
*    ps  - Pointer to multi-byte conversion state.
*
*  Return value
*    If conversion stops because a wide character is reached that does not
*    correspond to a valid multi-byte character, an encoding error occurs:
*    wcsrtombs() stores the value of the macro EILSEQ in errno and returns
*    (size_t)(-1); the conversion state is unspecified. Otherwise, it returns
*    the number of bytes in the resulting multi-byte character sequence, not
*    including the terminating null character (if any).
*
*  Additional information
*    Converts a sequence of wide characters in the current locale from the
*    array indirectly pointed to by src into a sequence of corresponding
*    multi-byte characters that begins in the conversion state described by
*    the object pointed to by ps. If dst is not a null pointer, the converted
*    characters are then stored into the array pointed to by dst. Conversion
*    continues up to and including a terminating null wide character, which
*    is also stored.
*
*    Conversion stops earlier in two cases: when a wide character is reached
*    that does not correspond to a valid multi-byte character, or (if dst
*    is not a null pointer) when the next multi-byte character would exceed
*    the limit of len total bytes to be stored into the array pointed to by
*    dst. Each conversion takes place as if by a call to wcrtomb().
*
*    If dst is not a null pointer, the pointer object pointed to by src is
*    assigned either a null pointer (if conversion stopped due to reaching a
*    terminating null wide character) or the address just past the last wide
*    character converted (if any). If conversion stopped due to reaching a
*    terminating null wide character, the resulting state described is the
*    initial conversion state.
*
*  Thread safety
*    Safe [if configured].
*/
size_t __SEGGER_RTL_PUBLIC_API wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps) {
  return wcsnrtombs_l(dst, src, (size_t)~0, len, ps, __SEGGER_RTL_current_locale());
}

/*********************************************************************
*
*       wcsrtombs_l()
*
*  Function description
*    Convert wide string to multi-byte string, restartable (POSIX.1).
*
*  Parameters
*    dst - Pointer to array that receives the multi-byte string.
*    src - Indirect pointer to wide character string being converted.
*    len - Maximum number of bytes to write into the array pointed to by dst.
*    ps  - Pointer to multi-byte conversion state.
*    loc - Locale used for conversion.
*
*  Return value
*    If conversion stops because a wide character is reached that does not
*    correspond to a valid multi-byte character, an encoding error occurs:
*    wcsrtombs() stores the value of the macro EILSEQ in errno and returns
*    (size_t)(-1); the conversion state is unspecified. Otherwise, it returns
*    the number of bytes in the resulting multi-byte character sequence, not
*    including the terminating null character (if any).
*
*  Additional information
*    Converts a sequence of wide characters in the locale loc from the
*    array indirectly pointed to by src into a sequence of corresponding
*    multi-byte characters that begins in the conversion state described by
*    the object pointed to by ps. If dst is not a null pointer, the converted
*    characters are then stored into the array pointed to by dst. Conversion
*    continues up to and including a terminating null wide character, which
*    is also stored.
*
*    Conversion stops earlier in two cases: when a wide character is reached
*    that does not correspond to a valid multi-byte character, or (if dst
*    is not a null pointer) when the next multi-byte character would exceed
*    the limit of len total bytes to be stored into the array pointed to by
*    dst. Each conversion takes place as if by a call to wcrtomb_l().
*
*    If dst is not a null pointer, the pointer object pointed to by src is
*    assigned either a null pointer (if conversion stopped due to reaching a
*    terminating null wide character) or the address just past the last wide
*    character converted (if any). If conversion stopped due to reaching a
*    terminating null wide character, the resulting state described is the
*    initial conversion state.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API wcsrtombs_l(char *dst, const wchar_t **src, size_t len, mbstate_t *ps, locale_t loc) {
  return wcsnrtombs_l(dst, src, (size_t)~0, len, ps, loc);
}

/*********************************************************************
*
*       wcsnrtombs()
*
*  Function description
*    Convert wide string to multi-byte string, restartable (POSIX.1).
*
*  Parameters
*    dst - Pointer to array that receives the multi-byte string.
*    src - Indirect pointer to wide character string being converted.
*    nwc - Maximum number of wide characters to read from src.
*    len - Maximum number of bytes to write into the array pointed to by dst.
*    ps  - Pointer to multi-byte conversion state.
*
*  Return value
*    If conversion stops because a wide character is reached that does not
*    correspond to a valid multi-byte character, an encoding error occurs:
*    wcsrtombs() stores the value of the macro EILSEQ in errno and returns
*    (size_t)(-1); the conversion state is unspecified. Otherwise, it returns
*    the number of bytes in the resulting multi-byte character sequence, not
*    including the terminating null character (if any).
*
*  Additional information
*    Converts a sequence of wide characters in the locale loc from the
*    array indirectly pointed to by src into a sequence of corresponding
*    multi-byte characters that begins in the conversion state described by
*    the object pointed to by ps. If dst is not a null pointer, the converted
*    characters are then stored into the array pointed to by dst. Conversion
*    continues up to and including a terminating null wide character, which
*    is also stored.
*
*    Conversion stops earlier in two cases: when a wide character is reached
*    that does not correspond to a valid multi-byte character, or (if dst
*    is not a null pointer) when the next multi-byte character would exceed
*    the limit of len total bytes to be stored into the array pointed to by
*    dst. Each conversion takes place as if by a call to wcrtomb_l().
*
*    If dst is not a null pointer, the pointer object pointed to by src is
*    assigned either a null pointer (if conversion stopped due to reaching a
*    terminating null wide character) or the address just past the last wide
*    character converted (if any). If conversion stopped due to reaching a
*    terminating null wide character, the resulting state described is the
*    initial conversion state.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe [if configured].
*/
size_t __SEGGER_RTL_PUBLIC_API wcsnrtombs(char *dst, const wchar_t **src, size_t nwc, size_t len, mbstate_t *ps) {
  return wcsnrtombs_l(dst, src, nwc, len, ps, __SEGGER_RTL_current_locale());
}

/*********************************************************************
*
*       wcsnrtombs_l()
*
*  Function description
*    Convert wide string to multi-byte string, restartable (POSIX.1).
*
*  Parameters
*    dst - Pointer to array that receives the multi-byte string.
*    src - Indirect pointer to wide character string being converted.
*    nwc - Maximum number of wide characters to read from src.
*    len - Maximum number of bytes to write into the array pointed to by dst.
*    ps  - Pointer to multi-byte conversion state.
*    loc - Locale used for conversion.
*
*  Return value
*    If conversion stops because a wide character is reached that does not
*    correspond to a valid multi-byte character, an encoding error occurs:
*    wcsrtombs() stores the value of the macro EILSEQ in errno and returns
*    (size_t)(-1); the conversion state is unspecified. Otherwise, it returns
*    the number of bytes in the resulting multi-byte character sequence, not
*    including the terminating null character (if any).
*
*  Additional information
*    Converts a sequence of wide characters in the locale loc from the
*    array indirectly pointed to by src into a sequence of corresponding
*    multi-byte characters that begins in the conversion state described by
*    the object pointed to by ps. If dst is not a null pointer, the converted
*    characters are then stored into the array pointed to by dst. Conversion
*    continues up to and including a terminating null wide character, which
*    is also stored.
*
*    Conversion stops earlier in two cases: when a wide character is reached
*    that does not correspond to a valid multi-byte character, or (if dst
*    is not a null pointer) when the next multi-byte character would exceed
*    the limit of len total bytes to be stored into the array pointed to by
*    dst. Each conversion takes place as if by a call to wcrtomb_l().
*
*    If dst is not a null pointer, the pointer object pointed to by src is
*    assigned either a null pointer (if conversion stopped due to reaching a
*    terminating null wide character) or the address just past the last wide
*    character converted (if any). If conversion stopped due to reaching a
*    terminating null wide character, the resulting state described is the
*    initial conversion state.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API wcsnrtombs_l(char *dst, const wchar_t **src, size_t nwc, size_t len, mbstate_t *ps, locale_t loc) {
  char            buf[MB_LEN_MAX];
  int             i;
  size_t          nc = 0;
  size_t          mb_max;
  const wchar_t * wcs = *src;
  //
  // Compute mb_max for this locale once.
  //
  mb_max = __SEGGER_RTL_mb_max(loc);
  //
  if (ps == 0) {
    ps = &__SEGGER_RTL_wcsrtombs_mbstate;
  }
  //
  if (dst == 0) {
    while (nwc > 0) {
      //
      // Translate, don't store.
      //
      i = loc->__category[LC_CTYPE]->codeset->__wctomb(buf, *wcs++, ps);
      if (i < 0) {
        return (size_t)-1;
      } else if (0 < i && buf[i-1] == 0) {
        return nc + i - 1;
      }
      nc  += i;
      nwc -= 1;
    }
    return nc;
  } else {
    while (nwc > 0 && len > 0) {
      char *t;
      mbstate_t mbstsave;
      //
      if (len < mb_max) {
        t = buf;
        mbstsave = *ps;
      } else {
        t = dst;
      }
      //
      i = loc->__category[LC_CTYPE]->codeset->__wctomb(t, *wcs, ps);
      if (i < 0) {
        //
        // Invalid sequence.
        //
        nc = (size_t)-1;
        break;
      } if (len >= mb_max) {
        //
        // We copied directly...
        //
      } else if (len < (unsigned)i) {
        //
        // We can't fit this multi-byte char into the destination,
        // restore state.
        //
        *ps = mbstsave;
        break;
      } else {
        (memcpy)(dst, buf, i);
      }
      //
      if (i > 0 && dst[i-1] == 0) {
        *src = 0;
        return nc + i - 1;
      }
      //
      // Adjust counts, pointers, and lengths for this multi-byte encoding.
      //
      nc  += i;
      wcs += 1;
      dst += i;
      len -= i;
      nwc -= 1;
    }
    //
    // All done but not terminated with a zero.
    //
    *src = wcs;
    return nc;
  }
}

/*********************************************************************
*
*       btowc()
*
*  Function description
*    Convert single-byte character to wide character.
*
*  Parameters
*    c - Character to convert.
*
*  Return value
*    Returns WEOF if c has the value EOF or if c, converted to
*    an unsigned char and in the current locale, does not constitute
*    a valid single-byte character in the initial shift state.
*
*  Additional information
*    Determines whether c constitutes a valid single-byte character
*    in the current locale. If c is a valid single-byte character,
*    btowc() returns the wide character representation of that character.
*
*  Thread safety
*    Safe [if configured].
*/
wint_t __SEGGER_RTL_PUBLIC_API btowc(int c) {
  return btowc_l(c, __SEGGER_RTL_current_locale());
}

/*********************************************************************
*
*       btowc_l()
*
*  Function description
*    Convert single-byte character to wide character, per locale, (POSIX.1).
*
*  Parameters
*    c   - Character to convert.
*    loc - Locale used for conversion.
*
*  Return value
*    Returns WEOF if c has the value EOF or if c, converted to
*    an unsigned char and in the locale loc, does not constitute
*    a valid single-byte character in the initial shift state.
*
*  Additional information
*    Determines whether c constitutes a valid single-byte character
*    in the locale loc. If c is a valid single-byte character,
*    btowc_l() returns the wide character representation of that character.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
wint_t __SEGGER_RTL_PUBLIC_API btowc_l(int c, locale_t loc) {
  mbstate_t mbstate;
  wchar_t   wc;
  char      ch;
  //
  // Deal with special case.
  //
  if (c == EOF) {
    return WEOF;
  }
  //
  // Initialize state and convert a single character.  If can't convert
  // it successfully, return WEOF as required.
  //
  __SEGGER_RTL_init_mbstate(&mbstate);
  ch = c;
  return loc->__category[LC_CTYPE]->codeset->__mbtowc(&wc, &ch, 1, &mbstate) < 0 ? WEOF : (wint_t)wc;
}

/*********************************************************************
*
*       mblen()
*
*  Function description
*    Count number of bytes in multi-byte character.
*
*  Parameters
*    s - Pointer to multi-byte character.
*    n - Maximum number of bytes to examine.
*
*  Return value
*    If s is a null pointer, returns a nonzero or zero value, if multi-byte
*    character encodings, respectively, do or do not have state-dependent
*    encodings.
*    
*    If s is not a null pointer, either returns 0 (if s points to the
*    null character), or returns the number of bytes that are contained in the
*    multi-byte character (if the next n or fewer bytes form a valid multi-byte
*    character), or returns -1 (if they do not form a valid multi-byte
*    character).
*
*  Additional information
*    Determines the number of bytes contained in the multi-byte
*    character pointed to by s in the current locale.
*    
*    Except that the conversion state of the mbtowc() function is not
*    affected, it is equivalent to
*    
*    mbtowc(NULL, s, n);
*
*  Thread safety
*    Safe [if configured].
*
*  See also
*    mblen_l(), mbtowc()
*/
int __SEGGER_RTL_PUBLIC_API mblen(const char *s, size_t n) {
#if __SEGGER_RTL_MINIMAL_LOCALE
  return strnlen(s, n);
#else
  return mblen_l(s, n, __SEGGER_RTL_current_locale());
#endif
}

/*********************************************************************
*
*       mblen_l()
*
*  Function description
*    Count number of bytes in multi-byte character, per locale (POSIX.1).
*
*  Parameters
*    s   - Pointer to multi-byte character.
*    n   - Maximum number of bytes to examine.
*    loc - Locale to use for conversion.
*
*  Return value
*    If s is a null pointer, returns a nonzero or zero value, if multi-byte
*    character encodings, respectively, do or do not have state-dependent
*    encodings in locale loc.
*
*    If s is not a null pointer, either returns 0 (if s points to the
*    null character), or returns the number of bytes that are contained in the
*    multi-byte character (if the next n or fewer bytes form a valid multi-byte
*    character), or returns -1 (if they do not form a valid multi-byte
*    character).
*
*  Additional information
*    Determines the number of bytes contained in the multi-byte
*    character pointed to by s in the locale loc.
*    
*    Except that the conversion state of the mbtowc() function is not
*    affected, it is equivalent to
*    
*    mbtowc_l(NULL, s, n, loc);
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*
*  See also
*    mblen(), mbtowc()
*/
int __SEGGER_RTL_PUBLIC_API mblen_l(const char *s, size_t n, locale_t loc) {
  //
#if __SEGGER_RTL_MINIMAL_LOCALE
  //
  __SEGGER_RTL_USE_PARA(loc);
  //
  // Null input is special.
  //
  if (s == 0) {
    return 0;
  } else {
    return strnlen(s, n);
  }
  //
#else
  //
  mbstate_t mbstate;
  int      i;
  //
  // Null input is special.
  //
  if (s == 0) {
    return 0;
  }
  //
  // Initial state.
  //
  __SEGGER_RTL_init_mbstate(&mbstate);
  //
  // Try to convert, throwing away converted wide characters.
  //
  i = loc->__category[LC_CTYPE]->codeset->__mbtowc(0, s, n, &mbstate);
  //
  // Partial conversions and error conversions are errors.
  //
  return i < 0 ? -1 : i;
#endif
}

/*********************************************************************
*
*       mbrlen()
*
*  Function description
*    Count number of bytes in multi-byte character, restartable.
*
*  Parameters
*    s  - Pointer to multi-byte character.
*    n  - Maximum number of bytes to examine.
*    ps - Pointer to multi-byte conversion state.
*
*  Return value
*    Number of bytes in multi-byte character.
*
*  Additional information
*    Determines the number of bytes contained in the multi-byte
*    character pointed to by s in the current locale.
*    
*    Except that except that the expression designated by ps is
*    evaluated only once, this function is equivalent to the call:
*    
*    \code{c} mbrtowc(NULL, s, n, ps != NULL ? ps : &internal); \endcode
*    
*    where internal is the mbstate_t object for the mbrlen() function.
*
*  Thread safety
*    Safe [if configured].
*
*  See also
*    mbrlen_l(), mbrtowc()
*/
size_t __SEGGER_RTL_PUBLIC_API mbrlen(const char *s, size_t n, mbstate_t *ps) {
  return mbrlen_l(s, n, ps, __SEGGER_RTL_current_locale());
}

/*********************************************************************
*
*       mbrlen_l()
*
*  Function description
*    Count number of bytes in multi-byte character, restartable, per locale (POSIX.1).
*
*  Parameters
*    s   - Pointer to multi-byte character.
*    n   - Maximum number of bytes to examine.
*    ps  - Pointer to multi-byte conversion state.
*    loc - Locale used for conversion.
*
*  Return value
*    Number of bytes in multi-byte character.
*
*  Additional information
*    Determines the number of bytes contained in the multi-byte
*    character pointed to by s in the locale loc.
*    
*    Except that except that the expression designated by ps is
*    evaluated only once, this function is equivalent to the call:
*    
*    \code{c} mbrtowc_l(NULL, s, n, ps != NULL ? ps : &internal, loc); \endcode
*    
*    where internal is the mbstate_t object for the mbrlen() function,
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*
*  See also
*    mbrlen_l(), mbrtowc()
*/
size_t __SEGGER_RTL_PUBLIC_API mbrlen_l(const char *s, size_t n, mbstate_t *ps, locale_t loc) {
  //
  // Use internal mbstate if none specified.
  //
  if (ps == 0) {
    ps = &__SEGGER_RTL_mbrlen_mbstate;
  }
  //
  // Count, don't store.
  //
  return loc->__category[LC_CTYPE]->codeset->__mbtowc(0, s, n, ps);
}

/*********************************************************************
*
*       mbtowc()
*
*  Function description
*    Convert multi-byte character to wide character.
*
*  Parameters
*    pwc - Pointer to object that receives the wide character.
*    s   - Pointer to multi-byte character string.
*    n   - Maximum number of bytes that will be examined.
*
*  Return value
*    If s is a null pointer, mbtowc() returns a nonzero value if multi-byte
*    character encodings are state-dependent in the current locale, and
*    zero otherwise.
*    
*    If s is not null and the object that s points to is a
*    wide character null, mbtowc() returns 0.
*    
*    If s is not null and the object that s points to forms a valid
*    multi-byte character, mbtowc() returns the length in bytes of the
*    multi-byte character.
*    
*    If the object that mbtowc() points to does not form a valid multi-byte
*    character within the first n characters, it returns -1.
*
*  Additional information
*    Converts a single multi-byte character to a wide character in
*    the current locale.  The wide character, if the multi-byte character
*    string is converted correctly, is stored into the object pointed
*    to by pwc.
*
*  Thread safety
*    Safe [if configured].
*    
*  See also
*    mbtowc_l().
*/
int __SEGGER_RTL_PUBLIC_API mbtowc(wchar_t *pwc, const char *s, size_t n) {
  return mbtowc_l(pwc, s, n, __SEGGER_RTL_current_locale());
}

/*********************************************************************
*
*       mbtowc_l()
*
*  Function description
*    Convert multi-byte character to wide character, per locale (POSIX.1).
*
*  Parameters
*    pwc - Pointer to object that receives the wide character.
*    s   - Pointer to multi-byte character string.
*    n   - Maximum number of bytes that will be examined.
*    loc - Locale used to convert the multi-byte character.
*
*  Return value
*    If s is a null pointer, mbtowc_l() returns a nonzero value if multi-byte
*    character encodings are state-dependent in locale loc, and
*    zero otherwise.
*    
*    If s is not null and the object that s points to is a
*    wide null character, mbtowc_l() returns 0.
*    
*    If s is not null and the object that s points to forms a valid
*    multi-byte character, mbtowc_l() returns the length in bytes of the
*    multi-byte character.
*
*    If the object that mbtowc_l() points to does not form a valid multi-byte
*    character within the first n characters, it returns -1.
*
*  Additional information
*    Converts a single multi-byte character to a wide character in
*    the locale loc.  The wide character, if the multi-byte character
*    string is converted correctly, is stored into the object pointed
*    to by pwc.
*
*  Thread safety
*    Safe.
*
*  Notes
*    Conforms to POSIX.1-2017.
*    
*  See also
*    mbtowc()
*/
int __SEGGER_RTL_PUBLIC_API mbtowc_l(wchar_t *pwc, const char *s, size_t n, locale_t loc) {
  int i;
  //
  // Initial conversion.
  //
  i = loc->__category[LC_CTYPE]->codeset->__mbtowc(pwc, s, n, &__SEGGER_RTL_mbtowc_mbstate);
  //
  return i < 0 ? -1 : i;
}

/*********************************************************************
*
*       mbrtowc()
*
*  Function description
*    Convert multi-byte character to wide character, restartable.
*
*  Parameters
*    pwc - Pointer to object that receives the wide character.
*    s   - Pointer to multi-byte character string.
*    n   - Maximum number of bytes that will be examined.
*    ps  - Pointer to multi-byte conversion state.
*
*  Return value
*    If s is a null pointer, mbrtowc() is equivalent to
*    mbrtowc(NULL, "", 1, ps), ignoring pwc and n.
*    
*    If s is not null and the object that s points to is a
*    wide null character, mbrtowc() returns 0.
*    
*    If s is not null and the object that s points to forms a valid
*    multi-byte character in the current locale with a most n bytes,
*    mbrtowc() returns the length in bytes of the multi-byte character
*    and stores that wide character to the object pointed to by pwc
*    (if pwc is not null).
*    
*    If the object that s points to forms an incomplete, but possibly
*    valid, multi-byte character, mbrtowc() returns -2.
*
*    If the object that s points to does not form a partial multi-byte
*    character, mbrtowc() returns -1.
*
*  Additional information
*    Converts a single multi-byte character to a wide character in the
*    current locale.
*
*  Thread safety
*    Safe [if configured].
*    
*  See also
*    mbtowc(), mbrtowc_l()
*/
size_t __SEGGER_RTL_PUBLIC_API mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps) {
  return mbrtowc_l(pwc, s, n, ps, __SEGGER_RTL_current_locale());
}

/*********************************************************************
*
*       mbrtowc_l()
*
*  Function description
*    Convert multi-byte character to wide character, restartable,
*    per locale (POSIX.1).
*
*  Parameters
*    pwc - Pointer to object that receives the wide character.
*    s   - Pointer to multi-byte character string.
*    n   - Maximum number of bytes that will be examined.
*    ps  - Pointer to multi-byte conversion state.
*    loc - Locale used for conversion.
*
*  Return value
*    If s is a null pointer, mbrtowc() is equivalent to
*    mbrtowc(NULL, "", 1, ps), ignoring pwc and n.
*    
*    If s is not null and the object that s points to is a
*    wide null character, mbrtowc() returns 0.
*    
*    If s is not null and the object that s points to forms a valid
*    multi-byte character in the locale loc with a most n bytes,
*    mbrtowc() returns the length in bytes of the multi-byte character
*    and stores that wide character to the object pointed to by pwc
*    (if pwc is not null).
*    
*    If the object that s points to forms an incomplete, but possibly
*    valid, multi-byte character, mbrtowc() returns -2.
*
*    If the object that s points to does not form a partial multi-byte
*    character, mbrtowc() returns -1.
*
*  Additional information
*    Converts a single multi-byte character to a wide character in the
*    locale loc.
*    
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*
*  See also
*    mbtowc(), mbrtowc_l()
*/
size_t __SEGGER_RTL_PUBLIC_API mbrtowc_l(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps, locale_t loc) {
  if (ps == 0) {
    ps = &__SEGGER_RTL_mbrtowc_mbstate;
  }
  //
  if (s == 0) {
    s = "";
  }
  //
  return loc->__category[LC_CTYPE]->codeset->__mbtowc(pwc, s, n, ps);
}

/*********************************************************************
*
*       mbsinit()
*
*  Function description
*    Query initial conversion state.
*
*  Parameters
*    ps - Pointer to conversion state.
*
*  Return value
*    Returns nonzero (true) if ps is a null pointer or if the
*    pointed-to object describes an initial conversion state;
*    otherwise, returns zero.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API mbsinit(const mbstate_t *ps) {
  return ps == 0 || ps->__state == 0;
}

/*********************************************************************
*
*       mbstowcs()
*
*  Function description
*    Convert multi-byte string to wide string.
*
*  Parameters
*    pwcs - Pointer to array that receives the wide character string.
*    s    - Pointer to array that contains the multi-byte string.
*    n    - Maximum number of wide characters to write into pwcs.
*
*  Return value
*    Returns -1 if an invalid multi-byte character is encountered,
*    otherwise returns the number of array elements modified (if
*    any), not including a terminating null wide character.
*
*  Additional information
*    Converts a sequence of multi-byte characters, in the current locale,
*    that begins in the initial shift state from the array pointed to by
*    s into a sequence of corresponding wide characters and stores not
*    more than n wide characters into the array pointed to by pwcs.
*    
*    No multi-byte characters that follow a null character (which is converted
*    into a null wide character) will be examined or converted. Each multi-byte
*    character is converted as if by a call to the mbtowc() function, except
*    that the conversion state of the mbtowc() function is not affected.
*    
*    No more than n elements will be modified in the array pointed to by
*    pwcs. If copying takes place between objects that overlap, the behavior
*    is undefined.
*
*  Thread safety
*    Safe [if configured].
*/
size_t __SEGGER_RTL_PUBLIC_API mbstowcs(wchar_t *pwcs, const char *s, size_t n) {
  return mbstowcs_l(pwcs, s, n, __SEGGER_RTL_current_locale());
}

/*********************************************************************
*
*       mbstowcs_l()
*
*  Function description
*    Convert multi-byte string to wide string, per locale (POSIX.1).
*
*  Parameters
*    pwcs - Pointer to array that receives the wide character string.
*    s    - Pointer to array that contains the multi-byte string.
*    n    - Maximum number of wide characters to write into pwcs.
*    loc  - Locale to use for conversion.
*
*  Return value
*    Returns -1 if an invalid multi-byte character is encountered,
*    otherwise returns the number of array elements modified (if
*    any), not including a terminating null wide character.
*
*  Additional information
*    Converts a sequence of multi-byte characters, in the locale loc,
*    that begins in the initial shift state from the array pointed to by
*    s into a sequence of corresponding wide characters and stores not
*    more than n wide characters into the array pointed to by pwcs.
*    
*    No multi-byte characters that follow a null character (which is converted
*    into a null wide character) will be examined or converted. Each multi-byte
*    character is converted as if by a call to the mbtowc() function, except
*    that the conversion state of the mbtowc() function is not affected.
*    
*    No more than n elements will be modified in the array pointed to by
*    pwcs. If copying takes place between objects that overlap, the behavior
*    is undefined.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API mbstowcs_l(wchar_t *pwcs, const char *s, size_t n, locale_t loc) {
  mbstate_t mbstate;
  size_t    nw;
  wchar_t   wc;
  int       i;
  //
  // Initialize conversion state.
  //
  __SEGGER_RTL_init_mbstate(&mbstate);
  //
  // Null input converts nothing and answers whether there is state-dependent
  // encodings.  We don't support shift states at this time.
  //
  if (s == 0) {
    return 0;
  }
  //
  // Convert string one wide character at a time.
  //
  for (nw = 0; nw < n; ++nw) {
    i = loc->__category[LC_CTYPE]->codeset->__mbtowc(&wc, s, __SEGGER_RTL_SIZE_MAX, &mbstate);
    //
    // If error in converting, return error indicator.
    //
    if (i < 0) {
      return (size_t)(-1);
    }
    //
    // Null input counts but doesn't store.
    //
    if (pwcs) {
      *pwcs++ = wc;
    }
    //
    // If end of string, conversion is done.
    //
    if (wc == 0) {
      break;
    }
    //
    // Step over consumed bytes.
    //
    s += i;
  }
  //
  return nw;
}

/*********************************************************************
*
*       mbsrtowcs()
*
*  Function description
*    Convert multi-byte string to wide character string, restartable.
*
*  Parameters
*    dst - Pointer to object that receives the converted wide characters.
*    src - Pointer to pointer to multi-byte character string.
*    len - Maximum number of wide characters that will be written to dst.
*    ps  - Pointer to multi-byte conversion state.
*
*  Return value
*    The number of wide characters written to dst (not including the
*    eventual terminating null character).
*
*  Additional information
*    Converts a sequence of multi-byte characters, in the current locale,
*    that begins in the conversion state described by the object pointed
*    to by ps, from the array indirectly pointed to by src into a sequence
*    of corresponding wide characters.
*    
*    If dst is not a null pointer, the converted characters are stored
*    into the array pointed to by dst.  Conversion continues up to and
*    including a terminating null character, which is also stored.
*    
*    Conversion stops earlier in two cases: when a sequence of bytes is
*    encountered that does not form a valid multi-byte character, or
*    (if dst is not a null pointer) when len wide characters have been
*    stored into the array pointed to by dst.  Each conversion takes place
*    as if by a call to the mbrtowc() function.
*    
*    If dst is not a null pointer, the pointer object pointed to by
*    src is assigned either a null pointer (if conversion stopped due to
*    reaching a terminating null character) or the address just past the
*    last multi-byte character converted (if any). If conversion stopped
*    due to reaching a terminating null character and if dst is not a null
*    pointer, the resulting state described is the initial conversion state.
*
*  Thread safety
*    Safe [if configured].
*    
*  See also
*    mbsrtowcs_l(), mbrtowc()
*/
size_t __SEGGER_RTL_PUBLIC_API mbsrtowcs(wchar_t *dst, const char **src, size_t len, mbstate_t *ps) {
  return mbsnrtowcs_l(dst, src, (size_t)~0, len, ps, __SEGGER_RTL_current_locale());
}

/*********************************************************************
*
*       mbsrtowcs_l()
*
*  Function description
*    Convert multi-byte string to wide character string, restartable, per locale (POSIX.1).
*
*  Parameters
*    dst - Pointer to object that receives the converted wide characters.
*    src - Pointer to pointer to multi-byte character string.
*    len - Maximum number of wide characters that will be written to dst.
*    ps  - Pointer to multi-byte conversion state.
*    loc - Locale used for conversion.
*
*  Return value
*    The number of wide characters written to dst (not including the
*    eventual terminating null character).
*
*  Additional information
*    Converts a sequence of multi-byte characters, in the locale loc,
*    that begins in the conversion state described by the object pointed
*    to by ps, from the array indirectly pointed to by src into a sequence
*    of corresponding wide characters.
*    
*    If dst is not a null pointer, the converted characters are stored
*    into the array pointed to by dst.  Conversion continues up to and
*    including a terminating null character, which is also stored.
*    
*    Conversion stops earlier in two cases: when a sequence of bytes is
*    encountered that does not form a valid multi-byte character, or
*    (if dst is not a null pointer) when len wide characters have been
*    stored into the array pointed to by dst.  Each conversion takes place
*    as if by a call to the mbrtowc() function.
*    
*    If dst is not a null pointer, the pointer object pointed to by
*    src is assigned either a null pointer (if conversion stopped due to
*    reaching a terminating null character) or the address just past the
*    last multi-byte character converted (if any). If conversion stopped
*    due to reaching a terminating null character and if dst is not a null
*    pointer, the resulting state described is the initial conversion state.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*
*  See also
*    mbsrtowcs(), mbrtowc()
*/
size_t __SEGGER_RTL_PUBLIC_API mbsrtowcs_l(wchar_t *dst, const char **src, size_t len, mbstate_t *ps, locale_t loc) {
  return mbsnrtowcs_l(dst, src, (size_t)~0, len, ps, loc);
}

/*********************************************************************
*
*       mbsnrtowcs()
*
*  Function description
*    Convert multi-byte string to wide character string, restartable, per locale (POSIX.1).
*
*  Parameters
*    dst - Pointer to object that receives the converted wide characters.
*    src - Pointer to pointer to multi-byte character string.
*    nmc - Maximum number of bytes to be read from src.
*    len - Maximum number of wide characters that will be written to dst.
*    ps  - Pointer to multi-byte conversion state.
*
*  Return value
*    The number of wide characters written to dst (not including the
*    eventual terminating null character).
*
*  Additional information
*    Converts a sequence of multi-byte characters, in the locale loc,
*    that begins in the conversion state described by the object pointed
*    to by ps, from the array indirectly pointed to by src into a sequence
*    of corresponding wide characters.
*    
*    If dst is not a null pointer, the converted characters are stored
*    into the array pointed to by dst.  Conversion continues up to and
*    including a terminating null character, which is also stored.
*    
*    Conversion stops earlier in two cases: when a sequence of bytes is
*    encountered that does not form a valid multi-byte character, or
*    (if dst is not a null pointer) when len wide characters have been
*    stored into the array pointed to by dst.  Each conversion takes place
*    as if by a call to the mbrtowc() function.
*    
*    If dst is not a null pointer, the pointer object pointed to by
*    src is assigned either a null pointer (if conversion stopped due to
*    reaching a terminating null character) or the address just past the
*    last multi-byte character converted (if any). If conversion stopped
*    due to reaching a terminating null character and if dst is not a null
*    pointer, the resulting state described is the initial conversion state.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe [if configured].
*    
*  See also
*    mbsrtowcs(), mbrtowc()
*/
size_t __SEGGER_RTL_PUBLIC_API mbsnrtowcs(wchar_t *dst, const char **src, size_t nmc, size_t len, mbstate_t *ps) {
  return mbsnrtowcs_l(dst, src, nmc, len, ps, __SEGGER_RTL_current_locale());
}

/*********************************************************************
*
*       mbsnrtowcs_l()
*
*  Function description
*    Convert multi-byte string to wide character string, restartable, per locale (POSIX.1).
*
*  Parameters
*    dst - Pointer to object that receives the converted wide characters.
*    src - Pointer to pointer to multi-byte character string.
*    nmc - Maximum number of bytes to be read from src.
*    len - Maximum number of wide characters that will be written to dst.
*    ps  - Pointer to multi-byte conversion state.
*    loc - Locale used for conversion.
*
*  Return value
*    The number of wide characters written to dst (not including the
*    eventual terminating null character).
*
*  Additional information
*    Converts a sequence of multi-byte characters, in the locale loc,
*    that begins in the conversion state described by the object pointed
*    to by ps, from the array indirectly pointed to by src into a sequence
*    of corresponding wide characters.
*    
*    If dst is not a null pointer, the converted characters are stored
*    into the array pointed to by dst.  Conversion continues up to and
*    including a terminating null character, which is also stored.
*    
*    Conversion stops earlier in two cases: when a sequence of bytes is
*    encountered that does not form a valid multi-byte character, or
*    (if dst is not a null pointer) when len wide characters have been
*    stored into the array pointed to by dst.  Each conversion takes place
*    as if by a call to the mbrtowc() function.
*    
*    If dst is not a null pointer, the pointer object pointed to by
*    src is assigned either a null pointer (if conversion stopped due to
*    reaching a terminating null character) or the address just past the
*    last multi-byte character converted (if any). If conversion stopped
*    due to reaching a terminating null character and if dst is not a null
*    pointer, the resulting state described is the initial conversion state.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*    
*  See also
*    mbsrtowcs(), mbrtowc()
*/
size_t __SEGGER_RTL_PUBLIC_API mbsnrtowcs_l(wchar_t *dst, const char **src, size_t nmc, size_t len, mbstate_t *ps, locale_t loc) {
  const char * s;
  int          i;
  size_t       cnt;
  wchar_t      wc;
  //
  cnt = 0;
  s   = *src;
  if (ps == 0) {
    ps = &__SEGGER_RTL_mbsrtowcs_mbstate;
  }
  //
  if (dst == NULL) {
    //
    // Translate, don't store.
    //
    for (;;) {
      i = loc->__category[LC_CTYPE]->codeset->__mbtowc(&wc, s, __SEGGER_RTL_SIZE_MAX, ps);
      if (i < 0) {
        cnt = (size_t)(-1);
        break;
      } else if (i == 0 && wc == 0) {
        break;
      } else if ((size_t)i > nmc) {
        break;
      }
      //
      cnt += 1;
      s   += i;
      nmc -= i;
    }
  } else {
    while (nmc > 0 && len > 0) {
      i = loc->__category[LC_CTYPE]->codeset->__mbtowc(dst, s, __SEGGER_RTL_SIZE_MAX, ps);
      if (i < 0) {
        cnt = (size_t)(-1);
        break;
      } else if (i == 0 && *dst == 0) {
        s = 0;
        break;
      } else if ((size_t)i > nmc) {
        break;
      }
      //
      // Update indexes, pointers, and counts for this single character conversion.
      //
      cnt += 1;
      s   += i;
      nmc -= i;
      dst += 1;
      len -= 1;
    }
    *src = s;
  }
  //
  return cnt;
}

/*********************************************************************
*
*       newlocale()
*
*  Function description
*    Create or modify locale (POSIX.1).
*
*  Parameters
*    category_mask - Locale categories to be set or modified.
*    loc           - Locale name.
*    base          - Base to modify or NULL to create a new locale.
*
*  Return value
*    Pointer to modified locale.
*
*  Additional information
*    Creates a new locale object or modifies an existing one.
*    If the base argument is NULL, a new locale object is created.
*    
*    category_mask specifies the locale categories to be set or modified.
*    Values for category_mask are constructed by a bitwise-inclusive OR
*    of the symbolic constants LC_CTYPE_MASK, LC_NUMERIC_MASK, LC_TIME_MASK,
*    LC_COLLATE_MASK, LC_MONETARY_MASK, and LC_MESSAGES_MASK.
*    
*    For each category with the corresponding bit set in category_mask, the data
*    from the locale named by loc is used. In the case of modifying an existing
*    locale object, the data from the locale named by loc replaces the existing
*    data within the locale object. If a completely new locale object is created,
*    the data for all sections not requested by category_mask are taken from the
*    default locale.
*    
*    The locales "C" and "POSIX" are equivalent and defined for all settings
*    of category_mask:
*    
*    If loc is NULL, then the "C" locale is used. If loc is an empty
*    string, newlocale() will use the default locale.
*    
*    If base is NULL, the current locale is used. If base is LC_GLOBAL_LOCALE,
*    the global locale is used.
*    
*    If mask is LC_ALL_MASK, base is ignored.
*    
*  Notes
*    Conforms to POSIX.1-2017.
*
*    POSIX.1-2017 does not specify whether the locale object pointed to by
*    base is modified or whether it is freed and a new locale object created.
*    
*    The category mask LC_MESSAGES_MASK is not implemented as POSIX messages
*    are not implemented.
*
*  Thread safety
*    Safe [if configured].
*/
locale_t __SEGGER_RTL_PUBLIC_API newlocale(int category_mask, const char *loc, locale_t base) {
  const __SEGGER_RTL_locale_t * pLocale;
  int                           cat;
  //
  // Look up locale.
  //
  pLocale = __SEGGER_RTL_find_locale(loc);
  if (pLocale == 0) {
    //
    // Cannot find locale; this is only an error if category_mask
    // requires a partial copy.
    //
    errno = EINVAL;
    return 0;
  }
  //
  if (base == 0) {
    base = malloc(sizeof(*base));
    if (base == 0)
      {
        errno = ENOMEM;
        return 0;
      }
    (memset)(base, 0, sizeof(*base));
  }
  //
  for (cat = 0; cat < __SEGGER_RTL_MAX_CATEGORY; ++cat) {
    if (category_mask & (1 << cat)) {
      base->__category[cat] = pLocale;
    }
  }
  //
  return base;
}

/*********************************************************************
*
*       duplocale()
*
*  Function description
*    Duplicate locale data (POSIX.1).
*
*  Parameters
*    base - Locale to duplicate.
*
*  Return value
*    If there is insufficient memory to duplicate loc, returns
*    a NULL and sets errno to ENOMEM as required by POSIX.1-2017.
*    Otherwise, returns a new, duplicated locale.
*
*  Additional information
*    Duplicates the locale object referenced by loc.
*    Duplicated locales must be freed with freelocale().
*    
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe [if configured].
*/
locale_t __SEGGER_RTL_PUBLIC_API duplocale(locale_t base) {
  locale_t loc = malloc(sizeof(*loc));
  if (loc == 0)     {
    errno = ENOMEM;
  } else {
    *loc = *base;
  }
  return loc;
}

/*********************************************************************
*
*       freelocale()
*
*  Function description
*    Free locale (POSIX.1).
*
*  Parameters
*    loc - Locale to free.
*
*  Return value
*    Zero on success, -1 on error.  
*
*  Additional information
*    Frees the storage associated with loc.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe [if configured].
*/  
int __SEGGER_RTL_PUBLIC_API freelocale(locale_t loc) {
  //
  // Error if not freeing correctly.
  //
  if (loc == 0 || loc == LC_GLOBAL_LOCALE) {
    return -1;
  }
  //
  // Dump locale.
  //
  free(loc);
  //
  // Went OK.
  //
  return 0;
}

/*********************************************************************
*
*       uselocale()
*
*  Function description
*    Set current locale (POSIX.1).
*
*  Parameters
*    loc - Locale to use.
*
*  Return value
*    The locale set using the previous call to uselocale(), or
*    LC_GLOBAL_LOCALE if none was set.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe [if configured].
*/
locale_t __SEGGER_RTL_PUBLIC_API uselocale(locale_t loc) {
  locale_t prev;
  //
  prev = __SEGGER_RTL_current_locale();
  if (loc)
    __SEGGER_RTL_locale_ptr = loc;
  return prev;
}

/*********************************************************************
*
*       localeconv()
*
*  Function description
*    Get current locale data.
*
*  Return value
*    Returns a pointer to a structure of type lconv with the
*    corresponding values for the current locale filled in.
*
*  Thread safety
*    Safe [if configured].
*/
struct lconv * __SEGGER_RTL_PUBLIC_API localeconv(void) {
  return localeconv_l(__SEGGER_RTL_current_locale());
}

/*********************************************************************
*
*       localeconv_l()
*
*  Function description
*    Get locale data (POSIX.1).
*
*  Parameters
*    loc - Locale to inquire.
*
*  Return value
*    Returns a pointer to a structure of type lconv with the
*    corresponding values for the locale loc filled in.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
struct lconv * __SEGGER_RTL_PUBLIC_API localeconv_l(locale_t loc) {
  //
  // Grab fields from global locale and put them into lconv.
  // This is dumb, but it's the standard.  Sort of like
  // Cobol's MOVE CORRESPONDING.
  //
  const __SEGGER_RTL_locale_t *cat;
  //
  // Take these fields from numeric category.
  //
  cat = loc->__category[LC_NUMERIC];
  __SEGGER_RTL_lconv_data.decimal_point = (char *)cat->data->decimal_point;
  __SEGGER_RTL_lconv_data.thousands_sep = (char *)cat->data->thousands_sep;
  __SEGGER_RTL_lconv_data.grouping = (char *)cat->data->grouping;
  //
  // Take these fields from monetary category.
  //
  cat = loc->__category[LC_MONETARY];
  __SEGGER_RTL_lconv_data.int_curr_symbol = (char *)cat->data->int_curr_symbol;
  __SEGGER_RTL_lconv_data.currency_symbol = (char *)cat->data->currency_symbol;
  __SEGGER_RTL_lconv_data.mon_decimal_point = (char *)cat->data->mon_decimal_point;
  __SEGGER_RTL_lconv_data.mon_thousands_sep = (char *)cat->data->mon_thousands_sep;
  __SEGGER_RTL_lconv_data.mon_grouping = (char *)cat->data->mon_grouping;
  __SEGGER_RTL_lconv_data.positive_sign = (char *)cat->data->positive_sign;
  __SEGGER_RTL_lconv_data.negative_sign = (char *)cat->data->negative_sign;
  __SEGGER_RTL_lconv_data.int_frac_digits = cat->data->int_frac_digits;
  __SEGGER_RTL_lconv_data.frac_digits = cat->data->frac_digits;
  __SEGGER_RTL_lconv_data.p_cs_precedes = cat->data->p_cs_precedes;
  __SEGGER_RTL_lconv_data.p_sep_by_space = cat->data->p_sep_by_space;
  __SEGGER_RTL_lconv_data.n_cs_precedes = cat->data->n_cs_precedes;
  __SEGGER_RTL_lconv_data.n_sep_by_space = cat->data->n_sep_by_space;
  __SEGGER_RTL_lconv_data.p_sign_posn = cat->data->p_sign_posn;
  __SEGGER_RTL_lconv_data.n_sign_posn = cat->data->n_sign_posn;
  //
  // Extra monetary fields added by C99.
  //
  __SEGGER_RTL_lconv_data.int_p_cs_precedes = cat->data->int_p_cs_precedes;
  __SEGGER_RTL_lconv_data.int_n_cs_precedes = cat->data->int_n_cs_precedes;
  __SEGGER_RTL_lconv_data.int_p_sep_by_space = cat->data->int_p_sep_by_space;
  __SEGGER_RTL_lconv_data.int_n_sep_by_space = cat->data->int_n_sep_by_space;
  __SEGGER_RTL_lconv_data.int_p_sign_posn = cat->data->int_p_sign_posn;
  __SEGGER_RTL_lconv_data.int_n_sign_posn = cat->data->int_n_sign_posn;
  //
  return &__SEGGER_RTL_lconv_data;
}

/*********************************************************************
*
*       setlocale()
*
*  Function description
*    Set locale.
*
*  Parameters
*    category - Category of locale to set, see below.
*    loc      - Pointer to name of locale to set or, if NULL, the current locale.
*
*  Return value
*    Returns the name of the current locale if a locale name buffer has been
*    set using __SEGGER_RTL_set_locale_name_buffer(), else returns NULL.
*
*  Additional information
*    For ISO-correct operation, a local name buffer needs to be set using
*    __SEGGER_RTL_set_locale_name_buffer() when the name of the current or
*    global locale can be encoded.  In many cases the previous locale's name
*    is not required, yet would take static storage on a global or per-thread
*    basis.  In order to avoid this, the standard operation of setlocale() in
*    this library is to return NULL and not require any static data. If the
*    previous locale's name is required, at runtime startup or before calling
*    setlocale(), use __SEGGER_RTL_set_locale_name_buffer() to set the address
*    of the object to use where the locale name can be encoded.  To make this
*    thread-safe, the object where the locale name is stored must be
*    local to the thread.
*
*    The category parameter can have the following values:
*    
*    +-------------+--------------------------------------------------------------------------+
*    | Value       | Description                                                              |
*    +-------------+--------------------------------------------------------------------------+
*    | LC_ALL      | Entire locale.                                                           |
*    | LC_COLLATE  | Affects strcoll() and strxfrm().                                         |
*    | LC_CTYPE    | Affects character handling.                                              |
*    | LC_MONETARY | Affects monetary formatting information.                                 |
*    | LC_NUMERIC  | Affects decimal-point character in I/O and string formatting operations. |
*    | LC_TIME     | Affects strftime().                                                      |
*    +-------------+--------------------------------------------------------------------------+
*
*  Thread safety
*    Safe [if configured].
*/
char * __SEGGER_RTL_PUBLIC_API setlocale(int category, const char *loc) {
  //
#if __SEGGER_RTL_MINIMAL_LOCALE
  //
  __SEGGER_RTL_USE_PARA(category);
  __SEGGER_RTL_USE_PARA(loc);
  //
  return NULL;
  //
#else
  //
  // Encode existing locale.
  //
  if (__SEGGER_RTL_locale_name_buffer != NULL) {
    __SEGGER_RTL_encode_locale(__SEGGER_RTL_locale_name_buffer, __SEGGER_RTL_current_locale());
  }
  //
  // A null locale inquires the current locale's name.
  //
  if (loc == NULL) {
    return __SEGGER_RTL_locale_name_buffer;
  }
  //
  // Non-null locale requests setting the new locale.  We do this
  // for all locales specified.  First, test to ensure we can find
  // the locales.  This might well be a malformed locale string,
  // in which case we diagnose it early.
  //
  if (__SEGGER_RTL_set_locale(category, loc, 0) == NULL) {
    return NULL;
  }
  //
  // Do the setting for real.
  //
  return __SEGGER_RTL_set_locale(category, loc, 1);
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_X_find_locale()
*
*  Function description
*    Find locale.
*
*  Parameters
*    locale - Pointer to zero-terminated locale name.
*
*  Return value
*    Returns a pointer to a locale or NULL if none can be found.
*
*  Thread safety
*    Not applicable.
*/
const __SEGGER_RTL_locale_t * __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_find_locale(const char *locale) {
  __SEGGER_RTL_USE_PARA(locale);
  //
  return NULL;
}

/*********************************************************************
*
*       __SEGGER_RTL_set_locale_name_buffer()
*
*  Function description
*    Set buffer used to store locale names.
*
*  Parameters
*    buf - Pointer to object that stores name buffer.
*
*  Additional information
*    The function setlocale() returns a pointer to an object that contains
*    the previously-set locale.  As such, that object must be thread-local,
*    but its size is not known at compile time.  Additionally, to be thread-safe,
*    this buffer must be local to the thread and not shared between separate
*    execution contexts.
*
*    The user can set the location for the name buffer used by setlocale()
*    using this function.  The name buffer must be large enough to contain
*    six locale names separated by semicolons and terminated by a final null.
*    Assuming a locale name "hu_HU.iso_8859_2" of 16 characters, a buffer of
*    at least 102 characters is required for correct operation.
*
*    Note that if no name buffer is set using this function, setlocale() may
*    still be used but will return NULL as the result.  This enables use of
*    setlocale() but does incur an overhead for a thread-local or global
*    buffer which may never be required.
*
*  Thread safety
*    Safe [if configured].
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_set_locale_name_buffer(char *buf) {
  __SEGGER_RTL_locale_name_buffer = buf;
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

const __SEGGER_RTL_locale_codeset_t __SEGGER_RTL_codeset_ascii = {
  __SEGGER_RTL_ascii_isctype,
  __SEGGER_RTL_ascii_toupper,
  __SEGGER_RTL_ascii_tolower,
  __SEGGER_RTL_ascii_iswctype,  // wide versions default to narrow versions...
  __SEGGER_RTL_ascii_towupper,
  __SEGGER_RTL_ascii_towlower,
  __SEGGER_RTL_ascii_wctomb,
  __SEGGER_RTL_ascii_mbtowc
};

const __SEGGER_RTL_locale_codeset_t __SEGGER_RTL_codeset_utf8 = {
  __SEGGER_RTL_unicode_isctype,
  __SEGGER_RTL_unicode_toupper,
  __SEGGER_RTL_unicode_tolower,
  __SEGGER_RTL_unicode_iswctype,
  __SEGGER_RTL_unicode_towupper,
  __SEGGER_RTL_unicode_towlower,
  __SEGGER_RTL_utf8_wctomb,
  __SEGGER_RTL_utf8_mbtowc
};

const char __SEGGER_RTL_c_locale_abbrev_day_names[] = {
  "Sun\0"
  "Mon\0"
  "Tue\0"
  "Wed\0"
  "Thu\0"
  "Fri\0"
  "Sat\0"
};

const char __SEGGER_RTL_c_locale_abbrev_month_names[] = {
  "Jan\0"
  "Feb\0"
  "Mar\0"
  "Apr\0"
  "May\0"
  "Jun\0"
  "Jul\0"
  "Aug\0"
  "Sep\0"
  "Oct\0"
  "Nov\0"
  "Dec\0"
};

const char __SEGGER_RTL_data_utf8_period[] = {
  "."
};

const char __SEGGER_RTL_data_utf8_comma[] = {
  "."
};

const char __SEGGER_RTL_data_utf8_space[] = {
  " "
};

const char __SEGGER_RTL_data_utf8_plus[] = {
  "+"
};

const char __SEGGER_RTL_data_utf8_minus[] = {
  "-"
};

const char __SEGGER_RTL_data_empty_string[] = {
  ""
};

const __SEGGER_RTL_locale_data_t __SEGGER_RTL_c_locale_data = {
  __SEGGER_RTL_data_utf8_period,   // decimal_point
  __SEGGER_RTL_data_empty_string,  // thousands_sep
  __SEGGER_RTL_data_empty_string,  // grouping

  __SEGGER_RTL_data_empty_string,  // int_curr_symbol
  __SEGGER_RTL_data_empty_string,  // currency_symbol
  __SEGGER_RTL_data_empty_string,  // mon_decimal_point
  __SEGGER_RTL_data_empty_string,  // mon_thousands_sep
  __SEGGER_RTL_data_empty_string,  // mon_grouping
  __SEGGER_RTL_data_empty_string,  // positive_sign
  __SEGGER_RTL_data_empty_string,  // negative_sign

  CHAR_MAX,       // char int_frac_digits
  CHAR_MAX,       // char frac_digits

  CHAR_MAX,       // char p_cs_precedes
  CHAR_MAX,       // char p_sep_by_space

  CHAR_MAX,       // char n_cs_precedes
  CHAR_MAX,       // char n_sep_by_space

  CHAR_MAX,       // char n_sign_posn
  CHAR_MAX,       // char p_sign_posn

  CHAR_MAX,       // char int_p_cs_precedes
  CHAR_MAX,       // char int_n_cs_precedes
  CHAR_MAX,       // char int_p_sep_by_space
  CHAR_MAX,       // char int_n_sep_by_space
  CHAR_MAX,       // char int_p_sign_posn
  CHAR_MAX,       // char int_n_sign_posn

  __SEGGER_RTL_c_locale_day_names,
  __SEGGER_RTL_c_locale_abbrev_day_names,
  __SEGGER_RTL_c_locale_month_names,
  __SEGGER_RTL_c_locale_abbrev_month_names,
  __SEGGER_RTL_c_locale_am_pm_indicator,
  __SEGGER_RTL_c_locale_date_format,
  __SEGGER_RTL_c_locale_time_format,
  __SEGGER_RTL_c_locale_date_time_format
};

const __SEGGER_RTL_locale_t __SEGGER_RTL_c_locale = {
  "C",
  &__SEGGER_RTL_c_locale_data,
  &__SEGGER_RTL_codeset_ascii,
};

/*************************** End of file ****************************/

int __RAL_mb_max(const struct __SEGGER_RTL_POSIX_locale_s *)__attribute__((alias("__SEGGER_RTL_mb_max")));

extern struct __SEGGER_RTL_POSIX_locale_s __attribute__((alias("__SEGGER_RTL_global_locale"))) __RAL_global_locale;

