/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.1  1996/08/13 13:48:27  digulla
    Small Demo: Open a window, render some gfx and wait for a keypress

    Revision 1.5  1996/08/01 17:40:44  digulla
    Added standard header for all files

    Desc:
    Lang:
*/
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <dos/dos.h>
#include <dos/exall.h>
#include <dos/datetime.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <graphics/rastport.h>
#include <stdlib.h>

/* Don't define symbols before the entry point. */
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern const char dosname[];
static LONG tinymain(void);

__AROS_LH0(LONG,entry,struct ExecBase *,sysbase,,)
{
    __AROS_FUNC_INIT
    LONG error=RETURN_FAIL;

    SysBase=sysbase;
    DOSBase=(struct DosLibrary *)OpenLibrary((STRPTR)dosname,39);
    GfxBase=(struct GfxBase *)OpenLibrary(GRAPHICSNAME,39);
    IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",39);
    if(DOSBase && GfxBase && IntuitionBase)
    {
	error=tinymain();
	CloseLibrary((struct Library *)DOSBase);
	CloseLibrary((struct Library *)GfxBase);
	CloseLibrary((struct Library *)IntuitionBase);
    }
    return error;
    __AROS_FUNC_EXIT
}

struct ExecBase *SysBase;
struct DosLibrary *DOSBase;
struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;
const char dosname[]="dos.library";

void D(char *str)
{
    VPrintf(str,NULL);
    Flush(Output());
}

static LONG tinymain(void)
{
    struct NewWindow nw;
    struct Window * win;
    struct RastPort * rp;
    struct IntuiMessage * im;
    int cont;

    nw.LeftEdge = 100;
    nw.TopEdge = 100;
    nw.Width = 640;
    nw.Height = 512;
    nw.DetailPen = nw.BlockPen = (UBYTE)-1;
    nw.IDCMPFlags = IDCMP_RAWKEY;
    nw.Flags = 0L;
    nw.FirstGadget = NULL;
    nw.CheckMark = NULL;
    nw.Title = "Open a window demo";
    nw.Type = WBENCHSCREEN;

    D("OpenWindow\n");
    win = OpenWindow (&nw);

    rp = win->RPort;

    D("SetAPen\n");
    SetAPen (rp, 1);
    SetDrMd (rp, JAM2);

    D("Move\n");
    Move (rp, 0, 0);
    Draw (rp, 320, 256);

    D("Rectfill 1\n");
    SetAPen (rp, 0);
    RectFill (rp, 100, 10, 110, 20);

    D("Rectfill 2\n");
    SetAPen (rp, 1);
    RectFill (rp, 150, 10, 160, 20);

    D("Rectfill 3\n");
    SetAPen (rp, 2);
    RectFill (rp, 200, 10, 210, 20);

    D("Rectfill 4\n");
    SetAPen (rp, 3);
    RectFill (rp, 250, 10, 260, 20);

    cont = 1;

    while (cont)
    {
	if ((im = (struct IntuiMessage *)GetMsg (win->UserPort)))
	{
	    D("Got msg\n");
	    switch (im->Class)
	    {
	    case IDCMP_RAWKEY:
		cont = 0;
		break;

	    }
	}
	else
	{
	    D("Waiting\n");
	    Wait (1L << win->UserPort->mp_SigBit);
	}
    }

    D("CloseWindow\n");
    CloseWindow (win);

    return 0;
}

