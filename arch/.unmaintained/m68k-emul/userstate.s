/*
    (C) 1995-2000 AROS - The Amiga Research OS
    $Id$

    Desc: Exec function UserState
    Lang: english
*/

/******************************************************************************

    NAME
	AROS_LH0(void, UserState,

    LOCATION
	struct ExecBase *, SysBase, 26, Exec)

    FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

******************************************************************************/

	#include "machine.i"

	.text
	.balign 16
	.globl	AROS_SLIB_ENTRY(UserState,Exec)
	.type	AROS_SLIB_ENTRY(UserState,Exec),@function
AROS_SLIB_ENTRY(UserState,Exec):
	/* Dummy */
	rts

