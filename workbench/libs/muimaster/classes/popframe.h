#ifndef _MUI_CLASSES_POPFRAME_H
#define _MUI_CLASSES_POPFRAME_H

/*
    Copyright � 2002-2003, The AROS Development Team. All rights reserved.
    $Id$
*/

/*** Name *******************************************************************/
#define MUIC_Popframe              "Popframe.mui"

/*** Identifier base (for Zune extensions) **********************************/
#define MUIB_Popframe              (MUIB_ZUNE | 0x00002200)

/*** Methods ****************************************************************/
#define MUIM_Popframe_OpenWindow   (MUIB_MUI|0x0042c5c6)     /* PRIV */
#define MUIM_Popframe_CloseWindow  (MUIB_MUI|0x0042c5c7)     /* PRIV */
struct MUIP_Popframe_OpenWindow    {ULONG MethodID;};           /* PRIV */
struct MUIP_Popframe_CloseWindow   {ULONG MethodID; LONG ok;};  /* PRIV */


extern const struct __MUIBuiltinClass _MUI_Popframe_desc; /* PRIV */

#endif /* _MUI_CLASSES_POPFRAME_H */
