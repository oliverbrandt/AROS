/*
    Copyright � 2011, The AROS Development Team. All rights reserved.
    $Id: getpartitiontableattrs.c 38180 2011-04-12 12:32:14Z sonic $

*/
#include "partition_support.h"
#include "platform.h"

/*****************************************************************************

    NAME */
#include <utility/tagitem.h>
#include <libraries/partition.h>

   AROS_LH2(LONG, GetFileSystemAttrsA,

/*  SYNOPSIS */
   AROS_LHA(struct Node *, handle, A1),
   AROS_LHA(struct TagItem *, taglist, A2),

/*  LOCATION */
   struct Library *, PartitionBase, 21, Partition)

/*  FUNCTION
    get attributes of a partition table

    INPUTS
    handle      - Filesystem handle
    taglist - list of attributes; unknown tags are ignored
        FST_ID      (ULONG *)		    - Get 4-characters filesystem ID
        FST_NAME    (STRPTR *)		    - Get a pointer to filesystem name
        FST_FSENTRY (struct FileSysEntry *) - Fill in the given struct FileSysEntry.

    RESULT
    	Currently should always return zero. Nonzero means internal partition.library
    	error.

    NOTES
    	Name is returned as a pointer to internally allocated string. You should copy
    	it if you want to keep it after filesystem's partition table had been closed.

	The following fields in struct FileSysEntry will not be filled in:
	  - Node name
	  - fse_Handler
	  - fse_SegList
	You need to query for filesystem's name separately and copy it into BSTR
	yourself, if you need to. Loading the handler is done by LoadFileSystem()
	function.

    EXAMPLE

    BUGS

    SEE ALSO
    	FindFileSystemA()

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    const struct PTFunctionTable *handler = ((struct FileSysHandle *)handle)->part->table->handler;

    if (handler->getFileSystemAttrs)
        return handler->getFileSystemAttrs(PartitionBase, (struct FileSysHandle *)handle, taglist);

    return 1;

    AROS_LIBFUNC_EXIT
}
