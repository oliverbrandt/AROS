/*
    Copyright � 2002, The AROS Development Team. 
    All rights reserved.
    
    $Id$
*/

#include <proto/asl.h>
#ifdef __AROS__
#include <proto/muimaster.h>
#endif

#include "muimaster_intern.h"

/*****************************************************************************

    NAME */
#ifndef __AROS__
__asm VOID MUI_FreeAslRequest(register __a0 APTR requester)
#else
	AROS_LH1(VOID, MUI_FreeAslRequest,

/*  SYNOPSIS */
	AROS_LHA(APTR, requester, A0),

/*  LOCATION */
	struct Library *, MUIMasterBase, 10, MUIMaster)
#endif
/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS
	The function itself is a bug ;-) Remove it!

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct MUIMasterBase *,MUIMasterBase)

    FreeAslRequest(requester);

    AROS_LIBFUNC_EXIT

} /* MUIA_FreeAslRequestA */
