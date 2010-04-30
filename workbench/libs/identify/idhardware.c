/*
    Copyright � 2010, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/

#include <proto/utility.h>

#include <stdio.h>

#include "identify_intern.h"
#include "identify.h"

static CONST_STRPTR handle_osver(struct IdentifyBaseIntern *);

/*****************************************************************************

    NAME */
#include <proto/identify.h>

        AROS_LH2(CONST_STRPTR, IdHardware,

/*  SYNOPSIS */
        AROS_LHA(ULONG           , type   , D0),
        AROS_LHA(struct TagItem *, taglist, A0),

/*  LOCATION */
        struct IdentifyBaseIntern *, IdentifyBase, 6, Identify)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY


*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct TagItem *tag;
    const struct TagItem *tags;

    CONST_STRPTR result = NULL;
    BOOL null4na = FALSE;
    BOOL localize = TRUE;

    for (tags = taglist; (tag = NextTagItem(&tags)); )
    {
        switch (tag->ti_Tag)
        {
            case IDTAG_NULL4NA:
                null4na = tag->ti_Data ? TRUE : FALSE;
                break;

            case IDTAG_Localize:
                localize = tag->ti_Data ? TRUE : FALSE;
                break;
        }
    }

    switch(type)
    {
        case IDHW_SYSTEM:
            result = "AROS";
            break;

        case IDHW_CPU:
            break;

        case IDHW_FPU:
            break;

        case IDHW_MMU:
            break;

        case IDHW_OSVER:
            result = handle_osver(IdentifyBase);
            break;

        case IDHW_EXECVER:
            break;

        case IDHW_WBVER:
            break;

        case IDHW_ROMSIZE:
            break;

        case IDHW_CHIPSET:
            break;

        case IDHW_GFXSYS:
            break;

        case IDHW_CHIPRAM:
            break;

        case IDHW_FASTRAM:
            break;

        case IDHW_RAM:
            break;

        case IDHW_SETPATCHVER:
            break;

        case IDHW_AUDIOSYS:
            break;

        case IDHW_OSNR:
            break;

        case IDHW_VMMCHIPRAM:
            break;

        case IDHW_VMMFASTRAM:
            break;

        case IDHW_VMMRAM:
            break;

        case IDHW_PLNCHIPRAM:
            break;

        case IDHW_PLNFASTRAM:
            break;

        case IDHW_PLNRAM:
            break;

        case IDHW_VBR:
            break;

        case IDHW_LASTALERT:
            break;

        case IDHW_VBLANKFREQ:
            break;

        case IDHW_POWERFREQ:
            break;

        case IDHW_ECLOCK:
            break;

        case IDHW_SLOWRAM:
            break;

        case IDHW_GARY:
            break;

        case IDHW_RAMSEY:
            break;

        case IDHW_BATTCLOCK:
            break;

        case IDHW_CHUNKYPLANAR:
            break;

        case IDHW_POWERPC:
            break;

        case IDHW_PPCCLOCK:
            break;

        case IDHW_CPUREV:
            break;

        case IDHW_CPUCLOCK:
            break;

        case IDHW_FPUCLOCK:
            break;

        case IDHW_RAMACCESS:
            break;

        case IDHW_RAMWIDTH:
            break;

        case IDHW_RAMCAS:
            break;

        case IDHW_RAMBANDWIDTH:
            break;

        case IDHW_TCPIP:
            break;

        case IDHW_PPCOS:
            break;

        case IDHW_AGNUS:
            break;

        case IDHW_AGNUSMODE:
            break;

        case IDHW_DENISE:
            break;

        case IDHW_DENISEREV:
            break;
    }

    if ((result == NULL) && (null4na == FALSE))
    {
        result = "not available";
    }

    return result;

    AROS_LIBFUNC_EXIT
} /* IdHardware */


static CONST_STRPTR handle_osver(struct IdentifyBaseIntern *library)
{
    CONST_STRPTR result = NULL;
    if (*library->hwb.buf_OsVer == '\0')
    {
        ULONG num;
        //ULONG num = IdHardwareNum(IDHW_OSVER, NULL);
        ULONG version = num & 0xffff;
        ULONG revision = num >> 16;
        snprintf(library->hwb.buf_OsVer, STRBUFSIZE, "%u.%u", version, revision);
        result = library->hwb.buf_OsVer;
    }
    return result;
}
