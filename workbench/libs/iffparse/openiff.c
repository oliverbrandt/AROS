/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/
#include "iffparse_intern.h"

/*****************************************************************************

    NAME */
#include <proto/iffparse.h>

	AROS_LH2(LONG, OpenIFF,

/*  SYNOPSIS */
	AROS_LHA(struct IFFHandle *, iff, A0),
	AROS_LHA(LONG              , rwMode, D0),

/*  LOCATION */
	struct Library *, IFFParseBase, 6, IFFParse)

/*  FUNCTION
	Initializes an IFFHandle struct for a new session of reading or
	writing. The direction of the I/O is determined by the rwMode flags
	supplied (IFFF_READ or IFFF_WRITE).

    INPUTS
	iff - pointer to IFFHandle struct.
	ewMode - IFFF_READ or IFFF_WRITE


    RESULT
	error -  0 if successfull, IFFERR_#? elsewise.

    NOTES
	 This function tells the custom stream handler to initialize
	by sending it a IFFCMD_INIT IFFStreamCmd.

    EXAMPLE

    BUGS

    SEE ALSO
	CloseIFF(), InitIFF()

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct Library *,IFFParseBase)
    LONG  err;
    struct IFFStreamCmd cmd;
    struct ContextNode *cn;

    /* Check that a valid StreamHandler Hook has been supplied */
    if (!( GetIntIH(iff)->iff_StreamHandler) )
	return (IFFERR_NOHOOK);
    /* Tell the custom stream to initialize itself */

    cmd.sc_Command = IFFCMD_INIT;
    err = CallHookPkt (GetIntIH(iff)->iff_StreamHandler, iff, &cmd);

    if (!err)
    {
	/* If we are opend in read mode we should test if we have a valid IFF-File */
	if (rwMode == IFFF_READ)
	{
	    /* Get header of iff-stream */
	    err = GetChunkHeader(iff, IPB(IFFParseBase));

	    /* Valid IFF header ? */
	    if (!err)
	    {

		/* We have now entried the chunk */
		GetIntIH(iff)->iff_CurrentState = IFFSTATE_COMPOSITE;

		cn = TopChunk(iff);

		/* We must see if we have a IFF header ("FORM", "CAT" or "LIST") */
		if (GetIntCN(cn)->cn_Composite)
		{
		    /* Everything went OK */
		    /* Set the acess mode, and mark the stream as opened */
		    iff->iff_Flags |= (rwMode | IFFF_OPEN);
		    err = 0L;
		}
		else
		{
		    err = IFFERR_NOTIFF;

		    /* Pop the contextnode */
		    PopContextNode(iff, IPB(IFFParseBase));
		}
	    }
	    else
	    {
		if (err  == IFFERR_MANGLED)
		    err = IFFERR_NOTIFF;

		/* Fail. We should send CLEANUP to the stream */
		cmd.sc_Command = IFFCMD_CLEANUP;
		err = CallHookPkt
		(
		    GetIntIH(iff)->iff_StreamHandler,
		    iff,
		    &cmd
		);
	    }
	}
	else
	    err = 0L;
    }

    return (err);
    AROS_LIBFUNC_EXIT
} /* OpenIFF */
