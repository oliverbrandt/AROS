#ifndef PKG_BZIP2_H
#define PKG_BZIP2_H

/*
    Copyright � 2003, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <exec/types.h>

/****************************************************************************/
/*** Prototypes *************************************************************/

APTR BZ2_Open( CONST_STRPTR path, LONG mode );
void BZ2_Close( APTR file );
LONG BZ2_Read( APTR file, APTR buffer, LONG length );
LONG BZ2_Position( APTR file );

#endif /* PKG_BZIP2_H */
