#ifndef _MUI_CLASSES_ICONLIST_H
#define _MUI_CLASSES_ICONLIST_H

/*
	Copyright  2002-2007, The AROS Development Team. All rights reserved.
	$Id$
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuitionbase.h>
#include <intuition/classusr.h>
#include <clib/alib_protos.h>
#include <utility/utility.h>
#include <dos/dosextens.h>
#include <libraries/mui.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <proto/intuition.h>

#include "iconlist_attributes.h"

/****************************************************************************/
/*** Name *******************************************************************/
#define MUIC_IconList "IconList.mui"

/*** Methods ****************************************************************/
#define MUIM_IconList_Clear             (MUIB_IconList | 0x00000000)
#define MUIM_IconList_Update            (MUIB_IconList | 0x00000001)
#define MUIM_IconList_RethinkDimensions (MUIB_IconList | 0x00000002)
#define MUIM_IconList_CreateEntry       (MUIB_IconList | 0x00000010) /* returns 0 For Failure or (struct IconEntry *) */
#define MUIM_IconList_DestroyEntry      (MUIB_IconList | 0x00000011)
#define MUIM_IconList_DrawEntry         (MUIB_IconList | 0x00000012)
#define MUIM_IconList_DrawEntryLabel    (MUIB_IconList | 0x00000013)
#define MUIM_IconList_SelectAll         (MUIB_IconList | 0x00000020)
#define MUIM_IconList_UnselectAll       (MUIB_IconList | 0x00000021)
#define MUIM_IconList_GetIconPrivate    (MUIB_IconList | 0x00000022)
#define MUIM_IconList_NextIcon          (MUIB_IconList | 0x00000025)
#define MUIM_IconList_Sort              (MUIB_IconList | 0x00000031)
#define MUIM_IconList_CoordsSort        (MUIB_IconList | 0x00000032)
#define MUIM_IconList_PositionIcons     (MUIB_IconList | 0x00000033)
#define MUIM_IconList_ViewIcon          (MUIB_IconList | 0x00000034)

struct MUIP_IconList_Clear              {STACKED ULONG MethodID;};
struct MUIP_IconList_Update             {STACKED ULONG MethodID;};
struct MUIP_IconList_RethinkDimensions  {STACKED ULONG MethodID; STACKED struct IconEntry *singleicon;};
struct MUIP_IconList_CreateEntry        {STACKED ULONG MethodID; STACKED char *filename; STACKED char *label; STACKED struct FileInfoBlock *fib; STACKED struct DiskObject *icon_dob; STACKED ULONG type;};/* void *udata; More file attrs to add };*/
struct MUIP_IconList_DestroyEntry       {STACKED ULONG MethodID; STACKED struct IconEntry *icon;};
struct MUIP_IconList_DrawEntry          {STACKED ULONG MethodID; STACKED struct IconEntry *icon; STACKED IPTR drawmode;};
struct MUIP_IconList_DrawEntryLabel     {STACKED ULONG MethodID; STACKED struct IconEntry *icon; STACKED IPTR drawmode;};
struct MUIP_IconList_NextIcon           {STACKED ULONG MethodID; STACKED IPTR nextflag; STACKED struct IconList_Entry **entry;}; /* *entry maybe MUIV_IconList_NextIcon_Start, *entry is MUIV_IconList_NextIcon_End if no more entries are selected/visible */
struct MUIP_IconList_GetIconPrivate     {STACKED ULONG MethodID; STACKED struct IconList_Entry *entry;}; /* *entry must be a valid Icon */
struct MUIP_IconList_Sort               {STACKED ULONG MethodID;};
struct MUIP_IconList_PositionIcons      {STACKED ULONG MethodID;};
struct MUIP_IconList_ViewIcon           {STACKED ULONG MethodID; STACKED struct IconEntry *icon;};

/* used by MUIM_IconList_NextIcon */
struct IconList_Entry
{
    struct IconEntry    *ile_IconEntry;
    char                *label;     /* The label which is displayed (often FilePart(filename)) */
    LONG                type;
    LONG                flags;      /* If type == ST_ROOT, these flags will set volume attributes */
    void                *udata;     /* userdate given at MUIM_IconList_CreateEntry */
};

struct IconList_Click
{
    int         shift; /* TRUE for shift click */
    struct IconList_Entry *entry; /* might be NULL */
};

struct IconList_Drop_SourceEntry
{
    struct Node dropse_Node;
};

struct IconList_Drop_Event
{
    struct List drop_SourceList;       /* iconlist obj         */
    Object      *drop_TargetObj;        /* iconlist obj         */
    STRPTR      drop_TargetPath;        /* destination path     */
};

struct IconEntry
{
    struct Node                 ie_IconNode;
    struct Node                 ie_SelectionNode;

    struct IconList_Entry       ie_IconListEntry;

    struct DiskObject           *ie_DiskObj;                     /* The icons disk objects */
    struct FileInfoBlock        *ie_FileInfoBlock;

    LONG                        ie_IconX,                        /* Top Left Co-ords of Icons "AREA" */
                                ie_IconY;

    ULONG                       ie_IconWidth,                    /* Width/Height of Icon "Image" */
                                ie_IconHeight,
                                ie_AreaWidth,                    /* Width/Height of Icon "AREA" ..    */
                                ie_AreaHeight;                   /* if the icons Label Width is larger than
                                                                        ie_IconWidth, AreaWidth = the icons label Width
                                                                        else it will be the same as ie_IconWidth */

    ULONG                       ie_Flags;

    UBYTE                       *ie_TxtBuf_DisplayedLabel;
    ULONG                       ie_SplitParts;
    ULONG                       ie_TxtBuf_DisplayedLabelWidth;
    UBYTE                       *ie_TxtBuf_DATE;
    ULONG                       ie_TxtBuf_DATEWidth;
    UBYTE                       *ie_TxtBuf_TIME;
    ULONG                       ie_TxtBuf_TIMEWidth;
    UBYTE                       *ie_TxtBuf_SIZE;
    ULONG                       ie_TxtBuf_SIZEWidth;
    UBYTE                       *ie_TxtBuf_PROT;
};

/****************************************************************************/
/*** Name *******************************************************************/
#define MUIC_IconDrawerList         "IconDrawerList.mui"

/****************************************************************************/
/*** Name *******************************************************************/
#define MUIC_IconVolumeList         "IconVolumeList.mui"

extern const struct __MUIBuiltinClass _MUI_IconList_desc; /* PRIV */
extern const struct __MUIBuiltinClass _MUI_IconDrawerList_desc; /* PRIV */
extern const struct __MUIBuiltinClass _MUI_IconVolumeList_desc; /* PRIV */

#ifdef __AROS__
#define IconListObject       MUIOBJMACRO_START(MUIC_IconList)
#define IconVolumeListObject MUIOBJMACRO_START(MUIC_IconVolumeList)
#define IconDrawerListObject MUIOBJMACRO_START(MUIC_IconDrawerList)
#else
#define IconDrawerListObject   NewObject(IconDrawerList_Class->mcc_Class, NULL
#define IconVolumeListObject   NewObject(IconVolumeList_Class->mcc_Class, NULL
#define IconListObject         NewObject(IconList_Class->mcc_Class, NULL
#endif

#endif /* _MUI_CLASSES_ICONLIST_H */
