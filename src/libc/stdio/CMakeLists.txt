if(MICROCRT_BUILD_STDIO_LEGACY)
  microcrt_sources(microcrt_c
    legacy/fileops.c
    legacy/iob.c
    printf/dtoa_engine.c
    printf/dtox_engine.c
    printf/fprintf.c
    printf/iprintf.c
    printf/ldtoa_engine.c
    printf/ldtox_engine.c
    printf/matchcaseprefix.c
    printf/printf.c
    printf/snprintf.c
    printf/sprintf.c
    printf/vfprintf.c
    printf/vfprintf_integer.c
    printf/vprintf.c
  )
endif()

# This directory is the long-term home of the rewritten printf framework.
# The template sources under printf/ are staged here first and will be wired
# into build targets as the MicroCRT-native implementation becomes complete.
