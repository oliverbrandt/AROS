/*
    (C) 1995-2001 AROS - The Amiga Research OS
    $Id$

    Desc: Create a new BOOPSI object
    Lang: english
*/
#define AROS_ALMOST_COMPATIBLE
#include <exec/lists.h>
#include <intuition/classes.h>
#include <proto/exec.h>
#include <proto/alib.h>
#include <proto/utility.h>
#include "intuition_intern.h"
#include "intuition_private.h"

/*****************************************************************************

    NAME */
#include <intuition/classusr.h>
#include <proto/intuition.h>

#include "maybe_boopsi.h"

AROS_LH3(APTR, NewObjectA,

         /*  SYNOPSIS */
         AROS_LHA(struct IClass  *, classPtr, A0),
         AROS_LHA(UBYTE          *, classID, A1),
         AROS_LHA(struct TagItem *, tagList, A2),

         /*  LOCATION */
         struct IntuitionBase *, IntuitionBase, 106, Intuition)

/*  FUNCTION
	Use this function to create BOOPSI objects (BOOPSI stands for
	"Basic Object Oriented Programming System for Intuition).

	You may specify a class either by it's name (if it's a public class)
	or by a pointer to its definition (if it's a private class). If
	classPtr is NULL, classID is used.

    INPUTS
	classPtr - Pointer to a private class (or a public class if you
		happen to have a pointer to it)
	classID - Name of a public class
	tagList - Initial attributes. Read the documentation of the class
		carefully to find out which attributes must be specified
		here and which can.

    RESULT
	A BOOPSI object which can be manipulated with general functions and
	which must be disposed with DisposeObject() later.

    NOTES
	This functions send OM_NEW to the dispatcher of the class.

    EXAMPLE

    BUGS

    SEE ALSO
	DisposeObject(), SetAttrs(), GetAttr(), MakeClass(),
	"Basic Object-Oriented Programming System for Intuition" and
	"boopsi Class Reference" Dokument.

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    intuition_lib.fd and clib/intuition_protos.h

*****************************************************************************/
{
	AROS_LIBFUNC_INIT
	AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)

#if INTERNAL_BOOPSI

	Object * object;
	struct opSet method;

	DEBUG_NEWOBJECT(dprintf("NewObject[%x]: Class 0x%lx <%s> TagList 0x%lx\n",
				&method, /* some unique id to see matching debug info */
				classPtr,
				classID ? classID : (classPtr->cl_ID ? classPtr->cl_ID : "NULL"),
				tagList));

	EnterFunc(bug("intuition::NewObjectA()\n"));

#if 1
	if (tagList)
	{
		DEBUG_NEWOBJECT(
		APTR state = tagList;
		struct TagItem *tag;

		    while (tag = NextTagItem(&state))
		    {
		    dprintf("\t%08lx %08lx\n", tag->ti_Tag, tag->ti_Data);
		    }
		)
	}
#endif

	ObtainSemaphoreShared (&GetPrivIBase(IntuitionBase)->ClassListLock);

	/* No classPtr ? */
	if (!classPtr)
		classPtr = FindClass (classID);

	/* Make sure the class doesn't go away while we create the object */
	if (classPtr)
	{
		Forbid();
		++classPtr->cl_ObjectCount;
		Permit();
	}

	ReleaseSemaphore (&GetPrivIBase(IntuitionBase)->ClassListLock);

	if (!classPtr)
		return (NULL); /* Nothing found */

	D(bug("classPtr: %p\n", classPtr));

	/* Try to create a new object */
	method.MethodID = OM_NEW;
	method.ops_AttrList = tagList;
	method.ops_GInfo = NULL;
	object = (Object *) CoerceMethodA (classPtr, (Object *)classPtr, (Msg)&method);

	/* Release the lock on the class. Rootclass also has increased this count. */
	Forbid();
	--classPtr->cl_ObjectCount;
	Permit();

	DEBUG_NEWOBJECT(dprintf("NewObject[%x]: return 0x%lx\n", &method, object));

	ReturnPtr("intuition::NewObjectA()", Object *, object);

#else

/* Pass call to the BOOPSI.library */
	return NewObjectA(classPtr, classID, tagList);

#endif

	AROS_LIBFUNC_EXIT

} /* NewObjectA */
