/*
    Copyright � 1995-2002, The AROS Development Team. All rights reserved.
    $Id$
*/

#define DEBUG 1
#include <aros/debug.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/imageclass.h>
#include <libraries/mui.h>

#include "support.h"
#include "worker.h"
#include "desktop_intern.h"

#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/input.h>
#include <proto/muimaster.h>
#include <proto/intuition.h>
#include <proto/utility.h>

#include "presentation.h"
#include "iconcontainerclass.h"
#include "iconclass.h"

#include "desktop_intern_protos.h"

#include <string.h>

static ULONG DoSetupMethod(Object *obj, struct MUI_RenderInfo *info)
{
	/* MUI set the correct render info *before* it calls MUIM_Setup so please only use this function instead of DoMethodA() */
    muiRenderInfo(obj)=info;

	return DoMethod(obj, MUIM_Setup, info);
}

IPTR iconNew(Class *cl, Object *obj, struct opSet *msg)
{
	IPTR retval=0;
	struct IconClassData *data;
	struct TagItem *tag;
	struct DiskObject *diskobject=NULL;
	UBYTE *label=NULL;
	BOOL selected=FALSE;
	BOOL script=FALSE, pure=FALSE, archived=FALSE, readable=FALSE, writeable=FALSE, executable=FALSE, deleteable=FALSE;
	UBYTE *comment=NULL;
	UBYTE viewMode=IAVM_LARGEICON;
	struct DateStamp *lastChanged=NULL;
	LONG type=0;
	ULONG size=0;
	Object *desktop=NULL;

	tag=FindTagItem(IA_DiskObject, msg->ops_AttrList);
	if(tag)
	{
		diskobject=(struct DiskObject*)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Label, msg->ops_AttrList);
	if(tag)
	{
		label=(UBYTE*)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Selected, msg->ops_AttrList);
	if(tag)
	{
		selected=(BOOL)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Comment, msg->ops_AttrList);
	if(tag)
	{
		comment=(UBYTE*)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Script, msg->ops_AttrList);
	if(tag)
	{
		script=(BOOL)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Pure, msg->ops_AttrList);
	if(tag)
	{
		pure=(BOOL)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Archived, msg->ops_AttrList);
	if(tag)
	{
		archived=(BOOL)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Readable, msg->ops_AttrList);
	if(tag)
	{
		readable=(BOOL)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Writeable, msg->ops_AttrList);
	if(tag)
	{
		writeable=(BOOL)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Executable, msg->ops_AttrList);
	if(tag)
	{
		executable=(BOOL)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Deleteable, msg->ops_AttrList);
	if(tag)
	{
		deleteable=(BOOL)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_ViewMode, msg->ops_AttrList);
	if(tag)
	{
		viewMode=(UBYTE)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Type, msg->ops_AttrList);
	if(tag)
	{
		type=(LONG)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Size, msg->ops_AttrList);
	if(tag)
	{
		size=(ULONG)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_LastModified, msg->ops_AttrList);
	if(tag)
	{
		lastChanged=(struct DateStamp*)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	tag=FindTagItem(IA_Desktop, msg->ops_AttrList);
	if(tag)
	{
		desktop=(Object*)tag->ti_Data;
		tag->ti_Tag=TAG_IGNORE;
	}

	retval=DoSuperMethodA(cl, obj, (Msg)msg);

	if(retval)
	{
		obj=(Object*)retval;
		data=INST_DATA(cl, obj);
		data->diskObject=diskobject;
		data->label=label;
		data->selected=selected;
		data->viewMode=viewMode;
		data->type=type;
		data->size=size;
		if(lastChanged)
			data->lastChanged=*lastChanged;

		data->ehn.ehn_Priority=10;
		data->ehn.ehn_Flags=(1<<1);
		data->ehn.ehn_Object=obj;
		data->ehn.ehn_Class=cl;
		data->ehn.ehn_Events=IDCMP_MOUSEBUTTONS;

		data->imagePart=NULL;
		data->labelPart=NULL;
		data->sizePart=NULL;
		data->typePart=NULL;
		data->lastModifiedPart=NULL;

		data->desktop=desktop;
	}

	return retval;
}

IPTR iconSet(Class *cl, Object *obj, struct opSet *msg)
{
	struct IconClassData *data;
	IPTR retval=1;
	struct TagItem *tag, *tstate=msg->ops_AttrList;

	data=(struct IconClassData*)INST_DATA(cl, obj);

	while((tag=NextTagItem(&tstate)))
	{
		switch(tag->ti_Tag)
		{
			case IA_Label:
				SetAttrs(data->labelPart, MUIA_Text_Contents, tag->ti_Data, TAG_END);
				break;
			case IA_Selected:
				data->selected=tag->ti_Data;
				SetAttrs(data->imagePart, MUIA_Selected, tag->ti_Data, TAG_END);
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
			case IA_Executed:
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
			case IA_Directory:
				// this is the same as moving a file
				data->directory=(UBYTE*)tag->ti_Data;
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
			case IA_Comment:
				data->comment=(UBYTE*)tag->ti_Data;
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
			case IA_Script:
				data->script=(BOOL)tag->ti_Data;
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
			case IA_Pure:
				data->pure=(BOOL)tag->ti_Data;
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
			case IA_Archived:
				data->archived=(BOOL)tag->ti_Data;
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
			case IA_Readable:
				data->readable=(BOOL)tag->ti_Data;
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
			case IA_Writeable:
				data->writeable=(BOOL)tag->ti_Data;
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
			case IA_Executable:
				data->executable=(BOOL)tag->ti_Data;
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
			case IA_Deleteable:
				data->deleteable=(BOOL)tag->ti_Data;
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
			case IA_Type:
				data->type=(LONG)tag->ti_Data;
				break;
			case IA_Size:
				data->size=(ULONG)tag->ti_Data;
				break;
			case IA_LastModified:
				data->lastChanged=*((struct DateStamp*)tag->ti_Data);
				break;
			case IA_ViewMode:
			{
				data->viewMode=(ULONG)tag->ti_Data;
				if(data->viewMode==IAVM_DETAIL)
				{
					if(data->sizePart)
					{
//						muiNotifyData(data->sizePart)->mnd_ParentObject = obj;
//						DoMethod(data->sizePart, MUIM_ConnectParent, obj);
//						DoSetupMethod(data->sizePart, muiRenderInfo(obj));
					}
					if(data->typePart)
					{
//						muiNotifyData(data->typePart)->mnd_ParentObject = obj;
//						DoMethod(data->typePart, MUIM_ConnectParent, obj);
//						DoSetupMethod(data->typePart, muiRenderInfo(obj));
					}
					if(data->lastModifiedPart)
					{
//						muiNotifyData(data->lastModifiedPart)->mnd_ParentObject = obj;
//						DoMethod(data->lastModifiedPart, MUIM_ConnectParent, obj);
//						DoSetupMethod(data->lastModifiedPart, muiRenderInfo(obj));
					}
				}
				else
				{
					if(data->sizePart)
					{
//						DoMethod(data->sizePart, MUIM_Cleanup);
						// these should be disposed of....
//						data->sizePart=NULL;
					}
					if(data->typePart)
					{
//						DoMethod(data->typePart, MUIM_Cleanup);
//						data->typePart=NULL;
					}
					if(data->lastModifiedPart)
					{
//						DoMethod(data->lastModifiedPart, MUIM_Cleanup);
//						data->lastModifiedPart=NULL;
					}
				}

				break;
			}
			default:
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
		}
	}

	return retval;
}

IPTR iconGet(Class *cl, Object *obj, struct opGet *msg)
{
	IPTR retval=1;
	struct IconClassData *data;

	data=(struct IconClassData*)INST_DATA(cl, obj);

	switch(msg->opg_AttrID)
	{
		case IA_Desktop:
			*msg->opg_Storage=(ULONG)data->desktop;
			break;
		case IA_DiskObject:
			*msg->opg_Storage=(ULONG)data->diskObject;
			break;
		case IA_Label:
			*msg->opg_Storage=(ULONG)data->label;
			break;
		case IA_Selected:
			*msg->opg_Storage=(ULONG)data->selected;
			break;
		case IA_Directory:
			*msg->opg_Storage=(ULONG)data->directory;
			break;
		case IA_Comment:
			*msg->opg_Storage=(ULONG)data->comment;
			break;
		case IA_Script:
			*msg->opg_Storage=(ULONG)data->script;
			break;
		case IA_Pure:
			*msg->opg_Storage=(ULONG)data->pure;
			break;
		case IA_Archived:
			*msg->opg_Storage=(ULONG)data->archived;
			break;
		case IA_Readable:
			*msg->opg_Storage=(ULONG)data->readable;
			break;
		case IA_Writeable:
			*msg->opg_Storage=(ULONG)data->writeable;
			break;
		case IA_Executable:
			*msg->opg_Storage=(ULONG)data->executable;
			break;
		case IA_Deleteable:
			*msg->opg_Storage=(ULONG)data->deleteable;
			break;
		default:
			retval=DoSuperMethodA(cl, obj, (Msg)msg);
			break;
	}

	return retval;
}

IPTR iconDispose(Class *cl, Object *obj, Msg msg)
{
	IPTR retval;
	struct IconClassData *data;

	data=(struct IconClassData*)INST_DATA(cl, obj);

	retval=DoSuperMethodA(cl, obj, msg);

	return retval;
}

IPTR iconHandleInput(Class *cl, Object *obj, struct MUIP_HandleInput *msg)
{
	IPTR retval=0;
	struct IconClassData *data;

	data=(struct IconClassData*)INST_DATA(cl, obj);

	if(msg->imsg)
	{
		switch(msg->imsg->Class)
		{
			case IDCMP_MOUSEBUTTONS:
			{
				if(msg->imsg->Code==SELECTDOWN)
				{
					if(msg->imsg->MouseX >= _mleft(obj) && msg->imsg->MouseX <= _mright(obj) &&
						msg->imsg->MouseY >= _mtop(obj) && msg->imsg->MouseY <= _mbottom(obj))
					{
						ULONG nowSeconds=0, nowMicros=0;

						CurrentTime(&nowSeconds, &nowMicros);

						if(data->selected)
						{
							if(DoubleClick(data->lastClickSecs, data->lastClickMicros, nowSeconds, nowMicros))
								SetAttrs(obj, IA_Executed, TRUE, TAG_END);
							else
								SetAttrs(obj, IA_Selected, TRUE, TAG_END);
						}
						else
							SetAttrs(obj, IA_Selected, TRUE, TAG_END);

						data->lastClickSecs=nowSeconds;
						data->lastClickMicros=nowMicros;
					}
					else
					{
						if(data->selected==TRUE)
						{
							UWORD qualifiers;

							qualifiers=PeekQualifier();
							if(!(qualifiers & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)))
								SetAttrs(obj, IA_Selected, FALSE, TAG_END);
						}
					}
				}
				break;
			}
			default:
				retval=DoSuperMethodA(cl, obj, (Msg)msg);
				break;
		}
	}

	return retval;
}

IPTR iconSetup(Class *cl, Object *obj, struct MUIP_Setup *msg)
{
	IPTR retval=0;
	struct IconClassData *data;

	data=(struct IconClassData*)INST_DATA(cl, obj);

	retval=DoSuperMethodA(cl, obj, (Msg)msg);

	DoSetupMethod(data->imagePart, muiRenderInfo(obj));
	DoSetupMethod(data->labelPart, muiRenderInfo(obj));

	if(data->viewMode==IAVM_DETAIL)
	{
		if(data->sizePart)
			DoSetupMethod(data->sizePart, muiRenderInfo(obj));
		if(data->typePart)
			DoSetupMethod(data->typePart, muiRenderInfo(obj));
		if(data->lastModifiedPart)
			DoSetupMethod(data->lastModifiedPart, muiRenderInfo(obj));
	}

	DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehn);

	return retval;
}

IPTR iconCleanup(Class *cl, Object *obj, struct MUIP_Cleanup *msg)
{
	IPTR retval=0;
	struct IconClassData *data;

	data=(struct IconClassData*)INST_DATA(cl, obj);

	DoMethod(obj, MUIM_Window_RemEventHandler, &data->ehn);

	DoMethodA(data->imagePart, (Msg)msg);
	DoMethodA(data->labelPart, (Msg)msg);
	if(data->viewMode==IAVM_DETAIL)
	{
		if(data->sizePart)
			DoMethodA(data->sizePart, (Msg)msg);
		if(data->typePart)
			DoMethodA(data->typePart, (Msg)msg);
		if(data->lastModifiedPart)
			DoMethodA(data->lastModifiedPart, (Msg)msg);
	}

	retval=DoSuperMethodA(cl, obj, (Msg)msg);

	return retval;
}


IPTR iconDraw(Class *cl, Object *obj, struct MUIP_Draw *msg)
{
	IPTR retval;
	struct IconClassData *data;

	data=(struct IconClassData*)INST_DATA(cl, obj);

	retval=DoSuperMethodA(cl, obj, (Msg)msg);

	MUI_Redraw(data->imagePart, MADF_DRAWOBJECT);
	MUI_Redraw(data->labelPart, MADF_DRAWOBJECT);
	if(data->viewMode==IAVM_DETAIL)
	{
		if(data->sizePart)
			MUI_Redraw(data->sizePart, MADF_DRAWOBJECT);
		if(data->typePart)
			MUI_Redraw(data->typePart, MADF_DRAWOBJECT);
		if(data->lastModifiedPart)
			MUI_Redraw(data->lastModifiedPart, MADF_DRAWOBJECT);
	}


	return retval;
}

#define MAX(x,y) (x>y?x:y)

IPTR iconAskMinMax(Class *cl, Object *obj, struct MUIP_AskMinMax *msg)
{
	IPTR retval;
	struct MUI_MinMax minMax;
	struct IconClassData *data;
	ULONG iconWidth, iconHeight, labelWidth, labelHeight;
	ULONG sizeWidth, sizeHeight, typeWidth, typeHeight;
	ULONG lastModWidth, lastModHeight;

	data=(struct IconClassData*)INST_DATA(cl, obj);

	retval=DoSuperMethodA(cl, obj, (Msg)msg);

	if(data->viewMode==IAVM_LARGEICON)
	{
		iconWidth=30;
		iconHeight=30;
	}
	else
	{
		iconWidth=15;
		iconHeight=15;
	}

	// do minmax for icon
	minMax.MinWidth=0;
	minMax.DefWidth=0;
	minMax.MaxWidth=0;
	minMax.MinHeight=0;
	minMax.DefHeight=0;
	minMax.MaxHeight=0;
	DoMethod(data->imagePart, MUIM_AskMinMax, &minMax);

	_minwidth(data->imagePart)=iconWidth;
	_minheight(data->imagePart)=iconHeight;
    _maxwidth(data->imagePart)=iconWidth;
    _maxheight(data->imagePart)=iconHeight;
    _defwidth(data->imagePart)=iconWidth;
    _defheight(data->imagePart)=iconHeight;

	// do minmax for label
	minMax.MinWidth=0;
	minMax.DefWidth=0;
	minMax.MaxWidth=0;
	minMax.MinHeight=0;
	minMax.DefHeight=0;
	minMax.MaxHeight=0;
	DoMethod(data->labelPart, MUIM_AskMinMax, &minMax);

	_minwidth(data->labelPart)=minMax.MinWidth;
    _minheight(data->labelPart)=minMax.MinHeight;
    _maxwidth(data->labelPart)=minMax.MaxWidth;
    _maxheight(data->labelPart)=minMax.MaxHeight;
    _defwidth(data->labelPart)=minMax.DefWidth;
    _defheight(data->labelPart)=minMax.DefHeight;

	labelWidth=minMax.DefWidth;
	labelHeight=minMax.DefHeight;

	if(data->viewMode==IAVM_DETAIL)
	{
		// do minmax for size
		if(data->sizePart)
		{
			minMax.MinWidth=0;
			minMax.DefWidth=0;
			minMax.MaxWidth=0;
			minMax.MinHeight=0;
			minMax.DefHeight=0;
			minMax.MaxHeight=0;
			DoMethod(data->sizePart, MUIM_AskMinMax, &minMax);

			_minwidth(data->sizePart)=minMax.MinWidth;
			_minheight(data->sizePart)=minMax.MinHeight;
			_maxwidth(data->sizePart)=minMax.MaxWidth;
			_maxheight(data->sizePart)=minMax.MaxHeight;
			_defwidth(data->sizePart)=minMax.DefWidth;
			_defheight(data->sizePart)=minMax.DefHeight;

			sizeHeight=_defheight(data->sizePart);
			sizeWidth=_defwidth(data->sizePart);
		}


		if(data->typePart)
		{
			// do minmax for type
			minMax.MinWidth=0;
			minMax.DefWidth=0;
			minMax.MaxWidth=0;
			minMax.MinHeight=0;
			minMax.DefHeight=0;
			minMax.MaxHeight=0;
			DoMethod(data->typePart, MUIM_AskMinMax, &minMax);

			_minwidth(data->typePart)=minMax.MinWidth;
			_minheight(data->typePart)=minMax.MinHeight;
			_maxwidth(data->typePart)=minMax.MaxWidth;
			_maxheight(data->typePart)=minMax.MaxHeight;
			_defwidth(data->typePart)=minMax.DefWidth;
			_defheight(data->typePart)=minMax.DefHeight;

			typeHeight=_defheight(data->typePart);
			typeWidth=_defwidth(data->typePart);
		}

		if(data->lastModifiedPart)
		{
			// do minmax for last-modified
			minMax.MinWidth=0;
			minMax.DefWidth=0;
			minMax.MaxWidth=0;
			minMax.MinHeight=0;
			minMax.DefHeight=0;
			minMax.MaxHeight=0;
			DoMethod(data->lastModifiedPart, MUIM_AskMinMax, &minMax);

			_minwidth(data->lastModifiedPart)=minMax.MinWidth;
			_minheight(data->lastModifiedPart)=minMax.MinHeight;
			_maxwidth(data->lastModifiedPart)=minMax.MaxWidth;
			_maxheight(data->lastModifiedPart)=minMax.MaxHeight;
			_defwidth(data->lastModifiedPart)=minMax.DefWidth;
			_defheight(data->lastModifiedPart)=minMax.DefHeight;

			lastModHeight=_defheight(data->lastModifiedPart);
			lastModWidth=_defwidth(data->lastModifiedPart);
		}
	}

	switch(data->viewMode)
	{
		case IAVM_LARGEICON:
			msg->MinMaxInfo->DefWidth+=(MAX(iconWidth, labelWidth));
			msg->MinMaxInfo->DefHeight+=(iconWidth+labelHeight);
			msg->MinMaxInfo->MinHeight+=(iconWidth+labelHeight);
			msg->MinMaxInfo->MaxHeight+=(iconWidth+labelHeight);
			msg->MinMaxInfo->MinWidth+=(MAX(iconWidth, labelWidth));
			msg->MinMaxInfo->MaxWidth+=(MAX(iconWidth, labelWidth));
			break;
		case IAVM_SMALLICON:
			msg->MinMaxInfo->DefWidth+=(iconWidth+labelWidth);
			msg->MinMaxInfo->DefHeight+=(MAX(iconHeight, labelHeight));
			msg->MinMaxInfo->MinHeight+=(MAX(iconHeight, labelHeight));
			msg->MinMaxInfo->MaxHeight+=(MAX(iconHeight, labelHeight));
			msg->MinMaxInfo->MinWidth+=(iconWidth+labelWidth);
			msg->MinMaxInfo->MaxWidth+=(iconWidth+labelWidth);

			break;
		case IAVM_DETAIL:
		{
			struct DetailColumn *label, *size, *type, *modified;

			label=(struct DetailColumn*)DoMethod(_parent(obj), ICM_GetColumn, IA_Label);
			size=(struct DetailColumn*)DoMethod(_parent(obj), ICM_GetColumn, IA_Size);
			type=(struct DetailColumn*)DoMethod(_parent(obj), ICM_GetColumn, IA_Type);
			modified=(struct DetailColumn*)DoMethod(_parent(obj), ICM_GetColumn, IA_LastModified);

			msg->MinMaxInfo->DefWidth+=(modified->dc_X+modified->dc_Width);
			msg->MinMaxInfo->DefHeight+=(MAX(iconHeight, labelHeight));
			msg->MinMaxInfo->MinWidth+=(modified->dc_X+modified->dc_Width);
			msg->MinMaxInfo->MinHeight+=(MAX(iconHeight, labelHeight));
			msg->MinMaxInfo->MaxWidth+=(modified->dc_X+modified->dc_Width);
			msg->MinMaxInfo->MaxHeight+=(MAX(iconHeight, labelHeight));
			break;
		}
	}

	return retval;
}

IPTR iconShow(Class *cl, Object *obj, Msg msg)
{
	IPTR retval;
	struct IconClassData *data;

	data=(struct IconClassData*)INST_DATA(cl, obj);

	retval=DoSuperMethodA(cl, obj, (Msg)msg);

	DoMethodA(data->imagePart, msg);
	DoMethodA(data->labelPart, msg);
	if(data->viewMode==IAVM_DETAIL)
	{
		if(data->sizePart)
			DoMethodA(data->sizePart, msg);
		if(data->typePart)
			DoMethodA(data->typePart, msg);
		if(data->lastModifiedPart)
			DoMethodA(data->lastModifiedPart, msg);
	}

	return retval;
}

IPTR iconConnectParent(Class *cl, Object *obj, struct MUIP_ConnectParent *msg)
{
	struct IconClassData *data;
	IPTR retval;
	ULONG iconSize;

	data=(struct IconClassData*)INST_DATA(cl, obj);

	if(data->viewMode==IAVM_LARGEICON)
		iconSize=30;
	else
		iconSize=15;

	if(!data->imagePart)
	{
		data->imagePart=RectangleObject,
			ButtonFrame,
			MUIA_FixHeight, iconSize,
			MUIA_FixWidth, iconSize,
			End;
	}

	if(!data->labelPart)
	{
		data->labelPart=TextObject,
			MUIA_Text_Contents, data->label,
			End;
	}

	if(data->viewMode==IAVM_DETAIL)
	{
		struct DetailColumn *dc;

		dc=(struct DetailColumn*)DoMethod(_parent(obj), ICM_GetColumn, IA_Size);
		if(dc)
		{
			UBYTE *buffer;

			buffer=AllocVec(sizeof(UBYTE)*18, MEMF_ANY);

			__sprintf(buffer, "%u", data->size);

			data->sizePart=TextObject,
				MUIA_Text_Contents, buffer,
				End;

		}

		dc=(struct DetailColumn*)DoMethod(_parent(obj), ICM_GetColumn, IA_Type);
		if(dc)
		{
			UBYTE *buffer;
			buffer=AllocVec(sizeof(UBYTE)*10, MEMF_ANY);

			// this will change to tool/project/drawer/etc...
			if(data->type<0)
				strcpy(buffer, "File");
			else if(data->type>0)
				strcpy(buffer, "Drawer");
			else
				strcpy(buffer, "Unknown");

			data->typePart=TextObject,
				MUIA_Text_Contents, buffer,
				End;

		}

		dc=(struct DetailColumn*)DoMethod(_parent(obj), ICM_GetColumn, IA_LastModified);
		if(dc)
		{
			struct DateTime dt;
			UBYTE day[LEN_DATSTRING];
			UBYTE date[LEN_DATSTRING];
			UBYTE time[LEN_DATSTRING];
			UBYTE *buffer;

			buffer=AllocVec(sizeof(UBYTE)*(LEN_DATSTRING*3), MEMF_ANY);

			dt.dat_Stamp=data->lastChanged;
			dt.dat_Format=FORMAT_DOS;
			dt.dat_Flags=DTF_SUBST;
			dt.dat_StrDay=day;
			dt.dat_StrDate=date;
			dt.dat_StrTime=time;

			if(DateToStr(&dt))
			{
				if(strcmp(dt.dat_StrDay, "Yesterday")==0)
					strcpy(buffer, dt.dat_StrDay);
				else if(strcmp(dt.dat_StrDay, "Today")==0)
					strcpy(buffer, dt.dat_StrDay);
				else
					strcpy(buffer, dt.dat_StrDate);

				strcat(buffer, " ");
				strcat(buffer, dt.dat_StrTime);
			}
			else
				kprintf("ERROR\n");

			data->lastModifiedPart=TextObject,
				MUIA_Text_Contents, buffer,
				End;

		}
	}

	retval=DoSuperMethodA(cl, obj, (Msg)msg);

	data=(struct IconClassData*)INST_DATA(cl, obj);

	muiNotifyData(data->imagePart)->mnd_ParentObject=obj;
	DoMethod(data->imagePart, MUIM_ConnectParent, obj);
	muiNotifyData(data->labelPart)->mnd_ParentObject=obj;
	DoMethod(data->labelPart, MUIM_ConnectParent, obj);

	if(data->viewMode==IAVM_DETAIL)
	{
		if(data->sizePart)
		{
			muiNotifyData(data->sizePart)->mnd_ParentObject=obj;
			DoMethodA(data->sizePart, (Msg)msg);
			DoSetupMethod(data->sizePart, muiRenderInfo(obj));
		}
		if(data->typePart)
		{
			muiNotifyData(data->typePart)->mnd_ParentObject=obj;
			DoMethodA(data->typePart, (Msg)msg);
			DoSetupMethod(data->typePart, muiRenderInfo(obj));
		}
		if(data->lastModifiedPart)
		{
			muiNotifyData(data->lastModifiedPart)->mnd_ParentObject=obj;
			DoMethodA(data->lastModifiedPart, (Msg)msg);
			DoSetupMethod(data->lastModifiedPart, muiRenderInfo(obj));
		}
	}

	return retval;
}


ULONG iconLayout(Class *cl, Object *obj, Msg msg)
{
	ULONG retval;
	struct IconClassData *data;

	data=INST_DATA(cl, obj);
	retval=DoSuperMethodA(cl, obj, msg);

	switch(data->viewMode)
	{
		case IAVM_LARGEICON:
			MUI_Layout(data->imagePart, (_defwidth(obj)/2)-(_defwidth(data->imagePart)/2), 0, _defwidth(data->imagePart), _defheight(data->imagePart), 0);
			MUI_Layout(data->labelPart, (_defwidth(obj)/2)-(_defwidth(data->labelPart)/2), _defheight(data->imagePart)+1, _defwidth(data->labelPart), _defheight(data->labelPart), 0);
			break;
		case IAVM_SMALLICON:
			MUI_Layout(data->imagePart, 0, (_defheight(obj)/2)-(_defheight(data->imagePart)/2), _defwidth(data->imagePart), _defheight(data->imagePart), 0);
			MUI_Layout(data->labelPart, _defwidth(data->imagePart)+1, (_defheight(obj)/2)-(_defheight(data->labelPart)/2), _defwidth(data->labelPart), _defheight(data->labelPart), 0);
			break;
		case IAVM_DETAIL:
		{
			struct DetailColumn *dc;

			dc=(struct DetailColumn*)DoMethod(_parent(obj), ICM_GetColumn, IA_Label);
			if(dc)
			{
				MUI_Layout(data->imagePart, dc->dc_X, (_defheight(obj)/2)-(_defheight(data->labelPart)/2), _defwidth(data->imagePart), _defheight(data->imagePart), 0);
				MUI_Layout(data->labelPart, _defwidth(data->imagePart)+dc->dc_X+1, (_defheight(obj)/2)-(_defheight(data->labelPart)/2), _defwidth(data->labelPart), _defheight(data->labelPart), 0);
			}

			if(data->sizePart)
			{
				dc=(struct DetailColumn*)DoMethod(_parent(obj), ICM_GetColumn, IA_Size);
				if(dc)
					MUI_Layout(data->sizePart, dc->dc_X, (_defheight(obj)/2)-(_defheight(data->labelPart)/2), _defwidth(data->sizePart), _defheight(data->sizePart), 0);
			}
			if(data->typePart)
			{
				dc=(struct DetailColumn*)DoMethod(_parent(obj), ICM_GetColumn, IA_Type);
				if(dc)
					MUI_Layout(data->typePart, dc->dc_X, (_defheight(obj)/2)-(_defheight(data->labelPart)/2), _defwidth(data->typePart), _defheight(data->typePart), 0);
			}
			if(data->lastModifiedPart)
			{
				dc=(struct DetailColumn*)DoMethod(_parent(obj), ICM_GetColumn, IA_LastModified);
				if(dc)
					MUI_Layout(data->lastModifiedPart, dc->dc_X, (_defheight(obj)/2)-(_defheight(data->labelPart)/2), _defwidth(data->lastModifiedPart), _defheight(data->lastModifiedPart), 0);
			}
			break;
		}
	}

	return retval;
}

AROS_UFH3(IPTR, iconDispatcher,
	AROS_UFHA(Class  *, cl,  A0),
	AROS_UFHA(Object *, obj, A2),
	AROS_UFHA(Msg     , msg, A1))
{
	ULONG retval=0;

	switch(msg->MethodID)
	{
		case OM_NEW:
			retval=iconNew(cl, obj, (struct opSet*)msg);
			break;
		case OM_SET:
			retval=iconSet(cl, obj, (struct opSet*)msg);
			break;
		case OM_GET:
			retval=iconGet(cl, obj, (struct opGet*)msg);
			break;
		case OM_DISPOSE:
			retval=iconDispose(cl, obj, msg);
			break;
		case MUIM_HandleInput:
			retval=iconHandleInput(cl, obj, (struct MUIP_HandleInput*)msg);
			break;
		case MUIM_Setup:
			retval=iconSetup(cl, obj, (struct MUIP_Setup*)msg);
			break;
		case MUIM_Cleanup:
			retval=iconCleanup(cl, obj, (struct MUIP_Cleanup*)msg);
			break;
		case MUIM_Show:
			retval=iconShow(cl, obj, (struct MUIP_Setup*)msg);
			break;
		case MUIM_Draw:
			retval=iconDraw(cl, obj, (struct MUIP_Draw*)msg);
			break;
		case MUIM_ConnectParent:
			retval=iconConnectParent(cl, obj, (struct MUIP_ConnectParent*)msg);
			break;
		case MUIM_DragQuery:
			return 1;
			break;
		case MUIM_Layout:
			retval=iconLayout(cl, obj, msg);
			break;
		case MUIM_AskMinMax:
			retval=iconAskMinMax(cl, obj, (struct MUIP_AskMinMax*)msg);
			break;
		case MUIM_DrawBackground:
			break;
		default:
		{
			struct IconClassData *data;
			data=INST_DATA(cl, obj);
			DoMethodA(data->imagePart, msg);
			DoMethodA(data->labelPart, msg);
			if(data->viewMode==IAVM_DETAIL)
			{
				if(data->sizePart)
					DoMethodA(data->sizePart, msg);
				if(data->typePart)
					DoMethodA(data->typePart, msg);
				if(data->lastModifiedPart)
					DoMethodA(data->lastModifiedPart, msg);
			}
			retval=DoSuperMethodA(cl, obj, msg);
			break;
		}
	}

	return retval;
}

