#ifndef _INLINE_PARTITION_H
#define _INLINE_PARTITION_H

/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    *** Automatically generated by geninline.awk. Do not edit ***

    Desc: Inlines for Partition.library
    Lang: english
*/

#ifndef __INLINE_MACROS_H
#   include <inline/macros.h>
#endif

#ifndef PARTITION_BASE_NAME
#define PARTITION_BASE_NAME PartitionBase
#endif

/* Prototypes */
#define OpenRootPartition(Device, Unit) \
        LP2(0x1e, struct PartitionHandle *, OpenRootPartition, STRPTR, Device, a1, LONG, Unit, d1, \
        , PARTITION_BASE_NAME)

#define CloseRootPartition(ph) \
        LP1NR(0x24, CloseRootPartition, struct PartitionHandle *, ph, a1, \
        , PARTITION_BASE_NAME)

#define OpenPartitionTable(root) \
        LP1(0x2a, LONG, OpenPartitionTable, struct PartitionHandle *, root, a1, \
        , PARTITION_BASE_NAME)

#define ClosePartitionTable(root) \
        LP1NR(0x30, ClosePartitionTable, struct PartitionHandle *, root, a1, \
        , PARTITION_BASE_NAME)

#define WritePartitionTable(root) \
        LP1(0x36, LONG, WritePartitionTable, struct PartitionHandle *, root, a1, \
        , PARTITION_BASE_NAME)

#define CreatePartitionTable(root, type) \
        LP2(0x3c, LONG, CreatePartitionTable, struct PartitionHandle *, root, a1, ULONG, type, d1, \
        , PARTITION_BASE_NAME)

#define AddPartition(root, taglist) \
        LP2(0x42, struct PartitionHandle *, AddPartition, struct PartitionHandle *, root, a1, struct TagItem *, taglist, a1, \
        , PARTITION_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AddPartitionTags(a1, tags...) \
        ({ULONG _tags[] = { tags }; AddPartition((a1), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define DeletePartition(ph) \
        LP1NR(0x48, DeletePartition, struct PartitionHandle *, ph, a1, \
        , PARTITION_BASE_NAME)

#define GetPartitionTableAttrs(root, taglist) \
        LP2(0x4e, LONG, GetPartitionTableAttrs, struct PartitionHandle *, root, a1, struct TagItem *, taglist, a2, \
        , PARTITION_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define GetPartitionTableAttrsTags(a1, tags...) \
        ({ULONG _tags[] = { tags }; GetPartitionTableAttrs((a1), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define SetPartitionTableAttrs(root, taglist) \
        LP2(0x54, LONG, SetPartitionTableAttrs, struct PartitionHandle *, root, a1, struct TagItem *, taglist, a2, \
        , PARTITION_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define SetPartitionTableAttrsTags(a1, tags...) \
        ({ULONG _tags[] = { tags }; SetPartitionTableAttrs((a1), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define GetPartitionAttrs(ph, taglist) \
        LP2(0x5a, LONG, GetPartitionAttrs, struct PartitionHandle *, ph, a1, struct TagItem *, taglist, a2, \
        , PARTITION_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define GetPartitionAttrsTags(a1, tags...) \
        ({ULONG _tags[] = { tags }; GetPartitionAttrs((a1), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define SetPartitionAttrs(ph, taglist) \
        LP2(0x60, LONG, SetPartitionAttrs, struct PartitionHandle *, ph, a1, struct TagItem *, taglist, a2, \
        , PARTITION_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define SetPartitionAttrsTags(a1, tags...) \
        ({ULONG _tags[] = { tags }; SetPartitionAttrs((a1), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define QueryPartitionTableAttrs(table) \
        LP1(0x66, ULONG *, QueryPartitionTableAttrs, struct PartitionHandle *, table, a1, \
        , PARTITION_BASE_NAME)

#define QueryPartitionAttrs(table) \
        LP1(0x6c, ULONG *, QueryPartitionAttrs, struct PartitionHandle *, table, a1, \
        , PARTITION_BASE_NAME)

#endif /* _INLINE_PARTITION_H */
