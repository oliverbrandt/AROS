/*
    (C) 1996-97 AROS - The Amiga Research OS
    $Id$

    Desc: AROS frameiclass implementation
    Lang: english

    Original version 10/24/96 by caldi@usa.nai.net
*/

#include <exec/types.h>

#include <dos/dos.h>
#include <dos/dosextens.h>

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/imageclass.h>
#include <intuition/cghooks.h>
#include <intuition/icclass.h>

#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>

#include <utility/tagitem.h>
#include <utility/hooks.h>

#include <clib/macros.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>

#ifdef _AROS
#include <proto/boopsi.h>
#include <aros/asmcall.h>
#include <proto/alib.h>
#include "intuition_intern.h"
#endif

/****************************************************************************/

/* Some handy transparent base class object casting defines.
 */
#define G(o)  ((struct Gadget *)o)
#define EG(o) ((struct ExtGadget *)o)
#define IM(o) ((struct Image *)o)

/****************************************************************************/

/* FrameIClass specific instance data.
 */
struct FrameIData
{
    /* render bevel only with no fill? */
    BOOL fid_EdgesOnly;

    /* inverted bevel pens? */
    BOOL fid_Recessed;

    /* frame style? */
    WORD fid_FrameType;
};

/****************************************************************************/

#undef IntuitionBase
#define IntuitionBase	((struct IntuitionBase *)(cl->cl_UserData))

/* This is utility function used by frameiclass to draw a simple
 * bevel.
 */
static void DrawFrame(
	Class * cl,
	struct RastPort *rport,
	UWORD shine, UWORD shadow,
	WORD left, WORD top, WORD width, WORD height,
	BOOL thicken)
{
    /*
	Here we attempt to render a bevel as quickly as possible using
	as few system calls as possible. Note the ORDER of the rendering
	and pen (or lack of) setting in important. This routine can be
	useful alone as a basis for GadTools DrawBevelBox() perhaps, but
	its real function is rendering the frameiclass components for the
	various supported styles.

	It should be futher noted, on the native Amiga chipset, rendering
	bevels via serious of RectFill()'s is potentially as much as two
	times faster. However, in the case of AROS the implementation
	of the graphics drivers would ofcourse be the determining factor.
	Just as on the native OS, 3rd party gfx boards would be a factor.

	Additionally, if the frame metrics are changed here for whatever
	reasons, you MUST also compensate the change in the frame class
	render method, and framebox specifically the offset values.
    */
    height -= 1;
    width  -= 1;

    /* Top/Left */
    SetABPenDrMd(rport, shine, 0, JAM1);
    Move(rport, left, top + height);
    Draw(rport, left, top);
    Draw(rport, left + width, top);

    /* Bottom/Right */
    SetAPen(rport, shadow);
    Draw(rport, left + width, top + height);
    Draw(rport, left, top + height);

    if (thicken != FALSE)
    {
	/* Thicken Right Side */
	Move(rport, left + width - 1, top + height - 1);
	Draw(rport, left + width - 1, top + 1);

	/* Thicken Left Side */
	SetAPen(rport, shine);
	Move(rport, left + 1, top + height - 1);
	Draw(rport, left + 1, top + 1);
    } /* if */
} /* DrawFrame */

/****************************************************************************/

/* frame render method */
static ULONG draw_frameiclass(Class *cl, Object *o, struct impDraw *msg)
{
    struct FrameIData *fid = INST_DATA(cl, o);

    /*
	Default pen array, this should be globally accessable from
	all the boopsi objects, unless someone as a better idea...
    */
    UWORD default_pens[] =
    {
	1, /* detailPen      */
	0, /* blockPen	     */
	1, /* textPen	     */
	2, /* shinePen	     */
	1, /* shadowPen      */
	3, /* hiFillPen      */
	1, /* hifilltextPen  */
	0, /* backgroundPen  */
	2  /* hilighttextPen */
    };
    ULONG retval;

    /* we will check the rastport present */
    if(msg->imp_RPort != NULL)
    {
	UWORD *pens = default_pens;
	UWORD loffset = 0, toffset = 0;
	UWORD left, top;
	UWORD shine, shadow;
	BOOL selected;

	/* set up our rendering pens */
	if (msg->imp_DrInfo)
	{
	    pens = msg->imp_DrInfo->dri_Pens;

	    /*
		Fall back to mono color bevels if depth is only 1 bitplane.
	    */
	    if (msg->imp_DrInfo->dri_Depth == 1)
	    {
		shine  = pens[SHADOWPEN];
	    }
	    else
	    {
		shine  = pens[SHINEPEN];
	    } /* if */
	}
	else
	{
	    shine  = pens[SHINEPEN];
	} /* if */

	shadow	= pens[SHADOWPEN];

	switch(msg->imp_State)
	{
	case IDS_SELECTED:
	case IDS_INACTIVESELECTED:
	    selected = TRUE;
	    break;

	default:
	    selected = FALSE;
	    break;
	} /* switch */

	/*
	    I'm so clever :) We want to check if either of selected or
	    recessed is TRUE, and if so, swap the pens. However, if both
	    are true, they effectivly cancel each other out and we do
	    nothing. Rather than two compares against TRUE and a OR of the
	    results, pls the additional check to ignore the case where both
	    are TRUE, we will do an XOR of the bool's and check the result.
	    This should prove most efficient too.


	    Recess|select| XOR'd result
	    ------|------|-------
	      0   |  0	 |  0
	    ------|------|-------
	      0   |  1	 |  1
	    ------|------|-------
	      1   |  0	 |  1
	    ------|------|-------
	      1   |  1	 |  0
	    ------|------|-------
	*/

	if ( (fid->fid_Recessed ^ selected) != FALSE )
	{
		/* swap pens */
		shine  ^= shadow;
		shadow ^= shine;
		shine  ^= shadow;
	} /* if */

	left = IM(o)->LeftEdge + msg->imp_Offset.X;
	top  = IM(o)->TopEdge  + msg->imp_Offset.Y;

	switch(fid->fid_FrameType)
	{
	case FRAME_DEFAULT:
	    toffset = 1;
	    loffset = 1;
	    DrawFrame(
		cl,
		msg->imp_RPort,
		shine, shadow,
		left, top,
		IM(o)->Width, IM(o)->Height,
		FALSE
	    );
	    break;

	case FRAME_BUTTON:
	    toffset = 1;
	    loffset = 2;
	    DrawFrame(
		cl,
		msg->imp_RPort,
		shine, shadow,
		left, top,
		IM(o)->Width, IM(o)->Height,
		TRUE
	    );
	    break;

	case FRAME_RIDGE:
	    toffset = 1;
	    loffset = 2;

	    /* render outer pen-inverted thick bevel */
	    DrawFrame(
		cl,
		msg->imp_RPort,
		shine, shadow,
		left, top,
		IM(o)->Width, IM(o)->Height,
		TRUE
	    );

	    /* render inner thick bevel */
	    DrawFrame(
		cl,
		msg->imp_RPort,
		shadow, shine,
		left + loffset, top + toffset,
		IM(o)->Width - (loffset + loffset), IM(o)->Height - (toffset + toffset),
		TRUE
	    );

	    toffset += 1;
	    loffset += 2;
	    break;

	case FRAME_ICONDROPBOX:
	    toffset = 2;
	    loffset = 4;

	    /* render outer pen-inverted thick bevel */
	    DrawFrame(
		cl,
		msg->imp_RPort,
		shine, shadow,
		left, top,
		IM(o)->Width, IM(o)->Height,
		TRUE
	    );

	    /* render inner thick bevel */
	    DrawFrame(
		cl,
		msg->imp_RPort,
		shadow, shine,
		left + loffset, top + toffset,
		IM(o)->Width - (loffset + loffset), IM(o)->Height - (toffset + toffset),
		TRUE
	    );

	    toffset += 2;
	    loffset += 4;
	    break;
	} /* switch */

	if(fid->fid_EdgesOnly == FALSE)
	{
	    if(selected)
	    {
		SetABPenDrMd(msg->imp_RPort, pens[FILLPEN], pens[BACKGROUNDPEN], JAM1);
	    }
	    else
	    {
		SetABPenDrMd(msg->imp_RPort, pens[BACKGROUNDPEN], pens[BACKGROUNDPEN], JAM1);
	    } /* if */
	    RectFill(msg->imp_RPort,
		left + loffset,
		top  + toffset,
		left + IM(o)->Width  - loffset - 1,
		top  + IM(o)->Height - toffset - 1);
	} /* if */

	retval = 1UL;
    }
    else
    {
	/* return failure */
	retval = 0UL;
    } /* if */

    return retval;
} /* draw_frameiclass */


/****************************************************************************/

/* frame attribute setting method */
static ULONG set_frameiclass(Class *cl, Object *o, struct opSet *msg)
{
    struct FrameIData *fid = INST_DATA(cl, o);

    struct TagItem *tstate = msg->ops_AttrList;
    struct TagItem *tag;
    ULONG retval = 0UL;

    while ((tag = NextTagItem(&tstate)))
    {
	switch(tag->ti_Tag)
	{
	case IA_Recessed:
	    fid->fid_Recessed	= (BOOL)( ( (BOOL)tag->ti_Data != FALSE ) ? TRUE : FALSE );
	    break;

	case IA_EdgesOnly:
	    fid->fid_EdgesOnly	= (BOOL)( ( (BOOL)tag->ti_Data != FALSE ) ? TRUE : FALSE );
	    break;

	case IA_FrameType:
	    /*
		Data values for IA_FrameType (recognized by FrameIClass)

		FRAME_DEFAULT:	The standard V37-type frame, which has
			thin edges.
		FRAME_BUTTON:  Standard button gadget frames, having thicker
			sides and edged corners.
		FRAME_RIDGE:  A ridge such as used by standard string gadgets.
			You can recess the ridge to get a groove image.
		FRAME_ICONDROPBOX: A broad ridge which is the standard imagery
			for areas in AppWindows where icons may be dropped.
	    */
	    fid->fid_FrameType = (WORD)tag->ti_Data;
	    break;

	} /* switch */
    } /* while */

    return(retval);
} /* set_frameiclass */

/****************************************************************************/

/* frameiclass framebox method */
static ULONG framebox_frameiclass(Class *cl, Object *o, struct impFrameBox *msg)
{
    struct FrameIData *fid = INST_DATA(cl, o);

    if (msg->imp_FrameFlags & FRAMEF_SPECIFY)
    {
	/*
	    Would someone kindly explain RKM Libs page 898/899 in english :)
	    I'll just forget about this for the time being until I have
	    made more sence out of what is the "right thing" to do here.
	*/
    }
    else
    {
	switch(fid->fid_FrameType)
	{
	case FRAME_DEFAULT:
	    msg->imp_ContentsBox->Top	 = msg->imp_FrameBox->Top    - 1;
	    msg->imp_ContentsBox->Left	 = msg->imp_FrameBox->Left   - 1;
	    msg->imp_ContentsBox->Height = msg->imp_FrameBox->Height + 2;
	    msg->imp_ContentsBox->Width  = msg->imp_FrameBox->Width  + 2;
	    break;

	case FRAME_BUTTON:
	    msg->imp_ContentsBox->Top	 = msg->imp_FrameBox->Top    - 1;
	    msg->imp_ContentsBox->Left	 = msg->imp_FrameBox->Left   - 2;
	    msg->imp_ContentsBox->Height = msg->imp_FrameBox->Height + 2;
	    msg->imp_ContentsBox->Width  = msg->imp_FrameBox->Width  + 4;
	    break;

	case FRAME_RIDGE:
	    msg->imp_ContentsBox->Top	 = msg->imp_FrameBox->Top    - 2;
	    msg->imp_ContentsBox->Left	 = msg->imp_FrameBox->Left   - 4;
	    msg->imp_ContentsBox->Height = msg->imp_FrameBox->Height + 4;
	    msg->imp_ContentsBox->Width  = msg->imp_FrameBox->Width  + 8;
	    break;

	case FRAME_ICONDROPBOX:
	    msg->imp_ContentsBox->Top	 = msg->imp_FrameBox->Top    - 6;
	    msg->imp_ContentsBox->Left	 = msg->imp_FrameBox->Left   - 6;
	    msg->imp_ContentsBox->Height = msg->imp_FrameBox->Height + 12;
	    msg->imp_ContentsBox->Width  = msg->imp_FrameBox->Width  + 12;
	    break;
	}
    } /* if */

    return 1UL;
} /* framebox_frameiclass */

/****************************************************************************/

/* frameiclass boopsi dispatcher
 */
AROS_UFH3S(IPTR, dispatch_frameiclass,
    AROS_UFHA(Class *,  cl,  A0),
    AROS_UFHA(Object *, o,   A2),
    AROS_UFHA(Msg,      msg, A1)
)
{
    IPTR retval = 0UL;

    switch(msg->MethodID)
    {
    case IM_FRAMEBOX:
	retval = framebox_frameiclass(cl, o, (struct impFrameBox *)msg);
	break;

    case IM_DRAWFRAME:
	/* For this release, drawframe (wrongly) handled the same as... */

    case IM_DRAW:
	retval = draw_frameiclass(cl, o, (struct impDraw *)msg);
	break;

    case OM_SET:
	retval = DoSuperMethodA(cl, o, msg);
	retval += (IPTR)set_frameiclass(cl, o, (struct opSet *)msg);
	break;

    case OM_NEW:
	retval = DoSuperMethodA(cl, o, msg);
	if (retval)
	{
	    struct FrameIData *fid = INST_DATA(cl, retval);

	    /* set some defaults */
	    fid->fid_EdgesOnly = FALSE;
	    fid->fid_Recessed  = FALSE;
	    fid->fid_FrameType = FRAME_DEFAULT;

	    /* Handle our special tags - overrides defaults */
	    set_frameiclass(cl, (Object*)retval, (struct opSet *)msg);
	}
	break;

    default:
	retval = DoSuperMethodA(cl, o, msg);
	break;
    } /* switch */

    return retval;
}  /* dispatch_frameiclass */

#undef IntuitionBase

/****************************************************************************/

/* Initialize our image class. */
struct IClass *InitFrameIClass (struct IntuitionBase * IntuitionBase)
{
    struct IClass *cl = NULL;

    /* This is the code to make the frameiclass...
	*/
    if ((cl = MakeClass(FRAMEICLASS, IMAGECLASS, NULL, sizeof(struct FrameIData), 0)))
    {
	cl->cl_Dispatcher.h_Entry    = (APTR)AROS_ASMSYMNAME(dispatch_frameiclass);
	cl->cl_Dispatcher.h_SubEntry = NULL;
	cl->cl_UserData 	     = (IPTR)IntuitionBase;

	AddClass (cl);
    }

    return (cl);
}

