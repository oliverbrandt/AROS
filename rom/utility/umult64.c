/*
    $Id$
    $Log$
    Revision 1.1  1996/08/31 12:58:14  aros
    Merged in/modified for FreeBSD.

    Desc: Unsigned 64-bit product of two 32-bit numbers.
    Lang: english
*/
#include "utility_intern.h"

/*****************************************************************************

    NAME */
        #include <clib/utility_protos.h>

        __AROS_LH2(ULONG, UMult64,

/*  SYNOPSIS */
        __AROS_LHA(unsigned long, arg1, D0),
        __AROS_LHA(unsigned long, arg2, D1),

/*  LOCATION */
        struct UtilityBase *, UtilityBase, 34, Utility)

/*  FUNCTION
        Compute the unsigned 64-bit product of arg1 * arg2.

    INPUTS
        arg1, arg2  -   32 bit unsigned numbers.

    RESULT
        arg1 * arg2

    NOTES

    EXAMPLE

    BUGS
        There is a problem under the current system in that it is very
        hard to return a 64-bit value.

    SEE ALSO
        utility/SMult32(), utility/UMult32(), utility/SMult64()

    INTERNALS
        This is essentially UMult32(), but without the code to calculate
        the product of the high 32 bits of the multiplicands.

        In fact all that is added is the code to calculate 2^32 * ac.

    HISTORY
        29-10-95    digulla automatically created from
                            utility_lib.fd and clib/utility_protos.h
        18-08-96    iaint   Modified UMult32().

*****************************************************************************/
{
    __AROS_FUNC_INIT

#ifdef HAS_64BITMULU
    return arg1 * arg2;
#else

    unsigned long long product;
    UWORD a0, a1, b0, b1;

    a1 = (arg1 >> 16) & 0xffff;
    a0 = arg1 & 0xffff;
    b1 = (arg2 >> 16) & 0xffff;
    b0 = arg2 & 0xffff;

    /* In case number is quite small an optimization */
    if(a1 && b1)
        product = (unsigned long long)(a1 * b1) << 32;
    else
        product = 0;

    product += (((a1 * b0) + (a0 * b1)) << 16) + (a0 * b0);
    return product;
#endif
    __AROS_FUNC_EXIT
} /* UMult64 */
