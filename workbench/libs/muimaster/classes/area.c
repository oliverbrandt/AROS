/* 
    Copyright � 1999, David Le Corfec.
    Copyright � 2002, The AROS Development Team.
    All rights reserved.

    $Id$
*/

#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <graphics/gfxmacros.h>
#include <intuition/imageclass.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/diskfont.h>
#include <proto/dos.h>

extern struct Library *MUIMasterBase;
#include "muimaster_intern.h"

#include "mui.h"
#include "support.h"
#include "imspec.h"
#include "menu.h"
#include "prefs.h"

/*  #define MYDEBUG 1 */
#include "debug.h"


/* Returns a given text font, if necessary it opens the font, should be after MUIM_Setup */
static struct TextFont *zune_font_get(Object *obj, LONG preset)
{
    struct MUI_GlobalInfo *mgi = muiGlobalInfo(obj);
    struct MUI_RenderInfo *mri = muiRenderInfo(obj);

    if ((preset <= MUIV_Font_Inherit) && (preset >= MUIV_Font_NegCount))
    {
    	CONST_STRPTR name;
	if (preset > 0) return NULL;

	/* font already loaded, just return it */
	if (mri->mri_Fonts[-preset])
	{
/*  	    D(bug("zune_font_get : return mri_Fonts[-preset]=%lx\n", ri->mri_Fonts[-preset])); */
	    return mri->mri_Fonts[-preset];
	}

	/* font name given, load it */
        name = mgi->mgi_Prefs->fonts[-preset];
/*  	D(bug("zune_font_get : preset=%d, name=%s\n", preset, name)); */
	if (name)
	{
	    struct TextAttr ta;
	    if ((ta.ta_Name = (char*)AllocVec(strlen(name)+10,0)))
	    {
	    	char *p;
	    	LONG size;

	    	strcpy(ta.ta_Name,name);
	    	StrToLong(FilePart(ta.ta_Name),&size);
	    	ta.ta_YSize = size;
		ta.ta_Style = 0;
		ta.ta_Flags = 0;

		if ((p = PathPart(ta.ta_Name)))
		    strcpy(p,".font");
/*  		D(bug("zune_font_get : OpenDiskFont(%s)\n", ta.ta_Name)); */
		mri->mri_Fonts[-preset] = OpenDiskFont(&ta);

		FreeVec(ta.ta_Name);
	    }
	    
	}
	else /* fallback to window normal font */
	{
	    if (preset != MUIV_Font_Normal) /* avoid infinite recursion */
	    {
		return zune_font_get(obj, MUIV_Font_Normal);
	    }
	}

	/* no font loaded, fallback to screen font */
	if (!mri->mri_Fonts[-preset])
	{
	    struct TextAttr scr_attr;
	    scr_attr = *(_screen(obj)->Font);
	    scr_attr.ta_Flags = 0;
/*  	    D(bug("zune_font_get : OpenDiskFont(%s) (screen font)\n", scr_attr.ta_Name)); */
	    mri->mri_Fonts[-preset] = OpenDiskFont(&scr_attr);
	}
	return mri->mri_Fonts[-preset];
    }
    return (struct TextFont *)preset;
}

/*
Area.mui/MUIA_Background            done
Area.mui/MUIA_BottomEdge            done
Area.mui/MUIA_ContextMenu           done
Area.mui/MUIA_ContextMenuTrigger
Area.mui/MUIA_ControlChar           done
Area.mui/MUIA_CycleChain            done
Area.mui/MUIA_Disabled
Area.mui/MUIA_Draggable             done
Area.mui/MUIA_Dropable              done
Area.mui/MUIA_ExportID
Area.mui/MUIA_FillArea              done
Area.mui/MUIA_FixHeight             done
Area.mui/MUIA_FixHeightTxt          done
Area.mui/MUIA_FixWidth              done
Area.mui/MUIA_FixWidthTxt           done
Area.mui/MUIA_Font                  done
Area.mui/MUIA_Frame                 done
Area.mui/MUIA_FramePhantomHoriz     done
Area.mui/MUIA_FrameTitle            done
Area.mui/MUIA_Height                done
Area.mui/MUIA_HorizDisappear
Area.mui/MUIA_HorizWeight           done
Area.mui/MUIA_InnerBottom           done
Area.mui/MUIA_InnerLeft             done
Area.mui/MUIA_InnerRight            done
Area.mui/MUIA_InnerTop              done
Area.mui/MUIA_InputMode             done
Area.mui/MUIA_LeftEdge              done
Area.mui/MUIA_MaxHeight             done
Area.mui/MUIA_MaxWidth              done
Area.mui/MUIA_Pressed               done
Area.mui/MUIA_RightEdge             done
Area.mui/MUIA_Selected              done
Area.mui/MUIA_ShortHelp             done
Area.mui/MUIA_ShowMe                done
Area.mui/MUIA_ShowSelState          done (I only)
Area.mui/MUIA_Timer                 done
Area.mui/MUIA_TopEdge               done
Area.mui/MUIA_VertDisappear
Area.mui/MUIA_VertWeight            done
Area.mui/MUIA_Weight                done
Area.mui/MUIA_Width                 done
Area.mui/MUIA_Window                done
Area.mui/MUIA_WindowObject          done

Area.mui/MUIM_AskMinMax             done
Area.mui/MUIM_Cleanup               done
Area.mui/MUIM_ContextMenuBuild
Area.mui/MUIM_ContextMenuChoice
Area.mui/MUIM_CreateBubble
Area.mui/MUIM_CreateShortHelp
Area.mui/MUIM_DeleteBubble
Area.mui/MUIM_DeleteShortHelp
Area.mui/MUIM_DragBegin
Area.mui/MUIM_DragDrop
Area.mui/MUIM_DragFinish
Area.mui/MUIM_DragQuery
Area.mui/MUIM_DragReport
Area.mui/MUIM_Draw                  done
Area.mui/MUIM_DrawBackground        done
Area.mui/MUIM_HandleEvent           done
Area.mui/MUIM_HandleInput
Area.mui/MUIM_Hide                  done
Area.mui/MUIM_Setup                 done
Area.mui/MUIM_Show                  done
*/

static const int __version = 1;
static const int __revision = 1;

//#ifdef DEBUG
//static STRPTR zune_area_to_string (Object *area);
//#endif
static void area_update_data(Object *obj, struct MUI_AreaData *data);
static void setup_control_char (struct MUI_AreaData *data, Object *obj,
				struct IClass *cl);
static void cleanup_control_char (struct MUI_AreaData *data, Object *obj);

//static void setup_cycle_chain (struct MUI_AreaData *data, Object *obj);
//static void cleanup_cycle_chain (struct MUI_AreaData *data, Object *obj);


static void _zune_focus_new(Object *obj, int type)
{
    Object *parent = _parent(obj);
    struct RastPort *rp = _rp(obj);
    UWORD oldDrPt = rp->LinePtrn;

    int x1 = _left(obj) - 1;
    int y1 = _top(obj)  - 1;
    int x2 = _left(obj) + _width(obj);
    int y2 = _top(obj)  + _height(obj);

    if (!parent || parent == _win(obj)) return;

    SetABPenDrMd(rp, _pens(obj)[MPEN_SHINE], _pens(obj)[MPEN_SHADOW], JAM2);

    if (!type) SetDrPt(rp, 0xCCCC);
    else SetDrPt(rp,0x5555);

    Move(rp, x1, y1);
    Draw(rp, x2, y1);
    Draw(rp, x2, y2);
    Draw(rp, x1, y2);
    Draw(rp, x1, y1);
    SetDrPt(rp, oldDrPt);
}

static void _zune_focus_destroy(Object *obj)
{
    Object *parent = _parent(obj);

    int x1 = _left(obj) - 1;
    int y1 = _top(obj)  - 1;
    int x2 = _left(obj) + _width(obj);
    int y2 = _top(obj)  + _height(obj);
    int width = x2 - x1 + 1;
    int height = y2 - y1 + 1;

    if (!parent || parent == _win(obj)) return;

    DoMethod(parent, MUIM_DrawBackground, x1, y1, width, 1, 0, 0, 0);
    DoMethod(parent, MUIM_DrawBackground, x2, y1, 1, height, 0, 0, 0);
    DoMethod(parent, MUIM_DrawBackground, x1, y2, width, 1, 0, 0, 0);
    DoMethod(parent, MUIM_DrawBackground, x1, y1, 1, height, 0, 0, 0);
}


/**************************************************************************
 OM_NEW
**************************************************************************/
static ULONG Area_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
    struct MUI_AreaData *data;
    struct TagItem *tags,*tag;

    obj = (Object *)DoSuperMethodA(cl, obj, (Msg)msg);
    if (!obj) return FALSE;

    /* Initial local instance data */
    data = INST_DATA(cl, obj);

    data->mad_Flags = MADF_FILLAREA | MADF_SHOWME | MADF_SHOWSELSTATE | MADF_DROPABLE;
    data->mad_HorizWeight = data->mad_VertWeight = 100;
    data->mad_InputMode = MUIV_InputMode_None;

    /* parse initial taglist */

    for (tags = msg->ops_AttrList; (tag = NextTagItem((struct TagItem **)&tags)); )
    {
	switch (tag->ti_Tag)
	{
	    case    MUIA_Background:
		    data->mad_Flags |= MADF_OWNBG;
		    data->mad_BackgroundSpec = zune_image_spec_duplicate(tag->ti_Data);
		    break;

	    case MUIA_ControlChar:
		data->mad_ControlChar = tag->ti_Data;
		break;
	    case MUIA_CycleChain:
		_handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_CYCLECHAIN);
		break;

	    case MUIA_Disabled:
		if (tag->ti_Data)
		{
		    data->mad_DisableCount = 1;
		}
		break;

	    case MUIA_FillArea:
		_handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_FILLAREA);
		break;
	    case MUIA_Draggable:
		_handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_DRAGGABLE);
		break;
	    case MUIA_Dropable:
	        _handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_DROPABLE);
	        break;
	    case MUIA_FixHeight:
		data->mad_Flags |= MADF_FIXHEIGHT;
		data->mad_HardHeight = tag->ti_Data;
		break;
	    case MUIA_FixHeightTxt:
	    	data->mad_HardHeightTxt = (STRPTR)tag->ti_Data;
		break;
	    case MUIA_FixWidth:
		data->mad_Flags |= MADF_FIXWIDTH;
		data->mad_HardWidth = tag->ti_Data;
		break;
	    case MUIA_FixWidthTxt:
	    	data->mad_HardWidthTxt = (STRPTR)tag->ti_Data;
		break;
	    case MUIA_Font:
		data->mad_FontPreset = tag->ti_Data;
		break;
	    case MUIA_Frame:
		data->mad_Frame = tag->ti_Data;
		break;
	    case MUIA_FramePhantomHoriz:
		data->mad_Flags |= MADF_FRAMEPHANTOM;
		break;
	    case MUIA_FrameTitle:
		/* strdup after tags parsing */
		data->mad_FrameTitle = (STRPTR)tag->ti_Data;
		break;
	    case MUIA_HorizWeight:
		data->mad_HorizWeight = tag->ti_Data;
		break;
	    case MUIA_InnerBottom:
		data->mad_Flags |= MADF_INNERBOTTOM;
		data->mad_HardIBottom = CLAMP((ULONG)tag->ti_Data, 0, 32);
		break;
	    case MUIA_InnerLeft:
		data->mad_Flags |= MADF_INNERLEFT;
		data->mad_HardILeft = CLAMP((ULONG)tag->ti_Data, 0, 32);
		break;
	    case MUIA_InnerRight:
		data->mad_Flags |= MADF_INNERRIGHT;
		data->mad_HardIRight = CLAMP((ULONG)tag->ti_Data, 0, 32);
		break;
	    case MUIA_InnerTop:
		data->mad_Flags |= MADF_INNERTOP;
		data->mad_HardITop =  CLAMP((ULONG)tag->ti_Data, 0, 32);
		break;
	    case MUIA_InputMode:
		data->mad_InputMode = tag->ti_Data;
		break;
	    case MUIA_MaxHeight:
		data->mad_Flags |= MADF_MAXHEIGHT;
		data->mad_HardHeight = tag->ti_Data;
		break;
	    case MUIA_MaxWidth:
		data->mad_Flags |= MADF_MAXWIDTH;
		data->mad_HardWidth = tag->ti_Data;
		break;
	    case MUIA_Selected:
		_handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_SELECTED);
		break;
	    case MUIA_ShortHelp:
		data->mad_ShortHelp = (STRPTR)tag->ti_Data;
		break;
	    case MUIA_ShowMe:
		_handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_SHOWME);
		break;
	    case MUIA_ShowSelState:
		_handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_SHOWSELSTATE);
		break;
	    case MUIA_VertWeight:
		data->mad_VertWeight = tag->ti_Data;
		break;
	    case MUIA_Weight:
		data->mad_HorizWeight = data->mad_VertWeight = tag->ti_Data;
		break;
	    case MUIA_ContextMenu:
		data->mad_ContextMenu = (Object*)tag->ti_Data;
		break;
	}
    }

    /* In Soliton MUIA_Selected was setted to MUIV_InputMode_RelVerify (=1) for MUIA_Input_Mode
    ** MUIV_InputMode_RelVerify which is wrong of course but MUI seems to filter this out
    ** so we have to do it also
    */
    if (data->mad_InputMode == MUIV_InputMode_RelVerify)
    {
    	if (data->mad_Flags & MADF_SELECTED)
	    D(bug("MUIA_Selected was set in OM_NEW, although being in MUIV_InputMode_RelVerify\n"));
    	data->mad_Flags &= ~MADF_SELECTED;
    }

    if ((data->mad_Frame != 0) && (data->mad_FrameTitle))
    {
    	data->mad_FrameTitle = StrDup(data->mad_FrameTitle);
    }

    data->mad_ehn.ehn_Events = 0; /* Will be filled on demand */
    data->mad_ehn.ehn_Priority = -5;
    /* Please also send mui key events to us, no idea if mui handles this like this */
    data->mad_ehn.ehn_Flags    = MUI_EHF_ALWAYSKEYS;
    data->mad_ehn.ehn_Object   = obj;
    data->mad_ehn.ehn_Class    = cl;

    data->mad_hiehn.ehn_Events   = 0;
    data->mad_hiehn.ehn_Priority = -10;
    data->mad_hiehn.ehn_Flags    = MUI_EHF_HANDLEINPUT;
    data->mad_hiehn.ehn_Object   = obj;
    data->mad_hiehn.ehn_Class    = 0;

    return (ULONG)obj;
}

/**************************************************************************
 OM_DISPOSE
**************************************************************************/
static ULONG Area_Dispose(struct IClass *cl, Object *obj, Msg msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);

    if ((data->mad_Frame > 0) && (data->mad_FrameTitle))
    {
	FreeVec((APTR)data->mad_FrameTitle);
    }

    zune_image_spec_free(data->mad_BackgroundSpec); /* Safe to call this with NULL */

    return DoSuperMethodA(cl, obj, msg);
}


/**************************************************************************
 OM_SET
**************************************************************************/
static ULONG Area_Set(struct IClass *cl, Object *obj, struct opSet *msg)
{
    struct MUI_AreaData *data  = INST_DATA(cl, obj);
    struct TagItem             *tags  = msg->ops_AttrList;
    struct TagItem             *tag;

    int change_disable = 0; /* Has the disable state changed? */

    while ((tag = NextTagItem((struct TagItem **)&tags)) != NULL)
    {
	switch (tag->ti_Tag)
	{
	    case    MUIA_Background:
		    if (data->mad_Background)
		    {
			if (_flags(obj) & MADF_CANDRAW)
			{
			    zune_imspec_hide(data->mad_Background);
			}
			if (_flags(obj) & MADF_SETUP)
			{
			    zune_imspec_cleanup(data->mad_Background);
			    data->mad_Background = NULL;
			}
		    }

		    zune_image_spec_free(data->mad_BackgroundSpec);
		    if (tag->ti_Data)
		    {
			data->mad_BackgroundSpec = zune_image_spec_duplicate(tag->ti_Data);
			data->mad_Flags |= MADF_OWNBG;
		    }
		    else
		    {
			data->mad_BackgroundSpec = NULL;
			data->mad_Flags &= ~MADF_OWNBG;
		    }

		    if (_flags(obj) & MADF_SETUP)
		    {
			data->mad_Background =
			    zune_imspec_setup((IPTR)data->mad_BackgroundSpec,
					      muiRenderInfo(obj));
		    }
		    if (_flags(obj) & MADF_CANDRAW)
		    {
			zune_imspec_show(data->mad_Background, obj);
		    }
		    MUI_Redraw(obj, MADF_DRAWOBJECT);
		    break;

	    case    MUIA_FillArea:
		    _handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_FILLAREA);
		    break;

	    case MUIA_ControlChar:
		if (_flags(obj) & MADF_SETUP)
		    cleanup_control_char(data, obj);
		data->mad_ControlChar = tag->ti_Data;
		if (_flags(obj) & MADF_SETUP)
		    setup_control_char(data, obj, cl);
		break;

	    case MUIA_CycleChain:
//		if (data->mad_InputMode == MUIV_InputMode_None)
//		    break;

		if ((!(_flags(obj) & MADF_CYCLECHAIN) && tag->ti_Data)
		    || ((_flags(obj) & MADF_CYCLECHAIN) && !tag->ti_Data))
		{
		    if (_flags(obj) & MADF_SETUP)
		    {
		    	cleanup_control_char(data,obj);
			_handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_CYCLECHAIN);
			setup_control_char(data,obj,cl);
		    }   else _handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_CYCLECHAIN);
		}
		break;

	    case    MUIA_Disabled:
		    if (tag->ti_Data)
		    {
		    	if (!data->mad_DisableCount) change_disable = 1;
		    	data->mad_DisableCount++;
		    }   else 
		    {
		    	if (data->mad_DisableCount)
		    	{
			    data->mad_DisableCount--;
			    if (!data->mad_DisableCount) change_disable = 1;
			}
		    }
		    break;


	    case MUIA_HorizWeight:
		data->mad_HorizWeight = tag->ti_Data;
		break;

	    case MUIA_Pressed:
		if (tag->ti_Data)
		    data->mad_Flags |= MADF_PRESSED;
		else
		    data->mad_Flags &= ~MADF_PRESSED;
		break;

	    case MUIA_ShortHelp:
		data->mad_ShortHelp = (STRPTR)tag->ti_Data;
		break;

	    case MUIA_ShowMe:
		{
		    ULONG oldflags = data->mad_Flags;
		    int recalc = 0;

		    if (tag->ti_Data) data->mad_Flags |= MADF_SHOWME;
		    else data->mad_Flags &= ~MADF_SHOWME;

		    if (oldflags != data->mad_Flags)
		    {
			if (!tag->ti_Data)
			{
			    /* Should be made invisible, so send a MUIM_Hide and then a MUIM_Cleanup to the object if needed,
			    ** as objects with MUIA_ShowMe to false neighter get MUIM_Setup nor MUIM_Show */
			    if (_flags(obj)&MADF_CANDRAW)
			    {
			    	DoMethod(obj,MUIM_Hide);
			    	recalc = 1;
			    }
			    if (_flags(obj)&MADF_SETUP) DoMethod(obj,MUIM_Cleanup);
			} else
			{
			    Object *parent = _parent(obj); /* Will be NULL if direct child of a window! */
			    if (parent)
			    {
				if (_flags(parent) & MADF_SETUP) DoSetupMethod(obj,muiRenderInfo(parent));
				if (_flags(parent) & MADF_CANDRAW) DoMethod(obj,MUIM_Show);
			    } else
			    {
				/* Check if window is open... */
			    }
			}

		    	if (recalc)
		    	{
			    DoMethod(_win(obj), MUIM_Window_RecalcDisplay, (IPTR)_parent(obj));
			}
		    }
		}
		break;

	    case    MUIA_Selected:
		D(bug(" Area_Set(%p) : MUIA_Selected val=%ld sss=%d\n", obj, tag->ti_Data, !!(data->mad_Flags & MADF_SHOWSELSTATE)));
		if (tag->ti_Data) data->mad_Flags |= MADF_SELECTED;
		else data->mad_Flags &= ~MADF_SELECTED;
/*  		if (data->mad_Flags & MADF_SHOWSELSTATE) */
		    MUI_Redraw(obj, MADF_DRAWOBJECT);
/*  		else */
/*  		    MUI_Redraw(obj, MADF_DRAWUPDATE); */
		break;

	    case MUIA_Timer:
		data->mad_Timeval = tag->ti_Data;
		break;

	    case MUIA_VertWeight:
		data->mad_VertWeight = tag->ti_Data;
		break;

	    case MUIA_Draggable:
		_handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_DRAGGABLE);
		break;
	    case MUIA_Dropable:
	        _handle_bool_tag(data->mad_Flags, tag->ti_Data, MADF_DROPABLE);
	        break;
	    case MUIA_ContextMenu:
		data->mad_ContextMenu = (Object*)tag->ti_Data;
		break;
	}
    }

    if (change_disable)
    {
	MUI_Redraw(obj, MADF_DRAWOBJECT);
    }

    return DoSuperMethodA(cl, obj, (Msg)msg);
}


/**************************************************************************
 OM_GET
**************************************************************************/
static ULONG Area_Get(struct IClass *cl, Object *obj, struct opGet *msg)
{
#define STORE *(msg->opg_Storage)

    struct MUI_AreaData *data = INST_DATA(cl, obj);

    switch(msg->opg_AttrID)
    {
	case MUIA_BottomEdge:
	    STORE = (ULONG)_bottom(obj);
	    return(TRUE);
	case MUIA_ControlChar:
	    STORE = data->mad_ControlChar;
	    return(TRUE);
	case MUIA_CycleChain:
	    STORE = ((data->mad_Flags & MADF_CYCLECHAIN) != 0);
	    return(TRUE);

	case    MUIA_Disabled:
	    STORE = !!data->mad_DisableCount; /* BOOLEAN */
	    return(TRUE);

	case MUIA_Font:
	    STORE = (ULONG)data->mad_FontPreset;
	    return(TRUE);
	case MUIA_Height:
	    STORE = (ULONG)_height(obj);
	    return(TRUE);
	case MUIA_HorizWeight:
	    STORE = (ULONG)data->mad_HorizWeight;
	    return(TRUE);

	case    MUIA_InnerBottom:
		if (data->mad_Flags & MADF_INNERBOTTOM) STORE = (ULONG)data->mad_HardIBottom;
		else STORE = (ULONG)muiGlobalInfo(obj)->mgi_Prefs->frames[data->mad_Frame].innerBottom;
		return 1;

	case    MUIA_InnerLeft:
		if (data->mad_Flags & MADF_INNERLEFT) STORE = (ULONG)data->mad_HardILeft;
		else if (data->mad_Flags & MADF_FRAMEPHANTOM) STORE = 0;
		else STORE = (ULONG)muiGlobalInfo(obj)->mgi_Prefs->frames[data->mad_Frame].innerLeft;
		return 1;

	case    MUIA_InnerRight:
		if (data->mad_Flags & MADF_INNERRIGHT) STORE = (ULONG)data->mad_HardIRight;
		else if (data->mad_Flags & MADF_FRAMEPHANTOM) STORE = 0;
		else STORE = (ULONG)muiGlobalInfo(obj)->mgi_Prefs->frames[data->mad_Frame].innerRight;
		return 1;

	case	MUIA_InnerTop:
		if (data->mad_Flags & MADF_INNERTOP) STORE = (ULONG)data->mad_HardITop;
		else STORE = (ULONG)muiGlobalInfo(obj)->mgi_Prefs->frames[data->mad_Frame].innerTop;
		return 1;


	case MUIA_LeftEdge:
	    STORE = (ULONG)_left(obj);
	    return(TRUE);
	case MUIA_Pressed:
	    STORE = !!(data->mad_Flags & MADF_PRESSED);
	    return(TRUE);
	case MUIA_RightEdge:
	    STORE = (ULONG)_right(obj);
	    return(TRUE);
	case MUIA_Selected:
	    STORE = !!(data->mad_Flags & MADF_SELECTED);
	    return(TRUE);
	case MUIA_ShortHelp:
	    STORE = (ULONG)data->mad_ShortHelp;
	    return(TRUE);
	case MUIA_ShowMe:
	    STORE = (data->mad_Flags & MADF_SHOWME);
	    return(TRUE);
	case MUIA_Timer:
	    return(TRUE);
	case MUIA_TopEdge:
	    STORE = (ULONG)_top(obj);
	    return(TRUE);
	case MUIA_VertWeight:
	    STORE = (ULONG)data->mad_VertWeight;
	    return(TRUE);
	case MUIA_Width:
	    STORE = (ULONG)_width(obj);
	    return(TRUE);
	case MUIA_Window:
	    if (muiAreaData(obj)->mad_RenderInfo)
		STORE = (ULONG)_window(obj);
	    else
		STORE = 0L;
	    return(TRUE);
	case MUIA_WindowObject:
	    if (muiAreaData(obj)->mad_RenderInfo)
		STORE = (ULONG)_win(obj);
	    else
		STORE = 0L;
	    return(TRUE);
	case MUIA_ContextMenu:
	    STORE = (ULONG)data->mad_ContextMenu;
	    return 1;
    }

    return(DoSuperMethodA(cl, obj, (Msg) msg));
#undef STORE
}


/**************************************************************************
 MUIM_AskMinMax
**************************************************************************/
static ULONG Area_AskMinMax(struct IClass *cl, Object *obj, struct MUIP_AskMinMax *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);

    area_update_data(obj, data);
    
    msg->MinMaxInfo->MinWidth = _subwidth(obj);
    if (data->mad_TitleText)
    {
	/* Save the orginal font */
	struct TextFont *obj_font = _font(obj);

	_font(obj) = zune_font_get(obj,MUIV_Font_Title);
	zune_text_get_bounds(data->mad_TitleText, obj);

        /* restore the font */
	_font(obj) = obj_font;

	_subheight(obj) = _subheight(obj) - _addtop(obj) + data->mad_TitleText->height + 1;
	_addtop(obj) = data->mad_TitleText->height + 1;

#if 0
	if (muiGlobalInfo(obj)->mgi_Prefs->group_title_color == GROUP_TITLE_COLOR_3D)
	{
	    _subheight(obj) += 1;
	    _addtop(obj) += 1;
	}
#endif
    }

    msg->MinMaxInfo->MinHeight = _subheight(obj);

    msg->MinMaxInfo->MaxWidth = msg->MinMaxInfo->MinWidth;
    msg->MinMaxInfo->MaxHeight = msg->MinMaxInfo->MinHeight;
    msg->MinMaxInfo->DefWidth = msg->MinMaxInfo->MinWidth;
    msg->MinMaxInfo->DefHeight = msg->MinMaxInfo->MinHeight;

    return TRUE;
}

/*
 * Called after MUIM_AskMinMax, to handle fixed and max sizes.
 */
void __area_finish_minmax(Object *obj, struct MUI_MinMax *MinMaxInfo)
{
    struct MUI_AreaData *data = muiAreaData(obj);
    
    if ((_flags(obj) & MADF_FIXHEIGHT) && data->mad_HardHeight)
    {
	int h;

    	h = data->mad_HardHeight + data->mad_subheight;
	
	MinMaxInfo->MinHeight =
	MinMaxInfo->DefHeight = 
	MinMaxInfo->MaxHeight = CLAMP(h, MinMaxInfo->MinHeight, MinMaxInfo->MaxHeight);
    }
    else if (data->mad_HardHeightTxt)
    {
    	ZText *text;
	
	if ((text = zune_text_new(NULL, data->mad_HardHeightTxt, ZTEXT_ARG_NONE, 0)))
	{
	    zune_text_get_bounds(text, obj);
	    
	    MinMaxInfo->MinHeight =
	    MinMaxInfo->DefHeight =
	    MinMaxInfo->MaxHeight = 
	    	CLAMP(text->height + data->mad_subheight, MinMaxInfo->MinHeight, MinMaxInfo->MaxHeight);
	    
	    zune_text_destroy(text);
	}

    }
    else if (_flags(obj) & MADF_MAXHEIGHT)
    {	
	MinMaxInfo->MaxHeight =
	    CLAMP(data->mad_HardHeight + data->mad_subheight,
		  MinMaxInfo->MinHeight,
		  MinMaxInfo->MaxHeight);
    }

    if ((_flags(obj) & MADF_FIXWIDTH) && data->mad_HardWidth)
    {
	int w;

    	w = data->mad_HardWidth + data->mad_subwidth;
	
	MinMaxInfo->MinWidth =
	MinMaxInfo->DefWidth = 
	MinMaxInfo->MaxWidth = CLAMP(w, MinMaxInfo->MinWidth, MinMaxInfo->MaxWidth);
    }
    else if (data->mad_HardWidthTxt)
    {
    	ZText *text;
	
	if ((text = zune_text_new(NULL, data->mad_HardWidthTxt, ZTEXT_ARG_NONE, 0)))
	{
	    zune_text_get_bounds(text, obj);
	    
	    MinMaxInfo->MinWidth =
	    MinMaxInfo->DefWidth =
	    MinMaxInfo->MaxWidth = 
	    	CLAMP(text->width + data->mad_subwidth, MinMaxInfo->MinWidth, MinMaxInfo->MaxWidth);
	    
	    zune_text_destroy(text);
	}

    }
    else if (_flags(obj) & MADF_MAXWIDTH)
    {
	MinMaxInfo->MaxWidth =
	    CLAMP(data->mad_HardWidth + data->mad_subwidth,
		  MinMaxInfo->MinWidth,
		  MinMaxInfo->MaxWidth);
    }

    /* Set minmax */
    _minwidth(obj) = MinMaxInfo->MinWidth;
    _minheight(obj) = MinMaxInfo->MinHeight;
    _maxwidth(obj) = MinMaxInfo->MaxWidth;
    _maxheight(obj) = MinMaxInfo->MaxHeight;
    _defwidth(obj) = MinMaxInfo->DefWidth;
    _defheight(obj) = MinMaxInfo->DefHeight;
}


/**************************************************************************
 MUIM_Draw
**************************************************************************/
static ULONG Area_Draw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);
    struct ZuneFrameGfx *zframe;
    struct Rectangle mrect, current;
    //APTR areaclip;

/*      D(bug("Area_Draw(0x%lx) %ldx%ldx%ldx%ld\n",obj,_left(obj),_top(obj),_right(obj),_bottom(obj))); */

     D(bug(" Area_Draw(%p) msg=0x%08lx flags=0x%08lx\n",obj, msg->flags,_flags(obj)));

    if (msg->flags & MADF_DRAWALL)
	msg->flags |= MADF_DRAWOBJECT;

    if (!(msg->flags & MADF_DRAWOBJECT))
    {
	/* dont draw bg/frame, let subclass redraw content only
	**/
	return 0;
    }

//    if ((data->mad_Flags & MADF_SELECTED) &&
//	!(data->mad_Flags & MADF_SHOWSELSTATE))
//    {
//	if (!(msg->flags & MADF_DRAWALL))
//	    msg->flags &= ~MADF_DRAWOBJECT;
//	return 0;
//    }

    zframe = zune_zframe_get(&muiGlobalInfo(obj)->mgi_Prefs->frames[data->mad_Frame]);

    /* Background drawing */
    if (data->mad_Flags & MADF_FILLAREA)
    {
	struct MUI_ImageSpec_intern *background;
	int left,top,width,height;

	if (!(data->mad_Flags & MADF_SELECTED) || !(data->mad_Flags & MADF_SHOWSELSTATE))
	    background = data->mad_Background;
	else
	    background = data->mad_SelBack;

	left = _left(obj);
	top = _top(obj);
	width = _width(obj);
	height = _height(obj);

	if (data->mad_TitleText)
	{
            switch (muiGlobalInfo(obj)->mgi_Prefs->group_title_position)
            {
		case GROUP_TITLE_POSITION_ABOVE:
		    top += _font(obj)->tf_Baseline;
		    height -= _font(obj)->tf_Baseline;
		    break;
		case GROUP_TITLE_POSITION_CENTERED:
		    top += data->mad_TitleText->height / 2 + 2;
		    height -= data->mad_TitleText->height / 2 + 2;
		    break;
	    }
	    /* TODO: Fill the Area not covered here */
	}

	if (!background)
	{
	    /* This will do the rest, TODO: on MADF_DRAWALL we not really need to draw this */
	    D(bug(" Area_Draw(%p) MUIM_DrawBackground\n", obj));
	    DoMethod(obj, MUIM_DrawBackground, left, top, width, height, left, top, data->mad_Flags);
	}
	else
	{
	    D(bug(" Area_Draw(%p) zune_imspec_draw\n", obj));
	    zune_imspec_draw(background, data->mad_RenderInfo,
			    left, top, width, height, left, top, 0);
	}
    }


    /* Frame drawing */
    if (!(data->mad_Flags & MADF_FRAMEPHANTOM))
    {
	int state = muiGlobalInfo(obj)->mgi_Prefs->frames[data->mad_Frame].state;
	if ((data->mad_Flags & MADF_SELECTED) && (data->mad_Flags & MADF_SHOWSELSTATE))
	    state ^= 1;

	/* set clipping so that frame is not drawn behind title */

	if (data->mad_TitleText)
	{
	    APTR textdrawclip;

	    struct TextFont *obj_font = _font(obj);
	    struct Region *region;

	    int x, top;
	    int width, height;

	    width = data->mad_TitleText->width;
/*  	    if (muiGlobalInfo(obj)->mgi_Prefs->group_title_color == GROUP_TITLE_COLOR_3D) */
/*  		width += 1; */

	    x = _mleft(obj) + (_mwidth(obj) - width) / 2;

            switch (muiGlobalInfo(obj)->mgi_Prefs->group_title_position)
            {
		case GROUP_TITLE_POSITION_ABOVE:
		    top = _top(obj) + _font(obj)->tf_Baseline - 2;
		    height = _height(obj) - (_font(obj)->tf_Baseline - 2);
		    break;
		case GROUP_TITLE_POSITION_CENTERED:
		    top = _top(obj) + data->mad_TitleText->height / 2;
		    height = _height(obj) - data->mad_TitleText->height / 2;
		    break;
		default:
		    break;
	    }

            if (x < _mleft(obj) + 2)
		x = _mleft(obj) + 2;

	    if ((region = NewRegion()))
	    {
	    	struct Rectangle rect;
		int maxx;

		maxx = x + width + 1;
		if (muiGlobalInfo(obj)->mgi_Prefs->group_title_color
		    != GROUP_TITLE_COLOR_3D)
		    maxx--;
	    	rect.MinX = x - 2;
	    	rect.MinY = _top(obj);
	    	rect.MaxX = MIN(_mright(obj), maxx);
	    	rect.MaxY = rect.MinY + data->mad_TitleText->height - 1; // frame is not thick enough anywhy
		OrRectRegion(region,&rect);

	    	rect.MinX = _left(obj);
	    	rect.MinY = _top(obj);
	    	rect.MaxX = _right(obj);
	    	rect.MaxY = _bottom(obj);
		XorRectRegion(region,&rect);

		textdrawclip = MUI_AddClipRegion(muiRenderInfo(obj),region);
	    }
	    
	    zframe->draw[state](muiRenderInfo(obj), _left(obj), top, _width(obj), height);

	    if (region)
	    {
	    	MUI_RemoveClipRegion(muiRenderInfo(obj),textdrawclip);
/*		DisposeRegion(region);*/ /* sba: DisposeRegion happens in MUI_RemoveClipRegion, this seems wrong to me */
	    }

	    /* Title text drawing */
	    _font(obj) = zune_font_get(obj,MUIV_Font_Title);

            /* TODO: sba if a TextFit() for zune text is available one could disable the clipping */
	    textdrawclip = MUI_AddClipping(muiRenderInfo(obj), _mleft(obj) + 2, _top(obj),
					   _mwidth(obj) - 4, data->mad_TitleText->height + 1);

	    SetAPen(_rp(obj), _pens(obj)[MPEN_SHADOW]);
	    if (muiGlobalInfo(obj)->mgi_Prefs->group_title_color == GROUP_TITLE_COLOR_3D)
	    {
		zune_text_draw(data->mad_TitleText, obj, x + 1, x + width, _top(obj) + 1);
		SetAPen(_rp(obj), _pens(obj)[MPEN_SHINE]);
		zune_text_draw(data->mad_TitleText, obj, x, x + width - 1, _top(obj));
	    }
	    else
	    {
		
		if (muiGlobalInfo(obj)->mgi_Prefs->group_title_color == GROUP_TITLE_COLOR_HILITE)
		{
		    SetAPen(_rp(obj), _pens(obj)[MPEN_SHINE]);
		}
		zune_text_draw(data->mad_TitleText, obj, x, x + width - 1, _top(obj));
	    }

	    MUI_RemoveClipping(muiRenderInfo(obj), textdrawclip);

	    _font(obj) = obj_font;
	}
	else
	{
	    zframe->draw[state](muiRenderInfo(obj), _left(obj), _top(obj),
				_width(obj), _height(obj));
	}
    }

    mrect.MinX = _mleft(obj);
    mrect.MinY = _mtop(obj);
    mrect.MaxX = _mright(obj);
    mrect.MaxY = _mbottom(obj);

    current = muiRenderInfo(obj)->mri_ClipRect;
/*  	    g_print("intersect area: mrect=(%d, %d, %d, %d) current=(%d, %d, %d, %d)\n", */
/*  		    mrect.x, mrect.y, */
/*  		    mrect.width, mrect.height, */
/*  		    current.x, current.y, */
/*  		    current.width, current.height); */

#if 0
    if (!gdk_rectangle_intersect(&mrect, &current,
				 &muiRenderInfo(obj)->mri_ClipRect))
    {
/*  	g_print("failed\n"); */
	msg->flags &= ~MADF_DRAWOBJECT;
    }
#endif

/*    MUI_RemoveClipping(muiRenderInfo(obj), areaclip);*/

    return TRUE;
}

/**************************************************************************
 MUIM_DrawBackground
**************************************************************************/
static ULONG Area_DrawBackground(struct IClass *cl, Object *obj, struct MUIP_DrawBackground *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);
    struct MUI_ImageSpec_intern *bg;
    LONG state;

    if (!(data->mad_Flags & MADF_CANDRAW)) /* not between show/hide */
	return FALSE;

    if ((msg->flags & MADF_SELECTED) && (msg->flags & MADF_SHOWSELSTATE))
    { 
    	bg = data->mad_SelBack;
	state = IDS_SELECTED;
    }
    else
    {
	bg = data->mad_Background;
	state = IDS_NORMAL;
    }

    if (!bg)
    {
    	Object *parent;
    	get(obj, MUIA_Parent, &parent);
    	if (parent)
	    DoMethodA(parent, (Msg)msg);
    	else
	    DoMethod(_win(obj), MUIM_Window_DrawBackground, msg->left, msg->top,
		     msg->width, msg->height, msg->xoffset, msg->yoffset, msg->flags);
    	return TRUE;
    }

    zune_imspec_draw(bg, data->mad_RenderInfo,
		     msg->left, msg->top, msg->width, msg->height,
		     msg->xoffset, msg->yoffset, state);

    return TRUE;
}

/* Perverting the EventHandlerNode structure to specify a shortcut.
 */
static void setup_control_char (struct MUI_AreaData *data, Object *obj, struct IClass *cl)
{
/*    if (data->mad_InputMode != MUIV_InputMode_None) */ /* needed to be commented, because it checks also for the controlchar */
    {
	data->mad_ccn.ehn_Events = data->mad_ControlChar;
	switch (data->mad_InputMode)
	{
	    case MUIV_InputMode_RelVerify:
		data->mad_ccn.ehn_Flags = MUIKEY_PRESS;
		break;
	    case MUIV_InputMode_Toggle:
		data->mad_ccn.ehn_Flags = MUIKEY_TOGGLE;
		break;
	    case MUIV_InputMode_Immediate:
		data->mad_ccn.ehn_Flags = MUIKEY_PRESS;
		break;
	}
	data->mad_ccn.ehn_Priority = 0;
	data->mad_ccn.ehn_Object = obj;
	data->mad_ccn.ehn_Class = cl;
	DoMethod(_win(obj), MUIM_Window_AddControlCharHandler, (IPTR)&data->mad_ccn);
    }
}


static void cleanup_control_char (struct MUI_AreaData *data, Object *obj)
{
//    if (data->mad_InputMode != MUIV_InputMode_None)
    {
	DoMethod(_win(obj),
		 MUIM_Window_RemControlCharHandler, (IPTR)&data->mad_ccn);
    }
}

/**************************************************************************
 First method to be called after an OM_NEW, it is the place
 for all initializations depending on the environment, but not
 on the gadget size/position. Matched by MUIM_Cleanup.
**************************************************************************/
static ULONG Area_Setup(struct IClass *cl, Object *obj, struct MUIP_Setup *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);

    muiRenderInfo(obj) = msg->RenderInfo;

    area_update_data(obj,data);

    if (data->mad_Flags & MADF_OWNBG)
    {
	data->mad_Background = zune_imspec_setup((IPTR)data->mad_BackgroundSpec,
						 muiRenderInfo(obj));
    }

    if ((data->mad_Flags & MADF_SHOWSELSTATE) &&
	(data->mad_InputMode != MUIV_InputMode_None))
    {
	data->mad_SelBack = zune_imspec_setup(MUII_SelectedBack, muiRenderInfo(obj));
    }

    if (data->mad_InputMode != MUIV_InputMode_None || data->mad_ContextMenu)
    {
	data->mad_ehn.ehn_Events = IDCMP_MOUSEBUTTONS;
	DoMethod(_win(obj), MUIM_Window_AddEventHandler, (IPTR)&data->mad_ehn);
    }

    /* Those are filled by RequestIDCMP() */
    if (data->mad_hiehn.ehn_Events)
	DoMethod(_win(obj), MUIM_Window_AddEventHandler, (IPTR)&data->mad_hiehn);

    setup_control_char (data, obj, cl);
//    setup_cycle_chain (data, obj);

    if (data->mad_FontPreset == MUIV_Font_Inherit)
    {
	if (_parent(obj) != NULL && _parent(obj) != _win(obj))
	    data->mad_Font = _font(_parent(obj));
	else
	    data->mad_Font = zune_font_get(obj,MUIV_Font_Normal);
    }
    else
    {
	data->mad_Font = zune_font_get(obj,data->mad_FontPreset);
    }

    if (data->mad_FrameTitle)
    {
	data->mad_TitleText = zune_text_new(NULL, data->mad_FrameTitle,
				    ZTEXT_ARG_NONE, 0);
    }

    if (data->mad_Flags & MADF_ACTIVE)
    {
	set(_win(obj), MUIA_Window_ActiveObject, obj);
    }
    _flags(obj) |= MADF_SETUP;

    data->mad_Timer.ihn_Flags = MUIIHNF_TIMER;
    data->mad_Timer.ihn_Method = MUIM_Timer;
    data->mad_Timer.ihn_Object = obj;

    return TRUE;
}


/**************************************************************************
 Called to match a MUIM_Setup, when environment is no more available.
**************************************************************************/
static ULONG Area_Cleanup(struct IClass *cl, Object *obj, struct MUIP_Cleanup *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);

    _flags(obj) &= ~MADF_SETUP;

    /* don't let a pending pointer on us, but be ready to reacquire
     * focus on next setup, if any. This will call GoInactive, that's why
     * we must set the active flag again.
     */
    if (data->mad_Flags & MADF_ACTIVE)
    {
	set(_win(obj), MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);
	data->mad_Flags |= MADF_ACTIVE;
    }

    if (data->mad_TitleText)
    {
	zune_text_destroy(data->mad_TitleText);
	data->mad_TitleText = NULL;
    }

//    cleanup_cycle_chain (data, obj);
    cleanup_control_char (data, obj);

    if (data->mad_Timer.ihn_Millis)
    {
	DoMethod(_app(obj), MUIM_Application_RemInputHandler, (IPTR)&data->mad_Timer);
	data->mad_Timer.ihn_Millis = 0;
    }

    /* Remove the handler if it is added */
    if (data->mad_hiehn.ehn_Events)
	DoMethod(_win(obj), MUIM_Window_RemEventHandler, (IPTR)&data->mad_hiehn);

    /* Remove the event handler if it has been added */
    if (data->mad_ehn.ehn_Events)
	DoMethod(_win(obj), MUIM_Window_RemEventHandler, (IPTR)&data->mad_ehn);

    /* It's save to call the following function with NULL */
    if ((data->mad_Flags & MADF_SHOWSELSTATE) &&
	(data->mad_InputMode != MUIV_InputMode_None))
    {
	zune_imspec_cleanup(data->mad_SelBack);
	data->mad_SelBack = NULL;
    }
    if (data->mad_Flags & MADF_OWNBG)
    {
	zune_imspec_cleanup(data->mad_Background);
	data->mad_Background = NULL;
    }

    muiRenderInfo(obj) = NULL;

    return TRUE;
}


/**************************************************************************
 Called after the window is open and the area layouted, but before
 any drawing. Matched by one MUIM_Hide.
 Good place to init things depending on gadget size/position.
**************************************************************************/
static ULONG Area_Show(struct IClass *cl, Object *obj, struct MUIP_Show *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);
    Object *activeobj;

    _flags(obj) |= MADF_CANDRAW;
/*  g_print("show %p, bg=%p (%s)\n", obj, data->mad_Background, */
/*  	zune_imspec_to_string(data->mad_Background)); */
/*  g_print("dims=%dx%d\n", _width(obj), _height(obj)); */

/*      g_print("showing %s\n", zune_area_to_string(obj)); */

    zune_imspec_show(data->mad_Background, obj);

    if (data->mad_Flags & MADF_SHOWSELSTATE
	&& data->mad_InputMode != MUIV_InputMode_None)
    {
	zune_imspec_show(data->mad_SelBack, obj);
    }

    get(_win(obj), MUIA_Window_ActiveObject, &activeobj);

    if (obj == activeobj)
	_zune_focus_new(obj, 0);

    return TRUE;
}

/**************************************************************************
 Called when the window is about to be closed, to match MUIM_Show.
**************************************************************************/
static ULONG Area_Hide(struct IClass *cl, Object *obj, struct MUIP_Hide *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);
    Object *activeobj;

    get(_win(obj), MUIA_Window_ActiveObject, &activeobj);
    if (obj == activeobj)
	_zune_focus_destroy(obj);

    zune_imspec_hide(data->mad_Background);
    if (data->mad_Flags & MADF_SHOWSELSTATE
	&& data->mad_InputMode != MUIV_InputMode_None)
    {
	zune_imspec_hide(data->mad_SelBack);
    }

    if (data->mad_ContextZMenu)
    {
	zune_close_menu(data->mad_ContextZMenu);
	data->mad_ContextZMenu = NULL;
    }

    _flags(obj) &= ~MADF_CANDRAW;
    return TRUE;
}


/**************************************************************************
 called by parent object.
**************************************************************************/
static ULONG Area_DisconnectParent(struct IClass *cl, Object *obj, struct MUIP_DisconnectParent *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);

    /* don't be active if added elsewhere */
    data->mad_Flags &= ~MADF_ACTIVE;
    return DoSuperMethodA(cl,obj,(Msg)msg);
}

/**************************************************************************
 Called when gadget activated
**************************************************************************/
static ULONG Area_GoActive(struct IClass *cl, Object *obj, Msg msg)
{
    if (!(_flags(obj) & MADF_ACTIVE))
    {
	if (_flags(obj) & MADF_CANDRAW)
	    _zune_focus_new(obj, 0);

	_flags(obj) |= MADF_ACTIVE;
    }
    return TRUE;
}

/**************************************************************************
 Called when gadget deactivated
**************************************************************************/
static ULONG Area_GoInactive(struct IClass *cl, Object *obj, Msg msg)
{
    if (_flags(obj) & MADF_ACTIVE)
    {
	if (_flags(obj) & MADF_CANDRAW)
	    _zune_focus_destroy(obj);

	_flags(obj) &= ~MADF_ACTIVE;
    }
    return TRUE;
}

/**************************************************************************
 This one or derived methods wont be called if short help is
 not set in area instdata. So set this to a dummy val if overriding
**************************************************************************/
static ULONG Area_CreateShortHelp(struct IClass *cl, Object *obj, struct MUIP_CreateShortHelp *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);

    return (ULONG)data->mad_ShortHelp;
}

/**************************************************************************
 ...
**************************************************************************/
static ULONG Area_DeleteShortHelp(struct IClass *cl, Object *obj, struct MUIP_DeleteShortHelp *msg)
{
    return TRUE;
}

/* either lmb or press key */
static void handle_press(struct IClass *cl, Object *obj)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);

    switch (data->mad_InputMode)
    {
	case MUIV_InputMode_RelVerify:
	    set(obj, MUIA_Timer, ++muiAreaData(obj)->mad_Timeval);
	    if (!data->mad_Timer.ihn_Millis)
	    {
	    	data->mad_Timer.ihn_Millis = 300;
		DoMethod(_app(obj), MUIM_Application_AddInputHandler, (IPTR)&data->mad_Timer);
	    }
	    set(obj, MUIA_Selected, TRUE);
	    set(obj, MUIA_Pressed, TRUE);
	    break;

	case MUIV_InputMode_Immediate:
	{
	    ULONG selected;

	    get(obj, MUIA_Selected, &selected);
	    if (selected)
	    {
		D(bug("handle_press(%p) : nnset MUIA_Selected FALSE\n", obj));
		nnset(obj, MUIA_Selected, FALSE);
	    }
	    D(bug("handle_press(%p) : set MUIA_Selected TRUE\n", obj));
	    set(obj, MUIA_Selected, TRUE);
	    D(bug("handle_press(%p) : done\n", obj));
	    break;
	}
	case MUIV_InputMode_Toggle:
	    set(obj, MUIA_Selected, !(data->mad_Flags & MADF_SELECTED));
	    break;
    }
}

/* either lmb or release key */
static void handle_release(struct IClass *cl, Object *obj, int cancel)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);
/*  g_print("handle release\n"); */

    if (data->mad_InputMode == MUIV_InputMode_RelVerify)
    {
	if (data->mad_Flags & MADF_SELECTED)
	{
	    if (cancel) nnset(obj, MUIA_Pressed, FALSE);
	    else set(obj, MUIA_Pressed, FALSE);

	    set(obj, MUIA_Selected, FALSE);
	}
    }

    if (data->mad_Timer.ihn_Millis)
    {
	DoMethod(_app(obj), MUIM_Application_RemInputHandler, (IPTR)&data->mad_Timer);
    	data->mad_Timer.ihn_Millis = 0;
    }

}

static ULONG event_button(Class *cl, Object *obj, struct IntuiMessage *imsg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);
    BOOL in = _between(_left(obj), imsg->MouseX, _right(obj))
           && _between(_top(obj),  imsg->MouseY, _bottom(obj));

    switch (imsg->Code)
    {
	case	SELECTDOWN:
		if (data->mad_InputMode == MUIV_InputMode_None) break;

		if (in)
		{
//		    set(_win(obj), MUIA_Window_ActiveObject, obj);
		    data->mad_ClickX = imsg->MouseX;
		    data->mad_ClickY = imsg->MouseY;
		    
		    
		    if ((data->mad_InputMode != MUIV_InputMode_Toggle) && (data->mad_Flags & MADF_SELECTED))
			break;
		    nnset(obj,MUIA_Timer,0);
		    handle_press(cl, obj);
		    if (data->mad_InputMode == MUIV_InputMode_RelVerify)
		    {
			if (data->mad_ehn.ehn_Events) DoMethod(_win(obj), MUIM_Window_RemEventHandler, (IPTR)&data->mad_ehn);
			data->mad_ehn.ehn_Events |= IDCMP_MOUSEMOVE | IDCMP_RAWKEY;
	                DoMethod(_win(obj), MUIM_Window_AddEventHandler, (IPTR)&data->mad_ehn);
	            }
		    return MUI_EventHandlerRC_Eat;
		}

	case	SELECTUP:
		if (data->mad_InputMode == MUIV_InputMode_None) break;

		if (data->mad_ehn.ehn_Events != IDCMP_MOUSEBUTTONS)
		{
		    DoMethod(_win(obj), MUIM_Window_RemEventHandler, (IPTR)&data->mad_ehn);
	            data->mad_ehn.ehn_Events = IDCMP_MOUSEBUTTONS;
		    DoMethod(_win(obj), MUIM_Window_AddEventHandler, (IPTR)&data->mad_ehn);
		    if (!in) nnset(obj, MUIA_Pressed, FALSE);
		    handle_release(cl, obj, FALSE /*cancel*/ );
		    return MUI_EventHandlerRC_Eat;
		}
		break;

	case    MENUDOWN:
		if (in && data->mad_ContextMenu)
		{
		    Object *menuobj = (Object*)DoMethod(obj, MUIM_ContextMenuBuild, imsg->MouseX, imsg->MouseY);
		    if (menuobj)
		    {
			struct NewMenu *newmenu;
			get(menuobj,MUIA_Menuitem_NewMenu,&newmenu);
			if (newmenu)
			{
			    if (data->mad_ContextZMenu) zune_close_menu(data->mad_ContextZMenu);
			    data->mad_ContextZMenu = zune_open_menu(_window(obj),newmenu);
			}

			if (data->mad_ehn.ehn_Events) DoMethod(_win(obj), MUIM_Window_RemEventHandler, (IPTR)&data->mad_ehn);
			data->mad_ehn.ehn_Events |= IDCMP_MOUSEMOVE;
	                DoMethod(_win(obj), MUIM_Window_AddEventHandler, (IPTR)&data->mad_ehn);
		    }
	            return MUI_EventHandlerRC_Eat;
        	}
	        break;

	case    MENUUP:
		if (data->mad_ContextZMenu)
		{
		    zune_close_menu(data->mad_ContextZMenu);
		    data->mad_ContextZMenu = NULL;
		    DoMethod(_win(obj), MUIM_Window_RemEventHandler, (IPTR)&data->mad_ehn);
	            data->mad_ehn.ehn_Events = IDCMP_MOUSEBUTTONS;
		    DoMethod(_win(obj), MUIM_Window_AddEventHandler, (IPTR)&data->mad_ehn);
	            return MUI_EventHandlerRC_Eat;
		}
		break;
    }

    return 0;
}

static ULONG event_motion(Class *cl, Object *obj, struct IntuiMessage *imsg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);

    if ((imsg->Qualifier & IEQUALIFIER_RBUTTON) && data->mad_ContextZMenu)
    {
	zune_mouse_update(data->mad_ContextZMenu, 0);
	return MUI_EventHandlerRC_Eat;
    }

    if (imsg->Qualifier & IEQUALIFIER_LEFTBUTTON)
    {
	BOOL in = _between(_left(obj), imsg->MouseX, _right(obj))
	           && _between(_top(obj),  imsg->MouseY, _bottom(obj));

	if (in)
	{
	    if ((data->mad_Flags & MADF_DRAGGABLE) && ((abs(data->mad_ClickX-imsg->MouseX) >= 3) || (abs(data->mad_ClickY-imsg->MouseY)>=3))) /* should be user configurable */
	    {
		if (data->mad_InputMode == MUIV_InputMode_RelVerify)
		    set(obj, MUIA_Selected, FALSE);
		nnset(obj, MUIA_Pressed, FALSE);

		if (data->mad_ehn.ehn_Events) DoMethod(_win(obj), MUIM_Window_RemEventHandler, (IPTR)&data->mad_ehn);
		data->mad_ehn.ehn_Events = IDCMP_MOUSEBUTTONS;
		DoMethod(_win(obj), MUIM_Window_AddEventHandler, (IPTR)&data->mad_ehn);
		if (data->mad_Timer.ihn_Millis)
		{
		   DoMethod(_app(obj), MUIM_Application_RemInputHandler, (IPTR)&data->mad_Timer);
		   data->mad_Timer.ihn_Millis = 0;
		}

	    	DoMethod(obj,MUIM_DoDrag, data->mad_ClickX - _left(obj), data->mad_ClickY - _top(obj), 0);
		return MUI_EventHandlerRC_Eat;
	    }
	}

	if (data->mad_InputMode == MUIV_InputMode_RelVerify)
	{
	    if (!in && (data->mad_Flags & MADF_SELECTED)) /* going out */
	    {
		set(obj, MUIA_Selected, FALSE);
	    }
	    else if (in && !(data->mad_Flags & MADF_SELECTED)) /* going in */
	    {
	        set(obj, MUIA_Selected, TRUE);
	    }
	}
    }
    return MUI_EventHandlerRC_Eat;
}

/**************************************************************************
 ...
**************************************************************************/
static ULONG Area_HandleEvent(struct IClass *cl, Object *obj, struct MUIP_HandleEvent *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);

    if (data->mad_DisableCount) return 0;
    if (data->mad_InputMode == MUIV_InputMode_None && !data->mad_ContextMenu) return 0;

    if (msg->muikey != MUIKEY_NONE)
    {
	switch (msg->muikey)
	{
	    case    MUIKEY_PRESS:
		    if (data->mad_Flags & MADF_SELECTED) break;
		    handle_press(cl, obj);
		    if (data->mad_ehn.ehn_Events) DoMethod(_win(obj), MUIM_Window_RemEventHandler, (IPTR)&data->mad_ehn);
		    data->mad_ehn.ehn_Events |= IDCMP_RAWKEY;
		    DoMethod(_win(obj), MUIM_Window_AddEventHandler, (IPTR)&data->mad_ehn);
		    return MUI_EventHandlerRC_Eat;

	    case    MUIKEY_TOGGLE:
		    if (data->mad_InputMode == MUIV_InputMode_Toggle) set(obj, MUIA_Selected, !(data->mad_Flags & MADF_SELECTED));
		    return MUI_EventHandlerRC_Eat;

	    case    MUIKEY_RELEASE:
		    handle_release(cl, obj, FALSE /* cancel */);
		    if (data->mad_ehn.ehn_Events) DoMethod(_win(obj), MUIM_Window_RemEventHandler, (IPTR)&data->mad_ehn);
		    data->mad_ehn.ehn_Events = IDCMP_MOUSEBUTTONS;
		    DoMethod(_win(obj), MUIM_Window_AddEventHandler, (IPTR)&data->mad_ehn);
		    return MUI_EventHandlerRC_Eat;
	}
	return 0;
    }

    if (msg->imsg)
    {
	switch (msg->imsg->Class)
	{
	    case IDCMP_MOUSEBUTTONS: return event_button(cl, obj, msg->imsg);
	    case IDCMP_MOUSEMOVE: return event_motion(cl, obj, msg->imsg);
	    case IDCMP_RAWKEY:
	    {
	        if (msg->imsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
	        {
		    DoMethod(_win(obj), MUIM_Window_RemEventHandler, (IPTR)&data->mad_ehn);
		    data->mad_ehn.ehn_Events = IDCMP_MOUSEBUTTONS;
		    DoMethod(_win(obj), MUIM_Window_AddEventHandler, (IPTR)&data->mad_ehn);
		    handle_release(cl,obj, TRUE /*cancel */);
		}
		return MUI_EventHandlerRC_Eat;
	    }
	    break;
	}
    }
    return 0;
}

/**************************************************************************
 ...
**************************************************************************/
static ULONG Area_HandleInput(struct IClass *cl, Object *obj, struct MUIP_HandleInput *msg)
{
    /* Actually a dummy, but real MUI does handle here the input stuff which Zune
    ** has in Area_HandleEvent. For compatibility we should do this too
    **/
    return 0;
}

/**************************************************************************
 Trivial; custom classes may override this to get dynamic menus.
**************************************************************************/
static ULONG Area_ContextMenuBuild(struct IClass *cl, Object *obj, struct MUIP_ContextMenuBuild *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);
    return (ULONG)data->mad_ContextMenu; /* a Menustrip object */
}


/**************************************************************************
 MUIM_Export : to export an objects "contents" to a dataspace object.
**************************************************************************/
static ULONG Area_Export(struct IClass *cl, Object *obj, struct MUIP_Export *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);
    STRPTR id;

    if ((id = muiNotifyData(obj)->mnd_ObjectID))
    {
    	char selected = (data->mad_Flags & MADF_SELECTED)?1:0;
	DoMethod(msg->dataspace, MUIM_Dataspace_Add, (IPTR)&selected, sizeof(char),(IPTR)id);
    }
    return 0;
}


/**************************************************************************
 MUIM_Import : to import an objects "contents" from a dataspace object.
**************************************************************************/
static ULONG Area_Import(struct IClass *cl, Object *obj, struct MUIP_Import *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);
    STRPTR id;
    //BOOL val = FALSE;

    if ((id = muiNotifyData(obj)->mnd_ObjectID))
    {
    	char *selected = (char*)DoMethod(msg->dataspace, MUIM_Dataspace_Find, (IPTR)id);

	if (selected)
	{
	    if (*selected) data->mad_Flags |= MADF_SELECTED;
	    else data->mad_Flags &= ~MADF_SELECTED;
	}
    }
    return 0;
}

/**************************************************************************
 MUIM_Timer
**************************************************************************/
static ULONG Area_Timer(struct IClass *cl, Object *obj, Msg msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);
    if (data->mad_Timer.ihn_Millis)
	DoMethod(_app(obj), MUIM_Application_RemInputHandler, (IPTR)&data->mad_Timer);
    data->mad_Timer.ihn_Millis = 50;
    DoMethod(_app(obj), MUIM_Application_AddInputHandler, (IPTR)&data->mad_Timer);

    if (data->mad_Flags & MADF_SELECTED)
	set(obj, MUIA_Timer, ++muiAreaData(obj)->mad_Timeval);
    return 0;
}

/**************************************************************************
 MUIM_DoDrag
**************************************************************************/
static ULONG Area_DoDrag(struct IClass *cl, Object *obj, struct MUIP_DoDrag *msg)
{
    //struct MUI_AreaData *data = INST_DATA(cl, obj);
    DoMethod(_win(obj), MUIM_Window_DragObject, (IPTR)obj, msg->touchx, msg->touchy, msg->flags);
    return 0;
}

/**************************************************************************
 MUIM_CreateDragImage
**************************************************************************/
static ULONG Area_CreateDragImage(struct IClass *cl, Object *obj, struct MUIP_CreateDragImage *msg)
{
    struct MUI_DragImage *img = (struct MUI_DragImage *)AllocVec(sizeof(struct MUIP_CreateDragImage),MEMF_CLEAR);
    if (img)
    {
    	struct ZuneFrameGfx *zframe;
	LONG depth = GetBitMapAttr(_screen(obj)->RastPort.BitMap,BMA_DEPTH);

	zframe = zune_zframe_get(&muiGlobalInfo(obj)->mgi_Prefs->frames[MUIV_Frame_Drag]);

    	img->width = _width(obj) + 2*zframe->xthickness;
    	img->height = _height(obj) + 2*zframe->ythickness;

    	if ((img->bm = AllocBitMap(img->width,img->height,depth,BMF_MINPLANES,_screen(obj)->RastPort.BitMap)))
    	{
    	    /* Render the stuff now */
    	    struct RastPort *rp_save = muiRenderInfo(obj)->mri_RastPort;
    	    struct RastPort temprp;
    	    InitRastPort(&temprp);
    	    temprp.BitMap = img->bm;
    	    ClipBlit(_rp(obj),_left(obj),_top(obj),&temprp,zframe->xthickness,zframe->ythickness,_width(obj),_height(obj),0xc0);

	    muiRenderInfo(obj)->mri_RastPort = &temprp;
	    zframe->draw[0](muiRenderInfo(obj), 0, 0, img->width, img->height);
	    muiRenderInfo(obj)->mri_RastPort = rp_save;
#ifdef __AROS__
	    DeinitRastPort(&temprp);
#endif
    	}

    	img->touchx = msg->touchx;
    	img->touchy = msg->touchy;
    	img->flags = 0;
    }
    return (ULONG)img;
}

/**************************************************************************
 MUIM_DeleteDragImage
**************************************************************************/
static ULONG Area_DeleteDragImage(struct IClass *cl, Object *obj, struct MUIP_DeleteDragImage *msg)
{
    if (msg->di)
    {
	if (msg->di->bm) FreeBitMap(msg->di->bm);
	FreeVec(msg->di);
    }
    return NULL;
}

/**************************************************************************
 MUIM_DragQueryExtended
**************************************************************************/
static IPTR Area_DragQueryExtended(struct IClass *cl, Object *obj, struct MUIP_DragQueryExtended *msg)
{
    struct MUI_AreaData *data = INST_DATA(cl, obj);
    if (data->mad_Flags & MADF_DROPABLE)
    {
	if (_left(obj) <= msg->x && msg->x <= _right(obj) && _top(obj) <= msg->y && msg->y <= _bottom(obj))
	{
	    if (DoMethod(obj,MUIM_DragQuery,(IPTR)msg->obj) == MUIV_DragQuery_Accept)
		return (IPTR)obj;
	}
    }
    return NULL;
}

/**************************************************************************
 MUIM_DragBegin
**************************************************************************/
static ULONG Area_DragBegin(struct IClass *cl, Object *obj, struct MUIP_DragBegin *msg)
{
    //struct MUI_AreaData *data = INST_DATA(cl, obj);
    _zune_focus_new(obj, 1);
    return 0;
}

/**************************************************************************
 MUIM_DragFinish
**************************************************************************/
static ULONG Area_DragFinish(struct IClass *cl, Object *obj, struct MUIP_DragFinish *msg)
{
    //struct MUI_AreaData *data = INST_DATA(cl, obj);
    _zune_focus_destroy(obj);
    return 0;
}


/*
 * Calculates addleft, addtop, subwidth, subheight from current settings.
 * If frame phantom, ignore horizontal frame components.
 *
 * Because of BYTE storage, all values are clamped to 0..127
 * Inner dimensions being clamped to 0..32, it shouldnt cause too much harm
 */
static void area_update_data(Object *obj, struct MUI_AreaData *data)
{
    struct ZuneFrameGfx *zframe;
    struct MUI_FrameSpec_intern *frame = &muiGlobalInfo(obj)->mgi_Prefs->frames[data->mad_Frame];

    zframe = zune_zframe_get(frame);

    if (data->mad_Flags & MADF_FRAMEPHANTOM)
    {
	data->mad_addleft = ((data->mad_Flags & MADF_INNERLEFT) ? data->mad_HardILeft : 0);
	data->mad_subwidth = data->mad_addleft + ((data->mad_Flags & MADF_INNERRIGHT) ? data->mad_HardIRight : 0);
    }   else
    {
	if (data->mad_Flags & MADF_INNERLEFT) data->mad_addleft = CLAMP(data->mad_HardILeft + zframe->xthickness, 0, 127);
	else data->mad_addleft = CLAMP(frame->innerLeft + zframe->xthickness, 0, 127);

	if (data->mad_Flags & MADF_INNERRIGHT)
	{
	    data->mad_subwidth = CLAMP(data->mad_addleft + data->mad_HardIRight  + zframe->xthickness, 0, 127);
	} else
	{
	    data->mad_subwidth = CLAMP(data->mad_addleft + frame->innerRight + zframe->xthickness, 0, 127);
	}
    }

    if (data->mad_Flags & MADF_INNERTOP)
    {
	data->mad_addtop = CLAMP(data->mad_HardITop + zframe->ythickness, 0, 127);
    }   else
    {
	data->mad_addtop = CLAMP(frame->innerTop + zframe->ythickness, 0, 127);
    }

    if (data->mad_Flags & MADF_INNERBOTTOM)
    {
	data->mad_subheight = CLAMP(data->mad_addtop + data->mad_HardIBottom + zframe->ythickness, 0, 127);
    }   else
    {
	data->mad_subheight = CLAMP(data->mad_addtop + frame->innerBottom  + zframe->ythickness, 0, 127);
    }

/*      D(bug("area_update_data(%x,%d) => addleft/top=%d/%d, subwidth/height=%d/%d\n", */
/*  	  obj, data->mad_Frame, data->mad_addleft, data->mad_addtop, data->mad_subwidth, data->mad_subheight)); */
}


BOOPSI_DISPATCHER(IPTR, Area_Dispatcher, cl, obj, msg)
{
    switch (msg->MethodID)
    {
	/* Whenever an object shall be created using NewObject(), it will be
	** sent a OM_NEW method.
	*/
	case OM_NEW: return Area_New(cl, obj, (struct opSet *) msg);
	case OM_DISPOSE: return Area_Dispose(cl, obj, msg);
	case OM_SET: return Area_Set(cl, obj, (struct opSet *)msg);
	case OM_GET: return Area_Get(cl, obj, (struct opGet *)msg);
	case MUIM_AskMinMax: return Area_AskMinMax(cl, obj, (APTR)msg);
	case MUIM_Draw: return Area_Draw(cl, obj, (APTR)msg);
	case MUIM_DrawBackground: return Area_DrawBackground(cl, obj, (APTR)msg);
	case MUIM_Setup: return Area_Setup(cl, obj, (APTR)msg);
	case MUIM_Cleanup: return Area_Cleanup(cl, obj, (APTR)msg);
	case MUIM_Show: return Area_Show(cl, obj, (APTR)msg);
	case MUIM_Hide: return Area_Hide(cl, obj, (APTR)msg);
	case MUIM_DisconnectParent: return Area_DisconnectParent(cl, obj, (APTR)msg);
	case MUIM_GoActive: return Area_GoActive(cl, obj, (APTR)msg);
	case MUIM_GoInactive: return Area_GoInactive(cl, obj, (APTR)msg);
	case MUIM_Layout: return 1;
	case MUIM_CreateShortHelp: return Area_CreateShortHelp(cl, obj, (APTR)msg);
	case MUIM_DeleteShortHelp: return Area_DeleteShortHelp(cl, obj, (APTR)msg);
	case MUIM_HandleEvent: return Area_HandleEvent(cl, obj, (APTR)msg);
	case MUIM_ContextMenuBuild: return Area_ContextMenuBuild(cl, obj, (APTR)msg);
	case MUIM_Timer: return Area_Timer(cl,obj,msg);
	case MUIM_DragQuery: return MUIV_DragQuery_Refuse;
	case MUIM_DragBegin: return Area_DragBegin(cl,obj,(APTR)msg);
	case MUIM_DragDrop: return FALSE;
	case MUIM_DragFinish: return Area_DragFinish(cl,obj,(APTR)msg);
	case MUIM_DragReport: return MUIV_DragReport_Continue; /* or MUIV_DragReport_Abort? */
	case MUIM_DoDrag: return Area_DoDrag(cl, obj, (APTR)msg);
	case MUIM_CreateDragImage: return Area_CreateDragImage(cl, obj, (APTR)msg);
	case MUIM_DeleteDragImage: return Area_DeleteDragImage(cl, obj, (APTR)msg);
	case MUIM_DragQueryExtended: return Area_DragQueryExtended(cl, obj, (APTR)msg);
	case MUIM_HandleInput: return Area_HandleInput(cl, obj, (APTR)msg);

	case MUIM_Export: return Area_Export(cl, obj, (APTR)msg);
	case MUIM_Import: return Area_Import(cl, obj, (APTR)msg);
    }

    return DoSuperMethodA(cl, obj, msg);
}


/*
 * Class descriptor.
 */
const struct __MUIBuiltinClass _MUI_Area_desc = { 
    MUIC_Area, 
    MUIC_Notify, 
    sizeof(struct MUI_AreaData), 
    (void*)Area_Dispatcher 
};

