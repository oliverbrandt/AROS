/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$
*/
#ifndef PROTO_AROSSUPPORT_H
#define PROTO_AROSSUPPORT_H

#ifndef AROS_SYSTEM_H
#include <aros/system.h>
#endif

#include <clib/arossupport_protos.h>

#if defined(_AMIGA) && defined(__GNUC__)
/*#include <inline/arossupport.h>*/
#else
#include <defines/arossupport.h>
#endif

#endif /* PROTO_AROSSUPPORT_H */
