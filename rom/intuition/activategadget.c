/*
    (C) 1995-99 AROS - The Amiga Research OS
    $Id$

    Desc: Intuition function ActivateGadget()
    Lang: english
*/
#include "intuition_intern.h"
#include "inputhandler.h"

/*****************************************************************************

    NAME */
#include <proto/intuition.h>

	AROS_LH3(BOOL, ActivateGadget,

/*  SYNOPSIS */
	AROS_LHA(struct Gadget *   , gadget, A0),
	AROS_LHA(struct Window *   , window, A1),
	AROS_LHA(struct Requester *, requester, A2),

/*  LOCATION */
	struct IntuitionBase *, IntuitionBase, 77, Intuition)

/*  FUNCTION
	Activates the specified gadget.

    INPUTS
	gadget - The gadget to activate
	window - The window which contains the gadget
	requester - The requester which contains the gadget or
		NULL if it is not a requester gadget

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)
    
    
    struct IIHData	*iih;
    BOOL success = FALSE;
    
    iih = (struct IIHData *)GetPrivIBase(IntuitionBase)->InputHandler->is_Data;
    
    
    
    if (window != IntuitionBase->ActiveWindow)
    	return FALSE;
	
    if (NULL != iih->ActiveGadget)
    	return NULL;
	
    
    /* Only string and custom gadgets may be activated this way */
    switch (gadget->GadgetType & GTYP_GTYPEMASK)
    {
	
	
	
    	case GTYP_STRGADGET: {
	
	    UWORD imsgcode;
	    struct GadgetInfo gi;
	    struct InputEvent ie;
	    
	    /* Fake an inputevent to the gadget */
	    ie.ie_Class	= IECLASS_RAWMOUSE;
	    ie.ie_Code	= SELECTDOWN;
	
	    ie.ie_X	= GetLeft (gadget, window) + window->LeftEdge;
	    ie.ie_Y	= GetTop  (gadget, window) + window->TopEdge;
	    
	    gi.gi_Screen = window->WScreen;
	    gi.gi_Window = window;
	    gi.gi_Domain = *((struct IBox *)&window->LeftEdge);
	    gi.gi_RastPort = window->RPort;
	    gi.gi_Pens.DetailPen = gi.gi_Screen->DetailPen;
	    gi.gi_Pens.BlockPen  = gi.gi_Screen->BlockPen;
	    gi.gi_DrInfo	 = &(((struct IntScreen *)gi.gi_Screen)->DInfo);
	    
	    HandleStrInput(gadget, &gi, &ie, &imsgcode, IntuitionBase);
	    
	    break; }
	    
	case GTYP_CUSTOMGADGET: {
	    struct gpInput msg;
	    
	    LONG termination;
	    IPTR retval;
	    
	    msg.MethodID	= GM_GOACTIVE;
	    msg.gpi_GInfo	= NULL;
	    msg.gpi_IEvent	= NULL;
	    msg.gpi_Termination	= &termination;
	    msg.gpi_Mouse.X	= window->WScreen->MouseX - window->LeftEdge;
	    msg.gpi_Mouse.Y	= window->WScreen->MouseY - window->TopEdge;
	    msg.gpi_TabletData	= NULL;
	    
	
	    retval = DoGadgetMethodA(gadget, window, requester, (Msg)&msg);

	    if (retval != GMR_MEACTIVE)
	    {
		struct gpGoInactive gpgi;

		if (retval & GMR_REUSE)
			return FALSE;

#if 0

/* How do one handle this ? Is IDCMP_GADGETUP sent to the
   application or not ?
*/
		if (retval & GMR_VERIFY)
		{
		    im->Class = IDCMP_GADGETUP;
		    im->IAddress = gadget;
		    ptr	 = "GADGETUP";
		    im->Code = termination & 0x0000FFFF;
		}
		else
		{
		    im->Class = 0; /* Swallow event */
		}
#endif

		gpgi.MethodID = GM_GOINACTIVE;
		gpgi.gpgi_GInfo = NULL;
		gpgi.gpgi_Abort = 0;

		DoGadgetMethodA(gadget, window, requester, (Msg)&gpgi);
			    
		gadget->Activation &= ~GACT_ACTIVEGADGET;

		success = FALSE;
	    }
	    else
	    {
		/* Assure gadget is active */
		gadget->Activation |= GACT_ACTIVEGADGET;
		success = TRUE;
	    }
	    
	    
	    
	    break; }
	
	default:
		success = FALSE;
		break;
		
    }	/* switch () */
    
    
    return success;

    AROS_LIBFUNC_EXIT
} /* ActivateGadget */
