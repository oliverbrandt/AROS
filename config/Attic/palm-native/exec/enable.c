/*
    Copyright (C) 1995-1997 AROS - The Amiga Research OS
    $Id$

    Desc: Enable() - Allow interrupts to occur after Disable().
    Lang: english
*/
#include <exec/types.h>
#include <exec/execbase.h>
#include <aros/libcall.h>
#include <registers.h>

/*****************************************************************************/
#ifndef UseExecstubs

/*  NAME */
#include <proto/exec.h>

	AROS_LH0(void, Enable,

/*  LOCATION */
	struct ExecBase *, SysBase, 21, Exec)

/*  FUNCTION
	This function will allow interrupts to occur after they have
	been disabled by Disable().

	Note that calls to Disable() nest, and for every call to
	Disable() you need a matching call to Enable().

	***** WARNING *****

	Using this function is considered very harmful, and it is
	not recommended to use this function for ** ANY ** reason.

	It is quite possible to either crash the system, or to prevent
	normal activities (disk/port i/o) from occuring.

	Note: As taskswitching is driven by the interrupts subsystem,
	      this function has the side effect of disabling
	      multitasking.

    INPUTS
	None.

    RESULT
	Interrupts will be enabled again when this call returns.

    NOTES
	This function preserves all registers.

	To prevent deadlocks calling Wait() in disabled state breaks
	the disable - thus interrupts may happen again.

    EXAMPLE
	No you DEFINITATELY don't want to use this function.

    BUGS
	The only architecture that you can rely on the registers being
	saved is on the Motorola mc68000 family.

    SEE ALSO
	Forbid(), Permit(), Disable(), Wait()

    INTERNALS
	This function must be replaced in the $(KERNEL) or $(ARCH)
	directories in order to do some work.

    HISTORY

******************************************************************************/
#else
void _Exec_Enable(struct ExecBase * SysBase)
#endif
{
    AROS_LIBFUNC_INIT

    /* Only disable interrupts if they are not already disabled. The
       initial (enabled) value of IDNestCnt is -1
    */
    if( --SysBase->IDNestCnt < 0)
    {
	WREG_L(IMR) = ((ULONG)RREG_L(IMR) & (~(ULONG)0x400010));
	
	if ((BYTE)(SysBase->AttnResched & 0x80)) {
		SysBase->AttnResched &= 0xff7f;
		Switch();
	}
    }

    AROS_LIBFUNC_EXIT
} /* Enable() */
