/*
    Copyright (C) 1995-2001 AROS - The Amiga Research OS
    $Id$

    Desc: POSIX function readdir()
    Lang: english
*/

#include <dos/dos.h>
#include <proto/dos.h>

#include <string.h>
#include <errno.h>

#include "__open.h"

/*****************************************************************************

    NAME */
#include <dirent.h>

	struct dirent *readdir(

/*  SYNOPSIS */
	DIR *dir)

/*  FUNCTION
	 Reads a directory

    INPUTS
	dir - the directory stream pointing to the directory being read

    RESULT
	The  readdir()  function  returns  a  pointer  to a dirent
        structure, or NULL if an error occurs  or  end-of-file  is
        reached.

	The data returned by readdir() is  overwritten  by  subse�
        quent calls to readdir() for the same directory stream.

	According  to POSIX, the dirent structure contains a field
        char d_name[] of unspecified size, with at  most  NAME_MAX
        characters  preceding the terminating null character.  Use
        of other fields will harm the  portability  of  your  pro�
        grams.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
 	read(), opendir(), closedir(), rewinddir(), seekdir(),
	telldir(), scandir()

    INTERNALS

    HISTORY
	09.06.2001 falemagn created
******************************************************************************/
{
    GETUSER;

    int const max = MAXFILENAMELENGTH > NAME_MAX ? NAME_MAX : MAXFILENAMELENGTH;
    fdesc *desc;

    if (!dir)
    {
        errno = EFAULT;
	return NULL;
    }

    desc = __getfdesc(dir->fd);
    if (!desc)
    {
    	errno = EBADF;
    	return NULL;
    }

    if (dir->pos == 0)
    {
        dir->ent.d_name[0]='.';
    	dir->ent.d_name[1]='\0';
    }
    else
    if (dir->pos == 1)
    {
    	dir->ent.d_name[0]='.';
    	dir->ent.d_name[1]='.';
    	dir->ent.d_name[2]='\0';
    }
    else
    if (!ExNext(desc->fh, dir->priv))
    {
	dir->pos--;
	if (IoErr() != ERROR_NO_MORE_ENTRIES)
    	    errno = IoErr2errno(IoErr());

    	return NULL;
    }
    else
    strncpy
    (
	dir->ent.d_name,
	((struct FileInfoBlock *)dir->priv)->fib_FileName,
	max
    );

    dir->pos++;
    return &(dir->ent);
}
