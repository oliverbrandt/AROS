/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.2  1996/08/29 13:33:30  digulla
    Moved common code from driver to Intuition
    More docs

    Revision 1.1  1996/08/13 15:37:26  digulla
    First function for intuition.library


    Desc:
    Lang: english
*/
#include "intuition_intern.h"
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

extern void intui_CloseWindow (struct Window *, struct IntuitionBase *);
extern int  intui_GetWindowSize (void);

/*****************************************************************************

    NAME */
	#include <clib/intuition_protos.h>

	__AROS_LH1(void, CloseWindow,

/*  SYNOPSIS */
	__AROS_LHA(struct Window *, window, A0),

/*  LOCATION */
	struct IntuitionBase *, IntuitionBase, 12, Intuition)

/*  FUNCTION
	Closes a window. Depending on the display, this might not happen
	at the time when this function returns, but you must not use
	the window pointer after this function has been called.

    INPUTS
	window - The window to close

    RESULT
	None.

    NOTES
	The window might not have been disappeared when this function returns.

    EXAMPLE

    BUGS

    SEE ALSO
	OpenWindow(), OpenWindowTags()

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    intuition_lib.fd and clib/intuition_protos.h

*****************************************************************************/
{
    __AROS_FUNC_INIT
    __AROS_BASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)
    struct IntuiMessage * im;

    IntuitionBase->ActiveWindow = NULL;

    /* Remove window from the chain and find next active window */
    if (window->Descendant)
    {
	window->Descendant->Parent = window->Parent;
	IntuitionBase->ActiveWindow = window->Descendant;
    }
    if (window->Parent)
    {
	window->Parent->NextWindow =
	    window->Parent->Descendant =
	    window->Descendant;

	if (!IntuitionBase->ActiveWindow)
	    IntuitionBase->ActiveWindow = window->Parent;
    }

    /* Make sure the Screen is still valid */
    if (window == window->WScreen->FirstWindow)
	window->WScreen->FirstWindow = window->NextWindow;

    /* Let the driver clean up */
    intui_CloseWindow (window, IntuitionBase);

    /* Free resources */
    CloseFont (window->RPort->Font);

    FreeMem (window->RPort, sizeof (struct RastPort));

    if (window->UserPort)
    {
	/* Delete all pending messages */
	Forbid ();

	while ((im = (struct IntuiMessage *) GetMsg (window->UserPort)))
	    ReplyMsg ((struct Message *)im);

	Permit ();

	/* Delete message port */
	DeleteMsgPort (window->UserPort);
    }

    /* Free memory for the window */
    FreeMem (window, intui_GetWindowSize ());

    __AROS_FUNC_EXIT
} /* CloseWindow */
