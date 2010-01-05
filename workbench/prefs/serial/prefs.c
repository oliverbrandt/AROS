/*
    Copyright � 1995-2008, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: English
*/

/*********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <aros/macros.h>

/* #define DEBUG 1 */
#include <aros/debug.h>

#include <proto/iffparse.h>
#include <proto/dos.h>

#include <prefs/prefhdr.h>

#include "prefs.h"
#include "misc.h"

/*********************************************************************************************/

#define PREFS_PATH_ENVARC "ENVARC:SYS/serial.prefs"
#define PREFS_PATH_ENV    "ENV:SYS/serial.prefs"

/*********************************************************************************************/

struct FilePrefHeader
{
    UBYTE ph_Version;
    UBYTE ph_Type;
    UBYTE ph_Flags[4];
};

/*********************************************************************************************/

struct SerialPrefs serialprefs;

/*********************************************************************************************/

#define SER_DEFAULT_BAUDRATE       19200
#define SER_DEFAULT_IBUFFER        512
#define SER_DEFAULT_OBUFFER        512

#define SER_DEFAULT_IHANDSHAKE     HSHAKE_NONE
#define SER_DEFAULT_OHANDSHAKE     HSHAKE_NONE

#define SER_DEFAULT_PARITY         PARITY_NONE
#define SER_DEFAULT_BITS_PER_CHAR  8
#define SER_DEFAULT_STOP_BITS      1

/*********************************************************************************************/

static BOOL Prefs_Load(STRPTR from)
{
    BPTR fh = Open(from, MODE_OLDFILE);
    if (fh)
    {
        Prefs_ImportFH(fh);
        Close(fh);
        return TRUE;
    }
    return FALSE;
}

/*********************************************************************************************/

BOOL Prefs_ImportFH(BPTR fh)
{
    struct SerialPrefs  loadprefs;
    struct IFFHandle   *iff;
    BOOL                retval = FALSE;

    if ((iff = AllocIFF()))
    {
        iff->iff_Stream = (IPTR)fh;

        InitIFFasDOS(iff);

        if (!OpenIFF(iff, IFFF_READ))
        {
            D(bug("LoadPrefs: OpenIFF okay.\n"));

            if (!StopChunk(iff, ID_PREF, ID_SERL))
            {
                D(bug("LoadPrefs: StopChunk okay.\n"));

                if (!ParseIFF(iff, IFFPARSE_SCAN))
                {
                    struct ContextNode *cn;

                    D(bug("LoadPrefs: ParseIFF okay.\n"));

                    cn = CurrentChunk(iff);

                    if (cn->cn_Size == sizeof(struct SerialPrefs))
                    {
                        D(bug("LoadPrefs: ID_SERL chunk size okay.\n"));

                        if (ReadChunkBytes(iff, &loadprefs, sizeof(struct SerialPrefs)) == sizeof(struct SerialPrefs))
                        {
                            D(bug("LoadPrefs: Reading chunk successful.\n"));

                            serialprefs = loadprefs;

                            D(bug("LoadPrefs: Everything okay :-)\n"));

                            retval = TRUE;
                        }
                    }
                } /* if (!ParseIFF(iff, IFFPARSE_SCAN)) */
            } /* if (!StopChunk(iff, ID_PREF, ID_SERL)) */
            CloseIFF(iff);
        } /* if (!OpenIFF(iff, IFFF_READ)) */
        FreeIFF(iff);
    } /* if ((iff = AllocIFF())) */
    return retval;
}

/*********************************************************************************************/

BOOL Prefs_ExportFH(BPTR fh)
{
    struct SerialPrefs  saveprefs;
    struct IFFHandle   *iff;
    BOOL                retval = FALSE;
    BOOL                delete_if_error = FALSE;

    saveprefs = serialprefs;

    D(bug("SavePrefsFH: fh: %lx\n", fh));

    if ((iff = AllocIFF()))
    {
        iff->iff_Stream = (IPTR) fh;
        D(bug("SavePrefsFH: stream opened.\n"));

        delete_if_error = TRUE;

        InitIFFasDOS(iff);

        if (!OpenIFF(iff, IFFF_WRITE))
        {
            D(bug("SavePrefsFH: OpenIFF okay.\n"));

            if (!PushChunk(iff, ID_PREF, ID_FORM, IFFSIZE_UNKNOWN))
            {
                D(bug("SavePrefsFH: PushChunk(FORM) okay.\n"));

                if (!PushChunk(iff, ID_PREF, ID_PRHD, sizeof(struct FilePrefHeader)))
                {
                    struct FilePrefHeader head;

                    D(bug("SavePrefsFH: PushChunk(PRHD) okay.\n"));

                    head.ph_Version  = PHV_CURRENT;
                    head.ph_Type     = 0;
                    head.ph_Flags[0] =
                    head.ph_Flags[1] =
                    head.ph_Flags[2] =
                    head.ph_Flags[3] = 0;

                    if (WriteChunkBytes(iff, &head, sizeof(head)) == sizeof(head))
                    {
                        D(bug("SavePrefsFH: WriteChunkBytes(PRHD) okay.\n"));

                        PopChunk(iff);

                        if (!PushChunk(iff, ID_PREF, ID_SERL, sizeof(struct SerialPrefs)))
                        {
                            D(bug("SavePrefsFH: PushChunk(LCLE) okay.\n"));

                            if (WriteChunkBytes(iff, &saveprefs, sizeof(saveprefs)) == sizeof(saveprefs))
                            {
                                D(bug("SavePrefsFH: WriteChunkBytes(SERL) okay.\n"));
                                D(bug("SavePrefsFH: Everything okay :-)\n"));

                                retval = TRUE;
                            }
                            PopChunk(iff);
                        } /* if (!PushChunk(iff, ID_PREF, ID_SERL, sizeof(struct LocalePrefs))) */
                    } /* if (WriteChunkBytes(iff, &head, sizeof(head)) == sizeof(head)) */
                    else
                    {
                        PopChunk(iff);
                    }
                } /* if (!PushChunk(iff, ID_PREF, ID_PRHD, sizeof(struct PrefHeader))) */
                PopChunk(iff);
            } /* if (!PushChunk(iff, ID_PREF, ID_FORM, IFFSIZE_UNKNOWN)) */
            CloseIFF(iff);
        } /* if (!OpenIFF(iff, IFFFWRITE)) */
        FreeIFF(iff);

    } /* if ((iff = AllocIFF())) */

    #if 0
    if (!retval && delete_if_error)
    {
        DeleteFile(filename);
    }
    #endif


    return retval;
}

/*********************************************************************************************/

BOOL Prefs_HandleArgs(STRPTR from, BOOL use, BOOL save)
{
    BPTR fh;

    if (from)
    {
        if (!Prefs_Load(from))
        {
            ShowMessage("Can't read from input file");
            return FALSE;
        }
    }
    else
    {
        if (!Prefs_Load(PREFS_PATH_ENV))
        {
            if (!Prefs_Load(PREFS_PATH_ENVARC))
            {
                ShowMessage
                (
                    "Can't read from file " PREFS_PATH_ENV
                    ".\nUsing default values."
                );
                Prefs_Default();
            }
        }
    }

    if (use || save)
    {
        fh = Open(PREFS_PATH_ENV, MODE_NEWFILE);
        if (fh)
        {
            Prefs_ExportFH(fh);
            Close(fh);
        }
        else
        {
            ShowMessage("Cant' open " PREFS_PATH_ENV " for writing.");
        }
    }
    if (save)
    {
        fh = Open(PREFS_PATH_ENVARC, MODE_NEWFILE);
        if (fh)
        {
            Prefs_ExportFH(fh);
            Close(fh);
        }
        else
        {
            ShowMessage("Cant' open " PREFS_PATH_ENVARC " for writing.");
        }
    }
    return TRUE;
}

/*********************************************************************************************/

BOOL Prefs_Default(VOID)
{
    serialprefs.sp_Reserved[0]     = 0;
    serialprefs.sp_Reserved[1]     = 0;
    serialprefs.sp_Reserved[2]     = 0;
    serialprefs.sp_Unit0Map        = 0;
    serialprefs.sp_BaudRate        = SER_DEFAULT_BAUDRATE;
    serialprefs.sp_InputBuffer     = SER_DEFAULT_IBUFFER;
    serialprefs.sp_OutputBuffer    = SER_DEFAULT_OBUFFER;
    serialprefs.sp_InputHandshake  = SER_DEFAULT_IHANDSHAKE;
    serialprefs.sp_OutputHandshake = SER_DEFAULT_OHANDSHAKE;
    serialprefs.sp_Parity          = SER_DEFAULT_PARITY;
    serialprefs.sp_BitsPerChar     = SER_DEFAULT_BITS_PER_CHAR;
    serialprefs.sp_StopBits        = SER_DEFAULT_STOP_BITS;

    return TRUE;
}

