/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: DOS function InternalLoadSeg()
    Lang: english
*/
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <aros/debug.h>
#include "dos_intern.h"
#include "internalloadseg.h"



/*****************************************************************************

    NAME */
#include <proto/dos.h>

        AROS_LH4(BPTR, InternalLoadSeg,

/*  SYNOPSIS */
        AROS_LHA(BPTR     , fh           , D0),
        AROS_LHA(BPTR     , table        , A0),
        AROS_LHA(LONG_FUNC, functionarray, A1),
        AROS_LHA(LONG *   , stack        , A2),

/*  LOCATION */
        struct DosLibrary *, DOSBase, 126, Dos)

/*  FUNCTION
        Loads from fh.
        Functionarray is a pointer to an array of functions. See below.

        This function really only tries to load the different file
        formats aos, elf and aout.

    INPUTS
        fh            : Filehandle to load from
        table         : ignored
        functionarray : Array of function to be used fro read, alloc and free
           FuncTable[0] -> bytes  = ReadFunc(readhandle, buffer, length),DOSBase
                           D0                D1          A0      D0      A6
           FuncTable[1] -> Memory = AllocFunc(size,flags), ExecBase
                           D0                 D0   D1      A6
           FuncTable[2] -> FreeFunc(memory, size), ExecBase
                                    A1       D0    A6
        stack         : pointer to storage (ULONG) for stacksize.
                        (currently ignored)

    RESULT
        seglist  - pointer to loaded Seglist or NULL in case of failure.

    NOTES

    EXAMPLE

    BUGS
       Use of table and stack are not implemented, yet!

    SEE ALSO
        UnLoadSeg()

    INTERNALS

*****************************************************************************/
{
  AROS_LIBFUNC_INIT
  AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)

  BPTR segs = 0;

  /* Open the file */

  if (fh)
  {
    /* Then try to load the different file formats */
    if (!segs)
    {
      segs = InternalLoadSeg_ELF (fh, MKBADDR(NULL), functionarray, NULL, DOSBase);
#if DEBUG > 1
      if (segs)
        bug("Loaded as ELF exe\n");
#endif
    }
    if (!segs && IoErr()==ERROR_NOT_EXECUTABLE)
    {
      segs = InternalLoadSeg_AOS (fh, MKBADDR(NULL), functionarray, NULL, DOSBase);
#if DEBUG > 1
      if (segs)
        bug("Loaded as AmigaOS exe\n");
#endif
    }
    if (!segs && IoErr()==ERROR_NOT_EXECUTABLE)
    {
      segs = InternalLoadSeg_AOUT (fh, MKBADDR(NULL), functionarray, NULL, DOSBase);
#if DEBUG > 1
      if (segs)
        bug("Loaded as a.out exe\n");
#endif
    }
  }

  if (segs)
    SetIoErr (0);
#if DEBUG > 1
  else
    bug ("Loading failed\n");
#endif

  /* And return */
  return segs;
  AROS_LIBFUNC_EXIT
} /* InternalLoadSeg */
