/*
    Copyright  1995-2010, The AROS Development Team. All rights reserved.
    $Id: wrapper.c 33518 2010-06-09 13:36:09Z sonic $

    Desc: Disk-resident part of GDI display driver
    Lang: english
*/

#include <aros/debug.h>
#include <dos/dosextens.h>
#include <oop/oop.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/oop.h>
#include <proto/icon.h>

#include <stdlib.h>

#include "gdi_class.h"

/* Minimum required library version */
#define GDI_VERSION 41

/************************************************************************/

/*
 * This program actually just creates additional GDI displays.
 * It assumes that the driver itself is placed in kickstart in the form
 * of library
 */

extern struct WBStartup *WBenchMsg;

int __nocommandline = 1;

/* This function uses library open count as displays count */
static ULONG AddDisplays(ULONG num)
{
    struct Library *GDIBase;
    OOP_Object *gfxhidd;
    ULONG old;
    ULONG i;

    D(bug("[GDI] Making %u displays\n", num));
    /* First query current displays count */
    GDIBase = OpenLibrary(GDI_LIBNAME, GDI_VERSION);
    if (!GDIBase)
        return 0;

    old = GDIBase->lib_OpenCnt - 1;
    CloseLibrary(GDIBase);
    D(bug("[GDI] Current displays count: %u\n", old));

    /* Add displays if needed, open the library once more for every display */
    for (i = old; i < num; i++)
    {
        /* This increments counter */
	GDIBase = OpenLibrary(GDI_LIBNAME, GDI_VERSION);
	if (!GDIBase) {
	    D(bug("[GDI] Failed to open GDI library!\n"));
	    break;
	}

	gfxhidd = OOP_NewObject(NULL, CLID_Hidd_GDIGfx, NULL);
	D(bug("[GDI] Created display object 0x%p\n", gfxhidd));
	if (gfxhidd){
	    if (AddDisplayDriverA(gfxhidd, NULL)) {
		D(bug("[GDI] Failed to add display object\n"));
		OOP_DisposeObject(gfxhidd);
		gfxhidd = NULL;
	    }
	}

	/* If driver setup failed, decrement counter back and abort */
	if (!gfxhidd) {
	    CloseLibrary(GDIBase);
	    break;
	}
    }
    
    return i;
}

int main(void)
{
    BPTR olddir = NULL;
    STRPTR myname;
    struct DiskObject *icon;
    struct RDArgs *rdargs = NULL;
    int res = RETURN_OK;
    IPTR displays = 1;

    if (WBenchMsg) {
        olddir = CurrentDir(WBenchMsg->sm_ArgList[0].wa_Lock);
	myname = WBenchMsg->sm_ArgList[0].wa_Name;
    } else {
	struct Process *me = (struct Process *)FindTask(NULL);
    
	if (me->pr_CLI) {
            struct CommandLineInterface *cli = BADDR(me->pr_CLI);
	
	    myname = cli->cli_CommandName;
	} else
	    myname = me->pr_Task.tc_Node.ln_Name;
    }
    D(bug("[GDI] Command name: %s\n", myname));

    icon = GetDiskObject(myname);
    D(bug("[GDI] Icon 0x%p\n", icon));

    if (icon) {
        STRPTR str = FindToolType(icon->do_ToolTypes, "DISPLAYS");
        
	displays = atoi(str);
    }

    if (!WBenchMsg) {
        rdargs = ReadArgs("DISPLAYS/N/A", &displays, NULL);
	D(bug("[GDI] RDArgs 0x%p\n", rdargs));
    }
 
    AddDisplays(displays);

    if (rdargs)
        FreeArgs(rdargs);
    if (icon)
        FreeDiskObject(icon);
    if (olddir)
        CurrentDir(olddir);

    return res;
}
