/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.9  1996/12/09 13:53:40  aros
    Added empty templates for all missing functions

    Moved #include's into first column

    Revision 1.8  1996/10/24 15:50:35  aros
    Use the official AROS macros over the __AROS versions.

    Revision 1.7  1996/10/10 13:22:20  digulla
    Wrong cast (Fleischer)

    Revision 1.6  1996/09/13 17:50:09  digulla
    Use IPTR

    Revision 1.5  1996/09/11 16:54:23  digulla
    Always use AROS_SLIB_ENTRY() to access shared external symbols, because
	some systems name an external symbol "x" as "_x" and others as "x".
	(The problem arises with assembler symbols which might differ)

    Revision 1.4  1996/08/13 13:52:51  digulla
    Replaced <dos/dosextens.h> by "dos_intern.h" or added "dos_intern.h"
    Replaced AROS_LA by AROS_LHA

    Revision 1.3  1996/08/01 17:40:57  digulla
    Added standard header for all files

    Desc:
    Lang: english
*/
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <utility/tagitem.h>
#include <dos/filesystem.h>
#include <clib/dos_protos.h>
#include "dos_intern.h"

LONG AROS_SLIB_ENTRY(RunProcess,Dos)(struct Process *proc,
	struct StackSwapStruct *sss, STRPTR argptr, ULONG argsize,
	LONG_FUNC entry, struct DosLibrary *DOSBase);

/*****************************************************************************

    NAME */
#include <clib/dos_protos.h>

	AROS_LH4(LONG, RunCommand,

/*  SYNOPSIS */
	AROS_LHA(BPTR,   segList,   D1),
	AROS_LHA(ULONG,  stacksize, D2),
	AROS_LHA(STRPTR, argptr,    D3),
	AROS_LHA(ULONG,  argsize,   D4),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 84, Dos)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)

    STRPTR oldargs;
    LONG oldresult;

    /* Get pointer to process structure */
    struct Process *me=(struct Process *)FindTask(NULL);

    UBYTE *stack;
    LONG ret;
    struct StackSwapStruct sss;

    stack=(UBYTE *)AllocMem(stacksize,MEMF_ANY);
    if(stack==NULL)
	return -1;

    sss.stk_Lower=stack;
    sss.stk_Upper=(IPTR)stack+stacksize;

    oldresult=me->pr_Result2;
    if(me->pr_CIS)
	Flush(me->pr_CIS);
    if(me->pr_COS)
	Flush(me->pr_COS);
    if(me->pr_CES)
	Flush(me->pr_CES);
    me->pr_Result2=oldresult;

    oldargs=me->pr_Arguments;
    me->pr_Arguments=argptr;
    ret=AROS_SLIB_ENTRY(RunProcess,Dos)(me,&sss,argptr,argsize,
		(LONG_FUNC)((BPTR *)BADDR(segList)+1),DOSBase);
    me->pr_Arguments=oldargs;

    oldresult=me->pr_Result2;
    if(me->pr_CIS)
	Flush(me->pr_CIS);
    if(me->pr_COS)
	Flush(me->pr_COS);
    if(me->pr_CES)
	Flush(me->pr_CES);
    me->pr_Result2=oldresult;

    FreeMem(stack,stacksize);
    return ret;
    AROS_LIBFUNC_EXIT
} /* RunCommand */
