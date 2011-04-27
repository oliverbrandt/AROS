/*
    Copyright � 1995-2011, The AROS Development Team. All rights reserved.
    $Id$

*/

#include "partition_support.h"
#include "platform.h"

/* This what we can always do */
static const struct PartitionAttribute defaultPartitionAttrs[] =
{
    {PTA_GEOMETRY,  PLAM_READ},
    {PTA_DOSENVEC,  PLAM_READ},
    {PTA_DONE, 0}
};

/*****************************************************************************

    NAME */
#include <libraries/partition.h>

   AROS_LH1(const struct PartitionAttribute *, QueryPartitionAttrs,

/*  SYNOPSIS */
   AROS_LHA(struct PartitionHandle *, table,       A1),

/*  LOCATION */
   struct Library *, PartitionBase, 18, Partition)

/*  FUNCTION
    query partition attributes

    INPUTS
    ph      - PartitionHandle to the table

    RESULT
    list of NULL-terminated ULONGs with attributes

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
    21-02-02    first version

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    if (table->table)
    {
    	struct PTFunctionTable *handler = table->table->handler;

        return handler->partitionAttrs;
    }

    return defaultPartitionAttrs;

    AROS_LIBFUNC_EXIT
}
