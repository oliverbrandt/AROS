/*
    Copyright  1995-2010, The AROS Development Team. All rights reserved.
    $Id: gdi_init.c 27757 2008-01-26 15:05:40Z sonic $

    Desc: GDI hidd initialization code.
    Lang: English.
*/

#define __OOP_NOATTRBASES__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>

#include <proto/exec.h>
#include <proto/kernel.h>
#include <proto/oop.h>

#include <utility/utility.h>
#include <oop/oop.h>
#include <hidd/graphics.h>

#include <aros/symbolsets.h>
#include <aros/debug.h>

#include LC_LIBDEFS_FILE

#include "gdi.h"

/****************************************************************************************/

#undef XSD

/****************************************************************************************/

static BOOL initclasses( struct gdi_staticdata *xsd );
static VOID freeclasses( struct gdi_staticdata *xsd );

/****************************************************************************************/

static OOP_AttrBase HiddPixFmtAttrBase;

static struct OOP_ABDescr abd[] =
{
    { IID_Hidd_PixFmt   , &HiddPixFmtAttrBase   },
    { NULL     	    	, NULL	    	    	}
};

/****************************************************************************************/

static BOOL initclasses(struct gdi_staticdata *xsd)
{
    /* Get some attrbases */
    
    return OOP_ObtainAttrBases(abd);
}

/****************************************************************************************/

static VOID freeclasses(struct gdi_staticdata *xsd)
{
    OOP_ReleaseAttrBases(abd);
}

/****************************************************************************************/

static int GDI_Init(LIBBASETYPEPTR LIBBASE)
{
    struct Task *me;
    void *gfx_int;
    struct gdi_staticdata *xsd = &LIBBASE->xsd;

    D(bug("Entering GDI_Init\n"));
    if (LIBBASE->library.lib_OpenCnt)
	return TRUE;

    InitSemaphore( &xsd->sema );

    Forbid();
    xsd->display = GDICALL(CreateDC, "DISPLAY", NULL, NULL, NULL);
    Permit();
    if (xsd->display) {
/*
        xsd->delete_win_atom         = XCALL(XInternAtom, xsd->display, "WM_DELETE_WINDOW", FALSE);
        xsd->clipboard_atom          = XCALL(XInternAtom, xsd->display, "CLIPBOARD", FALSE);
        xsd->clipboard_property_atom = XCALL(XInternAtom, xsd->display, "AROS_HOSTCLIP", FALSE);
        xsd->clipboard_incr_atom     = XCALL(XInternAtom, xsd->display, "INCR", FALSE);
        xsd->clipboard_targets_atom  = XCALL(XInternAtom, xsd->display, "TARGETS", FALSE);
*/
	if (initclasses(xsd)) {
	    Forbid();
	    xsd->ctl = NATIVECALL(GDI_Init);
	    Permit();
	    if (xsd->ctl) {
	        me = FindTask(NULL);
	        gfx_int = KrnAddIRQHandler(xsd->ctl->IrqNum, GfxIntHandler, NULL, me);
	        if (gfx_int) {
	            SetSignal(0, SIGF_BLIT);
		    Forbid();
		    NATIVECALL(GDI_Start, xsd->ctl);
		    Permit();
		    Wait(SIGF_BLIT);
		    KrnRemIRQHandler(gfx_int);
		    if (xsd->ctl->window_ready) {
	    	        D(bug("GDI_Init succeeded\n"));
	    	        return TRUE;
		    }
	        }
		NATIVECALL(GDI_Shutdown, xsd->ctl);
	    }
	    freeclasses(xsd);
        }
    }

    D(bug("GDI_Init failed\n"));
    return FALSE;
}

/****************************************************************************************/

ADD2OPENLIB(GDI_Init, 0);
