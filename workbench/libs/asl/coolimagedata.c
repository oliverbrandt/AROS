/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$
*/

/**************************************************************************/

#include "asl_intern.h"

/**************************************************************************/

#if !USE_SHARED_COOLIMAGES

/**************************************************************************/

#include "coolimages.h"

#define COOL_SAVEIMAGE          1
#define COOL_LOADIMAGE          1
#define COOL_USEIMAGE           1
#define COOL_CANCELIMAGE        1
#define COOL_DOTIMAGE           1
#define COOL_DOTIMAGE2          1
#define COOL_WARNIMAGE          0
#define COOL_DISKIMAGE          0
#define COOL_SWITCHIMAGE        0
#define COOL_MONITORIMAGE       1
#define COOL_INFOIMAGE          0
#define COOL_ASKIMAGE           0
#define COOL_KEYIMAGE           0

/**************************************************************************/

#if COOL_SAVEIMAGE

#define SAVEIMAGE_WIDTH 23
#define SAVEIMAGE_HEIGHT 16
#define SAVEIMAGE_DEPTH 8

static const UBYTE saveimage_data[] =
{
        00,04,04,04,04,04,04,04,04,04,04,04,04,04,04,00,00,00,00,00,00,00,00,
        04,04,02,02,02,02,02,02,02,02,02,02,02,04,04,00,00,00,00,00,00,00,00,
        04,04,02,02,04,04,04,04,04,04,04,02,02,04,02,03,03,03,03,00,00,00,00,
        04,04,02,02,02,02,02,02,02,02,02,02,02,02,03,03,03,03,03,01,00,00,00,
        04,04,02,02,04,04,04,04,02,02,02,02,02,03,03,03,03,03,03,03,01,00,00,
        04,04,02,02,02,02,02,02,02,02,02,02,02,03,03,03,01,03,03,03,03,01,00,
        04,04,04,02,02,02,02,02,02,02,02,02,03,03,03,01,00,03,03,03,03,03,01,
        04,04,04,04,04,04,04,04,02,03,03,02,03,03,01,02,00,00,03,03,03,03,01,
        04,04,04,04,04,04,04,04,02,03,03,03,03,03,01,02,00,00,00,03,03,03,01,
        04,04,04,02,02,02,02,02,02,03,03,03,03,03,03,03,03,00,00,00,03,01,00,
        04,04,04,02,02,04,04,02,02,03,03,03,03,03,03,03,01,00,00,00,00,00,00,
        04,04,04,02,02,04,04,02,02,02,03,03,03,03,01,01,00,00,00,00,00,00,00,
        04,04,04,02,02,04,04,02,02,02,03,03,03,01,02,01,00,00,00,00,00,00,00,
        04,04,04,02,02,02,02,02,02,02,02,03,01,02,04,01,00,00,00,00,00,00,00,
        00,01,01,01,01,01,01,01,01,01,01,02,02,01,00,00,00,00,00,00,00,00,00,
        00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00
};

static const UBYTE saveimage_pal[] =
{
        0xb3,0xb3,0xb3,0x00,0x00,0x00,
        0xe0,0xe0,0xe0,0xdb,0x20,0x26,
        0x65,0x4b,0xbf,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,0xff,0xff
};

const struct CoolImage cool_saveimage =
{
        saveimage_data,
        saveimage_pal,
        SAVEIMAGE_WIDTH,
        SAVEIMAGE_HEIGHT,
        SAVEIMAGE_DEPTH
};

#endif /* COOL_SAVEIMAGE */

/**************************************************************************/

#if COOL_LOADIMAGE

#define LOADIMAGE_WIDTH 23
#define LOADIMAGE_HEIGHT 16
#define LOADIMAGE_DEPTH 8

static const UBYTE loadimage_data[] =
{
        00,04,04,04,04,04,04,04,04,04,04,04,04,04,04,00,00,00,00,00,00,00,00,
        04,04,02,02,02,02,02,02,02,02,02,02,02,02,02,01,00,00,00,00,00,00,00,
        04,04,02,02,04,04,04,04,04,04,04,02,02,03,03,03,03,00,00,00,00,00,00,
        04,04,02,02,02,02,02,02,02,02,02,02,03,03,03,03,03,03,00,00,00,00,00,
        04,04,02,02,04,04,04,04,02,02,02,03,03,03,01,03,03,03,01,00,00,00,00,
        04,04,02,02,02,02,02,02,02,02,03,03,03,01,02,03,03,03,01,00,00,00,00,
        04,04,04,02,02,02,02,02,02,03,03,03,01,02,04,02,03,03,03,01,00,00,00,
        04,04,04,04,04,04,04,04,04,02,03,01,02,04,04,01,02,03,03,01,00,03,03,
        04,04,04,04,04,04,04,04,04,04,02,02,04,04,04,02,02,03,03,03,03,03,01,
        04,04,04,02,02,02,02,02,02,02,02,02,02,04,02,03,03,03,03,03,03,03,01,
        04,04,04,02,02,04,04,02,02,02,02,02,02,04,02,03,03,03,03,03,03,03,01,
        04,04,04,02,02,04,04,02,02,02,02,02,02,04,04,01,03,03,03,03,03,01,00,
        04,04,04,02,02,04,04,02,02,02,02,02,02,04,04,01,00,00,03,03,03,01,00,
        04,04,04,02,02,02,02,02,02,02,02,02,02,04,04,01,00,00,00,03,01,00,00,
        00,01,01,01,01,01,01,01,01,01,01,01,01,01,01,00,00,00,00,00,00,00,00,
        00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00
};

static const UBYTE loadimage_pal[] =
{
        0xb3,0xb3,0xb3,0x00,0x00,0x00,
        0xe0,0xe0,0xe0,0x00,0x76,0x00,
        0x65,0x4b,0xbf,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,0xff,0xff
};

const struct CoolImage cool_loadimage =
{
        loadimage_data,
        loadimage_pal,
        LOADIMAGE_WIDTH,
        LOADIMAGE_HEIGHT,
        LOADIMAGE_DEPTH
};

#endif /* COOL_LOADIMAGE */

/**************************************************************************/

#if COOL_USEIMAGE

#define USEIMAGE_WIDTH 13
#define USEIMAGE_HEIGHT 16
#define USEIMAGE_DEPTH 4

static const UBYTE useimage_data[] =
{
        00,00,00,00,00,00,00,00,00,02,02,02,00,
        00,00,00,00,00,00,00,00,02,02,02,02,01,
        00,00,00,00,00,00,00,02,02,02,02,02,01,
        00,00,00,00,00,00,00,02,02,02,02,01,00,
        00,00,00,00,00,00,02,02,02,02,01,00,00,
        00,00,00,00,00,00,02,02,02,01,00,00,00,
        00,02,02,00,00,00,02,02,02,01,00,00,00,
        02,02,02,02,00,02,02,02,01,00,00,00,00,
        02,02,02,02,02,02,02,02,01,00,00,00,00,
        02,02,02,02,02,02,02,02,01,00,00,00,00,
        00,02,02,02,02,02,02,01,00,00,00,00,00,
        00,00,02,02,02,02,02,01,00,00,00,00,00,
        00,00,02,02,02,02,02,01,00,00,00,00,00,
        00,00,00,02,02,02,01,00,00,00,00,00,00,
        00,00,00,02,02,02,01,00,00,00,00,00,00,
        00,00,00,00,02,01,00,00,00,00,00,00,00
};

static const UBYTE useimage_pal[] =
{
        0xb3,0xb3,0xb3,0x00,0x00,0x00,
        0x00,0x76,0x00,0xff,0xff,0xff
};

const struct CoolImage cool_useimage =
{
        useimage_data,
        useimage_pal,
        USEIMAGE_WIDTH,
        USEIMAGE_HEIGHT,
        USEIMAGE_DEPTH
};

#endif /* COOL_USEIMAGE */

/**************************************************************************/

#if COOL_CANCELIMAGE

#define CANCELIMAGE_WIDTH 14
#define CANCELIMAGE_HEIGHT 16
#define CANCELIMAGE_DEPTH 4

static const UBYTE cancelimage_data[] =
{
        00,00,00,00,00,00,00,00,00,00,02,02,02,00,
        00,02,02,01,00,00,00,00,00,02,02,02,02,01,
        02,02,02,02,01,00,00,00,02,02,02,02,02,01,
        02,02,02,02,02,01,00,02,02,02,02,02,01,00,
        00,02,02,02,02,02,02,02,02,02,02,01,00,00,
        00,00,02,02,02,02,02,02,02,02,01,00,00,00,
        00,00,00,02,02,02,02,02,02,01,00,00,00,00,
        00,00,00,00,02,02,02,02,01,00,00,00,00,00,
        00,00,00,02,02,02,02,02,02,02,00,00,00,00,
        00,00,02,02,02,02,02,02,02,02,02,00,00,00,
        00,02,02,02,02,02,01,01,02,02,02,02,00,00,
        00,02,02,02,02,01,00,00,01,02,02,02,02,00,
        02,02,02,02,01,00,00,00,00,01,02,02,02,01,
        02,02,02,01,00,00,00,00,00,00,01,02,02,01,
        02,02,02,01,00,00,00,00,00,00,00,01,01,00,
        00,01,01,00,00,00,00,00,00,00,00,00,00,00
};

static const UBYTE cancelimage_pal[] =
{
        0xb3,0xb3,0xb3,0x00,0x00,0x00,
        0xdb,0x20,0x26,0xff,0xff,0xff
};

const struct CoolImage cool_cancelimage =
{
        cancelimage_data,
        cancelimage_pal,
        CANCELIMAGE_WIDTH,
        CANCELIMAGE_HEIGHT,
        CANCELIMAGE_DEPTH
};

#endif /* COOL_CANCELIMAGE */

/**************************************************************************/

#if COOL_DOTIMAGE

#define DOTIMAGE_WIDTH 9
#define DOTIMAGE_HEIGHT 9
#define DOTIMAGE_DEPTH 4

static const UBYTE dotimage_data[] =
{
        00,00,00,03,03,03,00,00,00,
        00,03,03,03,03,03,03,01,00,
        00,03,03,03,03,03,03,01,00,
        03,03,03,03,03,03,03,03,01,
        03,03,03,03,03,03,03,03,01,
        03,03,03,03,03,03,03,03,01,
        00,03,03,03,03,03,03,01,00,
        00,01,01,03,03,03,01,01,00,
        00,00,00,01,01,01,00,00,00
};

static const UBYTE dotimage_pal[] =
{
        0xb3,0xb3,0xb3,0x00,0x00,0x00,
        0xff,0xff,0xff,0x00,0x76,0x00
};

const struct CoolImage cool_dotimage =
{
        dotimage_data,
        dotimage_pal,
        DOTIMAGE_WIDTH,
        DOTIMAGE_HEIGHT,
        DOTIMAGE_DEPTH
};

#endif /* COOL_DOTIMAGE */

/**************************************************************************/

#if COOL_DOTIMAGE2

static const UBYTE dotimage2_pal[] =
{
        0xb3,0xb3,0xb3,0x00,0x00,0x00,
        0xff,0xff,0xff,0x00,0x00,0xA0
};

const struct CoolImage cool_dotimage2 =
{
        dotimage_data,
        dotimage2_pal,
        DOTIMAGE_WIDTH,
        DOTIMAGE_HEIGHT,
        DOTIMAGE_DEPTH
};

#endif

/**************************************************************************/

#if COOL_WARNIMAGE

#define WARNIMAGE_WIDTH 7
#define WARNIMAGE_HEIGHT 16
#define WARNIMAGE_DEPTH 4

static const UBYTE warnimage_data[] =
{
        00,00,02,02,02,00,00,
        00,00,02,02,02,02,01,
        00,00,02,02,02,02,01,
        00,00,02,02,02,02,01,
        00,02,02,02,02,01,00,
        00,02,02,02,02,01,00,
        00,02,02,02,02,01,00,
        00,02,02,02,02,01,00,
        00,02,02,02,01,00,00,
        00,02,02,02,01,00,00,
        00,00,01,01,01,00,00,
        00,00,00,00,00,00,00,
        00,02,02,02,02,00,00,
        02,02,02,02,02,01,00,
        02,02,02,02,02,01,00,
        00,01,01,01,01,00,00
};

static const UBYTE warnimage_pal[] =
{
        0xb3,0xb3,0xb3,0x00,0x00,0x00,
        0xb6,0x37,0xb6,0xff,0xff,0xff
};

const struct CoolImage cool_warnimage =
{
        warnimage_data,
        warnimage_pal,
        WARNIMAGE_WIDTH,
        WARNIMAGE_HEIGHT,
        WARNIMAGE_DEPTH
};

#endif /* COOL_WARNIMAGE */

/**************************************************************************/

#if COOL_DISKIMAGE

#define DISKIMAGE_WIDTH 16
#define DISKIMAGE_HEIGHT 16
#define DISKIMAGE_DEPTH 4

static const UBYTE diskimage_data[] =
{
        00,03,03,03,03,03,03,03,03,03,03,03,03,03,03,00,
        03,03,02,02,02,02,02,02,02,02,02,02,02,02,03,01,
        03,03,02,02,03,03,03,03,03,03,03,03,02,02,03,01,
        03,03,02,02,02,02,02,02,02,02,02,02,02,02,03,01,
        03,03,02,02,03,03,03,03,03,03,03,03,02,02,03,01,
        03,03,02,02,02,02,02,02,02,02,02,02,02,02,03,01,
        03,03,03,02,02,02,02,02,02,02,02,02,02,03,03,01,
        03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,01,
        03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,01,
        03,03,03,02,02,02,02,02,02,02,02,02,02,03,03,01,
        03,03,03,02,02,03,03,02,02,02,02,02,02,03,03,01,
        03,03,03,02,02,03,03,02,02,02,02,02,02,03,03,01,
        03,03,03,02,02,03,03,02,02,02,02,02,02,03,03,01,
        03,03,03,02,02,02,02,02,02,02,02,02,02,03,03,01,
        00,01,01,01,01,01,01,01,01,01,01,01,01,01,01,00,
        00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00
};

static const UBYTE diskimage_pal[] =
{
        0xb3,0xb3,0xb3,0x00,0x00,0x00,
        0xe0,0xe0,0xe0,0x65,0x4b,0xbf
};

const struct CoolImage cool_diskimage =
{
        diskimage_data,
        diskimage_pal,
        DISKIMAGE_WIDTH,
        DISKIMAGE_HEIGHT,
        DISKIMAGE_DEPTH
};

#endif /* COOL_DISKIMAGE */

/**************************************************************************/

#if COOL_SWITCHIMAGE

#define SWITCHIMAGE_WIDTH 18
#define SWITCHIMAGE_HEIGHT 16
#define SWITCHIMAGE_DEPTH 4

static const UBYTE switchimage_data[] =
{
        00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
        00,00,00,00,00,00,02,02,02,02,00,00,00,00,00,00,00,00,
        00,00,00,00,00,02,02,02,02,02,01,00,00,00,00,00,00,00,
        00,00,00,00,02,02,02,02,02,02,02,01,00,00,03,01,00,00,
        00,00,00,00,02,02,02,01,00,02,02,02,01,03,03,03,01,00,
        00,00,00,02,02,02,01,00,00,00,02,03,03,03,03,03,01,00,
        02,02,00,02,02,01,00,00,00,00,03,03,03,03,03,03,03,01,
        02,02,02,02,02,01,00,00,00,00,03,03,03,03,03,03,03,01,
        02,02,02,02,02,02,02,02,00,00,00,00,03,03,03,01,03,01,
        02,02,02,02,02,02,02,01,00,00,00,00,03,03,01,00,01,01,
        00,02,02,02,02,01,01,03,00,00,00,03,03,03,01,00,00,00,
        00,02,02,02,01,03,03,03,03,00,03,03,03,01,00,00,00,00,
        00,00,02,01,00,00,01,03,03,03,03,03,03,01,00,00,00,00,
        00,00,00,00,00,00,00,01,03,03,03,03,01,00,00,00,00,00,
        00,00,00,00,00,00,00,00,01,01,01,01,00,00,00,00,00,00,
        00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00
};

static const UBYTE switchimage_pal[] =
{
        0xb3,0xb3,0xb3,0x00,0x00,0x00,
        0xdb,0x20,0x26,0x00,0x76,0x00
};

const struct CoolImage cool_switchimage =
{
        switchimage_data,
        switchimage_pal,
        SWITCHIMAGE_WIDTH,
        SWITCHIMAGE_HEIGHT,
        SWITCHIMAGE_DEPTH
};

#endif /* COOL_SWITCHIMAGE */

/**************************************************************************/

#if COOL_MONITORIMAGE

#define MONITORIMAGE_WIDTH 15
#define MONITORIMAGE_HEIGHT 16
#define MONITORIMAGE_DEPTH 8

static const UBYTE monitorimage_data[] =
{
        07,07,07,07,07,07,07,07,07,07,07,07,07,07,03,
        07,05,05,05,05,05,05,05,05,05,05,05,05,05,03,
        07,05,06,06,06,06,06,06,06,06,06,06,02,05,03,
        07,05,06,01,04,01,01,01,01,01,01,01,02,05,03,
        07,05,06,04,01,01,01,01,01,01,01,01,02,05,03,
        07,05,06,01,01,01,01,01,01,01,01,01,02,05,03,
        07,05,06,01,01,01,01,01,01,01,01,01,02,05,03,
        07,05,06,01,01,01,01,01,01,01,01,01,02,05,03,
        07,05,06,01,01,01,01,01,01,01,01,01,02,05,03,
        07,05,06,02,02,02,02,02,02,02,02,02,02,05,03,
        07,05,05,05,05,05,05,05,05,05,05,05,05,05,03,
        07,05,05,05,05,05,05,05,05,05,03,05,03,05,03,
        03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,
        00,00,07,07,07,07,07,07,07,07,07,07,03,00,00,
        00,00,03,03,03,03,03,03,03,03,03,03,03,00,00,
        00,00,00,00,00,00,00,00,00,00,00,00,00,00,00
};

static const UBYTE monitorimage_pal[] =
{
        0xb3,0xb3,0xb3,0x28,0x14,0x88,
        0x3b,0x3b,0xe0,0x00,0x00,0x00,
        0xff,0xff,0xff,0xc4,0xc4,0xc4,
        0x00,0x00,0x63,0xe0,0xe0,0xe0
};

const struct CoolImage cool_monitorimage =
{
        monitorimage_data,
        monitorimage_pal,
        MONITORIMAGE_WIDTH,
        MONITORIMAGE_HEIGHT,
        MONITORIMAGE_DEPTH
};

#endif /* COOL_MONITORIMAGE */

/**************************************************************************/

#if COOL_INFOIMAGE

#define INFOIMAGE_WIDTH 16
#define INFOIMAGE_HEIGHT 16
#define INFOIMAGE_DEPTH 4

static const UBYTE infoimage_data[] =
{
        00,00,00,00,00,03,03,03,03,03,00,00,00,00,00,00,
        00,00,00,03,03,03,03,03,03,03,03,03,00,00,00,00,
        00,00,03,03,03,03,03,03,03,03,03,03,03,00,00,00,
        00,03,03,03,03,03,02,02,02,03,03,03,03,03,00,00,
        00,03,03,03,03,03,02,02,02,03,03,03,03,03,01,00,
        03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,00,
        03,03,03,03,03,02,02,02,02,03,03,03,03,03,03,01,
        03,03,03,03,03,03,02,02,02,03,03,03,03,03,03,01,
        03,03,03,03,03,03,02,02,02,03,03,03,03,03,03,01,
        03,03,03,03,03,03,02,02,02,03,03,03,03,03,03,01,
        00,03,03,03,03,03,02,02,02,03,03,03,03,03,01,01,
        00,03,03,03,03,02,02,02,02,02,03,03,03,03,01,00,
        00,00,03,03,03,03,03,03,03,03,03,03,03,01,01,00,
        00,00,00,03,03,03,03,03,03,03,03,03,01,01,00,00,
        00,00,00,00,01,03,03,03,03,03,01,01,01,00,00,00,
        00,00,00,00,00,00,01,01,01,01,01,00,00,00,00,00
};

static const UBYTE infoimage_pal[] =
{
        0xb3,0xb3,0xb3,0x00,0x00,0x00,
        0xe0,0xe0,0xe0,0x00,0x76,0x00
};

const struct CoolImage cool_infoimage =
{
        infoimage_data,
        infoimage_pal,
        INFOIMAGE_WIDTH,
        INFOIMAGE_HEIGHT,
        INFOIMAGE_DEPTH
};

#endif /* COOL_INFOIMAGE */

/**************************************************************************/

#if COOL_ASKIMAGE

#define ASKIMAGE_WIDTH 16
#define ASKIMAGE_HEIGHT 16
#define ASKIMAGE_DEPTH 4

static const UBYTE askimage_data[] =
{
        00,00,00,00,00,03,03,03,03,03,00,00,00,00,00,00,
        00,00,00,03,03,03,03,03,03,03,03,03,00,00,00,00,
        00,00,03,03,03,03,03,03,03,03,03,03,03,00,00,00,
        00,03,03,03,03,02,02,02,02,02,03,03,03,03,00,00,
        00,03,03,03,02,02,02,02,02,02,02,03,03,03,01,00,
        03,03,03,03,02,02,03,03,02,02,02,03,03,03,03,00,
        03,03,03,03,03,03,03,03,02,02,02,03,03,03,03,01,
        03,03,03,03,03,03,03,02,02,02,03,03,03,03,03,01,
        03,03,03,03,03,03,02,02,02,03,03,03,03,03,03,01,
        03,03,03,03,03,03,02,02,02,03,03,03,03,03,03,01,
        00,03,03,03,03,03,03,03,03,03,03,03,03,03,01,01,
        00,03,03,03,03,03,02,02,02,03,03,03,03,03,01,00,
        00,00,03,03,03,03,02,02,02,03,03,03,03,01,01,00,
        00,00,00,03,03,03,03,03,03,03,03,03,01,01,00,00,
        00,00,00,00,01,03,03,03,03,03,01,01,01,00,00,00,
        00,00,00,00,00,00,01,01,01,01,01,00,00,00,00,00
};

static const UBYTE askimage_pal[] =
{
        0xb3,0xb3,0xb3,0x00,0x00,0x00,
        0xe0,0xe0,0xe0,0xc8,0x00,0x00
};

const struct CoolImage cool_askimage =
{
        askimage_data,
        askimage_pal,
        ASKIMAGE_WIDTH,
        ASKIMAGE_HEIGHT,
        ASKIMAGE_DEPTH
};

#endif /* COOL_ASKIMAGE */

/**************************************************************************/

#if COOL_KEYIMAGE

#define KEYIMAGE_WIDTH 17
#define KEYIMAGE_HEIGHT 14
#define KEYIMAGE_DEPTH 8

static const UBYTE keyimage_data[] =
{
        00,04,04,04,04,04,04,04,04,04,04,04,04,04,04,04,00,
        04,02,02,02,02,02,02,02,02,02,02,02,02,02,02,01,04,
        04,02,02,04,04,04,04,04,04,04,04,04,03,03,04,01,04,
        04,02,04,03,03,03,03,03,03,03,03,03,03,03,04,01,04,
        04,02,04,03,03,03,03,03,03,03,03,03,03,03,04,01,04,
        04,02,04,03,03,03,03,03,03,03,03,03,03,03,04,01,04,
        04,02,04,03,03,03,03,03,03,03,03,03,03,03,04,01,04,
        04,02,04,03,03,03,03,03,03,03,03,03,03,03,04,01,04,
        04,02,04,03,03,03,03,03,03,03,03,03,03,03,04,01,04,
        04,02,04,03,03,03,03,03,03,03,03,03,03,03,04,01,04,
        04,02,03,03,03,03,03,03,03,03,03,03,03,03,04,01,04,
        04,02,03,04,04,04,04,04,04,04,04,04,04,04,04,01,04,
        04,02,01,01,01,01,01,01,01,01,01,01,01,01,01,01,04,
        00,04,04,04,04,04,04,04,04,04,04,04,04,04,04,04,00
};

static const UBYTE keyimage_pal[] =
{
        0xb3,0xb3,0xb3,0x38,0x38,0x38,
        0xcc,0xcc,0xcc,0xbb,0xbb,0xbb,
        0x4d,0x4d,0x4d,0x55,0xdd,0x55,
        0xee,0x99,0x00,0x00,0x44,0xdd
};

const struct CoolImage cool_keyimage =
{
        keyimage_data,
        keyimage_pal,
        KEYIMAGE_WIDTH,
        KEYIMAGE_HEIGHT,
        KEYIMAGE_DEPTH
};

#endif /* COOL_KEYIMAGE */

/**************************************************************************/

#endif /* USE_SHARED_COOLIMAGES */

/**************************************************************************/
