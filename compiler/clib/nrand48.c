/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Function nrand48()
    Lang: english
*/

#include <aros/machine.h>
extern void __calc_seed(unsigned short int xsubi[3]);

/*****************************************************************************

    NAME */
#include <stdlib.h>

	long int nrand48 (

/*  SYNOPSIS */
	unsigned short int xsubi[3])

/*  FUNCTION
        Compute a random integer between [0, 2^32-1]

    INPUTS
        None.

    RESULT
        Random number

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	srand48(), erand48(), drand48()

    INTERNALS

    HISTORY

******************************************************************************/
{
	unsigned long int retval;
	
	__calc_seed(xsubi);
#if (AROS_BIG_ENDIAN == 0)
	retval = *(long int *)&xsubi[1];
#else
	retval = (unsigned long int)xsubi[1] | ((unsigned long int)xsubi[0] << 16);
#endif

	return retval>>1;
} /* nrand48 */
