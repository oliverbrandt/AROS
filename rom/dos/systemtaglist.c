/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <utility/tagitem.h>
#include <proto/dos.h>

	AROS_LH2(LONG, SystemTagList,

/*  SYNOPSIS */
	AROS_LHA(STRPTR          , command, D1),
	AROS_LHA(struct TagItem *, tags, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 101, Dos)

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

#warning TODO: Write dos/SystemTagList()
    aros_print_not_implemented ("SystemTagList");

    return RETURN_FAIL;
    AROS_LIBFUNC_EXIT
} /* SystemTagList */
