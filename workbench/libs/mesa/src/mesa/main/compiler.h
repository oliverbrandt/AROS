/*
 * Mesa 3-D graphics library
 * Version:  7.5
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * \file compiler.h
 * Compiler-related stuff.
 */


#ifndef COMPILER_H
#define COMPILER_H


#include <assert.h>
#include <ctype.h>
#if defined(__alpha__) && defined(CCPML)
#include <cpml.h> /* use Compaq's Fast Math Library on Alpha */
#else
#include <math.h>
#endif
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(__linux__) && defined(__i386__)
#include <fpu_control.h>
#endif
#include <float.h>
#include <stdarg.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Get standard integer types
 */
#if defined(_MSC_VER)
   typedef __int8             int8_t;
   typedef unsigned __int8    uint8_t;
   typedef __int16            int16_t;
   typedef unsigned __int16   uint16_t;
   typedef __int32            int32_t;
   typedef unsigned __int32   uint32_t;
   typedef __int64            int64_t;
   typedef unsigned __int64   uint64_t;

#  if defined(_WIN64)
     typedef __int64            intptr_t;
     typedef unsigned __int64   uintptr_t;
#  else
     typedef __int32            intptr_t;
     typedef unsigned __int32   uintptr_t;
#  endif

#  define INT64_C(__val) __val##i64
#  define UINT64_C(__val) __val##ui64
#else
#  include <stdint.h>
#endif


/**
  * Sun compilers define __i386 instead of the gcc-style __i386__
 */
#ifdef __SUNPRO_C
# if !defined(__i386__) && defined(__i386)
#  define __i386__
# elif !defined(__amd64__) && defined(__amd64)
#  define __amd64__
# elif !defined(__sparc__) && defined(__sparc)
#  define __sparc__
# endif
# if !defined(__volatile)
#  define __volatile volatile
# endif
#endif


/**
 * finite macro.
 */
#if defined(_MSC_VER)
#  define finite _finite
#elif defined(__WATCOMC__)
#  define finite _finite
#endif


/**
 * Disable assorted warnings
 */
#if !defined(OPENSTEP) && (defined(__WIN32__) && !defined(__CYGWIN__)) && !defined(BUILD_FOR_SNAP)
#  if !defined(__GNUC__) /* mingw environment */
#    pragma warning( disable : 4068 ) /* unknown pragma */
#    pragma warning( disable : 4710 ) /* function 'foo' not inlined */
#    pragma warning( disable : 4711 ) /* function 'foo' selected for automatic inline expansion */
#    pragma warning( disable : 4127 ) /* conditional expression is constant */
#    if defined(MESA_MINWARN)
#      pragma warning( disable : 4244 ) /* '=' : conversion from 'const double ' to 'float ', possible loss of data */
#      pragma warning( disable : 4018 ) /* '<' : signed/unsigned mismatch */
#      pragma warning( disable : 4305 ) /* '=' : truncation from 'const double ' to 'float ' */
#      pragma warning( disable : 4550 ) /* 'function' undefined; assuming extern returning int */
#      pragma warning( disable : 4761 ) /* integral size mismatch in argument; conversion supplied */
#    endif
#  endif
#endif
#if defined(__WATCOMC__)
#  pragma disable_message(201) /* Disable unreachable code warnings */
#endif



/**
 * Function inlining
 */
#if defined(__GNUC__)
#  define INLINE __inline__
#elif defined(__MSC__)
#  define INLINE __inline
#elif defined(_MSC_VER)
#  define INLINE __inline
#elif defined(__ICL)
#  define INLINE __inline
#elif defined(__INTEL_COMPILER)
#  define INLINE inline
#elif defined(__WATCOMC__) && (__WATCOMC__ >= 1100)
#  define INLINE __inline
#elif defined(__SUNPRO_C) && defined(__C99FEATURES__)
#  define INLINE inline
#  define __inline inline
#  define __inline__ inline
#elif (__STDC_VERSION__ >= 199901L) /* C99 */
#  define INLINE inline
#else
#  define INLINE
#endif


/**
 * PUBLIC/USED macros
 *
 * If we build the library with gcc's -fvisibility=hidden flag, we'll
 * use the PUBLIC macro to mark functions that are to be exported.
 *
 * We also need to define a USED attribute, so the optimizer doesn't 
 * inline a static function that we later use in an alias. - ajax
 */
#if defined(__GNUC__) || (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590))
#  define PUBLIC __attribute__((visibility("default")))
#  define USED __attribute__((used))
#else
#  define PUBLIC
#  define USED
#endif


/**
 * Some compilers don't like some of Mesa's const usage.  In those places use
 * CONST instead of const.  Pass -DNO_CONST to compilers where this matters.
 */
#ifdef NO_CONST
#  define CONST
#else
#  define CONST const
#endif


/**
 * __builtin_expect macros
 */
#if !defined(__GNUC__)
#  define __builtin_expect(x, y) x
#endif

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) !!(x)
#define unlikely(x) !!(x)
#endif

/**
 * The __FUNCTION__ gcc variable is generally only used for debugging.
 * If we're not using gcc, define __FUNCTION__ as a cpp symbol here.
 * Don't define it if using a newer Windows compiler.
 */
#ifndef __FUNCTION__
# if defined(__VMS)
#  define __FUNCTION__ "VMS$NL:"
# elif !defined(__GNUC__) && !defined(__xlC__) &&	\
      (!defined(_MSC_VER) || _MSC_VER < 1300)
#  if (__STDC_VERSION__ >= 199901L) /* C99 */ || \
    (defined(__SUNPRO_C) && defined(__C99FEATURES__))
#   define __FUNCTION__ __func__
#  else
#   define __FUNCTION__ "<unknown>"
#  endif
# endif
#endif
#ifndef __func__
#  if (__STDC_VERSION__ >= 199901L) || \
      (defined(__SUNPRO_C) && defined(__C99FEATURES__))
       /* __func__ is part of C99 */
#  elif defined(_MSC_VER)
#    if _MSC_VER >= 1300
#      define __func__ __FUNCTION__
#    else
#      define __func__ "<unknown>"
#    endif
#  endif
#endif


/**
 * Either define MESA_BIG_ENDIAN or MESA_LITTLE_ENDIAN, and CPU_TO_LE32.
 * Do not use these unless absolutely necessary!
 * Try to use a runtime test instead.
 * For now, only used by some DRI hardware drivers for color/texel packing.
 */
#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
#if defined(__linux__)
#include <byteswap.h>
#define CPU_TO_LE32( x )	bswap_32( x )
#elif defined(__APPLE__)
#include <CoreFoundation/CFByteOrder.h>
#define CPU_TO_LE32( x )	CFSwapInt32HostToLittle( x )
#elif defined(__AROS__)
#define CPU_TO_LE32( x )    AROS_BE2LONG( x )
#elif (defined(_AIX) || defined(__blrts))
static INLINE GLuint CPU_TO_LE32(GLuint x)
{
   return (((x & 0x000000ff) << 24) |
           ((x & 0x0000ff00) <<  8) |
           ((x & 0x00ff0000) >>  8) |
           ((x & 0xff000000) >> 24));
}
#else /*__linux__ */
#include <sys/endian.h>
#define CPU_TO_LE32( x )	bswap32( x )
#endif /*__linux__*/
#define MESA_BIG_ENDIAN 1
#else
#define CPU_TO_LE32( x )	( x )
#define MESA_LITTLE_ENDIAN 1
#endif
#define LE32_TO_CPU( x )	CPU_TO_LE32( x )



#if !defined(CAPI) && defined(WIN32) && !defined(BUILD_FOR_SNAP)
#define CAPI _cdecl
#endif


/**
 * Create a macro so that asm functions can be linked into compilers other
 * than GNU C
 */
#ifndef _ASMAPI
#if defined(WIN32) && !defined(BUILD_FOR_SNAP)/* was: !defined( __GNUC__ ) && !defined( VMS ) && !defined( __INTEL_COMPILER )*/
#define _ASMAPI __cdecl
#else
#define _ASMAPI
#endif
#ifdef	PTR_DECL_IN_FRONT
#define	_ASMAPIP * _ASMAPI
#else
#define	_ASMAPIP _ASMAPI *
#endif
#endif

#ifdef USE_X86_ASM
#define _NORMAPI _ASMAPI
#define _NORMAPIP _ASMAPIP
#else
#define _NORMAPI
#define _NORMAPIP *
#endif


/* This is a macro on IRIX */
#ifdef _P
#undef _P
#endif


/* Turn off macro checking systems used by other libraries */
#ifdef CHECK
#undef CHECK
#endif


/**
 * ASSERT macro
 */
#if defined(__AROS__)
#undef ASSERT
#endif
#if !defined(_WIN32_WCE)
#if defined(BUILD_FOR_SNAP) && defined(CHECKED)
#  define ASSERT(X)   _CHECK(X) 
#elif defined(DEBUG)
#  define ASSERT(X)   assert(X)
#else
#  define ASSERT(X)
#endif
#endif

#if (__GNUC__ >= 3)
#define PRINTFLIKE(f, a) __attribute__ ((format(__printf__, f, a)))
#else
#define PRINTFLIKE(f, a)
#endif

#ifndef NULL
#define NULL 0
#endif


/**
 * LONGSTRING macro
 * gcc -pedantic warns about long string literals, LONGSTRING silences that.
 */
#if !defined(__GNUC__)
# define LONGSTRING
#else
# define LONGSTRING __extension__
#endif


#ifndef M_PI
#define M_PI (3.1415926536)
#endif

#ifndef M_E
#define M_E (2.7182818284590452354)
#endif

#ifndef ONE_DIV_LN2
#define ONE_DIV_LN2 (1.442695040888963456)
#endif

#ifndef ONE_DIV_SQRT_LN2
#define ONE_DIV_SQRT_LN2 (1.201122408786449815)
#endif

#ifndef FLT_MAX_EXP
#define FLT_MAX_EXP 128
#endif


/**
 * USE_IEEE: Determine if we're using IEEE floating point
 */
#if defined(__i386__) || defined(__386__) || defined(__sparc__) || \
    defined(__s390x__) || defined(__powerpc__) || \
    defined(__x86_64__) || \
    defined(ia64) || defined(__ia64__) || \
    defined(__hppa__) || defined(hpux) || \
    defined(__mips) || defined(_MIPS_ARCH) || \
    defined(__arm__) || \
    defined(__sh__) || defined(__m32r__) || \
    (defined(__sun) && defined(_IEEE_754)) || \
    (defined(__alpha__) && (defined(__IEEE_FLOAT) || !defined(VMS)))
#define USE_IEEE
#define IEEE_ONE 0x3f800000
#endif


/**
 * START/END_FAST_MATH macros:
 *
 * START_FAST_MATH: Set x86 FPU to faster, 32-bit precision mode (and save
 *                  original mode to a temporary).
 * END_FAST_MATH: Restore x86 FPU to original mode.
 */
#if defined(__GNUC__) && defined(__i386__)
/*
 * Set the x86 FPU control word to guarentee only 32 bits of precision
 * are stored in registers.  Allowing the FPU to store more introduces
 * differences between situations where numbers are pulled out of memory
 * vs. situations where the compiler is able to optimize register usage.
 *
 * In the worst case, we force the compiler to use a memory access to
 * truncate the float, by specifying the 'volatile' keyword.
 */
/* Hardware default: All exceptions masked, extended double precision,
 * round to nearest (IEEE compliant):
 */
#define DEFAULT_X86_FPU		0x037f
/* All exceptions masked, single precision, round to nearest:
 */
#define FAST_X86_FPU		0x003f
/* The fldcw instruction will cause any pending FP exceptions to be
 * raised prior to entering the block, and we clear any pending
 * exceptions before exiting the block.  Hence, asm code has free
 * reign over the FPU while in the fast math block.
 */
#if defined(NO_FAST_MATH)
#define START_FAST_MATH(x)						\
do {									\
   static GLuint mask = DEFAULT_X86_FPU;				\
   __asm__ ( "fnstcw %0" : "=m" (*&(x)) );				\
   __asm__ ( "fldcw %0" : : "m" (mask) );				\
} while (0)
#else
#define START_FAST_MATH(x)						\
do {									\
   static GLuint mask = FAST_X86_FPU;					\
   __asm__ ( "fnstcw %0" : "=m" (*&(x)) );				\
   __asm__ ( "fldcw %0" : : "m" (mask) );				\
} while (0)
#endif
/* Restore original FPU mode, and clear any exceptions that may have
 * occurred in the FAST_MATH block.
 */
#define END_FAST_MATH(x)						\
do {									\
   __asm__ ( "fnclex ; fldcw %0" : : "m" (*&(x)) );			\
} while (0)

#elif defined(__WATCOMC__) && defined(__386__)
#define DEFAULT_X86_FPU		0x037f /* See GCC comments above */
#define FAST_X86_FPU		0x003f /* See GCC comments above */
void _watcom_start_fast_math(unsigned short *x,unsigned short *mask);
#pragma aux _watcom_start_fast_math =                                   \
   "fnstcw  word ptr [eax]"                                             \
   "fldcw   word ptr [ecx]"                                             \
   parm [eax] [ecx]                                                     \
   modify exact [];
void _watcom_end_fast_math(unsigned short *x);
#pragma aux _watcom_end_fast_math =                                     \
   "fnclex"                                                             \
   "fldcw   word ptr [eax]"                                             \
   parm [eax]                                                           \
   modify exact [];
#if defined(NO_FAST_MATH)
#define START_FAST_MATH(x)                                              \
do {                                                                    \
   static GLushort mask = DEFAULT_X86_FPU;	                        \
   _watcom_start_fast_math(&x,&mask);                                   \
} while (0)
#else
#define START_FAST_MATH(x)                                              \
do {                                                                    \
   static GLushort mask = FAST_X86_FPU;                                 \
   _watcom_start_fast_math(&x,&mask);                                   \
} while (0)
#endif
#define END_FAST_MATH(x)  _watcom_end_fast_math(&x)

#elif defined(_MSC_VER) && defined(_M_IX86)
#define DEFAULT_X86_FPU		0x037f /* See GCC comments above */
#define FAST_X86_FPU		0x003f /* See GCC comments above */
#if defined(NO_FAST_MATH)
#define START_FAST_MATH(x) do {\
	static GLuint mask = DEFAULT_X86_FPU;\
	__asm fnstcw word ptr [x]\
	__asm fldcw word ptr [mask]\
} while(0)
#else
#define START_FAST_MATH(x) do {\
	static GLuint mask = FAST_X86_FPU;\
	__asm fnstcw word ptr [x]\
	__asm fldcw word ptr [mask]\
} while(0)
#endif
#define END_FAST_MATH(x) do {\
	__asm fnclex\
	__asm fldcw word ptr [x]\
} while(0)

#else
#define START_FAST_MATH(x)  x = 0
#define END_FAST_MATH(x)  (void)(x)
#endif


#ifndef Elements
#define Elements(x) (sizeof(x)/sizeof(*(x)))
#endif



#ifdef __cplusplus
}
#endif


#endif /* COMPILER_H */
