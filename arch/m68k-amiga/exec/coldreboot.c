/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: ColdReboot() - Reboot the computer.
    Lang: english
*/

#include <aros/debug.h>

#include "exec_intern.h"
#include "exec_util.h"

/*****************************************************************************

    NAME */
#include <proto/exec.h>

	AROS_LH0(void, ColdReboot,

/*  LOCATION */
	struct ExecBase *, SysBase, 121, Exec)

/*  FUNCTION
	This function will reboot the computer.

    INPUTS
	None.

    RESULT
	This function does not return.

    NOTES
	It can be quite harmful to call this function. It may be possible that
	you will lose data from other tasks not having saved, or disk buffers
	not being flushed. Plus you could annoy the (other) users.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS
	This function is not really necessary, and could be left unimplemented
	on many systems. It is best when using this function to allow the memory
	contents to remain as they are, since some programs may use this
	function when installing resident modules.

******************************************************************************/
{
    AROS_LIBFUNC_INIT

    /* Disable interrupts, and do all the reset callbacks
     */
    Exec_DoResetCallbacks((struct IntExecBase *)SysBase);

    /* Reset everything but the CPU, then restart
     * at the ROM exception vector
     */
    asm("reset\n"
        "move.l #4,%a0\n"
        "jmp    (%a0)\n");

    AROS_LIBFUNC_EXIT
} /* ColdReboot() */
