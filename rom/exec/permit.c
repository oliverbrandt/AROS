/*
    Copyright (C) 1995-1997 AROS - The Amiga Replacement OS
    $Id$

    Desc: Permit() - Allow tasks switches to occur.
    Lang: english
*/

#include <exec/types.h>
#include <exec/execbase.h>
#include <aros/libcall.h>
#include <proto/exec.h>

/*****************************************************************************/
#ifndef UseExecstubs

/*  NAME */

	AROS_LH0(void, Permit,

/*  LOCATION */
	struct ExecBase *, SysBase, 23, Exec)

/*  FUNCTION
	This function will reactivate the task dispatcher after a call
	to Forbid(). Note that calls to Forbid() nest, and for every
	call to Forbid() you need a matching call to Permit().

    INPUTS
	None.

    RESULT
	Multitasking will be re-enabled.

    NOTES
	This function preserves all registers.

	To prevent deadlocks calling Wait() in forbidden state breaks
	the forbid - thus taskswitches may happen again.

    EXAMPLE
	No you really don't want to use this function.

    BUGS
	The only architecture that you can rely on the registers being
	saved is on the Motorola mc68000 family.

    SEE ALSO
	Forbid(), Disable(), Enable(), Wait()

    INTERNALS
	If you want to preserve all the registers, replace this function
	in your $(KERNEL) directory. Otherwise this function is
	satisfactory.

    HISTORY

******************************************************************************/
#else
void _Exec_Permit(struct ExecBase * SysBase)
#endif
{
    AROS_LIBFUNC_INIT

    /*
	Task switches are allowed again, if a switch is pending, we
	should allow it.
    */
    if(    ( --SysBase->TDNestCnt < 0 )
	&& ( SysBase->IDNestCnt < 0 )
	&& ( SysBase->AttnResched & 0x80 ) )
    {
	/* Haha, you re-enabled multitasking, only to have the rug
	   pulled out from under you feet :)

	    Clear the Switch() pending flag.
	*/
	SysBase->AttnResched &= ~0x80;
	Switch();
    }

    AROS_LIBFUNC_EXIT
} /* Forbid() */
