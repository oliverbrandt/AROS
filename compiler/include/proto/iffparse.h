#ifndef PROTO_IFFPARSE_H
#define PROTO_IFFPARSE_H

/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$
*/

#ifndef AROS_SYSTEM_H
#include <aros/system.h>
#endif

#include <clib/iffparse_protos.h>

#if defined(_AMIGA) && defined(__GNUC__)
#include <inline/iffparse.h>
#else
#include <defines/iffparse.h>
#endif

#endif /* PROTO_IFFPARSE_H */
