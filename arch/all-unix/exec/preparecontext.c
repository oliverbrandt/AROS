/*
    Copyright (C) 1995-1997 AROS - The Amiga Replacement OS
    $Id$

    Desc: i386unix version of PrepareContext().
    Lang: english
*/

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include "sigcore.h"
#include "etask.h"
#include "exec_util.h"

#include <aros/libcall.h>

AROS_LH3(BOOL, PrepareContext,
    AROS_LHA(struct Task *, task, A0),
    AROS_LHA(APTR, entryPoint, A1),
    AROS_LHA(APTR, fallBack, A2),
    struct ExecBase *, SysBase, 6, Exec)
{
    AROS_LIBFUNC_INIT

    /*
	There is not much to do here, or at least that is how it
	appears. Most of the work is done in the sigcore.h macros.
    */

    if (!(task->tc_Flags & TF_ETASK) )
	return FALSE;

    GetIntETask (task)->iet_Context = AllocTaskMem (task
	, SIZEOF_ALL_REGISTERS
	, MEMF_PUBLIC|MEMF_CLEAR
    );

    if (!GetIntETask (task)->iet_Context)
	return FALSE;

    /* First we push the return address */
    _PUSH(GetSP(task), fallBack);

    /* Then set up the frame to be used by Dispatch() */
    PREPARE_INITIAL_FRAME(GetSP(task), entryPoint);
    PREPARE_INITIAL_CONTEXT(task, entryPoint);

    /* We return the new stack pointer back to the caller. */
    return TRUE;

    AROS_LIBFUNC_EXIT
}
