/*
    Copyright � 2002, The AROS Development Team.
    All rights reserved.

    $Id$
*/

#include <string.h>

#include <exec/memory.h>

#include <libraries/asl.h>
#include <libraries/mui.h>
#include <prefs/prefhdr.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/iffparse.h>

#ifdef __AROS__
/*  #define DEBUG 1 */
/*  #include <aros/debug.h> */
#include <proto/muimaster.h>
#endif

#include "buttonsp.h"
#include "groupsp.h"
#include "windowp.h"
#include "zunestuff.h"

struct Library *MUIMasterBase;

void load_prefs(char *filename);
void save_prefs(BOOL envarc);

#ifndef __AROS__

/* On AmigaOS we build a fake library base, because it's not compiled as sharedlibrary yet */
#include "muimaster_intern.h"

int open_muimaster(void)
{
    static struct MUIMasterBase_intern MUIMasterBase_instance;
    MUIMasterBase = (struct Library*)&MUIMasterBase_instance;

    MUIMasterBase_instance.sysbase = *((struct ExecBase **)4);
    MUIMasterBase_instance.dosbase = (void*)OpenLibrary("dos.library",37);
    MUIMasterBase_instance.utilitybase = (void*)OpenLibrary("utility.library",37);
    MUIMasterBase_instance.aslbase = OpenLibrary("asl.library",37);
    MUIMasterBase_instance.gfxbase = (void*)OpenLibrary("graphics.library",37);
    MUIMasterBase_instance.layersbase = OpenLibrary("layers.library",37);
    MUIMasterBase_instance.intuibase = (void*)OpenLibrary("intuition.library",37);
    MUIMasterBase_instance.cxbase = OpenLibrary("commodities.library",37);
    MUIMasterBase_instance.keymapbase = OpenLibrary("keymap.library",37);
    MUIMasterBase_instance.gadtoolsbase = OpenLibrary("gadtools.library",37);
    MUIMasterBase_instance.iffparsebase = OpenLibrary("iffparse.library",37);
    MUIMasterBase_instance.diskfontbase = OpenLibrary("diskfont.library",37);
    __zune_prefs_init(&__zprefs);
    InitSemaphore(&MUIMB(MUIMasterBase)->ZuneSemaphore);
    return 1;
}

void close_muimaster(void)
{
}

#else

int open_muimaster(void)
{
    if ((MUIMasterBase = OpenLibrary("muimaster.library", 0)))
	return 1;
    return 0;
}

void close_muimaster(void)
{
    if (MUIMasterBase)
	CloseLibrary(MUIMasterBase);
}

#endif


/****************************************************************
 ImageClipboard class (MUIC_Popimage)
*****************************************************************/
struct ImageClipboardData
{
    LONG dummy;
};

static ULONG ImageClipboard_DragQuery(struct IClass *cl, Object *obj, struct MUIP_DragQuery *msg)
{
    IPTR dummy;

/*      D(bug("ImageClipboard_DragQuery\n")); */
    if (msg->obj == obj)
	return MUIV_DragQuery_Refuse;
/*      D(bug("ImageClipboard_DragQuery 2\n")); */
    if (!get(msg->obj, MUIA_Imagedisplay_Spec, &dummy))
	return MUIV_DragQuery_Refuse;
/*      D(bug("ImageClipboard_DragQuery accepted\n")); */
    return MUIV_DragQuery_Accept;
}

static ULONG ImageClipboard_DragDrop(struct IClass *cl, Object *obj, struct MUIP_DragDrop *msg)
{
    IPTR spec;

/*      D(bug("ImageClipboard_DragDrop\n")); */
    get(msg->obj, MUIA_Imagedisplay_Spec, &spec);
    set(obj, MUIA_Imagedisplay_Spec, spec);
    return 0;
}

BOOPSI_DISPATCHER(IPTR, ImageClipboard_Dispatcher, cl, obj, msg)
{
    switch (msg->MethodID)
    {
	case MUIM_DragQuery:
	    return ImageClipboard_DragQuery(cl, obj, (APTR)msg);
	case MUIM_DragDrop:
	    return ImageClipboard_DragDrop(cl, obj, (APTR)msg);
    }
    return DoSuperMethodA(cl, obj, msg);
}



/****************************************************************
 Open needed libraries
*****************************************************************/
int open_libs(void)
{
    if (open_muimaster())
	return 1;

    return 0;
}

/****************************************************************
 Close opened libraries
*****************************************************************/
void close_libs(void)
{
    close_muimaster();
}

struct Hook hook_standard;

static Object *app;
static Object *open_menuitem;
static Object *append_menuitem;
static Object *saveas_menuitem;
static Object *aboutzune_menuitem;
static Object *quit_menuitem;

static Object *main_wnd;
static Object *main_page_list;
static Object *main_page_group; /* contains the selelected group */
static Object *main_page_group_displayed; /* The current displayed group */
static Object *main_page_space; /* a space object */

struct MUI_CustomClass *CL_ImageClipboard = NULL;

int open_classes(void)
{
     CL_ImageClipboard = MUI_CreateCustomClass(NULL, MUIC_Popimage, NULL,
					      sizeof(struct ImageClipboardData),
					      ImageClipboard_Dispatcher);
     if (CL_ImageClipboard)
	 return 1;
     return 0;
}

void close_classes(void)
{
    if (CL_ImageClipboard)
    {
	MUI_DeleteCustomClass(CL_ImageClipboard);
    }
}


struct page_entry
{
    char *name;
    struct MUI_CustomClass *cl; /* The class pointer,  maybe NULL */
    Object *group;  /* The group which should be is displayed, maybe NULL */
    const struct __MUIBuiltinClass *desc;
};

struct page_entry main_page_entries[] =
{
/*      {"Info",NULL,NULL,NULL}, */
/*      {"System",NULL,NULL,NULL}, */
    {"Windows",NULL,NULL,&_MUIP_Windows_desc},
    {"Buttons",NULL,NULL,&_MUIP_Buttons_desc},
    {"Groups",NULL,NULL,&_MUIP_Groups_desc},
    {NULL,NULL,NULL,NULL},
};

struct MUI_CustomClass *create_class(const struct __MUIBuiltinClass *desc)
{
    return MUI_CreateCustomClass(NULL,MUIC_Settingsgroup,NULL,desc->datasize,desc->dispatcher);
}

/****************************************************************
 Our standard hook function, for easy call backs
*****************************************************************/
static void hook_func_standard(struct Hook *h, void *dummy, ULONG * funcptr)
{
    void (*func) (ULONG *) = (void (*)(ULONG *)) (*funcptr);
    if (func) func(funcptr + 1);
}

/****************************************************************
 The display function for the page listview
*****************************************************************/
static void main_page_list_display(struct Hook *h, char **strings, struct page_entry *entry)
{
    if (entry)
    {
        strings[0] = entry->name;
    }
}


/****************************************************************
 A new entry has been selected
*****************************************************************/
void main_page_active(void)
{
    int new_active = xget(main_page_list,MUIA_List_Active);
    Object *new_group;

    if (new_active == -1) new_group = main_page_space;
    else
    {
	new_group = main_page_entries[new_active].group;
	if (!new_group) new_group = main_page_space;
    }

    if (new_group == main_page_group_displayed) return;

    DoMethod(main_page_group,MUIM_Group_InitChange);
    DoMethod(main_page_group,OM_REMMEMBER,main_page_group_displayed);
    DoMethod(main_page_group,OM_ADDMEMBER,new_group);
    DoMethod(main_page_group,MUIM_Group_ExitChange);
    main_page_group_displayed = new_group;
}

/****************************************************************
 Save pressed
*****************************************************************/
void main_save_pressed(void)
{
    save_prefs(TRUE);
    DoMethod(app, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
}

/****************************************************************
 Use pressed
*****************************************************************/
void main_use_pressed(void)
{
    save_prefs(FALSE);
    DoMethod(app, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
}

/****************************************************************
 Allocalte resources for gui
*****************************************************************/
int init_gui(void)
{
    Object *save_button;
    Object *use_button;
    Object *test_button;
    Object *cancel_button;

    static struct Hook page_display_hook;

    hook_standard.h_Entry = HookEntry;
    hook_standard.h_SubEntry = (APTR)hook_func_standard;
    page_display_hook.h_Entry = HookEntry;
    page_display_hook.h_SubEntry = (APTR)main_page_list_display;

    app = ApplicationObject,
	MUIA_Application_Menustrip, MenuitemObject,
	    MUIA_Family_Child, MenuitemObject,
	    	MUIA_Menuitem_Title, "Project",
	    	MUIA_Family_Child, open_menuitem = MenuitemObject, MUIA_Menuitem_Title, "Open...", MUIA_Menuitem_Shortcut, "O", End,
	    	MUIA_Family_Child, append_menuitem = MenuitemObject, MUIA_Menuitem_Title, "Append...", End,
	    	MUIA_Family_Child, saveas_menuitem = MenuitemObject, MUIA_Menuitem_Title, "Save As...", MUIA_Menuitem_Shortcut, "A", End,
	    	MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, ~0, End,
	    	MUIA_Family_Child, aboutzune_menuitem = MenuitemObject, MUIA_Menuitem_Title, "About Zune...", MUIA_Menuitem_Shortcut, "?", End,
	    	MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, ~0, End,
	    	MUIA_Family_Child, quit_menuitem = MenuitemObject, MUIA_Menuitem_Title, "Quit", MUIA_Menuitem_Shortcut, "Q", End,
	    	End,
	    End,
    	SubWindow, main_wnd = WindowObject,
    	    MUIA_Window_Title, "Zune - Preferences",
	    MUIA_Window_Activate, TRUE,

	    WindowContents, VGroup,
	        MUIA_Group_VertSpacing, 10,
    	    	Child, HGroup,
	          Child, VGroup,
		    Child, ListviewObject,
			MUIA_Listview_List, main_page_list = ListObject,
			    InputListFrame,
			    MUIA_List_DisplayHook, &page_display_hook,
			    End,
			End,
		    Child, HGroup,
			Child, MUI_NewObject(MUIC_Popimage,
					     MUIA_Disabled, TRUE,
					     MUIA_Draggable, TRUE,
					     MUIA_FixHeight, 20,
					     MUIA_FixWidth, 30,
					     MUIA_Imageadjust_Type, MUIV_Imageadjust_Type_All,
					     TAG_DONE), /* Popframe really */
      	               Child, NewObject(CL_ImageClipboard->mcc_Class, NULL,
					     MUIA_Draggable, TRUE,
					     MUIA_Dropable, TRUE,
					     MUIA_FixHeight, 20,
					     MUIA_FixWidth, 30,
					     MUIA_Imageadjust_Type, MUIV_Imageadjust_Type_All,
					     TAG_DONE),
		        End, /* HGroup */
	            End,
		    Child, VGroup,
	                TextFrame,
			InnerSpacing(4,4),
			Child, main_page_group = VGroup,
			    Child, main_page_group_displayed = main_page_space = HVSpace,
			    End,
			End,
		    End,
		Child, HGroup,
		    Child, save_button = MakeButton("Save"),
		    Child, use_button = MakeButton("Use"),
		    Child, test_button = MakeButton("Test"),
		    Child, cancel_button = MakeButton("Cancel"),
		    End,
		End,
    	    End,
    	End;

    if (app)
    {
    	int i;

	DoMethod(main_wnd, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
	DoMethod(cancel_button, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
	DoMethod(save_button, MUIM_Notify, MUIA_Pressed, FALSE, app, 3, MUIM_CallHook, &hook_standard, main_save_pressed);
	DoMethod(use_button, MUIM_Notify, MUIA_Pressed, FALSE, app, 3, MUIM_CallHook, &hook_standard, main_use_pressed);
//	DoMethod(test_button, MUIM_Notify, MUIA_Pressed, FALSE, app, 3, MUIM_CallHook, &hook_standard, main_test_pressed);
	DoMethod(quit_menuitem, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	for (i=0;main_page_entries[i].name;i++)
	{
	    struct page_entry *p = &main_page_entries[i];
	    if (p->desc)
	    {
		if ((p->cl = create_class(p->desc)))
		{
		    p->group = NewObject(p->cl->mcc_Class,NULL,TAG_DONE);
		}
	    }
	    DoMethod(main_page_list,MUIM_List_InsertSingle,p,MUIV_List_Insert_Bottom);
	}

	DoMethod(main_page_list, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, app, 3, MUIM_CallHook, &hook_standard, main_page_active);

	/* Activate first entry */
	set(main_page_list,MUIA_List_Active,0);

	return 1;
    }
    return 0;
}

/****************************************************************
 Deallocates all gui resources
*****************************************************************/
void deinit_gui(void)
{
    if (app) MUI_DisposeObject(app);
}

/****************************************************************
 Load the given prefs
*****************************************************************/
void load_prefs(char *filename)
{
    Object *configdata = MUI_NewObjectA(MUIC_Configdata, NULL);
    if (configdata)
    {
	int i;

/*  	D(bug("zune::load_prefs: created configdata %p\n", configdata)); */
	DoMethod(configdata, MUIM_Configdata_Load, (IPTR)filename);

        /* Call MUIM_Settingsgroup_ConfigToGadgets for every group */
	for (i=0;main_page_entries[i].name;i++)
	{
	    struct page_entry *p = &main_page_entries[i];
	    if (p->group)
		DoMethod(p->group,MUIM_Settingsgroup_ConfigToGadgets,configdata);
	}

    	MUI_DisposeObject(configdata);
/*  	D(bug("zune::save_prefs: disposed configdata %p\n", configdata)); */
    }
}

/****************************************************************
 Saves the done prefs
*****************************************************************/
void save_prefs(BOOL envarc)
{
    Object *configdata = MUI_NewObjectA(MUIC_Configdata, NULL);

    if (configdata)
    {
	int i;

/*  	D(bug("zune::save_prefs: created configdata %p\n", configdata)); */

        /* Call MUIM_Settingsgroup_GadgetsToConfig for every group */
	for (i=0;main_page_entries[i].name;i++)
	{
	    struct page_entry *p = &main_page_entries[i];
	    if (p->group)
		DoMethod(p->group,MUIM_Settingsgroup_GadgetsToConfig,configdata);
	}


	if (envarc)
	{
	    DoMethod(configdata, MUIM_Configdata_Save, (IPTR)"envarc:zune/global.prefs");
	}
	DoMethod(configdata, MUIM_Configdata_Save, (IPTR)"env:zune/global.prefs");

    	MUI_DisposeObject(configdata);
/*  	D(bug("zune::save_prefs: disposed configdata %p\n", configdata)); */
    }
}


/****************************************************************
 The message loop
*****************************************************************/
void loop(void)
{
    ULONG sigs = 0;

    while((LONG) DoMethod(app, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit)
    {
	if (sigs)
	{
	    sigs = Wait(sigs | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D);
	    if (sigs & SIGBREAKF_CTRL_C) break;
	    if (sigs & SIGBREAKF_CTRL_D) break;
	}
    }
}

/****************************************************************
 The main entry point
*****************************************************************/
int main(void)
{
    if (open_libs())
    {
	if (open_classes())
	{
	    if (init_gui())
	    {
		load_prefs("env:zune/global.prefs");
		set(main_wnd, MUIA_Window_Open, TRUE);
		if (xget(main_wnd,MUIA_Window_Open))
		{
		    loop();
		}

		deinit_gui();
	    }
	    close_classes();
	}
	close_libs();
    }
    return 0;
}
