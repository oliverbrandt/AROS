#ifndef PROTO_MATHFFP_H
#define PROTO_MATHFFP_H

#ifndef AROS_SYSTEM_H
#include <aros/system.h>
#endif

#include <clib/mathffp_protos.h>

#if defined(_AMIGA) && defined(__GNUC__)
#ifndef NO_INLINE_STDARG
#define NO_INLINE_STDARG
#endif
#include <inline/mathffp.h>
#else
#include <defines/mathffp.h>
#endif

#endif /* PROTO_MATHFFP_H */
