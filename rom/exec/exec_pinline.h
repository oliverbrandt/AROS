#ifndef _EXEC_PINLINE_H
#define _EXEC_PINLINE_H

/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$

    Desc: Private m68k inlines for exec.library
    Lang: english
*/

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif

#ifndef EXEC_BASE_NAME
#define EXEC_BASE_NAME SysBase
#endif

#define RawIOInit() \
	LP0NR(0x1f8, LONG, RawIOInit, \
	, EXEC_BASE_NAME)

#define RawMayGetChar() \
	LP0NR(0x1fe, LONG, RawMayGetChar, \
	, EXEC_BASE_NAME)

#define RawPutChar(chr) \
	LP1NR(0x204, RawPutChar, UBYTE, chr, d0, \
	, EXEC_BASE_NAME)

#endif /* _EXEC_PINLINE_H */
