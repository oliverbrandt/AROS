/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: Header for dos.library
    Lang: english
*/
#include <exec/types.h>
#include <exec/resident.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <exec/alerts.h>
#include <proto/exec.h>
#include <aros/libcall.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <proto/dos.h>
#include <utility/tagitem.h>
#include <aros/debug.h>
#include "libdefs.h"
#include "dos_intern.h"

#define INIT	AROS_SLIB_ENTRY(init,Dos)

static const char name[];
static const char version[];
static const APTR Dos_inittabl[4];
static void *const LIBFUNCTABLE[];
struct LIBBASETYPE *INIT ();
extern const char LIBEND;

struct DosLibrary *DOSBase;
struct DosLibrary **dosPtr = &DOSBase;

extern void DOSBoot(struct ExecBase *, struct DosLibrary *);

int Dos_entry(void)
{
    /* If the library was executed by accident return error code. */
    return -1;
}

const struct Resident Dos_resident=
{
    RTC_MATCHWORD,
    (struct Resident *)&Dos_resident,
    (APTR)&LIBEND,
    RTF_AUTOINIT,
    VERSION_NUMBER,
    NT_LIBRARY,
    -120,
    (char *)name,
    (char *)&version[6],
    (ULONG *)Dos_inittabl
};

static const char name[]=NAME_STRING;
static const char version[]=VERSION_STRING;

static const APTR Dos_inittabl[4]=
{
    (APTR)sizeof(struct LIBBASETYPE),
    (APTR)LIBFUNCTABLE,
    NULL,
    &INIT
};

void LDDemon();
void AROS_SLIB_ENTRY(OpenLibrary,Dos)();
void AROS_SLIB_ENTRY(OpenDevice,Dos)();
void AROS_SLIB_ENTRY(CloseLibrary,Dos)();
void AROS_SLIB_ENTRY(CloseDevice,Dos)();
void AROS_SLIB_ENTRY(RemLibrary,Dos)();
void LDFlush();

#undef SysBase

AROS_LH2(struct LIBBASETYPE *, init,
 AROS_LHA(struct LIBBASETYPE *, LIBBASE, D0),
 AROS_LHA(BPTR,               segList,   A0),
	   struct ExecBase *, SysBase, 0, BASENAME)
{
    AROS_LIBFUNC_INIT
    /* This function is single-threaded by exec by calling Forbid. */

    /* Store arguments */
    LIBBASE->dl_SysBase=SysBase;
    LIBBASE->dl_SegList=segList;

    InitSemaphore(&LIBBASE->dl_DosListLock);
    InitSemaphore(&LIBBASE->dl_LDSigSem);

    LIBBASE->dl_UtilityBase=OpenLibrary("utility.library",39);
    if(LIBBASE->dl_UtilityBase!=NULL)
    {
	/* iaint:
	    I know this is bad, but I also know that the timer.device
	    will never go away during the life of dos.library. I also
	    don't intend to make any I/O calls using this.

	    I also know that timer.device does exist in the device list
	    at this point in time.

	    I can't allocate a timerequest/MsgPort pair here anyway,
	    because I need a separate one for each caller to Delay()
	*/
	Forbid();
	LIBBASE->dl_TimerBase = (struct Device *)
	    FindName(&SysBase->DeviceList, "timer.device");

	if(LIBBASE->dl_TimerBase != NULL)
	{
	    struct TagItem tags[]=
	    {
		{ NP_Entry, (IPTR)LDDemon },
		{ NP_Input, 0 },
		{ NP_Output, 0 },
		{ NP_Name, (IPTR)"lib & dev loader demon" },
		{ NP_UserData, (IPTR)LIBBASE },
		{ TAG_END, 0 }
	    };

	    LIBBASE->dl_TimerBase->dd_Library.lib_OpenCnt++;
	    Permit();

	    LIBBASE->dl_LDDemon=CreateNewProc((struct TagItem *)tags);

	    if(LIBBASE->dl_LDDemon!=NULL)
	    {
		(void)SetFunction(&SysBase->LibNode,-92*LIB_VECTSIZE,AROS_SLIB_ENTRY(OpenLibrary,Dos));
		(void)SetFunction(&SysBase->LibNode,-74*LIB_VECTSIZE,AROS_SLIB_ENTRY(OpenDevice,Dos));
		(void)SetFunction(&SysBase->LibNode,-69*LIB_VECTSIZE,AROS_SLIB_ENTRY(CloseLibrary,Dos));
		(void)SetFunction(&SysBase->LibNode,-75*LIB_VECTSIZE,AROS_SLIB_ENTRY(CloseDevice,Dos));
		(void)SetFunction(&SysBase->LibNode,-67*LIB_VECTSIZE,AROS_SLIB_ENTRY(RemLibrary,Dos));
		(void)SetFunction(&SysBase->LibNode,-73*LIB_VECTSIZE,AROS_SLIB_ENTRY(RemLibrary,Dos));

		LIBBASE->dl_LDHandler.is_Node.ln_Name="lib & dev loader demon";
		LIBBASE->dl_LDHandler.is_Node.ln_Pri=0;
		LIBBASE->dl_LDHandler.is_Code=LDFlush;

		*dosPtr = LIBBASE;

		AddMemHandler(&LIBBASE->dl_LDHandler);
		AddLibrary((struct Library *)LIBBASE);

		/*
		    Here we have to get the first node of the mountlist,
		    and we try and boot from it, (assign it to SYS:).
		*/
		DOSBoot(SysBase, DOSBase);

		/* This is where we start the RTC_AFTERDOS residents */
		InitCode(RTF_AFTERDOS,0);

		/* We now restart the multitasking	- this is done
		   automatically by RemTask() when it switches.
		*/
		RemTask(NULL);
	    }
	}
	Permit();
	Alert(AT_DeadEnd | AG_OpenDev | AN_DOSLib | AO_TimerDev);
	CloseLibrary(LIBBASE->dl_UtilityBase);
    }
    Alert(AT_DeadEnd | AG_OpenLib | AN_DOSLib | AO_UtilityLib);

    return NULL;
    AROS_LIBFUNC_EXIT
}

#define SysBase     (LIBBASE->dl_SysBase)

AROS_LH1(struct LIBBASETYPE *, open,
    AROS_LHA(ULONG, version, D0),
    struct LIBBASETYPE *, LIBBASE, 1, BASENAME)
{
    AROS_LIBFUNC_INIT
    /*
	This function is single-threaded by exec by calling Forbid.
	If you break the Forbid() another task may enter this function
	at the same time. Take care.
    */

    /* Keep compiler happy */
    version=0;

    /* I have one more opener. */
    LIBBASE->dl_lib.lib_OpenCnt++;
    LIBBASE->dl_lib.lib_Flags&=~LIBF_DELEXP;

    /* You would return NULL if the open failed. */
    return LIBBASE;
    AROS_LIBFUNC_EXIT
}

AROS_LH0(BPTR, close,
	   struct LIBBASETYPE *, LIBBASE, 2, BASENAME)
{
    AROS_LIBFUNC_INIT
    /*
	This function is single-threaded by exec by calling Forbid.
	If you break the Forbid() another task may enter this function
	at the same time. Take care.
    */

    /* I have one fewer opener. */
    if(!--LIBBASE->dl_lib.lib_OpenCnt)
    {
	/* Delayed expunge pending? */
	if(LIBBASE->dl_lib.lib_Flags&LIBF_DELEXP)
	    /* Then expunge the library */
	    return expunge();
    }
    return 0;
    AROS_LIBFUNC_EXIT
}

AROS_LH0(BPTR, expunge,
	   struct LIBBASETYPE *, LIBBASE, 3, BASENAME)
{
    AROS_LIBFUNC_INIT

    BPTR ret;
    /*
	This function is single-threaded by exec by calling Forbid.
	Never break the Forbid() or strange things might happen.
    */

    /* Test for openers. */
    if(LIBBASE->dl_lib.lib_OpenCnt)
    {
	/* Set the delayed expunge flag and return. */
	LIBBASE->dl_lib.lib_Flags|=LIBF_DELEXP;
	return 0;
    }

    /* Get rid of the library. Remove it from the list. */
    Remove(&LIBBASE->dl_lib.lib_Node);

    /* Get returncode here - FreeMem() will destroy the field. */
    ret=LIBBASE->dl_SegList;

    /* Free the memory. */
    FreeMem((char *)LIBBASE-LIBBASE->dl_lib.lib_NegSize,
	    LIBBASE->dl_lib.lib_NegSize+LIBBASE->dl_lib.lib_PosSize);

    return ret;
    AROS_LIBFUNC_EXIT
}

AROS_LH0I(int, null,
	    struct LIBBASETYPE *, LIBBASE, 4, BASENAME)
{
    AROS_LIBFUNC_INIT
    return 0;
    AROS_LIBFUNC_EXIT
}
