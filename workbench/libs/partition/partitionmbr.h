#ifndef PARTITIONMBR_H
#define PARTITIONMBR_H

/*
    Copyright � 2007, The AROS Development Team. All rights reserved.
    $Id$

*/

#include <exec/types.h>
#include <libraries/partition.h>
#include <utility/tagitem.h>

#include "partition_intern.h"

struct PCPartitionTable {
   UBYTE status;
   UBYTE start_head;
   UBYTE start_sector;
   UBYTE start_cylinder;
   UBYTE type;
   UBYTE end_head;
   UBYTE end_sector;
   UBYTE end_cylinder;
   ULONG first_sector;
   ULONG count_sector;
} __attribute__((packed));

struct MBR {
   BYTE boot_data[0x1BE];
   struct PCPartitionTable pcpt[4];
   UWORD magic;
} __attribute__((packed));

void PartitionMBRSetGeometry
    (
        struct PartitionHandle *root,
        struct PCPartitionTable *entry,
        ULONG sector,
        ULONG count,
        ULONG relative_sector  
    );

#define MBR_MAX_PARTITIONS (4)
#define MBRT_EXTENDED      (0x05)
#define MBRT_EXTENDED2     (0x0f)


#endif /* PARTITIONMBR_H */
