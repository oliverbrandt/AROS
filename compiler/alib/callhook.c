/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: amiga.lib function CallHook()
    Lang: english
*/
#include <aros/system.h>
#include <stdarg.h>
#include "alib_intern.h"

/******************************************************************************

    NAME */
#include <proto/alib.h>

	IPTR CallHookA (

/*  SYNOPSIS */
	struct Hook * hook,
	APTR	      object,
	APTR	      param)

/*  FUNCTION
	Calls a hook with the specified object and parameters.

    INPUTS
	hook - Call this hook.
	object - This is the object which is passed to the hook. The valid
	    values for this parameter depends on the definition of the called
	    hook.
	param - Pass these parameters to the specified object

    RESULT
	The return value depends on the definition of the hook.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	CallHook()

    HISTORY
	22.11.96 digulla documented

******************************************************************************/
{
    return CallHookPkt (hook, object, param);
} /* CallHookA */

IPTR CallHook (struct Hook * hook, APTR object, ...)
{
    IPTR    retval;
    va_list args;

    va_start (args, object);

    retval = CallHookPkt (hook, object, args);

    va_end (args);

    return retval;
} /* CallHook */

