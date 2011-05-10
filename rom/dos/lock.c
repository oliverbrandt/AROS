/*
    Copyright � 1995-2011, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Locks a file or directory.
    Lang: English
*/

#define DEBUG 0
#define DLINK(x)

#include <aros/debug.h>
#include <proto/exec.h>
#include <utility/tagitem.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include "dos_intern.h"
#include "fs_driver.h"

static LONG InternalLock(CONST_STRPTR name, LONG accessMode, 
    BPTR *handle, LONG soft_nesting, struct DosLibrary *DOSBase);

#define MAX_SOFT_LINK_NESTING 16 /* Maximum level of soft links nesting */

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH2(BPTR, Lock,

/*  SYNOPSIS */
	AROS_LHA(CONST_STRPTR, name,       D1),
	AROS_LHA(LONG,         accessMode, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 14, Dos)

/*  FUNCTION
	Gets a lock on a file or directory. There may be more than one
	shared lock on a file but only one if it is an exclusive one.
	Locked files or directories may not be deleted.

    INPUTS
	name	   - NUL terminated name of the file or directory.
	accessMode - One of SHARED_LOCK
			    EXCLUSIVE_LOCK

    RESULT
	Handle to the file or directory or 0 if the object couldn't be locked.
	IoErr() gives additional information in that case.

    NOTES
	The lock structure returned by this function is different
	from that of AmigaOS (in fact it is identical to a filehandle).
	Do not try to read any internal fields.

*****************************************************************************/

{
    AROS_LIBFUNC_INIT

    BPTR fl;

    /* Sanity check */
    if (name == NULL)
        return BNULL;
    
    ASSERT_VALID_PTR(name);

    D(bug("[Lock] '%s':%d\n", name, accessMode));

    if (InternalLock(name, accessMode, &fl, MAX_SOFT_LINK_NESTING, DOSBase))
    {
    	D(bug("[Lock] returned 0x%p\n", fl));
        return fl;
    }

    D(bug("[Lock] failed, err=%d\n", IoErr()));
    return BNULL;

    AROS_LIBFUNC_EXIT
} /* Lock */

/* Try to lock name recursively calling itself in case it's a soft link. 
   Store result in handle. Return boolean value indicating result. */
static LONG InternalLock(CONST_STRPTR name, LONG accessMode, 
    BPTR *handle, LONG soft_nesting, struct DosLibrary *DOSBase)
{
    /* Get pointer to process structure */
    struct Process *me = (struct Process *)FindTask(NULL);
    BPTR cur = BNULL;
    struct DevProc *dvp = NULL;
    LONG ret = DOSFALSE;
    LONG error = 0;
    STRPTR filename;

    D(bug("[Lock] Process: 0x%p \"%s\", Window: 0x%p, Name: \"%s\", \n", me, me->pr_Task.tc_Node.ln_Name, me->pr_WindowPtr, name));

    if(soft_nesting == 0)
    {
	SetIoErr(ERROR_TOO_MANY_LEVELS);
	return DOSFALSE;
    }

    filename = strchr(name, ':');
    if (!filename)
    {
	/* No ':' in the pathname, path is relative to current directory */
	cur = me->pr_CurrentDir;
	if (!cur)
	    cur = DOSBase->dl_SYSLock;

        if (cur && (cur != (BPTR)-1))
            error = fs_LocateObject(handle, cur, NULL, name, accessMode, DOSBase);
        else 
            error = ERROR_OBJECT_NOT_FOUND;

        SetIoErr(error);
    }
    else 
    {
    	filename++;
        do
        {
            if ((dvp = GetDeviceProc(name, dvp)) == NULL) 
            {
                error = IoErr();
                break;
            }

	    error = fs_LocateObject(handle, BNULL, dvp, filename, accessMode, DOSBase);

        } while (error == ERROR_OBJECT_NOT_FOUND);

	/* FIXME: On Linux hosted we sometimes get ERROR_IS_SOFTLINK with dvp == NULL,
	 * which causes segfaults below if we don't change "error". Adding !dvp below
         * is probably a hack
         */
        if (error == ERROR_NO_MORE_ENTRIES || !dvp)
            error = me->pr_Result2 = ERROR_OBJECT_NOT_FOUND;
    }

    if (error == ERROR_IS_SOFT_LINK)
    {
        STRPTR softname = ResolveSoftlink(cur, dvp, name, DOSBase);

        if (softname)
        {
            BPTR olddir = BNULL;

            /*
             * ResolveSoftLink() gives us path relative to either 'cur' lock
             * (if on current volume), or 'dvp' volume root (if on different volume).
             * In the latter case we need to change current directory to volume's root
             * in order to follow the link correctly.
             */
            if (dvp)
            {
                olddir = me->pr_CurrentDir;
            	error = RootDir(dvp, DOSBase);
            }
            else
            	error = 0;

            if (!error)
            {
            	ret = InternalLock(softname, accessMode, handle, soft_nesting - 1, DOSBase);
            	error = ret ? 0 : IoErr();
            	D(bug("[Lock] Resolve error %d\n", error));

		if (olddir)
            	    UnLock(CurrentDir(olddir));
            }

	    FreeVec(softname);
        }
        else
            error = IoErr();
    }

    FreeDeviceProc(dvp);

    if (error)
    {
    	SetIoErr(error);
    	ret = DOSFALSE;
    }
    else
    	ret = DOSTRUE;

    return ret;
}

/*
 * Resolve a softlink.
 * Returns AllocVec()ed buffer with softlink contents.
 */
STRPTR ResolveSoftlink(BPTR cur, struct DevProc *dvp, CONST_STRPTR name, struct DosLibrary *DOSBase)
{
    ULONG buffer_size = 256;
    STRPTR softname;
    LONG continue_loop;
    LONG written;

    DLINK(bug("[Softlink] Resolving softlink %s...\n", name));

    do
    {
        continue_loop = FALSE;

        if (!(softname = AllocVec(buffer_size, MEMF_PUBLIC|MEMF_CLEAR)))
        {
            SetIoErr(ERROR_NO_FREE_STORE);
            break;
        }

        written = fs_ReadLink(cur, dvp, name, softname, buffer_size, DOSBase);

	switch (written)
	{
	case -1:
            /* An error occured */
            DLINK(bug("[Softlink] Error %d reading softlink\n", IoErr()));
            break;

        case -2:
            /* If there's not enough space in the buffer, increase it and try again */
            continue_loop = TRUE;
            buffer_size <<= 1;

            DLINK(bug("[Softlink] Increased buffer size up to %u\n", buffer_size));
            break;

	default:
            /* All OK */
            DLINK(bug("[Softlink] Resolved path: %s\n", softname));
            return softname;
        }
                
        FreeVec(softname);
    }
    while(continue_loop);

    return NULL;
}

/* Change to root directory of the specified device */
LONG RootDir(struct DevProc *dvp, struct DosLibrary *DOSBase)
{
    BPTR lock = BNULL;
    LONG error;

    /* We already have a DeviceProc structure, so just use internal routine. */
    error = fs_LocateObject(&lock, BNULL, dvp, "", SHARED_LOCK, DOSBase);

    if (!error)
    	CurrentDir(lock);

    return error;
}
