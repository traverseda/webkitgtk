#if __APPLE__

#define HAVE_FUNC_ISINF 1
#define HAVE_FUNC_ISNAN 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_TIME_H 1
#define TIME_WITH_SYS_TIME 1

#elif WIN32

#define HAVE_FLOAT_H 1
#define HAVE_FUNC__FINITE 1
#define HAVE_SYS_TIMEB_H 1

#else

#define HAVE_FUNC_ISINF 1
#define HAVE_FUNC_ISNAN 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_TIME_H 1
#define TIME_WITH_SYS_TIME 1
#define HAVE_ERRNO_H 1

#endif

#define HAVE_FUNC_STRTOLL 1
#define HAVE_ICU 1
#define HAVE_PCREPOSIX 1
#define HAVE_STRING_H 1
#define HAVE_STDINT_H 1

#define HAVE_MMAP 1
#define HAVE_SBRK 1

#ifdef __ppc__
#define WORDS_BIGENDIAN 1
#endif

/* define to debug garbage collection */
#undef DEBUG_COLLECTOR

#define KXC_CHANGES 1

#ifdef __cplusplus
#undef new
#undef delete
#include <kxmlcore/FastMalloc.h>
#endif
