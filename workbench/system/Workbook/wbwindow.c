/*
    Copyright © 2011, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Workbook Window Class
    Lang: english
*/

#define DEBUG 0
#include <aros/debug.h>

#include <string.h>
#include <limits.h>
#include <intuition/icclass.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/gadtools.h>
#include <proto/workbench.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/icon.h>

#include <intuition/classusr.h>
#include <libraries/gadtools.h>
#include <exec/rawfmt.h>


#include "workbook_intern.h"
#include "workbook_menu.h"
#include "classes.h"

static inline WORD max(WORD a, WORD b)
{
    return (a > b) ? a : b;
}

struct wbWindow {
    STRPTR         Path;
    BPTR           Lock;
    struct Window *Window;
    struct Menu   *Menu;
    Object        *ScrollH;
    Object        *ScrollV;
    Object        *Area;      /* Virual area of icons */
    Object        *Set;       /* Set of icons */

    ULONG          Flags;
    IPTR           Tick;

    /* Temporary path buffer */
    TEXT           PathBuffer[PATH_MAX];
    TEXT           ScreenTitle[256];
};

#define WBWF_USERPORT   (1 << 0)    /* Window has a custom port */

#define Broken NM_ITEMDISABLED |

static const struct NewMenu WBWindow_menu[] =  {
    WBMENU_TITLE(WBMENU_WB),
        WBMENU_ITEM(WBMENU_WB_BACKDROP),
        WBMENU_ITEM(WBMENU_WB_EXECUTE),
        WBMENU_ITEM(WBMENU_WB_SHELL),
        WBMENU_ITEM(WBMENU_WB_ABOUT),
        WBMENU_BAR,
        WBMENU_ITEM(WBMENU_WB_QUIT),
        WBMENU_ITEM(WBMENU_WB_SHUTDOWN),
    WBMENU_TITLE(WBMENU_WN),
        WBMENU_ITEM(WBMENU_WN_NEW_DRAWER),
        WBMENU_ITEM(WBMENU_WN_OPEN_PARENT),
        WBMENU_ITEM(WBMENU_WN_UPDATE),
        WBMENU_ITEM(WBMENU_WN_SELECT_ALL),
        WBMENU_ITEM(WBMENU_WN_SELECT_NONE),
        WBMENU_SUBTITLE(WBMENU_WN__SNAP),
            WBMENU_SUBITEM(WBMENU_WN__SNAP_WINDOW),
            WBMENU_SUBITEM(WBMENU_WN__SNAP_ALL),
        WBMENU_SUBTITLE(WBMENU_WN__SHOW),
            WBMENU_SUBITEM(WBMENU_WN__SHOW_ICONS),
            WBMENU_SUBITEM(WBMENU_WN__SHOW_ALL),
        WBMENU_SUBTITLE(WBMENU_WN__VIEW),
            WBMENU_SUBITEM(WBMENU_WN__VIEW_ICON),
            WBMENU_SUBITEM(WBMENU_WN__VIEW_DETAILS),
    WBMENU_TITLE(WBMENU_IC),
        WBMENU_ITEM(WBMENU_IC_OPEN),
        WBMENU_ITEM(WBMENU_IC_COPY),
        WBMENU_ITEM(WBMENU_IC_RENAME),
        WBMENU_ITEM(WBMENU_IC_INFO),
        WBMENU_BAR,
        WBMENU_ITEM(WBMENU_IC_SNAPSHOT),
        WBMENU_ITEM(WBMENU_IC_UNSNAPSHOT),
        WBMENU_ITEM(WBMENU_IC_LEAVE_OUT),
        WBMENU_ITEM(WBMENU_IC_PUT_AWAY),
        WBMENU_BAR,
        WBMENU_ITEM(WBMENU_IC_DELETE),
        WBMENU_ITEM(WBMENU_IC_FORMAT),
        WBMENU_ITEM(WBMENU_IC_EMPTY_TRASH),
    { NM_END },
};

static BOOL wbMenuEnable(Class *cl, Object *obj, int id, BOOL onoff)
{
    struct WorkbookBase *wb = (APTR)cl->cl_UserData;
    struct wbWindow *my = INST_DATA(cl, obj);
    int i, menu = -1, item = -1, sub = -1;
    UWORD MenuNumber = MENUNULL;
    BOOL rc = FALSE;

    for (i = 0; WBWindow_menu[i].nm_Type != NM_END; i++) {
    	const struct NewMenu *nm = &WBWindow_menu[i];

    	switch (nm->nm_Type) {
    	case NM_TITLE:
    	    menu++;
    	    item = -1;
    	    sub = -1;
    	    break;
    	case IM_ITEM:
    	case NM_ITEM:
    	    item++;
    	    sub = -1;
    	    break;
    	case IM_SUB:
    	case NM_SUB:
    	    sub++;
    	    break;
    	}

    	if (nm->nm_UserData == (APTR)id) {
    	    MenuNumber = FULLMENUNUM(menu, item, sub);
    	    break;
    	}
    }

    if (MenuNumber != MENUNULL) {
    	if (onoff)
    	    OnMenu(my->Window, MenuNumber);
    	else
    	    OffMenu(my->Window, MenuNumber);

    	rc = TRUE;
    }

    return rc;
}

AROS_UFH3(ULONG, wbIgnoreInfo_Hook,
    AROS_UFHA(struct Hook*, hook, A0),
    AROS_UFHA(struct ExAllData*, ead, A2),
    AROS_UFHA(LONG *, type, A1))
{
    int i;

    AROS_USERFUNC_INIT

    i = strlen(ead->ed_Name);
    if (i >= 5 && stricmp(&ead->ed_Name[i-5], ".info") == 0)
    	return FALSE;
    if (stricmp(ead->ed_Name, ".backdrop") == 0)
    	return FALSE;
    return TRUE;
    
    AROS_USERFUNC_EXIT
}

static void wbAddFiles(Class *cl, Object *obj, CONST_STRPTR path)
{
    struct WorkbookBase *wb = (APTR)cl->cl_UserData;
    struct wbWindow *my = INST_DATA(cl, obj);
    struct ExAllControl *eac;
    struct ExAllData *ead;
    BPTR lock;
    const ULONG eadSize = sizeof(struct ExAllData) + 1024;

    lock = Lock(path, SHARED_LOCK);
    if (lock != BNULL) {
    	STRPTR text,cp;
    	ULONG size = sizeof(my->PathBuffer);
    	text = &my->PathBuffer[0];
    	cp = stpcpy(text, path);

    	ead = AllocVec(eadSize, MEMF_CLEAR);
    	if (ead != NULL) {
    	    eac = AllocDosObject(DOS_EXALLCONTROL, NULL);
    	    if (eac != NULL) {
    	    	struct Hook hook;
    	    	BOOL more = TRUE;

    	    	hook.h_Entry = wbIgnoreInfo_Hook;

    	    	eac->eac_MatchFunc = &hook;
    		while (more) {
    		    struct ExAllData *tmp = ead;
    		    int i;

    		    more = ExAll(lock, ead, eadSize, ED_NAME, eac);
    		    for (i = 0; i < eac->eac_Entries; i++, tmp=tmp->ed_Next) {
    		    	*cp = 0;
    		    	AddPart(text, tmp->ed_Name, size);
    			Object *iobj = NewObject(WBIcon, NULL,
    					   WBIA_File, text,
    					   WBIA_Label, tmp->ed_Name,
    					   TAG_END);
    			if (iobj != NULL)
    			    DoMethod(my->Set, OM_ADDMEMBER, iobj);
    		    }
    		}
    		FreeDosObject(DOS_EXALLCONTROL, eac);
    	    }
    	    FreeVec(ead);
    	}
    	UnLock(lock);
    } else {
    	D(bug("Can't lock %s\n", path));
    }
}

static void wbAddVolumeIcons(Class *cl, Object *obj)
{
    struct WorkbookBase *wb = (APTR)cl->cl_UserData;
    struct wbWindow *my = INST_DATA(cl, obj);
    struct DosList *dl;
    char text[NAME_MAX];

    /* Add all the DOS disks */
    dl = LockDosList(LDF_VOLUMES | LDF_READ);

    if (dl != BNULL) {
    	struct DosList *tdl;

    	tdl = dl;
    	while ((tdl = NextDosEntry(tdl, LDF_VOLUMES))) {
    	    Object *iobj;
    	   
    	    CopyMem(AROS_BSTR_ADDR(tdl->dol_Name), text, AROS_BSTR_strlen(tdl->dol_Name));
    	    CopyMem(":",&text[AROS_BSTR_strlen(tdl->dol_Name)],2);
    	    
    	    iobj = NewObject(WBIcon, NULL,
    	    	    WBIA_File, text,
    	    	    WBIA_Label, AROS_BSTR_ADDR(tdl->dol_Name),
    	    	    TAG_END);
    	    D(bug("Volume: %s => %p\n", text, iobj));
    	    if (iobj)
        	DoMethod(my->Set, OM_ADDMEMBER, iobj);
        }
        UnLockDosList(LDF_VOLUMES | LDF_READ);
    }
}

static void wbAddAppIcons(Class *cl, Object *obj)
{
    struct WorkbookBase *wb = (APTR)cl->cl_UserData;
    struct wbWindow *my = INST_DATA(cl, obj);
    struct DiskObject *icon;
    char text[NAME_MAX];


    /* Add all the AppIcons */
    icon = NULL;
    while ((icon = GetNextAppIcon(icon, &text[0]))) {
        Object *iobj = NewObject(WBIcon, NULL,
        	              WBIA_Icon, icon,
        	              TAG_END);
        if (iobj != NULL) {
        	DoMethod(my->Set, OM_ADDMEMBER, iobj);
        }
    }
}

static void wbRedimension(Class *cl, Object *obj)
{
    struct WorkbookBase *wb = (APTR)cl->cl_UserData;
    struct wbWindow *my = INST_DATA(cl, obj);
    struct Window *win = my->Window;
    struct IBox    real;     /* pos & size of the inner window area */

    real.Left = win->BorderLeft;
    real.Top  = win->BorderTop;
    real.Width = win->Width - (win->BorderLeft + win->BorderRight);
    real.Height= win->Height- (win->BorderTop  + win->BorderBottom + 6);

    D(bug("%s: (%d,%d) %dx%d\n", __func__,
    		real.Left, real.Top, real.Width, real.Height));

    SetAttrs(my->Area, GA_Top, real.Top,
    	               GA_Left,  real.Left,
    	               GA_Width, real.Width,
    	               GA_Height, real.Height,
    	               TAG_END);

    SetAttrs(my->ScrollH, PGA_Visible, real.Width,
    	                  GA_Left, real.Left,
    	                  GA_RelBottom, -my->Window->BorderBottom - 5,
    	                  GA_Width, real.Width,
    	                  GA_Height, my->Window->BorderBottom,
    	                  TAG_END);

    SetAttrs(my->ScrollV, PGA_Visible, real.Height,
    	                  GA_RelRight, -my->Window->BorderRight + 5,
    	                  GA_Top, real.Top,
    	                  GA_Width, my->Window->BorderRight,
    	                  GA_Height, real.Height - 10,
    	                  TAG_END);

    {
    	IPTR tot = 0, vis = 0;
    	GetAttr(PGA_Visible, my->ScrollV, &vis);
    	GetAttr(PGA_Total, my->ScrollV, &tot);
    	D(bug("%s: VScroll Total=%d Visible=%d\n", __func__, tot, vis));
    }
}

const struct TagItem scrollv2window[] = {
	{ PGA_Top, WBVA_VirtTop },
	{ TAG_END, 0 },
};
const struct TagItem scrollh2window[] = {
	{ PGA_Top, WBVA_VirtLeft },
	{ TAG_END, 0 },
};
const struct TagItem set2window[] = {
	{ GA_Width, WBVA_VirtWidth },
	{ GA_Height, WBVA_VirtHeight },
	{ TAG_END, 0 },
};


static IPTR WBWindowNew(Class *cl, Object *obj, struct opSet *ops)
{
    struct WorkbookBase *wb = (APTR)cl->cl_UserData;
    struct wbWindow *my;
    struct MsgPort *userport;
    CONST_STRPTR path;
    ULONG idcmp;
    IPTR rc = 0;
    APTR vis;

    rc = DoSuperMethodA(cl, obj, (Msg)ops);
    if (rc == 0)
    	return rc;

    obj = (Object *)rc;
    my = INST_DATA(cl, obj);

    path = (CONST_STRPTR)GetTagData(WBWA_Path, (IPTR)NULL, ops->ops_AttrList);
    if (path == NULL) {
    	my->Lock = BNULL;
    	my->Path = NULL;
    } else {
    	my->Lock = Lock(path, SHARED_LOCK);
    	if (my->Lock == BNULL)
    	    goto error;

    	my->Path = AllocVec(strlen(path)+1, MEMF_ANY);
    	if (my->Path == NULL)
    	    goto error;
    	
    	strcpy(my->Path, path);
    }

    /* Create icon set */
    my->Set = NewObject(WBSet, NULL,
    		ICA_TARGET, (IPTR)obj,
    		ICA_MAP, (IPTR)set2window,
    		TAG_END);

    idcmp = IDCMP_MENUPICK | IDCMP_INTUITICKS;
    if (my->Path == NULL) {
    	my->Window = OpenWindowTags(NULL,
    			WA_IDCMP, 0,
    			WA_Backdrop,    TRUE,
    			WA_WBenchWindow, TRUE,
    			WA_Borderless,  TRUE,
    			WA_Activate,    TRUE,
    			WA_SmartRefresh, TRUE,
    			WA_NewLookMenus, TRUE,
    			WA_PubScreen, NULL,
    			TAG_MORE, ops->ops_AttrList );
    	my->Window->BorderTop = my->Window->WScreen->BarHeight+1;
    } else {
    	struct DiskObject *icon;
    	struct NewWindow *nwin = NULL;

    	icon = GetDiskObjectNew(my->Path);
    	if (icon == NULL)
    	    goto error;

    	if (icon->do_DrawerData) {
    	    nwin = &icon->do_DrawerData->dd_NewWindow;
    	    D(bug("%s: NewWindow %p\n", __func__, nwin));
    	}

    	idcmp |= IDCMP_NEWSIZE | IDCMP_CLOSEWINDOW;
    	my->Window = OpenWindowTags(nwin,
    			WA_IDCMP, 0,
    			WA_MinWidth, 100,
    			WA_MinHeight, 100,
    			WA_MaxWidth, ~0,
    			WA_MaxHeight, ~0,
    			WA_Backdrop, FALSE,
    			WA_WBenchWindow, TRUE,
    			WA_Title,    my->Path,
    			WA_SmartRefresh, TRUE,
    			WA_SizeGadget, TRUE,
    			WA_DragBar, TRUE,
    			WA_DepthGadget, TRUE,
    			WA_CloseGadget, TRUE,
    			WA_Activate, TRUE,
    			WA_NewLookMenus, TRUE,
    			WA_AutoAdjust, TRUE,
    			WA_PubScreen, NULL,
    			TAG_MORE, ops->ops_AttrList );

    	FreeDiskObject(icon);
    }

    if (!my->Window)
    	goto error;

    /* If we want a shared port, do it. */
    userport = (struct MsgPort *)GetTagData(WBWA_UserPort, (IPTR)NULL, ops->ops_AttrList);
    if (userport) {
    	my->Flags |= WBWF_USERPORT;
    	my->Window->UserPort = userport;
    }
    ModifyIDCMP(my->Window, idcmp);

    /* The gadgets' layout will be performed during wbRedimension
     */
    AddGadget(my->Window, (struct Gadget *)(my->Area = NewObject(WBVirtual, NULL,
    		WBVA_Gadget, (IPTR)my->Set,
    		TAG_END)), 0);

    /* Add the verical scrollbar */
    AddGadget(my->Window, (struct Gadget *)(my->ScrollV = NewObject(NULL, "propgclass",
    		GA_RightBorder, TRUE,

    		ICA_TARGET, (IPTR)obj,
    		ICA_MAP, (IPTR)scrollv2window,
    		PGA_Borderless, TRUE,
    		PGA_Freedom, FREEVERT,
    		PGA_NewLook, TRUE,
    		PGA_Total, 1,
    		PGA_Visible, 1,
    		PGA_Top, 0,
    		TAG_END)), 0);
    
    /* Add the horizontal scrollbar */
    AddGadget(my->Window, (struct Gadget *)(my->ScrollH = NewObject(NULL, "propgclass",
    		ICA_TARGET, (IPTR)obj,
    		ICA_MAP, (IPTR)scrollh2window,
    		PGA_Freedom, FREEHORIZ,
    		PGA_NewLook, TRUE,
    		PGA_Total, 1,
    		PGA_Visible, 1,
    		PGA_Top, 0,
    		TAG_END)), 0);

    /* Send first intuitick */
    DoMethod(obj, WBWM_INTUITICK);

    D(bug("BUSY....\n"));
    SetWindowPointer(my->Window, WA_BusyPointer, TRUE, TAG_END);
    if (my->Path == NULL) {
    	wbAddVolumeIcons(cl, obj);
    	wbAddAppIcons(cl, obj);
    } else
    	wbAddFiles(cl, obj, my->Path);
    SetWindowPointer(my->Window, WA_BusyPointer, FALSE, TAG_END);
    D(bug("Not BUSY....\n"));

    wbRedimension(cl, obj);

    my->Menu = CreateMenusA((struct NewMenu *)WBWindow_menu, NULL);
    if (my->Menu == NULL)
    	goto error;

    vis = GetVisualInfo(my->Window->WScreen, TAG_END);
    LayoutMenus(my->Menu, vis, TAG_END);
    FreeVisualInfo(vis);

    SetMenuStrip(my->Window, my->Menu);

    /* Disable opening the parent for root window
     * and disk paths.
     */
    if (my->Lock == BNULL) {
    	wbMenuEnable(cl, obj, WBMENU_ID(WBMENU_WN_OPEN_PARENT), FALSE);
    } else {
    	BPTR lock = ParentDir(my->Lock);
    	if (lock == BNULL) {
    	    wbMenuEnable(cl, obj, WBMENU_ID(WBMENU_WN_OPEN_PARENT), FALSE);
    	} else {
    	    UnLock(lock);
    	}
    }

    RefreshGadgets(my->Window->FirstGadget, my->Window, NULL);

    return rc;

error:
    if (my->Set)
    	DisposeObject(my->Set);

    if (my->Window)
    	CloseWindow(my->Window);

    if (my->Path)
    	FreeVec(my->Path);

    if (my->Lock == BNULL)
    	UnLock(my->Lock);

    DoSuperMethod(cl, obj, OM_DISPOSE, 0);
    return 0;
}

static IPTR WBWindowDispose(Class *cl, Object *obj, Msg msg)
{
    struct WorkbookBase *wb = (APTR)cl->cl_UserData;
    struct wbWindow *my = INST_DATA(cl, obj);

    ClearMenuStrip(my->Window);
    FreeMenus(my->Menu);

    /* If we have a custom user port, be paranoid.
     * See the Autodocs for CloseWindow().
     */
    if (my->Flags & WBWF_USERPORT) {
    	struct IntuiMessage *msg;
    	struct Node *succ;

    	Forbid();
    	msg = (APTR)my->Window->UserPort->mp_MsgList.lh_Head;
    	while ((succ = msg->ExecMessage.mn_Node.ln_Succ )) {
    	    if (msg->IDCMPWindow == my->Window) {
    	    	Remove((APTR)msg);
    	    	ReplyMsg((struct Message *)msg);
    	    }

    	    msg = (struct IntuiMessage *) succ;
    	}

    	my->Window->UserPort = NULL;
    	ModifyIDCMP(my->Window, 0);

    	Permit();
    }

    /* As a side effect, this will close all the
     * gadgets attached to it.
     */
    CloseWindow(my->Window);

    /* .. except for my->Set */
    DisposeObject(my->Set);

    if (my->Path)
    	FreeVec(my->Path);

    if (my->Lock)
    	UnLock(my->Lock);

    return DoSuperMethodA(cl, obj, msg);
}

// OM_GET
static IPTR WBWindowGet(Class *cl, Object *obj, struct opGet *opg)
{
    struct wbWindow *my = INST_DATA(cl, obj);
    IPTR rc = TRUE;

    switch (opg->opg_AttrID) {
    case WBWA_Path:
    	*(opg->opg_Storage) = (IPTR)my->Path;
    	break;
    case WBWA_Window:
    	*(opg->opg_Storage) = (IPTR)my->Window;
    	break;
    default:
    	rc = DoSuperMethodA(cl, obj, (Msg)opg);
    	break;
    }

    return rc;
}

// OM_UPDATE
static IPTR WBWindowUpdate(Class *cl, Object *obj, struct opUpdate *opu)
{
    struct WorkbookBase *wb = (APTR)cl->cl_UserData;
    struct wbWindow *my = INST_DATA(cl, obj);
    const struct TagItem *tstate;
    struct TagItem *tag;
    IPTR rc;

    rc = DoSuperMethodA(cl, obj, (Msg)opu);

    /* Also send these to the Area */
    rc |= DoMethodA(my->Area, (Msg)opu);

    /* Update scrollbars if needed */
    tstate = opu->opu_AttrList;
    while ((tag = NextTagItem(&tstate))) {
    	switch (tag->ti_Tag) {
    	case WBVA_VirtLeft:
    	    rc = TRUE;
    	    break;
    	case WBVA_VirtTop:
    	    rc = TRUE;
    	    break;
    	case WBVA_VirtWidth:
    	    SetAttrs(my->ScrollH, PGA_Total, tag->ti_Data, TAG_END);
    	    rc = TRUE;
    	    break;
    	case WBVA_VirtHeight:
    	    SetAttrs(my->ScrollV, PGA_Total, tag->ti_Data, TAG_END);
    	    rc = TRUE;
    	    break;
    	}
    }

    if (rc && !(opu->opu_Flags & OPUF_INTERIM))
    	RefreshGadgets(my->Window->FirstGadget, my->Window, NULL);

    return rc;
}

// WBWM_NEWSIZE
static IPTR WBWindowNewSize(Class *cl, Object *obj, Msg msg)
{
    struct WorkbookBase *wb = (APTR)cl->cl_UserData;
    struct wbWindow *my = INST_DATA(cl, obj);

    wbRedimension(cl, obj);
    RefreshWindowFrame(my->Window);
    RefreshGadgets(my->Window->FirstGadget, my->Window, NULL);

    return 0;
}

static void NewCLI(struct WorkbookBase *wb, BPTR lock)
{
    BPTR cd;
    IPTR rc;

    if (lock == BNULL)
    	cd = Lock("SYS:", SHARED_LOCK);
    else
    	cd = DupLock(lock);

    D(bug("Lock=%p\n", BADDR(cd)));
    rc = SystemTags("NewShell", NP_CurrentDir, (IPTR)cd, TAG_DONE);
    if (rc == -1)
    	UnLock(cd);
}

// WBWM_MENUPICK
static IPTR WBWindowMenuPick(Class *cl, Object *obj, struct wbwm_MenuPick *wbwmp)
{
    struct WorkbookBase *wb = (APTR)cl->cl_UserData;
    struct wbWindow *my = INST_DATA(cl, obj);
    struct MenuItem *item = wbwmp->wbwmp_MenuItem;
    BPTR lock;
    BOOL rc = TRUE;

    switch (WBMENU_ITEM_ID(item)) {
    case WBMENU_ID(WBMENU_WN_OPEN_PARENT):
    	lock = ParentDir(my->Lock);
    	if (NameFromLock(lock, my->PathBuffer, sizeof(my->PathBuffer))) {
    	    OpenWorkbenchObject(my->PathBuffer, TAG_END);
    	}
    	UnLock(lock);
    	break;
    case WBMENU_ID(WBMENU_WB_SHELL):
    	NewCLI(wb, my->Lock);
    	break;
    default:
    	rc = FALSE;
    	break;
    }

    return rc;
}

// WBWM_INTUITICK
static IPTR WBWindowIntuiTick(Class *cl, Object *obj, Msg msg)
{
    struct WorkbookBase *wb = (APTR)cl->cl_UserData;
    struct wbWindow *my = INST_DATA(cl, obj);
    IPTR rc = FALSE;

    if (my->Tick == 0) {
	IPTR val[5];

	val[0] = WB_VERSION;
	val[1] = WB_REVISION;
	val[2] = AvailMem(MEMF_CHIP) / 1024;
	val[3] = AvailMem(MEMF_FAST) / 1024;
	val[4] = AvailMem(MEMF_ANY) / 1024;

	/* Update the window's title */
	RawDoFmt("Workbook %ld.%ld  Chip: %ldk, Fast: %ldk, Any: %ldk", val, 
		 RAWFMTFUNC_STRING, my->ScreenTitle);

	SetWindowTitles(my->Window, (CONST_STRPTR)-1, my->ScreenTitle);
	rc = TRUE;
    }

    /* Approx 10 IntuiTicks per second */
    my->Tick = (my->Tick + 1) % 10;

    return rc;
}

static IPTR dispatcher(Class *cl, Object *obj, Msg msg)
{
    IPTR rc = 0;

    switch (msg->MethodID) {
    case OM_NEW:         rc = WBWindowNew(cl, obj, (APTR)msg); break;
    case OM_DISPOSE:     rc = WBWindowDispose(cl, obj, (APTR)msg); break;
    case OM_GET:         rc = WBWindowGet(cl, obj, (APTR)msg); break;
    case OM_UPDATE:      rc = WBWindowUpdate(cl, obj, (APTR)msg); break;
    case WBWM_NEWSIZE:   rc = WBWindowNewSize(cl, obj, (APTR)msg); break;
    case WBWM_MENUPICK:  rc = WBWindowMenuPick(cl, obj, (APTR)msg); break;
    case WBWM_INTUITICK: rc = WBWindowIntuiTick(cl, obj, (APTR)msg); break;
    default:             rc = DoSuperMethodA(cl, obj, msg); break;
    }

    return rc;
}

Class *WBWindow_MakeClass(struct WorkbookBase *wb)
{
    Class *cl;

    cl = MakeClass( NULL, "rootclass", NULL,
                    sizeof(struct wbWindow),
                    0);
    if (cl != NULL) {
    	cl->cl_Dispatcher.h_Entry = HookEntry;
    	cl->cl_Dispatcher.h_SubEntry = dispatcher;
    	cl->cl_Dispatcher.h_Data = NULL;
    	cl->cl_UserData = (IPTR)wb;
    }

    return cl;
}
