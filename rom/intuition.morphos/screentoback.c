/*
    (C) 1995-99 AROS - The Amiga Research OS
    $Id$
 
    Desc: Move a screen behind all other screens.
    Lang: english
*/
#include "intuition_intern.h"

/*****************************************************************************
 
    NAME */
#include <proto/intuition.h>

AROS_LH1(void, ScreenToBack,

         /*  SYNOPSIS */
         AROS_LHA(struct Screen *, screen, A0),

         /*  LOCATION */
         struct IntuitionBase *, IntuitionBase, 41, Intuition)

/*  FUNCTION
	Move a screen behind all other screens. If the screen is in a
	group, the screen will be moved behind all other screens in the
	group only. If the screen is the parent of a group, the whole
	group will be moved in the back.
 
    INPUTS
	screen - Move this screen.
 
    RESULT
	You will see the screen move behind all other screens. If some
	screen before this screen occupies the whole display, then it
	will disappear completely. If all other screens occupy only part
	of the display, the screen will appear behind the screens.
 
    NOTES
 
    EXAMPLE
 
    BUGS
 
    SEE ALSO
	ScreenToFront(), ScreenDepth()
 
    INTERNALS
 
    HISTORY
 
*****************************************************************************/
{
	AROS_LIBFUNC_INIT
	AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)

	ScreenDepth ( screen, SDEPTH_TOBACK, NULL );

	AROS_LIBFUNC_EXIT
} /* ScreenToBack */
