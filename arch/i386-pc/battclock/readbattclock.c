/*
    Copyright (C) 1995-1997 AROS - The Amiga Research OS
    $Id$

    Desc: ReadBattClock() function.
    Lang: english
*/
#include "battclock_intern.h"

#define	ReadRTC(in,out)				\
    asm volatile (				\
    "outb	%%al,$0x70	\n\t"		\
    "inb	$0x71,%%al	\n\t"		\
    "movl	%%eax,%%ebx	\n\t"		\
    "andl	$0x0f,%%eax	\n\t"		\
    "shrl	$4,%%ebx	\n\t"		\
    "imul	$10,%%ebx	\n\t"		\
    "addl	%%ebx,%%eax"			\
    : "=al"(out)				\
    : "0"(in)					\
    : "%ebx","cc");

/*****************************************************************************

    NAME */
#include <proto/battclock.h>
#include <proto/utility.h>
#include <utility/date.h>

	AROS_LH0(ULONG, ReadBattClock,

/*  SYNOPSIS */
	/* void */

/*  LOCATION */
	struct BattClockBase *, BattClockBase, 2, Battclock)

/*  FUNCTION
	Return the value stored in the battery back up clock. This value
	is the number of seconds that have elapsed since midnight on the
	1st of January 1978 (00:00:00 1.1.1978).

	If the value of the battery clock is invalid, then the clock will
	be reset.

    INPUTS

    RESULT
	The number of seconds since 1.1.1978 00:00:00

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	WriteBattClock, ResetBattClock

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    battclock_lib.fd and clib/battclock_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct ClockData date;        
    UWORD  year;
    ULONG  secs;

    ReadRTC(0,date.sec);
    ReadRTC(2,date.min);
    ReadRTC(4,date.hour);
    ReadRTC(7,date.mday);
    ReadRTC(8,date.month);
    ReadRTC(9,date.year);
    ReadRTC(50,year);
    date.year+=100*year;

    secs=Date2Amiga(&date);

    return secs;

    AROS_LIBFUNC_EXIT
} /* ReadBattClock */
