/*
    Copyright � 2002-2003, The AROS Development Team. All rights reserved.
    $Id$
*/

#define MUIMASTER_YES_INLINE_STDARG

#include <graphics/gfx.h>
#include <graphics/view.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>

#include "mui.h"
#include "muimaster_intern.h"
#include "support.h"
#include "support_classes.h"

extern struct Library *MUIMasterBase;

struct Settingsgroup_DATA
{
    int dummy;
};

/**************************************************************************
 OM_NEW
**************************************************************************/
static IPTR Settingsgroup_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
    struct MUI_Settingsgroup *data;
    struct TagItem  	    *tag, *tags;

    obj = (Object *)DoSuperNewTags(cl, obj, NULL,
			TAG_MORE, msg->ops_AttrList);
    if (!obj) return FALSE;
    
    data = INST_DATA(cl, obj);

    /* parse initial taglist */

    for (tags = msg->ops_AttrList; (tag = NextTagItem(&tags)); )
    {
	switch (tag->ti_Tag)
	{
    	}
    }

    return (IPTR)obj;
}

/**************************************************************************
 MUIM_Settingsgroup_ConfigToGadgets
**************************************************************************/
static IPTR Settingsgroup_ConfigToGadgets(struct IClass *cl, Object *obj, struct MUIP_Settingsgroup_ConfigToGadgets *msg)
{
    //struct MUI_Settingsgroup *data = INST_DATA(cl, obj);
    return 0;
}

/**************************************************************************
 MUIM_Settingsgroup_GadgetsToConfig
**************************************************************************/
static IPTR Settingsgroup_GadgetsToConfig(struct IClass *cl, Object *obj, struct MUIP_Settingsgroup_GadgetsToConfig *msg)
{
    //struct MUI_Settingsgroup *data = INST_DATA(cl, obj);
    return 0;
}

#if ZUNE_BUILTIN_SETTINGSGROUP
BOOPSI_DISPATCHER(IPTR, Settingsgroup_Dispatcher, cl, obj, msg)
{
    switch (msg->MethodID)
    {
	case OM_NEW:
	    return Settingsgroup_New(cl, obj, (struct opSet *)msg);
	case MUIM_Settingsgroup_ConfigToGadgets:
	    return Settingsgroup_ConfigToGadgets(cl,obj,(APTR)msg);
	case MUIM_Settingsgroup_GadgetsToConfig:
	    return Settingsgroup_GadgetsToConfig(cl,obj,(APTR)msg);
    }
    
    return DoSuperMethodA(cl, obj, msg);
}

const struct __MUIBuiltinClass _MUI_Settingsgroup_desc =
{ 
    MUIC_Settingsgroup,
    MUIC_Group, 
    sizeof(struct Settingsgroup_DATA), 
    (void*)Settingsgroup_Dispatcher 
};
#endif /* ZUNE_BUILTIN_SETTINGSGROUP */
