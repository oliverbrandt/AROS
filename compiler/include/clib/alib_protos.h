#ifndef  CLIB_ALIB_PROTOS_H
#define  CLIB_ALIB_PROTOS_H

/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Prototypes for amiga.lib
    Lang: english
*/

#if defined(RT_ENABLE) && RT_ENABLE
#   include <aros/rt.h>
#endif
#ifndef  EXEC_TYPES_H
#   include <exec/types.h>
#endif
#ifndef INTUITION_INTUITION_H
#   include <intuition/intuition.h>
#endif
#ifndef INTUITION_SCREENS_H
#   include <intuition/screens.h>
#endif

/* #ifndef INTUITION_CLASSUSR_H
#   include <intuition/classusr.h>
#endif
#ifndef INTUITION_CLASSES_H
#   include <intuition/classes.h>
#endif */
#ifndef LIBRARIES_COMMODITIES_H
#   include <libraries/commodities.h>
#endif
#ifndef AROS_ASMCALL_H
#   include <aros/asmcall.h>
#endif
#ifndef LIBRARIES_GADTOOLS_H
#   include <libraries/gadtools.h>
#endif
#ifndef DEVICES_KEYMAP_H
#   include <devices/keymap.h>
#endif
#ifndef DEVICES_INPUTEVENT_H
#   include <devices/inputevent.h>
#endif
#include <rexx/storage.h>

struct MsgPort;
struct IORequest;
struct Task;
struct InputEvent;
struct Hook;
struct Locale;
/*
    Prototypes
*/
IPTR CallHookA (struct Hook * hook, APTR obj, APTR param);
IPTR CallHook (struct Hook * hook, APTR obj, ...);

/* Exec support */
struct IORequest * CreateExtIO (struct MsgPort * port, ULONG iosize);
struct IOStdReq * CreateStdIO (struct MsgPort * port);
void DeleteExtIO (struct IORequest * ioreq);
void DeleteStdIO (struct IOStdReq * ioreq);
struct Task * CreateTask (STRPTR name, LONG pri, APTR initpc, ULONG stacksize);
void DeleteTask (struct Task * task);
void NewList (struct List *);
#if !defined(ENABLE_RT) || !ENABLE_RT
struct MsgPort * CreatePort (STRPTR name, LONG pri);
void DeletePort (struct MsgPort * mp);
#endif

/* Extra */
ULONG RangeRand (ULONG maxValue);
ULONG FastRand (ULONG seed);
LONG TimeDelay (LONG unit, ULONG secs, ULONG microsecs);
void waitbeam (LONG pos);
void __sprintf(UBYTE *buffer, UBYTE *format, ...);

/* Commodities */
CxObj  *HotKey (STRPTR description, struct MsgPort *port, LONG id);
VOID    FreeIEvents (volatile struct InputEvent *events);
UBYTE **ArgArrayInit(ULONG argc, UBYTE **argv);
VOID    ArgArrayDone(VOID);
LONG    ArgInt(UBYTE **tt, STRPTR entry, LONG defaultVal);
STRPTR  ArgString(UBYTE **tt, STRPTR entry, STRPTR defaultstring);
struct  InputEvent *InvertString(STRPTR str, struct KeyMap *km);

/* Gadtools */
APTR GetVisualInfo (struct Screen * screen, ULONG tag1, ...);
struct Gadget * CreateGadget(	ULONG kind, 
			struct Gadget * previous, 
			struct NewGadget * ng,
			ULONG tag1,
			...);
void DrawBevelBox(	struct RastPort * rp,
			WORD left,
			WORD top,
			WORD width,
			WORD height,
			ULONG tag1,
			...);
void GT_SetGadgetAttrs(	struct Gadget * gad, 
			struct Window * win, 
			struct Requester * req,
			ULONG  tag1,
			...);
LONG GT_GetGadgetAttrs(	struct Gadget * gad,
			struct Window * win,
			struct Requester * req,
			ULONG	tag1,
			...);
struct Menu * CreateMenus( struct NewMenu * newmenu, ULONG tag1, ...);
BOOL LayoutMenus( struct Menu * menu, APTR vi, ULONG tag1, ...);

/* Graphics */
LONG ObtainBestPen( struct ColorMap * cm, LONG R, LONG G, LONG B, ULONG tag1, ...);
void GetRPAttrs( struct RastPort * rp, Tag tag1, ...);

/* Intuition */
void SetWindowPointer( struct Window * window, ULONG tag1, ...);

/* Locale */
struct Catalog *OpenCatalog(	struct Locale * locale,
				STRPTR name,
				ULONG tag1,
				...);

/* Pools */
APTR LibCreatePool (ULONG requirements, ULONG puddleSize, ULONG threshSize);
void LibDeletePool (APTR poolHeader);
APTR LibAllocPooled (APTR poolHeader, ULONG memSize);
void LibFreePooled (APTR poolHeader, APTR memory, ULONG memSize);

/* Hook Support */
AROS_UFP3(IPTR, HookEntry,
    AROS_UFPA(struct Hook *, hook,  A0),
    AROS_UFPA(APTR,          obj,   A2),
    AROS_UFPA(APTR,          param, A1)
);

#ifndef AROS_METHODRETURNTYPE
#   define AROS_METHODRETURNTYPE IPTR
#endif
#ifdef AROS_SLOWSTACKMETHODS
    Msg  GetMsgFromStack  (ULONG MethodID, va_list args);
    void FreeMsgFromStack (Msg msg);

#   define AROS_SLOWSTACKMETHODS_PRE(arg)       \
    AROS_METHODRETURNTYPE retval;		\
						\
    va_list args;				\
    Msg     msg;				\
						\
    va_start (args, arg);                       \
						\
    if ((msg = GetMsgFromStack (arg, args)))    \
    {						\
#   define AROS_SLOWSTACKMETHODS_ARG(arg) msg
#   define AROS_SLOWSTACKMETHODS_POST		\
	FreeMsgFromStack (msg);                 \
    }						\
    else					\
	retval = (AROS_METHODRETURNTYPE)0L;     \
						\
    va_end (args);                              \
						\
    return retval;
#else
#   define AROS_SLOWSTACKMETHODS_PRE(arg)   AROS_METHODRETURNTYPE retval;
#   define AROS_SLOWSTACKMETHODS_ARG(arg)   ((Msg)&(arg))
#   define AROS_SLOWSTACKMETHODS_POST	    return retval;
#endif /* AROS_SLOWSTACKMETHODS */

#ifdef AROS_SLOWSTACKTAGS
    struct TagItem * GetTagsFromStack  (ULONG firstTag, va_list args);
    void	     FreeTagsFromStack (struct TagItem * tags);
#endif /* AROS_SLOWSTACKTAGS */

/* Rexx support */
BOOL CheckRexxMsg(struct RexxMsg *);
LONG SetRexxVar(struct RexxMsg *, char *, char *, ULONG length);
LONG GetRexxVar(struct RexxMsg *, char *, char **value);

#endif /* CLIB_ALIB_PROTOS_H */
