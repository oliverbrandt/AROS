/*
    (C) 1995-97 AROS - The Amiga Research OS
    $Id$

    Desc: Graphics function AndRectRegion()
    Lang: english
*/
#include "graphics_intern.h"
#include <graphics/regions.h>
#include <proto/exec.h>
#include <clib/macros.h>
#include "intregions.h"

/*****************************************************************************

    NAME */
#include <proto/graphics.h>

	AROS_LH2(struct Region *, ClearRectRegionND,

/*  SYNOPSIS */
	AROS_LHA(struct Region    *, Reg, A0),
	AROS_LHA(struct Rectangle *, Rect, A1),

/*  LOCATION */
	struct GfxBase *, GfxBase, 124, Graphics)

/*  FUNCTION
	Clear the given Rectangle from the given Region

    INPUTS
	region - pointer to Region structure
	rectangle - pointer to Rectangle structure

    RESULT
        The resulting region or NULL in case there's no enough free memory

    NOTES

    BUGS

    SEE ALSO
	AndRegionRegion() OrRectRegion() XorRectRegion() ClearRectRegion()
	NewRegion()

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    graphics_lib.fd and clib/graphics_protos.h
	16-01-97    mreckt  initial version

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct Region *Res = NewRegion();

    if (Res)
    {

        struct RegionRectangle rr;

        rr.bounds = *Rect;
        rr.Next   = NULL;
        rr.Prev   = NULL;

        if
        (
            _DoOperationBandBand
            (
                _ClearBandBand,
                0,
                MinX(Reg),
                0,
	        MinY(Reg),
                &rr,
                Reg->RegionRectangle,
                &Res->RegionRectangle,
                &Res->bounds,
                GfxBase
            )
        )
        {

            _TranslateRegionRectangles(Res->RegionRectangle, -MinX(Res), -MinY(Res));

            return Res;
        }

        DisposeRegion(Res);
    }

    return NULL;

    AROS_LIBFUNC_EXIT
} /* AndRectRegion */




