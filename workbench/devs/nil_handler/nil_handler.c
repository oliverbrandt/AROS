/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang:
*/
#include <exec/errors.h>
#include <exec/resident.h>
#include <exec/memory.h>
#include <libraries/expansion.h>
#include <proto/exec.h>
#include <proto/expansion.h>
#include <utility/tagitem.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include <proto/dos.h>
#include <aros/libcall.h>
#include <aros/asmcall.h>
#ifdef __GNUC__
#include "nil_handler_gcc.h"
#endif

#include <string.h>

static const char name[];
static const char version[];
static const APTR inittabl[4];
static void *const functable[];
struct nilbase *AROS_SLIB_ENTRY(init,nil_handler)();
void AROS_SLIB_ENTRY(open,nil_handler)();
BPTR AROS_SLIB_ENTRY(close,nil_handler)();
BPTR AROS_SLIB_ENTRY(expunge,nil_handler)();
int AROS_SLIB_ENTRY(null,nil_handler)();
void AROS_SLIB_ENTRY(beginio,nil_handler)();
LONG AROS_SLIB_ENTRY(abortio,nil_handler)();
static const char end;

int nil_handler_zentry(void)
{
    /* If the handler was executed by accident return error code. */
    return -1;
}

const struct Resident nil_handler_resident=
{
    RTC_MATCHWORD,
    (struct Resident *)&nil_handler_resident,
    (APTR)&end,
    RTF_AUTOINIT | RTF_AFTERDOS,
    41,
    NT_DEVICE,
    -127,
    (char *)name,
    (char *)&version[6],
    (ULONG *)inittabl
};

static const char name[]="nil.handler";

static const char version[]="$VER: nil-handler 41.1 (8.6.96)\r\n";

static const APTR inittabl[4]=
{
    (APTR)sizeof(struct nilbase),
    (APTR)functable,
    NULL,
    &AROS_SLIB_ENTRY(init,nil_handler)
};

static void *const functable[]=
{
    &AROS_SLIB_ENTRY(open,nil_handler),
    &AROS_SLIB_ENTRY(close,nil_handler),
    &AROS_SLIB_ENTRY(expunge,nil_handler),
    &AROS_SLIB_ENTRY(null,nil_handler),
    &AROS_SLIB_ENTRY(beginio,nil_handler),
    &AROS_SLIB_ENTRY(abortio,nil_handler),
    (void *)-1
};

AROS_UFH3(struct nilbase *, AROS_SLIB_ENTRY(init,nil_handler),
    AROS_UFHA(struct nilbase *, nilbase, D0),
    AROS_UFHA(BPTR,             segList, A0),
    AROS_UFHA(struct ExecBase *, sysBase, A6)
)
{
    AROS_USERFUNC_INIT

    /* Store arguments */
    nilbase->sysbase=sysBase;
    nilbase->seglist=segList;
    nilbase->dosbase=(struct DosLibrary *)OpenLibrary("dos.library",39);
    if(nilbase->dosbase!=NULL)
    {
	if (!segList) /* Are we a ROM module? */
	{
	    struct DeviceNode *dn;
            /* Install NIL: handler into device list
	     *
	     * KLUDGE: nil.handler should create only one device node, depending on
	     * the startup packet it gets. The mountlists for NIL: should be into dos.library bootstrap
	     * routines.
	     */

	    if((dn = AllocMem(sizeof (struct DeviceNode) + 4 + 3 + 2, MEMF_CLEAR|MEMF_PUBLIC)))
	    {
	        struct IOFileSys dummyiofs;

	        AROS_LC3(void, open,
	        AROS_UFHA(struct IOFileSys *, &dummyiofs, A1),
    	        AROS_UFHA(ULONG,              0, D0),
	        AROS_UFHA(ULONG,              0, D1),
	     	         struct nilbase *, nilbase, 1, nil_handler);

  	        if (!dummyiofs.IOFS.io_Error)
	        {
		    STRPTR s = (STRPTR)(((IPTR)dn + sizeof(struct DeviceNode) + 4) & ~3);

	    	    AROS_BSTR_putchar(s, 0, 'N');
	    	    AROS_BSTR_putchar(s, 1, 'I');
	    	    AROS_BSTR_putchar(s, 2, 'L');
	    	    AROS_BSTR_setstrlen(s, 3);

	    	    dn->dn_Type	= DLT_DEVICE;
	    	    dn->dn_Unit	= dummyiofs.IOFS.io_Unit;
	    	    dn->dn_Device	= dummyiofs.IOFS.io_Device;
	    	    dn->dn_Handler	= NULL;
	    	    dn->dn_Startup	= NULL;
	    	    dn->dn_OldName	= MKBADDR(s);
	    	    dn->dn_NewName	= AROS_BSTR_ADDR(dn->dn_OldName);

		    if (AddDosEntry((struct DosList *)dn))
		        return nilbase;
	        }

	        FreeMem(dn, sizeof (struct DeviceNode));
	    }
	}
        else
	    return nilbase;
    }

    return NULL;
    AROS_USERFUNC_EXIT
}

AROS_LH3(void, open,
 AROS_LHA(struct IOFileSys *, iofs, A1),
 AROS_LHA(ULONG,              unitnum, D0),
 AROS_LHA(ULONG,              flags, D1),
	   struct nilbase *, nilbase, 1, nil_handler)
{
    AROS_LIBFUNC_INIT
    ULONG *dev;

    /* Get compiler happy */
    unitnum=flags=0;

    /* I have one more opener. */
    nilbase->device.dd_Library.lib_OpenCnt++;

    /* Mark Message as recently used. */
    iofs->IOFS.io_Message.mn_Node.ln_Type=NT_REPLYMSG;

    dev=AllocMem(sizeof(ULONG),MEMF_PUBLIC|MEMF_CLEAR);
    if(dev!=NULL)
    {
        iofs->IOFS.io_Unit=(struct Unit *)dev;
        iofs->IOFS.io_Device=&nilbase->device;
    	nilbase->device.dd_Library.lib_Flags&=~LIBF_DELEXP;
    	iofs->IOFS.io_Error=0;
    	return;
    }else
	iofs->io_DosError=ERROR_NO_FREE_STORE;

    iofs->IOFS.io_Error=IOERR_OPENFAIL;
    nilbase->device.dd_Library.lib_OpenCnt--;

    AROS_LIBFUNC_EXIT
}

AROS_LH1(BPTR, close,
 AROS_LHA(struct IOFileSys *, iofs, A1),
	   struct nilbase *, nilbase, 2, nil_handler)
{
    AROS_LIBFUNC_INIT
    ULONG *dev;

    dev=(ULONG *)iofs->IOFS.io_Unit;
    if(*dev)
    {
	iofs->io_DosError=ERROR_OBJECT_IN_USE;
	return 0;
    }

    /* Let any following attemps to use the device crash hard. */
    iofs->IOFS.io_Device=(struct Device *)-1;
    FreeMem(dev,sizeof(ULONG));
    iofs->io_DosError=0;

    /* I have one fewer opener. */
    if(!--nilbase->device.dd_Library.lib_OpenCnt)
    {
	/* Delayed expunge pending? */
	if(nilbase->device.dd_Library.lib_Flags&LIBF_DELEXP)
	    /* Then expunge the device */
	    return expunge();
    }
    return 0;
    AROS_LIBFUNC_EXIT
}

AROS_LH0(BPTR, expunge, struct nilbase *, nilbase, 3, nil_handler)
{
    AROS_LIBFUNC_INIT

    BPTR ret;
    /*
	This function is single-threaded by exec by calling Forbid.
	Never break the Forbid() or strange things might happen.
    */

    /* Test for openers. */
    if(nilbase->device.dd_Library.lib_OpenCnt || !nilbase->seglist)
    {
	/* Set the delayed expunge flag and return. */
	nilbase->device.dd_Library.lib_Flags|=LIBF_DELEXP;
	return 0;
    }

    /* Free all resources */
    CloseLibrary((struct Library *)nilbase->dosbase);

    /* Get rid of the device. Remove it from the list. */
    Remove(&nilbase->device.dd_Library.lib_Node);

    /* Get returncode here - FreeMem() will destroy the field. */
    ret=nilbase->seglist;

    /* Free the memory. */
    FreeMem((char *)nilbase-nilbase->device.dd_Library.lib_NegSize,
	    nilbase->device.dd_Library.lib_NegSize+nilbase->device.dd_Library.lib_PosSize);

    return ret;
    AROS_LIBFUNC_EXIT
}

AROS_LH0I(int, null, struct nilbase *, nilbase, 4, nil_handler)
{
    AROS_LIBFUNC_INIT
    return 0;
    AROS_LIBFUNC_EXIT
}

AROS_LH1(void, beginio,
 AROS_LHA(struct IOFileSys *, iofs, A1),
	   struct nilbase *, nilbase, 5, nil_handler)
{
    AROS_LIBFUNC_INIT
    LONG error=0;

    /*
	Do everything quick no matter what. This is possible
	because I never need to Wait().
    */
    switch(iofs->IOFS.io_Command)
    {
	case FSA_OPEN:
	case FSA_OPEN_FILE:
	    /* No names allowed on NIL: */
	    if
	    (
	       stricmp(iofs->io_Union.io_NamedFile.io_Filename, "NIL:") != 0 &&
	       iofs->io_Union.io_NamedFile.io_Filename[0]
            )
	    {
		error=ERROR_OBJECT_NOT_FOUND;
		break;
	    }
	    Forbid();
	    ++*(ULONG *)iofs->IOFS.io_Unit;
	    Permit();
	    break;
	    
	case FSA_READ:
	    iofs->io_Union.io_READ.io_Length=0;
	    break;

	case FSA_WRITE:
	    break;

	case FSA_SEEK:
	    iofs->io_Union.io_SEEK.io_Offset = 0;
	    break;

	case FSA_CLOSE:
	    Forbid();
	    --*(ULONG *)iofs->IOFS.io_Unit;
	    Permit();
	    break;

	case FSA_IS_INTERACTIVE:
	    iofs->io_Union.io_IS_INTERACTIVE.io_IsInteractive = TRUE;
	    break;
	case FSA_SET_FILE_SIZE:
        case FSA_EXAMINE:
        case FSA_EXAMINE_NEXT:
        case FSA_EXAMINE_ALL:
        case FSA_CREATE_DIR:
        case FSA_CREATE_HARDLINK:
        case FSA_CREATE_SOFTLINK:
        case FSA_RENAME:
        case FSA_DELETE_OBJECT:
            error = ERROR_NOT_IMPLEMENTED;
            break;

	default:
	    error = ERROR_ACTION_NOT_KNOWN;
	    break;
    }

    /* Set error code */
    iofs->io_DosError=error;

    /* If the quick bit is not set send the message to the port */
    if(!(iofs->IOFS.io_Flags&IOF_QUICK))
	ReplyMsg(&iofs->IOFS.io_Message);

    AROS_LIBFUNC_EXIT
}

AROS_LH1(LONG, abortio,
 AROS_LHA(struct IOFileSys *, iofs, A1),
	   struct nilbase *, nilbase, 6, nil_handler)
{
    AROS_LIBFUNC_INIT
    /* Everything already done. */
    return 0;
    AROS_LIBFUNC_EXIT
}

static const char end=0;
