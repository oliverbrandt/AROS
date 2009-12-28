/*
    Copyright � 1995-2009, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Default trap handler
    Lang: english
*/

#include <exec/alerts.h>
#include <exec/tasks.h>
#include <proto/exec.h>
#include <proto/arossupport.h>

/* In original AmigaOS the trap handler is entered in supervisor mode with the
 * following on the supervisor stack:
 *  0(sp).l = trap#
 *  4(sp) Processor dependent exception frame
 * This generic implementation is quite incomplete. See arch/all-mingw32 for
 * more correct and complete one.
 */

void Exec_TrapHandler(ULONG trapNum)
{
    Alert(AT_DeadEnd | trapNum);
} /* TrapHandler */
