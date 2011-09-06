/*
    Copyright � 1995-2011, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/

#include <aros/debug.h>
#include <graphics/rastport.h>
#include <hidd/graphics.h>

#include "cybergraphics_intern.h"
#include "gfxfuncsupport.h"

/*****************************************************************************

    NAME */
#include <clib/cybergraphics_protos.h>

	AROS_LH11(LONG, WriteLUTPixelArray,

/*  SYNOPSIS */
	AROS_LHA(APTR             , srcRect, A0),
	AROS_LHA(UWORD            , SrcX, D0),
	AROS_LHA(UWORD            , SrcY, D1),
	AROS_LHA(UWORD            , SrcMod, D2),
	AROS_LHA(struct RastPort *, rp, A1),
	AROS_LHA(APTR             , CTable, A2),
	AROS_LHA(UWORD            , DestX, D3),
	AROS_LHA(UWORD            , DestY, D4),
	AROS_LHA(UWORD            , SizeX, D5),
	AROS_LHA(UWORD            , SizeY, D6),
	AROS_LHA(UBYTE            , CTabFormat, D7),

/*  LOCATION */
	struct Library *, CyberGfxBase, 33, Cybergraphics)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    cybergraphics_lib.fd and clib/cybergraphics_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    
    ULONG depth;
    
    HIDDT_PixelLUT pixlut;
    HIDDT_Pixel pixtab[256];
    HIDDT_Color col;
    ULONG i;
    
    /* This is cybergraphx. We only work wih HIDD bitmaps */
    if (!IS_HIDD_BM(rp->BitMap)) {
    	D(bug("!!!!! Trying to use CGFX call on non-hidd bitmap in WriteLUTPixelArray()!!!\n"));
    	return 0;
    }
    
    pixlut.entries	= 256;
    pixlut.pixels	= pixtab;
    
    depth = GetBitMapAttr(rp->BitMap, BMA_DEPTH);
    
    /* This call does only support bitmaps with depth > 8. Use WritePixelArray8
       for other bitmaps
    */
    
    if (depth <= 8) {
    	D(bug("!!! TRYING TO USE WriteLUTPixelArray() ON BITMAP WITH DEPTH < 8\n"));
    	return 0;
    }
	
    /* Curently only one format is supported */
    if (CTABFMT_XRGB8 != CTabFormat) {
    	D(bug("!!! WriteLUTPixelArray() CALLED WITH UNSUPPORTED CTAB FORMAT %d\n"
		, CTabFormat));
    	return 0;
    }

    /* Convert the coltab into native pixels */
    col.alpha = 0;
    for (i = 0; i < 256; i ++)
    {
    	register ULONG rgb = ((ULONG *)CTable)[i];

    	col.red	  = (HIDDT_ColComp)((rgb & 0x00FF0000) >> 8);
	col.green = (HIDDT_ColComp)(rgb & 0x0000FF00);
	col.blue  = (HIDDT_ColComp)((rgb & 0x000000FF) << 8);

	pixtab[i] = HIDD_BM_MapColor(HIDD_BM_OBJ(rp->BitMap), &col);
    }

    /* Now blit the colors onto the screen */
    return WritePixels8(rp, srcRect + CHUNKY8_COORD_TO_BYTEIDX(SrcX, SrcY, SrcMod), SrcMod,
			DestX, DestY, DestX + SizeX - 1, DestY + SizeY - 1, &pixlut, TRUE);

    AROS_LIBFUNC_EXIT
} /* WriteLUTPixelArray */
