/**************************************************************************
 * 
 * Copyright 2008 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/**
 * @file
 * Gallium configuration defines.
 * 
 * This header file sets several defines based on the compiler, processor 
 * architecture, and operating system being used. These defines should be used 
 * throughout the code to facilitate porting to new platforms. It is likely that 
 * this file is auto-generated by an autoconf-like tool at some point, as some 
 * things cannot be determined by pre-defined environment alone. 
 * 
 * See also:
 * - http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
 * - echo | gcc -dM -E - | sort
 * - http://msdn.microsoft.com/en-us/library/b0084kay.aspx
 * 
 * @author José Fonseca <jrfonseca@tungstengraphics.com>
 */

#ifndef P_CONFIG_H_
#define P_CONFIG_H_


/*
 * Compiler
 */

#if defined(__GNUC__)
#define PIPE_CC_GCC
#define PIPE_CC_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#endif

/*
 * Meaning of _MSC_VER value:
 * - 1400: Visual C++ 2005
 * - 1310: Visual C++ .NET 2003
 * - 1300: Visual C++ .NET 2002
 * 
 * __MSC__ seems to be an old macro -- it is not pre-defined on recent MSVC 
 * versions.
 */
#if defined(_MSC_VER) || defined(__MSC__)
#define PIPE_CC_MSVC
#endif

#if defined(__ICL)
#define PIPE_CC_ICL
#endif


/*
 * Processor architecture
 */

#if defined(__i386__) /* gcc */ || defined(_M_IX86) /* msvc */ || defined(_X86_) || defined(__386__) || defined(i386) || defined(__i386) /* Sun cc */
#define PIPE_ARCH_X86
#endif

#if defined(__x86_64__) /* gcc */ || defined(_M_X64) /* msvc */ || defined(_M_AMD64) /* msvc */ || defined(__x86_64) /* Sun cc */
#define PIPE_ARCH_X86_64
#endif

#if defined(PIPE_ARCH_X86) || defined(PIPE_ARCH_X86_64)
#if defined(PIPE_CC_GCC) && !defined(__SSE2__)
/* #warning SSE2 support requires -msse -msse2 compiler options */
#else
#define PIPE_ARCH_SSE
#endif
#if defined(PIPE_CC_GCC) && !defined(__SSSE3__)
/* #warning SSE3 support requires -msse3 compiler options */
#else
#define PIPE_ARCH_SSSE3
#endif
#endif

#if defined(__PPC__)
#define PIPE_ARCH_PPC
#if defined(__PPC64__)
#define PIPE_ARCH_PPC_64
#endif
#endif

#if defined(__mc68000) /* gcc */
#define PIPE_ARCH_M68K
#endif


/*
 * Endian detection.
 */

#if defined(PIPE_ARCH_X86) || defined(PIPE_ARCH_X86_64)
#define PIPE_ARCH_LITTLE_ENDIAN
#elif defined(PIPE_ARCH_PPC) || defined(PIPE_ARCH_PPC_64)
#define PIPE_ARCH_BIG_ENDIAN
#else
#define PIPE_ARCH_UNKNOWN_ENDIAN
#endif


#if !defined(PIPE_OS_EMBEDDED)

/*
 * Auto-detect the operating system family.
 * 
 * See subsystem below for a more fine-grained distinction.
 */

#if defined(__linux__)
#define PIPE_OS_LINUX
#define PIPE_OS_UNIX
#endif

#if defined(__FreeBSD__)
#define PIPE_OS_FREEBSD
#define PIPE_OS_BSD
#define PIPE_OS_UNIX
#endif

#if defined(__OpenBSD__)
#define PIPE_OS_OPENBSD
#define PIPE_OS_BSD
#define PIPE_OS_UNIX
#endif

#if defined(__NetBSD__)
#define PIPE_OS_NETBSD
#define PIPE_OS_BSD
#define PIPE_OS_UNIX
#endif

#if defined(__GNU__)
#define PIPE_OS_HURD
#define PIPE_OS_UNIX
#endif

#if defined(__sun)
#define PIPE_OS_SOLARIS
#define PIPE_OS_UNIX
#endif

#if defined(__APPLE__)
#define PIPE_OS_APPLE
#define PIPE_OS_UNIX
#endif

#if defined(_WIN32) || defined(WIN32)
#define PIPE_OS_WINDOWS
#endif

#if defined(__HAIKU__)
#define PIPE_OS_HAIKU
#define PIPE_OS_UNIX
#endif

#if defined(__CYGWIN__)
#define PIPE_OS_CYGWIN
#define PIPE_OS_UNIX
#endif

#if defined(__AROS__)
#define PIPE_OS_AROS
#endif

/*
 * Try to auto-detect the subsystem.
 * 
 * NOTE: There is no way to auto-detect most of these.
 */

#if defined(PIPE_OS_LINUX) || defined(PIPE_OS_BSD) || defined(PIPE_OS_SOLARIS)
#define PIPE_SUBSYSTEM_DRI
#endif /* PIPE_OS_LINUX || PIPE_OS_BSD || PIPE_OS_SOLARIS */

#if defined(PIPE_OS_WINDOWS)
#if defined(PIPE_SUBSYSTEM_WINDOWS_DISPLAY)
/* Windows 2000/XP Display Driver */ 
#elif defined(PIPE_SUBSYSTEM_WINDOWS_MINIPORT)
/* Windows 2000/XP Miniport Driver */ 
#elif defined(PIPE_SUBSYSTEM_WINDOWS_USER)
/* Windows User-space Library */
#elif defined(PIPE_SUBSYSTEM_WINDOWS_CE)
/* Windows CE 5.0/6.0 */
#else
#ifdef _WIN32_WCE
#define PIPE_SUBSYSTEM_WINDOWS_CE
#else /* !_WIN32_WCE */
#error No PIPE_SUBSYSTEM_WINDOWS_xxx subsystem defined. 
#endif /* !_WIN32_WCE */
#endif
#endif /* PIPE_OS_WINDOWS */

#endif /* !PIPE_OS_EMBEDDED */


#endif /* P_CONFIG_H_ */
