/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <dos/dosextens.h>
#include <proto/dos.h>

	AROS_LH2(void, AbortPkt,

/*  SYNOPSIS */
	AROS_LHA(struct MsgPort   *, port, D1),
	AROS_LHA(struct DosPacket *, pkt, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 44, Dos)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)

    if (NULL != (struct IORequest *)pkt->dp_Arg7) {
        if (0 == AbortIO((struct IORequest*)pkt->dp_Arg7)) 
        {
#warning Still have to decide where to finally free the IORequest structure if this goes well
            FreeMem(pkt->dp_Arg7, sizeof(struct IOFileSys));
            pkt->dp_Arg7 = NULL;
        }
    }
    AROS_LIBFUNC_EXIT
} /* AbortPkt */
