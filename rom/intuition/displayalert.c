/*
    (C) 1995-99 AROS - The Amiga Research OS
    $Id$

    Desc: Intuition function DisplayAlert()
    Lang: english
*/
#include "intuition_intern.h"

/*****************************************************************************

    NAME */
#include <proto/intuition.h>

	AROS_LH3(BOOL, DisplayAlert,

/*  SYNOPSIS */
	AROS_LHA(ULONG , alertnumber, D0),
	AROS_LHA(UBYTE*, string, A0),
	AROS_LHA(UWORD , height, D1),

/*  LOCATION */
	struct IntuitionBase *, IntuitionBase, 15, Intuition)

/*  FUNCTION
	Bring up an alert with the given message.
	A system recoverable alert is a RECOVERY_ALERT which waits until
	the user presses a mouse button. The display is then restored and
	a boolean value will be returned showing if the has pressed the
	left mouse button.
	A DEADEND_ALERT is an alert from which the system cannot recover.
	This alert immediately returns with FALSE after creating the
	alert display.
	If the system can not get enough memory for a RECOVERY_ALERT,
	this function returns FALSE.

    INPUTS
	alertnumber - The
	string - The
	height - The

    RESULT
	Always FALSE if DEADEND_ALERT. RECOVERY_ALERT returns TRUE if
	the user pressed the left mouse button and FALSE for other
	mouse button or if the alert could not be posted.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)

#warning TODO: Write intuition/DisplayAlert()
    aros_print_not_implemented ("DisplayAlert");

    return FALSE;

    AROS_LIBFUNC_EXIT
} /* DisplayAlert */
