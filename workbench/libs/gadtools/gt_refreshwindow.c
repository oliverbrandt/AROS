/*
    (C) 1997 - 2000 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include <proto/intuition.h>
#include "gadtools_intern.h"

/*********************************************************************

    NAME */
#include <proto/gadtools.h>
#include <intuition/intuition.h>

        AROS_LH2(void, GT_RefreshWindow,

/*  SYNOPSIS */
	AROS_LHA(struct Window *, win, A0),
	AROS_LHA(struct Requester *,req,A1),

/*  LOCATION */
	struct Library *, GadToolsBase, 14, GadTools)

/*  FUNCTION
	You have to call this function as soon as a window containing
	gadtools gadgets was opened or after you have performed a
	RefreshGList().

    INPUTS
        win - Window to refresh.
	req - Not used. Provide NULL for now.

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

***************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct GadToolsBase *,GadToolsBase)

    /* Don't do anything here, see autodoc. No GT_BeginRefresh/GT_EndRefresh
       either!!
       
       This func is just there because in AmigaOS the gadtools gadgets
       aren't boopsi gadgets and half the stuff there is handled by
       app task and not Intuition.
       
    */
       
    AROS_LIBFUNC_EXIT
    
} /* GT_RefreshWindow */
