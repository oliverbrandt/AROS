/*
    (C) 1995-98 AROS - The Amiga Research OS
    $Id$

    Desc: Graphics function AreaEnd()
    Lang: english
*/
#include <exec/types.h>
#include <graphics/rastport.h>
#include "graphics_intern.h"

/*****************************************************************************

    NAME */
#include <proto/graphics.h>

	AROS_LH1(LONG, AreaEnd,

/*  SYNOPSIS */
	AROS_LHA(struct RastPort *, rp, A1),

/*  LOCATION */
	struct GfxBase *, GfxBase, 44, Graphics)

/*  FUNCTION
        Process the filled vector buffer. 
        After the operation the buffer is reinitilized for
        processing of further Area functions.
        Makes use of the raster given by the TmpRas structure that
        is linked to the rastport.

    INPUTS
	rp - pointer to a valid RastPort structure with a pointer to
	     the previously initilized AreaInfo structure.


    RESULT
	error -  0 for success
	        -1 a error occurred

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	InitArea() AreaDraw() AreaEllipse() AreaCircle() graphics/rastport.h

    INTERNALS

    HISTORY

*****************************************************************************/
{
  AROS_LIBFUNC_INIT
  AROS_LIBBASE_EXT_DECL(struct GfxBase *,GfxBase)

  struct AreaInfo * areainfo = rp->AreaInfo;

  /* is there anything in the matrix at all ? */
  if (0 != (areainfo -> Count))
  {

    UWORD * CurVctr  = areainfo -> VctrTbl;
    BYTE  * CurFlag  = areainfo -> FlagTbl;
    UWORD Count;
    UWORD Rem_APen   = GetAPen(rp);
    /* I don't know whether this function may corrupt the 
       cursor position of the rastport. So I save it for later.*/

    UWORD Rem_cp_x   = rp->cp_x;
    UWORD Rem_cp_y   = rp->cp_y;

    if (0 != (rp->Flags & AREAOUTLINE))
       SetAPen(rp, GetOutlinePen(rp));

    /* if the last polygon is not closed, then I should do so */
    if ( areainfo->FlagPtr[-1] != 0x02 &&
        (areainfo->VctrPtr[-1] != areainfo->FirstY ||
         areainfo->VctrPtr[-2] != areainfo->FirstX   ) )
       if (-1 == AreaDraw(rp, areainfo->FirstX, areainfo->FirstY))
            return -1;

    Count = areainfo->Count;  

    /* process the list of vectors */
    while (Count > 0)
    {
      switch((LONG)(char)CurFlag[0])
      {
        case 0x00:
          /* fill the previous shape if there was one to fill */
          /* !!! missing !!!! */

	  /* set the graphical cursor to a starting position */
            Move(rp, CurVctr[0], CurVctr[1]);
            CurVctr = &CurVctr[2];
            CurFlag = &CurFlag[1];
          break;

        case 0x02:
          /* Nothing to do here */
            CurVctr = &CurVctr[2];
            CurFlag = &CurFlag[1];
          break;

        case 0x03:
          /* Draw a line to new position */
            Draw(rp, CurVctr[0], CurVctr[1]);
          break;

        case 0x83:
          /* Draw an Ellipse and fill it */
          /* see how the data are stored by the second entry */
          if (0x00 == CurFlag[1])
	  {
            /* I get cx,cy,cx+a,cy+b*/
            DrawEllipse(rp,CurVctr[0], 
                           CurVctr[1],
                           CurVctr[2]-CurVctr[0],
                           CurVctr[3]-CurVctr[1]);
	  }
          else if (0x02 == CurFlag[1])
	       {
                 /* I get -a+cx,-b+cy,cx+a,cy+b */
                 DrawEllipse(rp,(CurVctr[0]+CurVctr[2])>>1,
                                (CurVctr[1]+CurVctr[3])>>1,
                                (CurVctr[2]-CurVctr[0])>>1,
                                (CurVctr[3]-CurVctr[1])>>1 );
	       }

            /* area-fill the ellipse with the pattern given
               in rp->AreaPtrn , AreaPtSz */
            /* !!!! missing !!! */
 
            CurVctr = &CurVctr[4];
            CurFlag = &CurFlag[2];
            Count--;
          
          break;

	default:
            /* this is an error */
            SetAPen(rp, Rem_APen);
            /* also restore old graphics cursor position */
            rp->cp_x = Rem_cp_x;
            rp->cp_y = Rem_cp_y;
          return -1;
      }
      Count--;
      
    }

    /* restore old APen */
    SetAPen(rp, Rem_APen);    
    /* also restore old graphics cursor position */
    rp->cp_x = Rem_cp_x;
    rp->cp_y = Rem_cp_y;
  } /* if vectorlist is not empty */

  return 0;

  AROS_LIBFUNC_EXIT
} /* AreaEnd */
