/*
    (C) 1999 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: English
*/

#include "datatypes_intern.h"
#include <proto/exec.h>
#include <proto/alib.h>
#include <utility/tagitem.h>
#include <datatypes/datatypesclass.h>
#include <intuition/classusr.h>

/*****************************************************************************

    NAME */

	AROS_LH2(APTR, ObtainDTDrawInfoA,

/*  SYNOPSIS */
	AROS_LHA(Object         *, o  ,   A0),
	AROS_LHA(struct TagItem *, attrs, A1),

/*  LOCATION */
	struct Library *, DTBase, 20, DataTypes)

/*  FUNCTION

    Prepare a data type object for drawing into a RastPort; this function
    will send the DTM_OBTAINDRAWINFO method the object using an opSet
    message.

    INPUTS

    o      --  pointer to the data type object to obtain the drawinfo for
    attrs  --  additional attributes

    RESULT

    A private handle that must be passed to ReleaseDTDrawInfo when the
    application is done drawing the object, or NULL if failure.

    TAGS

    PDTA_Screen     --  for pictureclass objects
    ADTA_Screen     --  for animationclass objects

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    DrawDTObjectA(), ReleaseDTDrawInfo()

    INTERNALS

    HISTORY

    29.8.99  SDuvan  implemented

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct opSet ops;

    ops.MethodID     = DTM_OBTAINDRAWINFO;
    ops.ops_AttrList = attrs;
    ops.ops_GInfo    = NULL;

    return (APTR)DoMethodA(o, (Msg)&ops);

    AROS_LIBFUNC_EXIT
} /* ObtainDTDrawInfoA */
