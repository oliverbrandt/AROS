/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang:
*/
#include <exec/execbase.h>
#include <proto/exec.h>
#include <signal.h>

void _os_enable (void);

/******************************************************************************

    NAME
	AROS_LH0(void, Enable,

    LOCATION
	struct ExecBase *, SysBase, 21, Exec)

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

void _Enable (struct ExecBase * SysBase)
{
    if (--SysBase->IDNestCnt < 0)
    {
	_os_enable ();

	if ((SysBase->AttnResched & 0x80)
	    || SysBase->TDNestCnt >= 0
	)
	{
	    SysBase->AttnResched &= ~0x80;

	    Switch ();
	}
    }
} /* _Enable */

void _os_enable (void)
{
    sigset_t set;

    sigfillset (&set);

    sigprocmask (SIG_UNBLOCK, &set, NULL);
} /* _os_enable */
