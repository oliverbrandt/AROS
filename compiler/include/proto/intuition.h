/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$
*/
#ifndef PROTO_INTUITION_H
#define PROTO_INTUITION_H

#ifndef AROS_SYSTEM_H
#include <aros/system.h>
#endif

#include <clib/intuition_protos.h>

#if defined(_AMIGA) && defined(__GNUC__)
#include <inline/intuition.h>
#else
#include <defines/intuition.h>
#endif

#endif /* PROTO_INTUITION_H */
