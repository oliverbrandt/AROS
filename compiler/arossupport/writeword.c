/*
    Copyright (C) 1995-2000 AROS - The Amiga Research OS
    $Id$

    Desc: Write a big endian word (16bit) from a streamhook
    Lang: english
*/
#include <proto/dos.h>

/******************************************************************************

    NAME */
#include <stdio.h>
#include <aros/bigendianio.h>
#include <proto/alib.h>

	BOOL WriteWord (

/*  SYNOPSIS */
	struct Hook * hook,
	UWORD	      data,
	void	    * stream)

/*  FUNCTION
	Writes one big endian 16bit value to a streamhook.

    INPUTS
	hook - Write to this streamhook
	data - Data to be written
	stream - Stream passed to streamhook

    RESULT
	The function returns TRUE on success and FALSE otherwise.
	See IoErr() for the reason in case of an error.

    NOTES
	This function writes big endian values to a file even on little
	endian machines.

    EXAMPLE

    BUGS

    SEE ALSO
	ReadByte(), ReadWord(), ReadLong(), ReadFloat(), ReadDouble(),
	ReadString(), ReadStruct(), WriteByte(), WriteWord(), WriteLong(),
	WriteFloat(), WriteDouble(), WriteString(), WriteStruct()

    HISTORY

******************************************************************************/
{
    if (CallHook (hook, stream, BEIO_WRITE, data >> 8) == EOF)
	return FALSE;

    return (CallHook (hook, stream, BEIO_WRITE, data & 0xFF) != EOF);
} /* WriteWord */

