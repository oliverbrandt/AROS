/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: Library header for intuition
    Lang: english
*/
#define AROS_ALMOST_COMPATIBLE

#define INIT AROS_SLIB_ENTRY(init,Intuition)

#include <string.h>
#include <exec/lists.h>
#include <exec/resident.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/alerts.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/alib.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <proto/dos.h>
#include <proto/arossupport.h>
#ifndef INTUITION_CLASSES_H
#   include <intuition/classes.h>
#endif
#ifndef UTILITY_HOOKS_H
#   include <utility/hooks.h>
#endif
#include <utility/utility.h>
#include <aros/asmcall.h>
#include "libdefs.h"
#include "intuition_intern.h"

static const char name[];
static const char version[];
static const APTR inittabl[4];
static void *const FUNCTABLE[];

struct LIBBASETYPE *INIT();

extern const char END;

/* There has to be a better way... */
struct IClass *InitImageClass (struct LIBBASETYPE * LIBBASE);
struct IClass *InitFrameIClass (struct LIBBASETYPE * LIBBASE);
struct IClass *InitSysIClass (struct LIBBASETYPE * LIBBASE);
struct IClass *InitGadgetClass (struct LIBBASETYPE * LIBBASE);
struct IClass *InitButtonGClass (struct LIBBASETYPE * LIBBASE);
struct IClass *InitFrButtonClass (struct LIBBASETYPE * LIBBASE);
struct IClass *InitPropGClass (struct LIBBASETYPE * LIBBASE);
struct IClass *InitStrGClass (struct LIBBASETYPE * LIBBASE);

int Intuition_entry(void)
{
    /* If the library was executed by accident return error code. */
    return -1;
}

const struct Resident Intuition_resident=
{
    RTC_MATCHWORD,
    (struct Resident *)&Intuition_resident,
    (APTR)&END,
    RTF_AUTOINIT|RTF_COLDSTART,
    LIBVERSION,
    NT_LIBRARY,
    0,
    (char *)name,
    (char *)&version[6],
    (ULONG *)inittabl
};

static const char name[]=INTUITIONNAME;

static const char version[]=VERSION;

static const APTR inittabl[4]=
{
    (APTR)sizeof(struct IntIntuitionBase),
    (APTR)FUNCTABLE,
    NULL,
    &INIT
};

void intui_ProcessEvents (void);

struct Process * inputDevice;

AROS_LH2(struct LIBBASETYPE *, init,
 AROS_LHA(struct LIBBASETYPE *, LIBBASE, D0),
 AROS_LHA(BPTR,               segList,   A0),
	   struct ExecBase *, sysBase, 0, Intuition)
{
    AROS_LIBFUNC_INIT
    SysBase = sysBase;

    if (!intui_init (LIBBASE))
	return NULL;

    /*  We have to open this here, but it doesn't do any allocations,
	so it shouldn't fail...
    */
    if(!(BOOPSIBase = OpenLibrary("boopsi.library", 0)))
    {
	/* Intuition couldn't open unknown library */
	Alert(AT_DeadEnd | AN_Intuition | AG_OpenLib | AO_Unknown);
	return NULL;
    }

    /* Create semaphore and initialize it */
    GetPrivIBase(LIBBASE)->IBaseLock = AllocMem (sizeof(struct SignalSemaphore), MEMF_PUBLIC|MEMF_CLEAR);

    if (!GetPrivIBase(LIBBASE)->IBaseLock)
	return NULL;

    InitSemaphore (GetPrivIBase(LIBBASE)->IBaseLock);

    /* Add all other classes */
    InitImageClass (LIBBASE); /* After ROOTCLASS */
    InitFrameIClass (LIBBASE); /* After IMAGECLASS */
    InitSysIClass (LIBBASE); /* After IMAGECLASS */

    InitGadgetClass (LIBBASE); /* After ROOTCLASS */
    InitButtonGClass (LIBBASE); /* After GADGETCLASS */
    InitFrButtonClass (LIBBASE); /* After BUTTONGCLASS */
    InitPropGClass (LIBBASE);    /* After GADGETCLASS */
    InitStrGClass (LIBBASE);    /* After GADGETCLASS */

    /* You would return NULL if the init failed */
    return LIBBASE;
    AROS_LIBFUNC_EXIT
}

AROS_LH1(struct LIBBASETYPE *, open,
 AROS_LHA(ULONG, version, D0),
	   struct LIBBASETYPE *, LIBBASE, 1, Intuition)
{
    AROS_LIBFUNC_INIT
    struct TagItem screenTags[] =
    {
	{ SA_Depth, 4			},
	{ SA_Type,  WBENCHSCREEN	},
	{ SA_Title, (IPTR)"Workbench"   },
	{ TAG_END, 0 }
    };
    struct TagItem inputTask[]=
    {
	{ NP_UserData,	0L },
	{ NP_Entry,	(IPTR)intui_ProcessEvents },
	{ NP_Input,	0L },
	{ NP_Output,	0L },
	{ NP_Name,	(IPTR)"input.device" },
	{ NP_Priority,	50 },
	{ TAG_END, 0 }
    };

    /* Keep compiler happy */
    version=0;

    /* TODO Create input.device. This is a bad hack. */
    if (!inputDevice)
    {
	struct Task *idleT;
	inputTask[0].ti_Data = (IPTR)LIBBASE;

	inputDevice = CreateNewProc (inputTask);

	if (!inputDevice)
	    return NULL;

	idleT = FindTask("Idle Task");
	if( idleT )
		Signal(idleT, SIGBREAKF_CTRL_F);
    }

    if (!GfxBase)
    {
	if (!(GfxBase = (void *)OpenLibrary (GRAPHICSNAME, 39)) )
	    return NULL;
    }

    if (!UtilityBase)
    {
	if (!(UtilityBase = (void *)OpenLibrary (UTILITYNAME, 39)) )
	    return NULL; /* don't close anything */
    }

    if (!GetPrivIBase(LIBBASE)->WorkBench)
    {
	struct Screen * screen;

	screen = OpenScreenTagList (NULL, screenTags);

	if (!screen)
	    return NULL;

	LIBBASE->FirstScreen =
	    LIBBASE->ActiveScreen =
	    GetPrivIBase(LIBBASE)->WorkBench = screen;
    }

    if (!intui_open (LIBBASE))
	return NULL;

    /* I have one more opener. */
    LIBBASE->LibNode.lib_OpenCnt++;
    LIBBASE->LibNode.lib_Flags&=~LIBF_DELEXP;

    /* You would return NULL if the open failed. */
    return LIBBASE;
    AROS_LIBFUNC_EXIT
}

AROS_LH0(BPTR, close,
	   struct LIBBASETYPE *, LIBBASE, 2, Intuition)
{
    AROS_LIBFUNC_INIT

    /* I have one fewer opener. */
    if(!--LIBBASE->LibNode.lib_OpenCnt)
    {
	intui_close (LIBBASE);

	/* Delayed expunge pending? */
	if(LIBBASE->LibNode.lib_Flags&LIBF_DELEXP)
	    /* Then expunge the library */
	    return expunge();
    }
    return 0;
    AROS_LIBFUNC_EXIT
}

AROS_LH0(BPTR, expunge,
	   struct LIBBASETYPE *, LIBBASE, 3, Intuition)
{
    AROS_LIBFUNC_INIT

    /* Test for openers. */
    if(LIBBASE->LibNode.lib_OpenCnt)
    {
	/* Set the delayed expunge flag and return. */
	LIBBASE->LibNode.lib_Flags|=LIBF_DELEXP;
	return 0;
    }

    /* Free unecessary memory */
    if (GetPrivIBase(LIBBASE)->WorkBench)
	CloseScreen (GetPrivIBase(LIBBASE)->WorkBench);

    if (UtilityBase)
	CloseLibrary ((struct Library *)UtilityBase);

    if (GfxBase)
	CloseLibrary ((struct Library *)GfxBase);

    /* Let the driver do the same */
    intui_expunge (LIBBASE);

#ifdef DISK_BASED /* Don't remove a ROM library */
    FreeImageClass ();

    /* Get rid of the library. Remove it from the list. */
    Remove(&LIBBASE->LibNode.lib_Node);

    /* Free the memory. */
    FreeMem((char *)LIBBASE-LIBBASE->LibNode.lib_NegSize,
	    LIBBASE->LibNode.lib_NegSize+LIBBASE->LibNode.lib_PosSize);
#endif

    return 0L;
    AROS_LIBFUNC_EXIT
}

AROS_LH0I(int, null,
	    struct LIBBASETYPE *, LIBBASE, 4, Intuition)
{
    AROS_LIBFUNC_INIT
    return 0;
    AROS_LIBFUNC_EXIT
}
