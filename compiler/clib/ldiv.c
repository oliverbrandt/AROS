/*
    Copyright � 1995-2002, The AROS Development Team. All rights reserved.
    $Id$

    Desc: BSD function ldiv
*/

/*****************************************************************************

    NAME */
#include <stdlib.h>

	ldiv_t ldiv (

/*  SYNOPSIS */
	long int numer,
	long int denom)

/*  FUNCTION
 	Compute quotient en remainder of two long variables

    INPUTS
        numer = the numerator
        denom = the denominator

    RESULT
        a struct with two long ints quot and rem with
        quot = numer / denom and rem = numer % denom.
 
        typedef struct ldiv_t {
            long int quot;
            long int rem;
        } ldiv_t;

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	div()

    INTERNALS

******************************************************************************/
{
    ldiv_t ret;

    ret.quot = numer / denom;
    ret.rem  = numer % denom;
  
    return ret;
}
