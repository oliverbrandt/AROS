/*
    (C) 1997-99 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: English
*/
#include "datatypes_intern.h"
#include <datatypes/datatypesclass.h>
#include <proto/dos.h>
#include <proto/exec.h>

/*****************************************************************************

    NAME */
#include <proto/datatypes.h>

	AROS_LH1(void, DisposeDTObject,

/*  SYNOPSIS */
	AROS_LHA(Object *, o, A0),

/*  LOCATION */
	struct Library *, DataTypesBase, 9, DataTypes)

/*  FUNCTION

    Dispose a data type object obtained by NewDTObjectA().

    INPUTS

    o   --  The data type object to dispose of; may be NULL.

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    NewDTObjectA()

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    if(o != NULL)
    {
	struct DTSpecialInfo *dtsi = ((struct Gadget *)o)->SpecialInfo;
	struct Library *DTClassBase = NULL;
	
	GetAttr(DTA_Data, o, (IPTR *)DTClassBase);
	
	while(dtsi->si_Flags & (DTSIF_PRINTING | DTSIF_LAYOUTPROC))
	{
	    Delay(50);
	}
	
	DisposeObject(o);
	
	CloseLibrary(DTClassBase);
    }
    
    AROS_LIBFUNC_EXIT
} /* DisposeDTObject */
