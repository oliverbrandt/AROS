/*
    Copyright � 1995-2002, The AROS Development Team. All rights reserved.
    $Id$

    Desc: RexxSupport initialization code.
    Lang: English
*/

#include <stddef.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <aros/libcall.h>
#include <aros/debug.h>

#include <proto/exec.h>
#include <proto/alib.h>

#include "rexxsupport_intern.h"
#include "libdefs.h"

#undef SysBase

#define LC_NO_OPENLIB
#define LC_NO_CLOSELIB
#define LC_NO_EXPUNGELIB

#define LC_LIBHEADERTYPEPTR        LIBBASETYPEPTR
#define LC_LIB_FIELD(libBase)      (libBase)->library.lh_LibNode
#define LC_SYSBASE_FIELD(libBase)  (libBase)->library.lh_SysBase
#define LC_SEGLIST_FIELD(libBase)  (libBase)->library.lh_SegList
#define LC_LIBBASESIZE             (sizeof(LIBBASETYPE))

#include <libcore/libheader.c>

#define SysBase LC_SYSBASE_FIELD(lh)

struct RxsLib *RexxSysBase;
int errno;

ULONG SAVEDS STDARGS LC_BUILDNAME(L_InitLib) (LC_LIBHEADERTYPEPTR lh)
{
    RexxSysBase = OpenLibrary("rexxsyslib.library", 0);
    if (RexxSysBase == NULL)
        return FALSE;
    else
        return TRUE;
}

ULONG SAVEDS STDARGS LC_BUILDNAME(L_ExpungeLib) (LC_LIBHEADERTYPEPTR lh)
{
    CloseLibrary(RexxSysBase);
}
