/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: ANSI C function fgets()
    Lang: english
*/
#include <errno.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>

/*****************************************************************************

    NAME */
#include <stdio.h>

	char * fgets (

/*  SYNOPSIS */
	char * buffer,
	int    size,
	FILE * stream)

/*  FUNCTION
	Read one line of characters from the stream into the buffer.
	Reading will stop, when a newline ('\n') is encountered, EOF
	or when the buffer is full. If a newline is read, then it is
	put into the buffer. The last character in the buffer is always
	'\0' (Therefore at most size-1 characters can be read in one go).

    INPUTS
	buffer - Write characters into this buffer
	size - This is the size of the buffer in characters.
	stream - Read from this stream

    RESULT
	buffer or NULL in case of an error or EOF.

    NOTES

    EXAMPLE
	// Read a file line by line
	char line[256];

	// Read until EOF
	while (fgets (line, sizeof (line), fh))
	{
	    // Evaluate the line
	}

    BUGS

    SEE ALSO
	fopen(), gets(), fputs(), putc()

    INTERNALS

    HISTORY
	10.12.1996 digulla created

******************************************************************************/
{
    BPTR fh;

    switch ((IPTR)stream)
    {
    case 1: /* Stdin */
	fh = Input ();

    case 2: /* Stdout */
    case 3: /* Stderr */
	errno = EINVAL;
	return NULL;

    default:
	fh = (BPTR)stream->fh;
	break;
    }

    buffer = FGets (fh, buffer, size);

    if (!buffer)
    {
	if (IoErr ())
	    stream->flags |= _STDIO_FILEFLAG_ERROR;
	else
	    stream->flags |= _STDIO_FILEFLAG_EOF;
    }

    return buffer;
} /* fgets */

