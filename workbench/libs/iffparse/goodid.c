/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include "iffparse_intern.h"

#define DEBUG_GOODID(x)

/*****************************************************************************

    NAME */
#include <proto/iffparse.h>

	AROS_LH1(LONG, GoodID,

/*  SYNOPSIS */
	AROS_LHA(LONG, id, D0),

/*  LOCATION */
	struct Library *, IFFParseBase, 43, IFFParse)

/*  FUNCTION
	Determines whether an ID is valid according to the IFF specification.

    INPUTS
	id - An IFF chunk ID to be tested.

    RESULT
	TRUE if valid.
	FALSE otherwise.

    NOTES
	Assumes input to be in local byte order.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
  27-11-96    digulla automatically created from
	  iffparse_lib.fd and clib/iffparse_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct Library *,IFFParseBase)

    UBYTE theId[4];

    theId[0] = id >> 24;
    theId[1] = id >> 16;
    theId[2] = id >> 8;
    theId[3] = id;

    DEBUG_GOODID(dprintf("theid: 0x%08lx [%c%c%c%c]\n",
			 id, theId[0], theId[1], theId[2], theId[3]));

    /* If the ID starts with a space, but is not all spaces, then invalid */
    if((theId[0] == 0x20) && (id != 0x20202020))
    {
	DEBUG_GOODID(dprintf("badid 1\n"));
        return (FALSE);
    }

    /*
	Check whether the ID is within the allowed character ranges.
	This loop is unrolled
    */

    if( (theId[0] < 0x20) || (theId[0] > 0x7e))
    {
	DEBUG_GOODID(dprintf("badid 2\n"));
	return (FALSE);
    }
    if( (theId[1] < 0x20) || (theId[1] > 0x7e))
    {
	DEBUG_GOODID(dprintf("badid 3\n"));
	return (FALSE);
    }
    if( (theId[2] < 0x20) || (theId[2] > 0x7e))
    {
	DEBUG_GOODID(dprintf("badid 4\n"));
	return (FALSE);
    }
    if( (theId[3] < 0x20) || (theId[3] > 0x7e))
    {
	DEBUG_GOODID(dprintf("badid 5\n"));
	return (FALSE);
    }

    DEBUG_GOODID(dprintf("goodid\n"));
    return (TRUE);

    AROS_LIBFUNC_EXIT
} /* GoodID */
