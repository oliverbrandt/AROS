/*
    Copyright � 1995-2004, The AROS Development Team. All rights reserved.
    $Id$

    GradientSlider initialization code.
*/

#include <stddef.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <aros/libcall.h>
#include <aros/debug.h>

#include <proto/exec.h>
#include <proto/intuition.h>

#include "gradientslider_intern.h"
#include LC_LIBDEFS_FILE

/***************************************************************************************************/

/* Global IntuitionBase */
#ifdef GLOBAL_INTUIBASE
struct IntuitionBase *IntuitionBase;
#endif

#undef SysBase

/* Customize libheader.c */
#define LC_SYSBASE_FIELD(lib)   (((LIBBASETYPEPTR       )(lib))->sysbase)
#define LC_SEGLIST_FIELD(lib)   (((LIBBASETYPEPTR       )(lib))->seglist)
#define LC_LIBBASESIZE          sizeof(LIBBASETYPE)
#define LC_LIBHEADERTYPEPTR     LIBBASETYPEPTR
#define LC_LIB_FIELD(lib)       (((LIBBASETYPEPTR)(lib))->library)

#define LC_NO_OPENLIB
#define LC_NO_CLOSELIB

#include <libcore/libheader.c>

#undef  SDEBUG
#undef  DEBUG
#define DEBUG 0
#include <aros/debug.h>

#define SysBase      (LC_SYSBASE_FIELD(lh))


#define GradientSliderBase ((LIBBASETYPEPTR) lh)

/***************************************************************************************************/

ULONG SAVEDS STDARGS LC_BUILDNAME(L_InitLib) (LC_LIBHEADERTYPEPTR lh)
{
    D(bug("Inside initfunc of gradientslider.gadget\n"));

    if (!GfxBase)
    	GfxBase = (GraphicsBase *) OpenLibrary ("graphics.library", 37);
    if (!GfxBase)
	return FALSE;

    if (!UtilityBase)
	UtilityBase = OpenLibrary ("utility.library", 37);
    if (!UtilityBase)
	return FALSE;

    if (!IntuitionBase)
    	IntuitionBase = (IntuiBase *) OpenLibrary ("intuition.library", 37);
    if (!IntuitionBase)
	return FALSE;

    if (!LayersBase)
    	LayersBase = OpenLibrary ("layers.library", 39);
    if (!LayersBase)
    	return FALSE;

    if (!CyberGfxBase)
        CyberGfxBase = OpenLibrary ("cybergraphics.library", 0);
    /* we can live even without CyberGfxBase */
    
    if (!GradientSliderBase->classptr)
    	GradientSliderBase->classptr = InitGradientSliderClass (GradientSliderBase);
    if (!GradientSliderBase->classptr)
    	return FALSE;

    return TRUE;
}

/***************************************************************************************************/

void SAVEDS STDARGS LC_BUILDNAME(L_ExpungeLib) (LC_LIBHEADERTYPEPTR lh)
{
    if (GradientSliderBase->classptr)
    {
	RemoveClass (GradientSliderBase->classptr);
	FreeClass (GradientSliderBase->classptr);
	GradientSliderBase->classptr = NULL;
    }

    /* CloseLibrary() checks for NULL-pointers */
    CloseLibrary (UtilityBase);
    UtilityBase = NULL;
    
    CloseLibrary ((struct Library *) GfxBase);
    GfxBase = NULL;
    
    CloseLibrary ((struct Library *) IntuitionBase);
    IntuitionBase = NULL;
    
    CloseLibrary(LayersBase);
    LayersBase = NULL;
    
    CloseLibrary(CyberGfxBase);
    CyberGfxBase = NULL;
}

/*****************************************************************************/

void *const LIBFUNCTABLE[]=
{
    AROS_SLIB_ENTRY(LC_BUILDNAME(OpenLib),LibHeader),
    AROS_SLIB_ENTRY(LC_BUILDNAME(CloseLib),LibHeader),
    AROS_SLIB_ENTRY(LC_BUILDNAME(ExpungeLib),LibHeader),
    AROS_SLIB_ENTRY(LC_BUILDNAME(ExtFuncLib),LibHeader),
    (void *)-1L
};
