/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: English
*/

/*********************************************************************************************/

#include "global.h"
#include "version.h"
#include "calendarclass.h"
#include "clockclass.h"

#include <libraries/coolimages.h>
#include <aros/debug.h>

#include <stdlib.h> /* for exit() */
#include <stdio.h>
#include <string.h>

/*********************************************************************************************/

#define ARG_TEMPLATE    "FROM,EDIT/S,USE/S,SAVE/S,PUBSCREEN/K"

#define ARG_FROM        0
#define ARG_EDIT    	1
#define ARG_USE     	2
#define ARG_SAVE      	3
#define ARG_PUBSCREEN   4

#define NUM_ARGS        5

/*********************************************************************************************/

static struct libinfo
{
    APTR        var;
    STRPTR      name;
    WORD        version;
    BOOL    	required;
}
libtable[] =
{
    {&IntuitionBase     , "intuition.library"	 , 39, TRUE  },
    {&GfxBase           , "graphics.library" 	 , 40, TRUE  }, /* 40, because of WriteChunkyPixels */
    {&GadToolsBase      , "gadtools.library" 	 , 39, TRUE  },
    {&UtilityBase       , "utility.library"  	 , 39, TRUE  },
    {&IFFParseBase      , "iffparse.library" 	 , 39, TRUE  },
    {&CyberGfxBase  	, "cybergraphics.library", 39, FALSE },
    {&MUIMasterBase 	, "muimaster.library"	 , 0 , TRUE  },
    {NULL                                            	     }
};

/*********************************************************************************************/

static struct Hook  	    	yearhook, clockhook, activehook;	    	
static struct RDArgs        	*myargs;
static Object	    	    	*activetimestrobj;
static IPTR                 	args[NUM_ARGS];

static STRPTR monthlabels[] =
{
    "January",
    "Frebruary",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
    NULL
};

/*********************************************************************************************/

static void CloseLibs(void);
static void CloseTimerDev(void);
static void FreeArguments(void);
static void FreeVisual(void);
static void KillGUI(void);

/*********************************************************************************************/

WORD ShowMessage(STRPTR title, STRPTR text, STRPTR gadtext)
{
    struct EasyStruct es;
    
    es.es_StructSize   = sizeof(es);
    es.es_Flags        = 0;
    es.es_Title        = title;
    es.es_TextFormat   = text;
    es.es_GadgetFormat = gadtext;
   
    return EasyRequestArgs(win, &es, NULL, NULL);  
}

/*********************************************************************************************/

void Cleanup(STRPTR msg)
{
    if (msg)
    {
	if (IntuitionBase && !((struct Process *)FindTask(NULL))->pr_CLI)
	{
	    ShowMessage("Time", msg, MSG(MSG_OK));     
	}
	else
	{
	    printf("Time: %s\n", msg);
	}
    }
    
    KillGUI();
    FreeVisual();
    FreeArguments();
    CloseTimerDev();
    CloseLibs();
    CleanupLocale();
    
    exit(prog_exitcode);
}


/*********************************************************************************************/

static void OpenLibs(void)
{
    struct libinfo *li;
    
    for(li = libtable; li->var; li++)
    {
	if (!((*(struct Library **)li->var) = OpenLibrary(li->name, li->version)))
	{
	    if (li->required)
	    {
	    	sprintf(s, MSG(MSG_CANT_OPEN_LIB), li->name, li->version);
	    	Cleanup(s);
	    }
	}       
    }
       
}

/*********************************************************************************************/

static void CloseLibs(void)
{
    struct libinfo *li;
    
    for(li = libtable; li->var; li++)
    {
	if (*(struct Library **)li->var) CloseLibrary((*(struct Library **)li->var));
    }
}

/*********************************************************************************************/

static void OpenTimerDev(void)
{
    if ((TimerMP = CreateMsgPort()))
    {
    	if ((TimerIO = (struct timerequest *)CreateIORequest(TimerMP, sizeof(struct timerequest))))
	{
	    if (!OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *)TimerIO, 0))
	    {
	    	TimerBase = (struct Device *)TimerIO->tr_node.io_Device;
	    }
	}
    }
    
    if (!TimerBase)
    {
    	sprintf(s, MSG(MSG_CANT_OPEN_LIB), "timer.device", 0);
	Cleanup(s);
    }
}

/*********************************************************************************************/

static void CloseTimerDev(void)
{
    if (TimerIO)
    {
    	CloseDevice((struct IORequest *)TimerIO);
	DeleteIORequest((struct IORequest *)TimerIO);
    }
    
    if (TimerMP)
    {
    	DeleteMsgPort(TimerMP);
    }
}

/*********************************************************************************************/

static void GetArguments(void)
{
    if (!(myargs = ReadArgs(ARG_TEMPLATE, args, NULL)))
    {
	Fault(IoErr(), 0, s, 256);
	Cleanup(s);
    }
    
    // if (!args[ARG_FROM]) args[ARG_FROM] = (IPTR)CONFIGNAME_ENV;
}

/*********************************************************************************************/

static void FreeArguments(void)
{
    if (myargs) FreeArgs(myargs);
}

/*********************************************************************************************/

static void GetVisual(void)
{
}

/*********************************************************************************************/

static void FreeVisual(void)
{
}

/*********************************************************************************************/

static void YearFunc(struct Hook *hook, Object *obj, IPTR *param)
{
    IPTR year;
    
    get(obj, MUIA_String_Integer, &year);
    
    if ((LONG)*param == -1)
    	year--;
    else if ((LONG)*param == 1)
    	year++;
    
    if (year < 1978)
    {
    	year = 1978;
	nnset(obj, MUIA_String_Integer, year);
    }
    else if (year > 2099)
    {
    	year = 2099;
	nnset(obj, MUIA_String_Integer, year);
    }
    else if (*param)
    {
    	nnset(obj, MUIA_String_Integer, year);
    }
    
    nnset(calobj, MUIA_Calendar_Year, year);

}

/*********************************************************************************************/

static void ActiveFunc(struct Hook *hook, Object *obj, IPTR *param)
{
    Object *active;

    active = *(Object **)param;
    
    if ((active == hourobj) || (active == minobj) || (active == secobj))
    {
    	activetimestrobj = active;
    }

}

/*********************************************************************************************/

static void ClockFunc(struct Hook *hook, Object *obj, IPTR *param)
{
    struct ClockData *cd;
    UBYTE s[3];

    get(obj, MUIA_Clock_Time, (IPTR *)&cd);

    if (*param == 0)
    {
	sprintf(s, "%02d", cd->hour);
	nnset(hourobj, MUIA_String_Contents, s);

	sprintf(s, "%02d", cd->min);  
	nnset(minobj, MUIA_String_Contents, s);

	sprintf(s, "%02d", cd->sec);      
	nnset(secobj, MUIA_String_Contents, s);
    }
    else
    {
    	struct ClockData  cd2;	
    	LONG 	    	  diff = (LONG)*param;
	LONG 	    	  max = 0;
	UWORD 	    	 *cd2_member = NULL;

    	if (diff == 100) diff = 0; /* 100 means string gadget acknowledge */
	
	if (activetimestrobj == hourobj)
	{
	    max = 23;
	    cd2_member = &cd2.hour;
	}
	else if (activetimestrobj == minobj)
	{
	    max = 59;
	    cd2_member = &cd2.min;
	}
	else if (activetimestrobj == secobj)
	{
	    max = 59;
	    cd2_member = &cd2.sec;
	}    
	
	if (max)
	{
	    IPTR number = 0;
	    
	    set(obj, MUIA_Clock_Frozen, TRUE);
	    get(obj, MUIA_Clock_Time, (IPTR)&cd);
	    
	    cd2 = *cd;
	    
	    get(activetimestrobj, MUIA_String_Integer, &number);
	    
	    number += diff;
	    
	    if ((LONG)number < 0)
	    {
	    	number = max;
	    }
	    else if ((LONG)number > max)
	    {
	    	number = 0;
	    }
	    *cd2_member = number;
	  
	    sprintf(s, "%02d", number);
  
	    nnset(activetimestrobj, MUIA_String_Contents, s);
	   
	    set(obj, MUIA_Clock_Time, (IPTR)&cd2); 
	}
    }
    
}

/*********************************************************************************************/

static void MakeGUI(void)
{
    extern struct NewMenu nm;
    
    Object *menu, *yearaddobj, *yearsubobj, *timeaddobj, *timesubobj;
    
    if (!MakeCalendarClass() || !MakeClockClass())
    {
    	Cleanup(MSG(MSG_CANT_CREATE_APP));
    }
    
    yearhook.h_Entry = HookEntry;
    yearhook.h_SubEntry = (HOOKFUNC)YearFunc;
    
    clockhook.h_Entry = HookEntry;
    clockhook.h_SubEntry = (HOOKFUNC)ClockFunc;
    
    activehook.h_Entry = HookEntry;
    activehook.h_SubEntry = (HOOKFUNC)ActiveFunc;
    
    if (LocaleBase)
    {
    	struct Locale *locale = OpenLocale(NULL);
	
	if (locale)
	{
	    WORD i;
	    
	    for(i = 0; i < 12; i++)
	    {
	    	monthlabels[i] = GetLocaleStr(locale, MON_1 + i);
	    }
	    
	    firstweekday = locale->loc_CalendarType;
	    CloseLocale(locale);
	}
	
    }
    
    menu = MUI_MakeObject(MUIO_MenustripNM, &nm, 0);
        	
    app = ApplicationObject,
	MUIA_Application_Title, (IPTR)"Time",
	MUIA_Application_Version, (IPTR)VERSIONSTR,
	MUIA_Application_Copyright, (IPTR)"Copyright � 1995-2002, The AROS Development Team",
	MUIA_Application_Author, (IPTR)"The AROS Development Team",
	MUIA_Application_Description, (IPTR)MSG(MSG_WINTITLE),
	MUIA_Application_Base, (IPTR)"Time",
	menu ? MUIA_Application_Menustrip : TAG_IGNORE, menu,
  	SubWindow, wnd = WindowObject,
	    MUIA_Window_Title, (IPTR)MSG(MSG_WINTITLE),
	    MUIA_Window_ID, MAKE_ID('T','W','I','N'),
	    WindowContents, VGroup,
	    	Child, HGroup, /* Group containing calendar box and clock box */
		    MUIA_Group_SameWidth, TRUE,
		    Child, VGroup, /* Calendar box */
		    	GroupFrame,
			Child, HGroup, /* Month/year row */
		    	    Child, monthobj = MUI_MakeObject(MUIO_Cycle, NULL, monthlabels),
			    Child, HVSpace,
			    Child, yearsubobj = TextObject, /* year [-] gadget */
			    	ButtonFrame,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_Text_Contents, "\033c-",
				MUIA_FixWidthTxt, (IPTR)"+",
				End,
			    Child, yearobj = StringObject, /* year gadget */
				StringFrame,
				MUIA_String_Accept, (IPTR)"0123456789",
				MUIA_FixWidthTxt, (IPTR)"55555",
				End,
			    Child, yearaddobj = TextObject, /* year [-] gadget */
			    	ButtonFrame,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_Text_Contents, "\033c+",
				MUIA_FixWidthTxt, (IPTR)"-",
				End,
			    End,
    			Child, calobj = NewObject(calendarmcc->mcc_Class, NULL, TAG_DONE),
			End,
		    Child, VGroup, /* Clock box */
		    	GroupFrame,
			Child, clockobj = NewObject(clockmcc->mcc_Class, NULL, TAG_DONE),
			Child, HGroup,
			    Child, HVSpace,
			    Child, PageGroup,
			    	Child, HVSpace,
				Child, HGroup,
				    MUIA_Group_Spacing, 0,
				    Child,  TextObject, /* phantom time [-] gadget */
			    		ButtonFrame,
					MUIA_Background, MUII_ButtonBack,
					MUIA_Font, MUIV_Font_Button,
					MUIA_InputMode, MUIV_InputMode_RelVerify,
					MUIA_Text_Contents, "\033c-",
					MUIA_FixWidthTxt, (IPTR)"+",
					End,
				    Child,  TextObject, /* phantom time [+] gadget */
			    		ButtonFrame,
					MUIA_Background, MUII_ButtonBack,
					MUIA_Font, MUIV_Font_Button,
					MUIA_InputMode, MUIV_InputMode_RelVerify,
					MUIA_Text_Contents, "\033c+",
					MUIA_FixWidthTxt, (IPTR)"-",
					End,			    
			    	    End,
				End,
			    Child, hourobj = StringObject, /* hour gadget */
			    	StringFrame,
				MUIA_String_Accept, (IPTR)"0123456789",
				MUIA_FixWidthTxt, (IPTR)"555",
				End,
			    Child, CLabel2(":"),
			    Child, minobj = StringObject, /* min gadget */
			    	StringFrame,
				MUIA_String_Accept, (IPTR)"0123456789",
				MUIA_FixWidthTxt, (IPTR)"555",
				End,
			    Child, CLabel2(":"),		    
			    Child, secobj = StringObject, /* sec gadget */
			    	StringFrame,
				MUIA_String_Accept, (IPTR)"0123456789",
				MUIA_FixWidthTxt, (IPTR)"555",
				End,
			    Child, HGroup,
			    	MUIA_Group_Spacing, 0,
				Child,  timesubobj = TextObject, /* time [-] gadget */
			    	    ButtonFrame,
				    MUIA_Background, MUII_ButtonBack,
				    MUIA_Font, MUIV_Font_Button,
				    MUIA_InputMode, MUIV_InputMode_RelVerify,
				    MUIA_Text_Contents, "\033c-",
				    MUIA_FixWidthTxt, (IPTR)"+",
				    End,
				Child,  timeaddobj = TextObject, /* time [+] gadget */
			    	    ButtonFrame,
				    MUIA_Background, MUII_ButtonBack,
				    MUIA_Font, MUIV_Font_Button,
				    MUIA_InputMode, MUIV_InputMode_RelVerify,
				    MUIA_Text_Contents, "\033c+",
				    MUIA_FixWidthTxt, (IPTR)"-",
				    End,
				End,		    
			    Child, HVSpace,
			    End,
		    	End,
		    End,
		Child, HGroup, /* save/use/cancel button row */
		    MUIA_FixHeight, 1,
		    MUIA_Group_SameWidth, TRUE,
		    Child, CoolImageIDButton(MSG(MSG_GAD_SAVE), COOL_SAVEIMAGE_ID),
		    Child, CoolImageIDButton(MSG(MSG_GAD_USE), COOL_DOTIMAGE_ID),
		    Child, CoolImageIDButton(MSG(MSG_GAD_CANCEL), COOL_CANCELIMAGE_ID),
		    End,
		End,
	    End,
	End;
	
    if (!app) Cleanup(MSG(MSG_CANT_CREATE_APP));

    DoMethod(wnd, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
    DoMethod(wnd, MUIM_Notify, MUIA_Window_MenuAction, MSG_MEN_PROJECT_QUIT, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
    DoMethod(wnd, MUIM_Notify, MUIA_Window_ActiveObject, MUIV_EveryTime, app, 3, MUIM_CallHook, (IPTR)&activehook, MUIV_TriggerValue);
    DoMethod(monthobj, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, calobj, 3, MUIM_NoNotifySet, MUIA_Calendar_Month0, MUIV_TriggerValue);
    DoMethod(yearobj, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, yearobj, 3, MUIM_CallHook, (IPTR)&yearhook, 0);
    DoMethod(yearaddobj, MUIM_Notify, MUIA_Timer, MUIV_EveryTime, yearobj, 3, MUIM_CallHook, (IPTR)&yearhook, 1);
    DoMethod(yearsubobj, MUIM_Notify, MUIA_Timer, MUIV_EveryTime, yearobj, 3, MUIM_CallHook, (IPTR)&yearhook, -1);
    DoMethod(timeaddobj, MUIM_Notify, MUIA_Timer, MUIV_EveryTime, clockobj, 3, MUIM_CallHook, (IPTR)&clockhook, 1);
    DoMethod(timesubobj, MUIM_Notify, MUIA_Timer, MUIV_EveryTime, clockobj, 3, MUIM_CallHook, (IPTR)&clockhook, -1);
    DoMethod(clockobj, MUIM_Notify, MUIA_Clock_Ticked, TRUE, clockobj, 3, MUIM_CallHook, (IPTR)&clockhook, 0);
    DoMethod(hourobj, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, clockobj, 3, MUIM_CallHook, (IPTR)&clockhook, 100);
    DoMethod(minobj, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, clockobj, 3, MUIM_CallHook, (IPTR)&clockhook, 100);
    DoMethod(secobj, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, clockobj, 3, MUIM_CallHook, (IPTR)&clockhook, 100);
    
    set(calobj, MUIA_Calendar_Date, &clockdata);
    set(monthobj, MUIA_Cycle_Active, clockdata.month - 1);
    set(yearobj, MUIA_String_Integer, clockdata.year);
    
    CallHook(&clockhook, clockobj, 0);
}

/*********************************************************************************************/

static void KillGUI(void)
{
    DisposeObject(app);
    KillCalendarClass();
    KillClockClass();
}

/*********************************************************************************************/

static void HandleAll(void)
{
    ULONG sigs = 0;
    
    set (wnd, MUIA_Window_Open, TRUE);
    
    while((LONG) DoMethod(app, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit)
    {
	if (sigs)
	{
	    sigs = Wait(sigs | SIGBREAKF_CTRL_C);
	    if (sigs & SIGBREAKF_CTRL_C) break;
	}
    }

}

/*********************************************************************************************/

int main(void)
{
    InitLocale("Sys/timeprefs.catalog", 1);
    InitMenus();
    OpenLibs();
    OpenTimerDev();
    GetArguments();
    InitPrefs((args[ARG_USE] ? TRUE : FALSE), (args[ARG_SAVE] ? TRUE : FALSE));
    GetVisual();
    MakeGUI();
    HandleAll();
    Cleanup(NULL);
    
    return 0;
}

/*********************************************************************************************/


