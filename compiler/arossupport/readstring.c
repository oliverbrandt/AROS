/*
    Copyright (C) 1995-2000 AROS - The Amiga Research OS
    $Id$

    Desc: Read one C string from a streamhook
    Lang: english
*/
#include <proto/dos.h>
#include <exec/memory.h>
#include <proto/exec.h>

/******************************************************************************

    NAME */
#include <stdio.h>
#include <aros/bigendianio.h>
#include <proto/alib.h>

	BOOL ReadString (

/*  SYNOPSIS */
	struct Hook * hook,
	STRPTR	    * dataptr,
	void	    * stream)

/*  FUNCTION
	Reads one C string from a streamhook.

    INPUTS
	hook - Streamhook
	dataptr - Put the data here. If you don't need the string anymore,
	    call FreeVec() to free it.
	stream - Read from this stream

    RESULT
	The function returns TRUE on success. On success, the string
	read is written into dataptr. On failure, FALSE is returned and the
	contents of dataptr are not changed. The string must be freed with
	FreeVec().

    NOTES
	This function reads big endian values from a streamhook even on
	little endian machines.

    EXAMPLE

    BUGS

    SEE ALSO
	ReadByte(), ReadWord(), ReadLong(), ReadFloat(), ReadDouble(),
	ReadString(), ReadStruct(), WriteByte(), WriteWord(), WriteLong(),
	WriteFloat(), WriteDouble(), WriteString(), WriteStruct()

    HISTORY
	14.09.93    ada created

******************************************************************************/
{
    STRPTR buffer;
    LONG   size, maxsize;
    LONG   c;

    size = maxsize = 0;
    buffer = NULL;

    for (;;)
    {
	c = CallHook (hook, stream, BEIO_READ);

	if (c == EOF)
	{
	    FreeVec (buffer);
	    return FALSE;
	}

	if (size == maxsize)
	{
	    STRPTR tmp;

	    tmp = AllocVec (maxsize + 16, MEMF_ANY);

	    if (!tmp)
	    {
		FreeVec (buffer);
		return FALSE;
	    }

	    if (buffer)
	    {
		strcpy (tmp, buffer);
		FreeVec (buffer);
	    }

	    buffer = tmp;
	}

	buffer[size ++] = c;

	if (!c)
	    break;
    }

    *dataptr = buffer;

    return TRUE;
} /* ReadString */

