/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$

    Desc: Move window behind all other windows.
    Lang: english
*/
#include "intuition_intern.h"

/*****************************************************************************

    NAME */
#include <intuition/intuition.h>
#include <proto/intuition.h>

	AROS_LH1(void, WindowToBack,

/*  SYNOPSIS */
	AROS_LHA(struct Window *, window, A0),

/*  LOCATION */
	struct IntuitionBase *, IntuitionBase, 51, Intuition)

/*  FUNCTION
	Bring a window to the back (ie. behind any other window).

    INPUTS
	window - Which window

    RESULT
	None.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    intuition_lib.fd and clib/intuition_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)

    struct DeferedActionMessage * msg;
    
    msg = AllocMem(sizeof(struct DeferedActionMessage), MEMF_CLEAR);
 
    if (NULL != msg)
    {
      msg->Code        = AMCODE_WINDOWTOBACK;
      msg->Window      = window;
      
      SendDeferedActionMsg(msg, IntuitionBase); 
    }   

    AROS_LIBFUNC_EXIT
} /* WindowToBack */
