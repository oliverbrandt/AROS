#ifndef AROS_AROSBASE_H
#define AROS_AROSBASE_H
/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$

    Desc: aros.library general defines
    Lang: english
*/

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

#define AROSLIBNAME "aros.library"

/* Minimum version that supports everything from the current includes. */
/* Will be bumped whenever new functions are added to the library.     */
#define AROSLIBVERSION 41
#define AROSLIBREVISION 1


#endif /* AROS_AROSBASE_H */
