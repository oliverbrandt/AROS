/*
    (C) 2000 AROS - The Amiga Research OS
    $Id$

    Desc: Graphics function ScrollVPort()
    Lang: english
*/
#include <graphics/view.h>

/*****************************************************************************

    NAME */
#include <proto/graphics.h>

        AROS_LH1(void, ScrollVPort,

/*  SYNOPSIS */
        AROS_LHA(struct ViewPort *, vp, A0),

/*  LOCATION */
        struct GfxBase *, GfxBase, 98, Graphics)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY


******************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct GfxBase *,GfxBase)

#warning TODO: Write graphics/ScrollVPort()
    aros_print_not_implemented ("ScrollVPort");

    AROS_LIBFUNC_EXIT
} /* ScrollVPort */
