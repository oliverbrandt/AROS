/*
    Copyright (C) 1995-1997 AROS - The Amiga Replacement OS
    $Id$

    Desc: ReadBattClock() function.
    Lang: english
*/
#include "battclock_intern.h"

#include <proto/battclock.h>
#include <time.h>

AROS_LH0(ULONG, ReadBattClock, APTR, BattClockBase, 2, Battclock)
{
    AROS_LIBFUNC_INIT

    /*
        This is mostly an example.
	It is quite possible that this time value is not for the local
	timezone, so it has to be converted.
    */
    time_t t;
    struct tm *tm;    

    time(&t);
    tm = localtime(&t);

    /*
	This time is however relative to 1.1.1970, so we have to subtract a
	large number of seconds so that things actually work correctly.

	There is a problem here that 8 years before the Amiga clock dies,
	the Unix system clock will have problems as it will wrap around.
	Still, I'll be dead, and by then they won't be using 32-bit clocks
	I expect...
    */
    return (t - 252460800 + tm->tm_gmtoff);

    AROS_LIBFUNC_EXIT
} /* ReadBattClock */
