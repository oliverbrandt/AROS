#ifndef _MUI_CLASSES_POPOBJECT_H
#define _MUI_CLASSES_POPOBJECT_H

/****************************************************************************/
/** Popobject                                                              **/
/****************************************************************************/

#ifdef _DCC
extern char MUIC_Popobject[];
#else
#define MUIC_Popobject "Popobject.mui"
#endif

/* Attributes */

#define MUIA_Popobject_Follow               0x80424cb5 /* V7  isg BOOL              */
#define MUIA_Popobject_Light                0x8042a5a3 /* V7  isg BOOL              */
#define MUIA_Popobject_Object               0x804293e3 /* V7  i.g Object *          */
#define MUIA_Popobject_ObjStrHook           0x8042db44 /* V7  isg struct Hook *     */
#define MUIA_Popobject_StrObjHook           0x8042fbe1 /* V7  isg struct Hook *     */
#define MUIA_Popobject_Volatile             0x804252ec /* V7  isg BOOL              */
#define MUIA_Popobject_WindowHook           0x8042f194 /* V9  isg struct Hook *     */



/****************************************************************************/
/** Poplist                                                                **/
/****************************************************************************/

#ifdef _DCC
extern char MUIC_Poplist[];
#else
#define MUIC_Poplist "Poplist.mui"
#endif

/* Attributes */

#define MUIA_Poplist_Array                  0x8042084c /* V8  i.. char **           */



/****************************************************************************/
/** Popscreen                                                              **/
/****************************************************************************/

#ifdef _DCC
extern char MUIC_Popscreen[];
#else
#define MUIC_Popscreen "Popscreen.mui"
#endif

/* Attributes */




/****************************************************************************/
/** Popasl                                                                 **/
/****************************************************************************/

#ifdef _DCC
extern char MUIC_Popasl[];
#else
#define MUIC_Popasl "Popasl.mui"
#endif

/* Attributes */

#define MUIA_Popasl_Active                  0x80421b37 /* V7  ..g BOOL              */
#define MUIA_Popasl_StartHook               0x8042b703 /* V7  isg struct Hook *     */
#define MUIA_Popasl_StopHook                0x8042d8d2 /* V7  isg struct Hook *     */
#define MUIA_Popasl_Type                    0x8042df3d /* V7  i.g ULONG             */

extern const struct __MUIBuiltinClass _MUI_Popasl_desc;
extern const struct __MUIBuiltinClass _MUI_Popobject_desc;

#endif
