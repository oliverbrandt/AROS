/*
    (C) 1998 AROS - The Amiga Research OS
    $Id$

    Desc: Gameport device
    Lang: English
*/

#define AROS_ALMOST_COMPATIBLE 1
#include <exec/resident.h>
#include <exec/interrupts.h>
#include <devices/inputevent.h>
#include <devices/gameport.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/oop.h>
#include <exec/memory.h>
#include <exec/errors.h>
#include <exec/lists.h>
#include <oop/oop.h>
#include <utility/utility.h>
#include <hidd/mouse.h>
#include <aros/libcall.h>
#include <aros/asmcall.h>
#include "gameport_intern.h"
#include "devs_private.h"

#ifdef  __GNUC__
#include "gameport_gcc.h"
#endif

#define DEBUG 0
#include <aros/debug.h>


#define ioStd(x)  ((struct IOStdReq *)x)
#define gpUn      ((struct GPUnit *)(ioreq->io_Unit))

#define min(a,b)  ((a) < (b)) ? (a) : (b)
#define ALIGN(x)  ((((x) + (__AROS_STRUCTURE_ALIGNMENT - 1)) / __AROS_STRUCTURE_ALIGNMENT) * __AROS_STRUCTURE_ALIGNMENT)



static const char name[];
static const char version[];
static const APTR inittabl[4];
static void *const functable[];
static const UBYTE datatable;

struct GameportBase *AROS_SLIB_ENTRY(init, Gameport)();
void AROS_SLIB_ENTRY(open, Gameport)();
BPTR AROS_SLIB_ENTRY(close, Gameport)();
BPTR AROS_SLIB_ENTRY(expunge, Gameport)();
int  AROS_SLIB_ENTRY(null, Gameport)();
void AROS_SLIB_ENTRY(beginio, Gameport)();
LONG AROS_SLIB_ENTRY(abortio, Gameport)();

static const char end;

int AROS_SLIB_ENTRY(entry, Gameport)(void)
{
    /* If the device was executed by accident return error code. */
    return -1;
}

const struct Resident Gameport_resident =
{
    RTC_MATCHWORD,
    (struct Resident *)&Gameport_resident,
    (APTR)&end,
    RTF_AUTOINIT|RTF_COLDSTART,
    41,
    NT_DEVICE,
    44,
    (char *)name,
    (char *)&version[6],
    (ULONG *)inittabl
};

static const char name[] = "gameport.device";

static const char version[] = "$VER: gameport.device 41.0 (7.3.1998)\r\n";

static const APTR inittabl[4] =
{
    (APTR)sizeof(struct GameportBase),
    (APTR)functable,
    (APTR)&datatable,
    &AROS_SLIB_ENTRY(init, Gameport)
};

static void *const functable[] =
{
    &AROS_SLIB_ENTRY(open, Gameport),
    &AROS_SLIB_ENTRY(close, Gameport),
    &AROS_SLIB_ENTRY(expunge, Gameport),
    &AROS_SLIB_ENTRY(null, Gameport),
    &AROS_SLIB_ENTRY(beginio, Gameport),
    &AROS_SLIB_ENTRY(abortio, Gameport),
    (void *)-1
};


static AttrBase HiddMouseAB = 0;

static BOOL fillrequest(struct IORequest *ioreq, struct GameportBase *GPBase);
static VOID mouseCallback(struct GameportBase *GPBase, struct pHidd_Mouse_Event *ev);
AROS_UFP3(static VOID, sendQueuedEvents,
    AROS_UFPA(struct GameportBase *, GPBase, A1),
    AROS_UFPA(APTR, thisfunc, A1),
    AROS_UFPA(struct ExecBase *, SysBase, A6));
    

AROS_LH2(struct GameportBase *,  init,
 AROS_LHA(struct GameportBase *, GPBase, D0),
 AROS_LHA(BPTR,         segList, A0),
	  struct ExecBase *, sysBase, 0, Gameport)
{
    AROS_LIBFUNC_INIT


    /* Store arguments */
    GPBase->gp_sysBase = sysBase;
    GPBase->gp_seglist = segList;
    
    InitSemaphore(&GPBase->gp_QueueLock);
    NEWLIST(&GPBase->gp_PendingQueue);
    
    return GPBase;
    AROS_LIBFUNC_EXIT
}


AROS_LH3(void, open,
 AROS_LHA(struct IORequest *, ioreq, A1),
 AROS_LHA(ULONG,              unitnum, D0),
 AROS_LHA(ULONG,              flags, D1),
	   struct GameportBase *, GPBase, 1, Gameport)
{
    AROS_LIBFUNC_INIT

    /* Keep compiler happy */
    unitnum = 0;
    flags   = 0;

    if(GPBase->gp_eventBuffer == NULL)
    {
	GPBase->gp_eventBuffer = AllocMem(sizeof(UWORD)*GP_BUFFERSIZE, MEMF_ANY);
    }

    /* No memory for key buffer? */
    if(GPBase->gp_eventBuffer == NULL)
    {
	ioreq->io_Error = IOERR_OPENFAIL;
	return;
    }

    if((ioreq->io_Unit = AllocMem(sizeof(GPUnit), MEMF_CLEAR)) == NULL)
    {
	ioreq->io_Error = IOERR_OPENFAIL;
	return;
    }

    
    if (!GPBase->gp_OOPBase)
    {
	GPBase->gp_OOPBase = OpenLibrary(AROSOOP_NAME, 0);
	if (!GPBase->gp_OOPBase)
	{
	    ioreq->io_Error = IOERR_OPENFAIL;
	    return;
	}
    }
    
    if (!HiddMouseAB)
    {
        HiddMouseAB = ObtainAttrBase(IID_Hidd_Mouse);
	if (!HiddMouseAB)
	{
	    ioreq->io_Error = IOERR_OPENFAIL;
	    D(bug("gameport.device: Could not get attrbase\n"));
	    return;
	}
    }
    D(bug("gameport.device: Attrbase: %x\n", HiddMouseAB));
    
    GPBase->gp_Interrupt.is_Node.ln_Type = NT_INTERRUPT;
    GPBase->gp_Interrupt.is_Node.ln_Pri = 0;
    GPBase->gp_Interrupt.is_Data = (APTR)GPBase;
    GPBase->gp_Interrupt.is_Code = sendQueuedEvents;
	
/******* nlorentz: End of stuff added by me ********/



    /* I have one more opener. */
    GPBase->gp_device.dd_Library.lib_OpenCnt++;

    AROS_LIBFUNC_EXIT
}


AROS_LH1(BPTR, close,
 AROS_LHA(struct IORequest *,    ioreq,  A1),
	  struct GameportBase *, GPBase, 2, Gameport)
{
    AROS_LIBFUNC_INIT

    FreeMem(ioreq->io_Unit, sizeof(GPUnit));

    /* Let any following attemps to use the device crash hard. */
    ioreq->io_Device = (struct Device *)-1;

    GPBase->gp_device.dd_Library.lib_OpenCnt--;
    if(GPBase->gp_device.dd_Library.lib_OpenCnt == 0)
	expunge();

    return 0;
    AROS_LIBFUNC_EXIT
}


AROS_LH0(BPTR, expunge, struct GameportBase *, GPBase, 3, Gameport)
{
    AROS_LIBFUNC_INIT

    /* Do not expunge the device. Set the delayed expunge flag and return. */

    /* TODO: Deallocate key buffer. */
    GPBase->gp_device.dd_Library.lib_Flags |= LIBF_DELEXP;
    return 0;
    AROS_LIBFUNC_EXIT
}


AROS_LH0I(int, null, struct GameportBase *, GPBase, 4, Gameport)
{
    AROS_LIBFUNC_INIT
    return 0;
    AROS_LIBFUNC_EXIT
}


AROS_LH1(void, beginio,
 AROS_LHA(struct IORequest *, ioreq, A1),
	   struct GameportBase *, GPBase, 5, Gameport)
{
    AROS_LIBFUNC_INIT

    
    BOOL request_queued = FALSE;
    
    
    D(bug("gpd: beginio(ioreq=%p, cmd=%d)\n", ioreq, ioreq->io_Command));
    
    /* WaitIO will look into this */
    ioreq->io_Message.mn_Node.ln_Type = NT_MESSAGE;
    
    switch (ioreq->io_Command)
    {
    case CMD_CLEAR:
	gpUn->gpu_readPos = GPBase->gp_writePos;
	break;
	
	
    case GPD_READEVENT:
	
	/* TODO */
	
	/* Is it OK to presuppose that __AROS_STRUCTURE_ALIGNMENT is 2^n ? */
	/* if((&ioStd(ioreq)->io_Data != ALIGN(&iostd(ioreq)->io_Data))
	   well, this should actually be more like
	   if((&ioStd(ioreq)->io_Data) < (AROS_PTR_MAX - __AROS_STRUCTURE_ALIGNMENT) ?
	   (&ioStd(ioreq)->io_Data != ALIGN(&ioStd(ioreq)->io_Data)) : &ioStd(ioreq)->io_Data != ... */
	
	/* Hmm... (int) */
	if(((IPTR)(&(ioStd(ioreq)->io_Data)) & (__AROS_STRUCTURE_ALIGNMENT - 1)) != 0)
	{
	    D(bug("gpd: Bad address\n"));
	    ioreq->io_Error = IOERR_BADADDRESS;
	    break;
	}

	D(bug("gpd: Readpos: %d, Writepos: %d\n", gpUn->gpu_readPos, GPBase->gp_writePos));
	
	if(gpUn->gpu_readPos == GPBase->gp_writePos)
	{
	    ioreq->io_Flags &= ~IOF_QUICK;
	    request_queued = TRUE;
	    
	    D(bug("gpd: No mouse events, putting request in queue\n"));
	    
	    Disable();
	    gpUn->gpu_flags |= GBUF_PENDING;
	    AddTail((struct List *)&GPBase->gp_PendingQueue, (struct Node *)ioreq);
	    Enable();
	    
	    break;
	}
	
	D(bug("gpd: Events ready for prosessing\n"));
	
	/* Fill the request with info from buffer */
	fillrequest(ioreq, GPBase);
	
	break;
	
/* nlorentz: This command lets the gameport.device initialize
   the HIDD to use. It must be done this way, because
   HIDDs might be loaded from disk, and gameport.device is
   inited before DOS is up and running.
   The name of the HIDD class is in
   ioStd(rew)->io_Data. Note that maybe we should
   receive a pointer to an allreay created HIDD object instead.
   Also note that the below is just a temporary hack, should
   probably use IRQ HIDD instead to set the IRQ handler.
*/   
	
    case CMD_HIDDINIT: {
        struct TagItem tags[] =
	{
	    { aHidd_Mouse_IrqHandler, 		(IPTR)mouseCallback},
	    { aHidd_Mouse_IrqHandlerData,	(IPTR)GPBase },
	    { TAG_DONE, 0UL }
	};
	D(bug("gameport.device: Received CMD_HIDDINIT, hiddname=\"%s\"\n"
		, (STRPTR)ioStd(ioreq)->io_Data ));

	GPBase->gp_Hidd = NewObject(NULL, (STRPTR)ioStd(ioreq)->io_Data, tags);
	if (!GPBase->gp_Hidd)
	{
	    D(bug("gameport.device: Failed to open hidd\n"));
	    ioreq->io_Error = IOERR_OPENFAIL;
	}
	break; }
        
    
    default:
	ioreq->io_Error = IOERR_NOCMD;
	break;
    }
    
    /* If the quick bit is not set, send the message to the port */
    if(!(ioreq->io_Flags & IOF_QUICK) && !request_queued)
	ReplyMsg(&ioreq->io_Message);
    
    AROS_LIBFUNC_EXIT
}


AROS_LH1(LONG, abortio,
 AROS_LHA(struct IORequest *,    ioreq,  A1),
	  struct GameportBase *, GPBase, 6,  Gameport)
{
    AROS_LIBFUNC_INIT

    if(gpUn->gpu_flags & GBUF_PENDING)
    {
	Disable();
	
/*	Remove((struct Node *)ioreq);		 Correct? Interference? */
	ReplyMsg(&ioreq->io_Message);
	gpUn->gpu_flags &= ~GBUF_PENDING;
	
	Enable();
    }
    return 0;

    AROS_LIBFUNC_EXIT
}




static VOID mouseCallback(struct GameportBase *GPBase, struct pHidd_Mouse_Event *ev)
{
    UWORD amigacode = 0;
    
    D(bug("mouseCallBack(GPBase=%p, button=%d, x=%d, y=%d, type=%d)\n"
    		, GPBase, ev->button, ev->x, ev->y, ev->type));
		
    /* Convert the event */
    switch (ev->button)
    {
	    
	case vHidd_Mouse_Button1:
	    amigacode = IECODE_LBUTTON;
	    break;
	
	case vHidd_Mouse_Button2:
	    amigacode = IECODE_RBUTTON;
	    break;
	    
    }
    
    switch (ev->type)
    {
    	case vHidd_Mouse_Release:
	    amigacode |= IECODE_UP_PREFIX;
	    break;
	    
	case vHidd_Mouse_Motion:
	    amigacode = IECODE_NOBUTTON;
	    break;
    }
    
    GPBase->gp_eventBuffer[GPBase->gp_writePos ++] = amigacode;
    GPBase->gp_eventBuffer[GPBase->gp_writePos ++] = ev->x;
    GPBase->gp_eventBuffer[GPBase->gp_writePos ++] = ev->y;
    

D(bug("Wrote to buffer\n"));
    if(GPBase->gp_writePos == GP_NUMELEMENTS)
	GPBase->gp_writePos = 0;


    if(!IsListEmpty(&GPBase->gp_PendingQueue))
    {
D(bug("doing software irq, node type=%d\n", GPBase->gp_Interrupt.is_Node.ln_Type));

#warning Cause() does not seem to work.
/* It seems Cause() doesn't work, so I just call it directly for now 
	Cause(&GPBase->gp_Interrupt);
*/
	AROS_UFC3(void, sendQueuedEvents,
		AROS_UFCA(struct GameportBase *, GPBase, A1),
		AROS_UFCA(APTR, sendQueuedEvents, A5),
		AROS_UFCA(struct ExecBase *, SysBase, A6));
	
    }
}


     

/* nlorentz: Software interrupt to be called when keys are received
Copied and pasted from the function above */
#undef SysBase
AROS_UFH3(static VOID, sendQueuedEvents,
    AROS_UFHA(struct GameportBase *, GPBase, A1),
    AROS_UFHA(APTR, thisfunc, A5),
    AROS_UFHA(struct ExecBase *, SysBase, A6))
{
    /* Broadcast keys */
    struct IORequest *ioreq;
    struct List *pendingList;
    

    pendingList = (struct List *)&GPBase->gp_PendingQueue;

    D(bug("Inside software irq\n"));

    ForeachNode(pendingList, ioreq)
    {
        BOOL moreevents;
	
        D(bug("Replying msg\n"));
	moreevents = fillrequest(ioreq, GPBase);
 	ReplyMsg((struct Message *)&ioreq->io_Message);
/*	Remove((struct Node *)ioreq); Should NOT be done as this is done in ReplyMsg */
	gpUn->gpu_flags &= ~GBUF_PENDING;
	
	if (!moreevents)
	    break;
    }
}

/* When this function is called, there *must* be at least one event ready for
processing. It returns TRUE as long as there are more events to preocess */

static BOOL fillrequest(struct IORequest *ioreq, struct GameportBase *GPBase)
{
    BOOL moreevents;
    int    i;			     /* Loop variable */
    int    nEvents;                  /* Number of struct InputEvent that there is
					room for in memory pointed to by io_Data */
    struct InputEvent *event;        /* Temporary variable */
	
    /* Number of InputEvents we can store in io_Data */
    nEvents = (ioStd(ioreq)->io_Length)/ALIGN(sizeof(struct InputEvent));
    if(nEvents == 0 && ioStd(ioreq)->io_Length < sizeof(struct InputEvent))
    {
	ioreq->io_Error = IOERR_BADLENGTH;
	D(bug("gpd: Bad length\n"));
	return TRUE; /* Continue processing events */
    }
    else
    {
	nEvents = 1;
    }
	
    event = (struct InputEvent *)(ioStd(ioreq)->io_Data);
	
    for(i = 0; i < nEvents; i++)
    {
    	UWORD code;
	WORD x;
	WORD y;
	
	    
	code = GPBase->gp_eventBuffer[gpUn->gpu_readPos ++];
	x = GPBase->gp_eventBuffer[gpUn->gpu_readPos ++];
	y = GPBase->gp_eventBuffer[gpUn->gpu_readPos ++];

    
	if (gpUn->gpu_readPos == GP_NUMELEMENTS)
	    gpUn->gpu_readPos = 0;
	    
	    
	D(bug("gpd: Adding event of code %d\n", code));
	    
	event->ie_Class = IECLASS_RAWMOUSE;
	event->ie_SubClass = 0; /* Only port 0 for now */
	event->ie_Code = code;
	event->ie_Qualifier = 0; /* Do it simple for now */
	event->ie_Qualifier = 0; /* Do it simple for now */
	    
	event->ie_X = x;
	event->ie_Y = y;
	    
	event->ie_TimeStamp.tv_secs = 0;
	event->ie_TimeStamp.tv_micro = 0;
	    
	    
	/* No more keys in buffer? */
	if(gpUn->gpu_readPos == GPBase->gp_writePos)
	{
	    moreevents = FALSE;
	    break;
	}
	    
	event->ie_NextEvent = (struct InputEvent *) ((UBYTE *)event
				 + ALIGN(sizeof(struct InputEvent)));
    }
    event->ie_NextEvent = NULL;
    
    return moreevents;
    
}

static const char end = 0;

