#ifndef PROTO_MATHIEEESINGBAS_H
#define PROTO_MATHIEEESINGBAS_H

#ifndef AROS_SYSTEM_H
#include <aros/system.h>
#endif

#include <clib/mathieeesingbas_protos.h>

#if defined(_AMIGA) && defined(__GNUC__)
#ifndef NO_INLINE_STDARG
#define NO_INLINE_STDARG
#endif
#include <inline/mathieeesingbas.h>
#else
#include <defines/mathieeesingbas.h>
#endif

#endif /* PROTO_MATHIEEESINGBAS_H */
