/*
    (C) 2001 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: English
*/

/*********************************************************************************************/

#include "global.h"
#include <string.h>

/*********************************************************************************************/

static struct Gadget *gadlist, *countrygad, *gad;
static WORD domleft, domtop, domwidth, domheight;

/*********************************************************************************************/

static LONG country_makegadgets(void)
{
    struct NewGadget ng;
    
    gad = CreateContext(&gadlist);
    
    ng.ng_LeftEdge   = domleft;
    ng.ng_TopEdge    = domtop;
    ng.ng_Width      = domwidth;
    ng.ng_Height     = domheight;
    ng.ng_GadgetText = NULL;
    ng.ng_TextAttr   = 0;
    ng.ng_GadgetID   = MSG_GAD_TAB_COUNTRY;
    ng.ng_Flags      = 0;
    ng.ng_VisualInfo = vi;
    
    gad = countrygad = CreateGadget(LISTVIEW_KIND, gad, &ng, GTLV_Labels, (IPTR)&country_list,
    	    	    	    	    	    	    	     GTLV_ShowSelected, NULL,
    	    	    	    	    	    	    	     TAG_DONE);
    

    return gad ? TRUE : FALSE;
}

/*********************************************************************************************/

static void country_cleanup(void)
{
    if (gadlist) FreeGadgets(gadlist);
    gadlist = NULL;
}

/*********************************************************************************************/

LONG page_country_handler(LONG cmd, IPTR param)
{
    LONG retval = TRUE;
    
    switch(cmd)
    {
    	case PAGECMD_INIT:
	    break;
	    
	case PAGECMD_LAYOUT:
	    break;
	    
	case PAGECMD_GETMINWIDTH:
	    retval = 20;
	    break;
	    
	case PAGECMD_GETMINHEIGHT:
	    retval = 20;
	    break;
	    
	case PAGECMD_SETDOMLEFT:
	    domleft = param;
	    break;
	    
	case PAGECMD_SETDOMTOP:
	    domtop = param;
	    break;
	    
	case PAGECMD_SETDOMWIDTH:
	    domwidth = param;
	    break;
	    
	case PAGECMD_SETDOMHEIGHT:
	    domheight = param;
	    break;
	    
	case PAGECMD_MAKEGADGETS:
	    retval = country_makegadgets();
	    break;
	    
	case PAGECMD_ADDGADGETS:
	    AddGList(win, gadlist, -1, -1, NULL);
	    GT_RefreshWindow(win, NULL);
	    RefreshGList(gadlist, win, NULL, -1);
    	    break;
	    
	case PAGECMD_REMGADGETS:
	    RemoveGList(win, gadlist, -1);
	    break;
	    
	case PAGECMD_HANDLEINPUT:
	    retval = FALSE;
	    break;
	    
	case PAGECMD_CLEANUP:
	    country_cleanup();
	    break;
    }
    
    return retval;
}
