/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: ANSI C function strtol()
    Lang: english
*/

#include <ctype.h>
#include <errno.h>
//#include <limits.h>

/*****************************************************************************

    NAME */
#include <stdlib.h>

	long strtol (

/*  SYNOPSIS */
	const char * str,
	char	  ** endptr,
	int	     base)

/*  FUNCTION
	Convert a string of digits into an integer according to the
	given base.

    INPUTS
	str - The string which should be converted. Leading
		whitespace are ignored. The number may be prefixed
		by a '+' or '-'. If base is above 10, then the
		alphabetic characters from 'A' are used to specify
		digits above 9 (ie. 'A' or 'a' is 10, 'B' or 'b' is
		11 and so on until 'Z' or 'z' is 35).
	endptr - If this is non-NULL, then the address of the first
		character after the number in the string is stored
		here.
	base - The base for the number. May be 0 or between 2 and 36,
		including both. 0 means to autodetect the base. strtoul()
		selects the base by inspecting the first characters
		of the string. If they are "0x", then base 16 is
		assumed. If they are "0", then base 8 is assumed. Any
		other digit will assume base 10. This is like in C.

		If you give base 16, then an optional "0x" may
		precede the number in the string.

    RESULT
	The value of the string. The first character after the number
	is returned in *endptr, if endptr is non-NULL. If no digits can
	be converted, *endptr contains str (if non-NULL) and 0 is
	returned.

    NOTES

    EXAMPLE
	// returns 1, ptr points to the 0-Byte
	strol ("  \t +0x1", &ptr, 0);

	// Returns 15. ptr points to the a
	strol ("017a", &ptr, 0);

	// Returns 215 (5*36 + 35)
	strol ("5z", &ptr, 36);

    BUGS

    SEE ALSO
        atof(), atoi(), atol(), strtod(), strtoul()

    INTERNALS

    HISTORY
	12.12.1996 digulla created

******************************************************************************/
{
    long val	= 0;
    char	* ptr;
    char	* copy;

    copy = (char *)str;

#if 0
    while (isspace (*str))
	str ++;

    if (*str)
    {
	val = strtoul (str, &ptr, base);

	if (endptr)
	{
	    if (ptr == str)
		str = copy;
	    else
		str = ptr;
	}

	if (*copy == '-')
	{
	    if ((signed long)val > 0)
	    {
		errno = ERANGE;

		val = LONG_MIN;
	    }
	}
	else
	{
	    if ((signed long)val < 0)
	    {
		errno = ERANGE;

		val = LONG_MAX;
	    }
	}
    }

    if (endptr)
	*endptr = (char *)str;

    return val;
#else
	return 0;
#endif
} /* strtol */

