/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$
*/
#ifndef PROTO_CARDRES_H
#define PROTO_CARDRES_H

#ifndef AROS_SYSTEM_H
#include <aros/system.h>
#endif

#include <clib/cardres_protos.h>

#if defined(_AMIGA) && defined(__GNUC__)
#include <inline/cardres.h>
#else
#include <defines/cardres.h>
#endif

#endif /* PROTO_CARDRES_H */
