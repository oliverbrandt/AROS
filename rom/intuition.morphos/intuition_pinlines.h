#ifndef _INTUITION_PINLINES_H
#define _INTUITION_PINLINES_H
/*
    Copyright (C) 1997-2001 AROS - The Amiga Research OS
    $Id$
 
    Desc: Private inlines for intuition.library
    Lang: english
*/

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif

#ifndef INTUI_BASE_NAME
#define INTUI_BASE_NAME IntuitionBase
#endif

#define LateIntuiInit(data) \
LP1( , BOOL, LateIntuiInit, \
APTR, data, a0, \
INTUI_BASE_NAME )

#define DoNotify(cl, o, ic, msg) \
LP4(0x366, IPTR, DoNotify, \
Class *, (cl), a0, \
Object *, (o), a1, \
struct ICData *, (ic), a2, \
struct opUpdate *, (msg), a3, \
, INTUI_BASE_NAME )

#define FreeICData(ic) \
LP1NR(0x36c, FreeICData, \
struct ICData *, (ic), a0, \
, INTUI_BASE_NAME )

#endif _INTUITION_PINLINES_H
