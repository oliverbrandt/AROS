#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <libraries/commodities.h>
#include <libraries/locale.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/layers.h>
#include <proto/commodities.h>
#include <proto/locale.h>
#include <proto/alib.h>

#include <aros/debug.h>

#include <stdio.h>
#include <stdlib.h>


#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
#define CATCOMP_ARRAY

#include "commodities_strings.h"

/************************************************************************************/

UBYTE version[] = "$VER: Opaque 0.2 (13.10.2001)";

#define ARG_TEMPLATE "CX_PRIORITY=PRI/N/K"

#define ARG_PRI   0
#define NUM_ARGS  1

/* Libraries to open */
struct LibTable
{
 APTR   lT_Library;
 STRPTR lT_Name;
 ULONG  lT_Version;
}
libTable[] =
{
 { &IntuitionBase,      "intuition.library",    39L},
 { &LayersBase,         "layers.library",       39L},
 { &CxBase,             "commodities.library",  39L},
 { NULL }
};

struct IntuitionBase *IntuitionBase = NULL;
struct Library *LayersBase = NULL;
struct Library *CxBase = NULL;

static struct NewBroker nb =
{
   NB_VERSION,
   NULL,
   NULL,
   NULL,
   NBU_NOTIFY | NBU_UNIQUE, 
   0,
   0,
   NULL,                             
   0 
};

static struct Catalog *catalogPtr;
static struct MsgPort *cxport;
static struct Window *actionwin;
static struct Task *maintask;

static struct RDArgs *myargs;
static CxObj *cxbroker, *cxcust;
static ULONG cxmask, actionmask;
static WORD  winoffx, winoffy;
static UBYTE actionsig;
static BOOL quitme, disabled;

static LONG args[NUM_ARGS];
static char s[256];

/************************************************************************************/

STRPTR getCatalog(struct Catalog *catalogPtr, ULONG id)
{
    STRPTR string;

    if(catalogPtr)
        string = GetCatalogStr(catalogPtr, id, CatCompArray[id].cca_Str);
    else
        string = CatCompArray[id].cca_Str;

    return(string);
}

/************************************************************************************/

static void Cleanup(char *msg)
{
    struct Message *cxmsg;
    struct LibTable *tmpLibTable = libTable;
    
    if (msg)
    {
	printf("%s", msg);
    }

    if(CxBase)
    {
	if (cxbroker) DeleteCxObjAll(cxbroker);
	if (cxport)
	{
	    while((cxmsg = GetMsg(cxport)))
	    {
		ReplyMsg(cxmsg);
	    }

	    DeleteMsgPort(cxport);
	}
    }

    if (myargs) FreeArgs(myargs);

    if(LocaleBase)
    {
	CloseCatalog(catalogPtr);
	CloseLibrary((struct Library *)LocaleBase); /* Passing NULL is valid */
	kprintf("Closed locale.library!\n");
    }
    
    while(tmpLibTable->lT_Name) /* Check for name rather than pointer */
    {
	if((*(struct Library **)tmpLibTable->lT_Library))
	{
	    CloseLibrary((*(struct Library **)tmpLibTable->lT_Library));
	    kprintf("Closed %s!\n", tmpLibTable->lT_Name);
	}

	tmpLibTable++;
    }

    if (actionsig) FreeSignal(actionsig);
    
    exit(0);
}

/************************************************************************************/

static void DosError(void)
{
    Fault(IoErr(),0,s,255);
    Cleanup(s);
}

/************************************************************************************/

static void Init(void)
{
    maintask = FindTask(0);
    actionsig = AllocSignal(-1);
    actionmask = 1L << actionsig;
}

/************************************************************************************/

static void OpenLibs(void)
{
    struct LibTable *tmpLibTable = libTable;
    UBYTE tmpString[128]; /* petah: What if library name plus error message exceeds 128 bytes? */

    if((LocaleBase = (struct LocaleBase *)OpenLibrary("locale.library", 40)))
    {
	catalogPtr = OpenCatalog(NULL, "Sys/Commodities.catalog", OC_BuiltInLanguage, "english", TAG_DONE);
    }
    else
	kprintf("Warning: Can't open locale.library V40!\n");

    while(tmpLibTable->lT_Library)
    {
	if(!((*(struct Library **)tmpLibTable->lT_Library = OpenLibrary(tmpLibTable->lT_Name, tmpLibTable->lT_Version))))
        {
	    sprintf(tmpString, getCatalog(catalogPtr, MSG_CANT_OPEN_LIB), tmpLibTable->lT_Name, tmpLibTable->lT_Version);
	    Cleanup(tmpString);
        }
	else
	    kprintf("Library %s opened!\n", tmpLibTable->lT_Name);

	tmpLibTable++;
    }
}

/************************************************************************************/

static void GetArguments(void)
{
    if (!(myargs = ReadArgs(ARG_TEMPLATE, args, 0)))
    {
	DosError();
    }
    
    if (args[ARG_PRI]) nb.nb_Pri = *(LONG *)args[ARG_PRI];
}

/************************************************************************************/

static void OpaqueAction(CxMsg *msg,CxObj *obj)
{
    static BOOL dragging = FALSE;
    
    struct InputEvent *ie = (struct InputEvent *)CxMsgData(msg);
    struct Screen *scr;
    
    if (ie->ie_Class == IECLASS_RAWMOUSE)
    {
        switch(ie->ie_Code)
	{
	    case SELECTDOWN:
	    	if (IntuitionBase->ActiveWindow)
		{
		    scr = IntuitionBase->ActiveWindow->WScreen;
		} else {
		    scr = IntuitionBase->ActiveScreen;
		}
		
	        if (!dragging && scr)
		{
		    struct Layer *lay = WhichLayer(&scr->LayerInfo, scr->MouseX, scr->MouseY);
		    struct Window *win = NULL;
		    
		    if (lay) win = (struct Window *)lay->Window;
		
		    if (win && !(ie->ie_Qualifier & (IEQUALIFIER_LCOMMAND | IEQUALIFIER_RCOMMAND)))
		    {
		        struct Gadget *gad;
			struct Window *newwin = NULL;
			
			for(gad = win->FirstGadget; gad; gad = gad->NextGadget)
			{
			    /* FIXME: does not handle app made dragging gadgets in
			       GZZ innerlayer or boopsi gadgets with special GM_HITTEST
			       method correctly! */
			       
			    if ((!(gad->Flags & GFLG_DISABLED)) &&
			        ((gad->GadgetType & GTYP_SYSTYPEMASK) == GTYP_WDRAGGING))
			    {
				WORD x = gad->LeftEdge;
				WORD y = gad->TopEdge;
				WORD w = gad->Width;
				WORD h = gad->Height;
			    
			        if (gad->Flags & GFLG_RELRIGHT)  x += win->Width  - 1;
				if (gad->Flags & GFLG_RELBOTTOM) y += win->Height - 1;
				if (gad->Flags & GFLG_RELWIDTH)  w += win->Width;
				if (gad->Flags & GFLG_RELHEIGHT) h += win->Height;
				
				if ((win->MouseX >= x) &&
				    (win->MouseY >= y) &&
				    (win->MouseX < x + w) &&
				    (win->MouseY < y + h))
				{
				    /* found dragging gadget */
				    newwin = win;
				    break;
				}
			    }
			    
			} /* for(gad = win->FirstGadget; gad; gad = gad->NextGadget) */
			
			win = newwin;
			
		    } /* if (win && !(ie->ie_Qualifier & (IEQUALIFIER_LCOMMAND | IEQUALIFIER_RCOMMAND))) */
		    
		    if (win)
		    {				   
			dragging = TRUE;
			if (IntuitionBase->ActiveWindow != win) ActivateWindow(win);
			actionwin = win;
			winoffx = win->WScreen->MouseX - win->LeftEdge;
			winoffy = win->WScreen->MouseY - win->TopEdge;
			DisposeCxMsg(msg);
		    }
		    
		} /* if (!dragging && scr) */
		break;
		
	    case SELECTUP:
	        if (dragging)
		{
		    dragging = FALSE;
		    DisposeCxMsg(msg);
		}
		break;
		
	    case IECODE_NOBUTTON:
	        if (dragging)
		{
		    Signal(maintask, actionmask);
		}
		break;
		
	} /* switch(ie->ie_Code) */
	
    } /* if (ie->ie_Class == IECLASS_RAWMOUSE) */
}

/************************************************************************************/

static void InitCX(void)
{
    if (!(cxport = CreateMsgPort()))
    {
        Cleanup(getCatalog(catalogPtr, MSG_CANT_CREATE_MSGPORT));
    }
    
    nb.nb_Port = cxport;
    
    cxmask = 1L << cxport->mp_SigBit;
    
    if (!(cxbroker = CxBroker(&nb, 0)))
    {
        Cleanup(getCatalog(catalogPtr, MSG_CANT_CREATE_BROKER));
    }
    
    if (!(cxcust = CxCustom(OpaqueAction, 0)))
    {
        Cleanup(getCatalog(catalogPtr, MSG_CANT_CREATE_CUSTOM));
    }
    
    AttachCxObj(cxbroker, cxcust);
    ActivateCxObj(cxbroker, 1);
    
}

/************************************************************************************/

static void HandleAction(void)
{
    WORD newx = actionwin->WScreen->MouseX - winoffx; 
    WORD newy = actionwin->WScreen->MouseY - winoffy;
    
//    MoveWindow(actionwin, newx - actionwin->LeftEdge, newy - actionwin->TopEdge);
    ChangeWindowBox(actionwin, newx, newy, actionwin->Width, actionwin->Height);
}

/************************************************************************************/

static void HandleCx(void)
{
    CxMsg *msg;
    while((msg = (CxMsg *)GetMsg(cxport)))
    {
       switch(CxMsgType(msg))
       {
	    case CXM_COMMAND:
               switch(CxMsgID(msg))
               {
        	  case CXCMD_DISABLE:
        	     ActivateCxObj(cxbroker,0L);
		     disabled = TRUE;
        	     break;

        	  case CXCMD_ENABLE:
        	     ActivateCxObj(cxbroker,1L);
		     disabled = FALSE;
        	     break;

        	  case CXCMD_KILL:
        	     quitme = TRUE;
        	     break;

               } /* switch(CxMsgID(msg)) */
               break;

       } /* switch (CxMsgType(msg))*/
       
       ReplyMsg((struct Message *)msg);
       
   } /* while((msg = (CxMsg *)GetMsg(cxport))) */
}

/************************************************************************************/

static void HandleAll(void)
{
    ULONG sigs;
    
    while(!quitme)
    {
        sigs = Wait(cxmask | actionmask | SIGBREAKF_CTRL_C);
	
	if (sigs & cxmask) HandleCx();
	if (sigs & actionmask) HandleAction();
	if (sigs & SIGBREAKF_CTRL_C) quitme = TRUE;
	
    } /* while(!quitme) */
    
}

/************************************************************************************/

int main(void)
{
    Init();
    OpenLibs();

    nb.nb_Name = getCatalog(catalogPtr, MSG_OPAQUE_CXNAME);
    nb.nb_Title = getCatalog(catalogPtr, MSG_OPAQUE_CXTITLE);
    nb.nb_Descr = getCatalog(catalogPtr, MSG_OPAQUE_CXDESCR);

    GetArguments();
    InitCX();
    HandleAll();
    Cleanup(0);
    return 0;
}


/************************************************************************************/
