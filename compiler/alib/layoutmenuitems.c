/*
    Copyright (C) 1997-2000 AROS - The Amiga Research OS
    $Id$

    Desc: Layout a gadtools menu item
    Lang: english
*/
#include <exec/types.h>
#define AROS_TAGRETURNTYPE BOOL

#include "alib_intern.h"

extern struct Library * GadToolsBase;

/*****************************************************************************

    NAME */
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include <libraries/gadtools.h>
#define NO_INLINE_STDARG /* turn off inline def */
#include <proto/gadtools.h>

	BOOL LayoutMenuItems (

/*  SYNOPSIS */
	struct MenuItem * menuitem,
	APTR              vi,
	ULONG		  tag1,
	...		  )

/*  FUNCTION
        Varargs version of gadtools.library/LayoutMenuItemsA().

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_SLOWSTACKTAGS_PRE(tag1)
    retval = LayoutMenuItemsA (menuitem, vi, AROS_SLOWSTACKTAGS_ARG(tag1));
    AROS_SLOWSTACKTAGS_POST
} /* LayoutMenuItems */
