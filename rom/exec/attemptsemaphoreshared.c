/*
    Copyright (C) 1995-2000 AROS - The Amiga Research OS
    $Id$

    Desc: Try to lock a semaphore shared.
    Lang: english
*/
#include "exec_intern.h"
#include "semaphores.h"
#include <exec/semaphores.h>
#include <proto/exec.h>

/*****************************************************************************

    NAME */

	AROS_LH1(ULONG, AttemptSemaphoreShared,

/*  SYNOPSIS */
	AROS_LHA(struct SignalSemaphore *, sigSem, A0),

/*  LOCATION */
	struct ExecBase *, SysBase, 120, Exec)

/*  FUNCTION
	Tries to get a shared lock on a signal semaphore. If the lock cannot
	be obtained false is returned. There may be more than one shared lock
	at a time but an exclusive lock prevents all other locks. The lock
	must be released with ReleaseSemaphore().

    INPUTS
	sigSem - pointer to semaphore structure

    RESULT
	True if the semaphore could be obtained, false otherwise.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	ReleaseSemaphore()

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    exec_lib.fd and clib/exec_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct ExecBase *,SysBase)

    struct Task *me = FindTask(NULL);

    /* Protect the semaphore structure */
    Forbid();

    /* Increment the queue count. This will need SMP protection */
    sigSem->ss_QueueCount++;

    if( sigSem->ss_QueueCount == 0 )
    {
	/* The semaphore wasn't owned. We can now own it */
	sigSem->ss_Owner = me;
	sigSem->ss_NestCount++;
    }
    else if( sigSem->ss_Owner == me )
    {
	/* The semaphore was owned by me, just increase the nest count */
	sigSem->ss_NestCount++;
    }
    else
    {
	/* We can't get ownership, just return it. */
	sigSem->ss_QueueCount--;
    }
    
    /* All done. */
    Permit();

    return (sigSem->ss_Owner == me ? TRUE : FALSE );

    AROS_LIBFUNC_EXIT
} /* AttemptSemaphoreShared */
