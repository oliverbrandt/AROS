/*
    Copyright � 2013, The AROS Development Team. All rights reserved.
    $Id$
*/

#define VCTAG_REQ	0x00000000
#define VCTAG_RESP	0x80000000

#define VCTAG_GETFWREV  0x00000001
#define VCTAG_GETMAC    0x00010003
#define VCTAG_GETRAM    0x00010005
#define VCTAG_GETPOWER  0x00020001
#define VCTAG_SETPOWER	0x00028001
#define VCTAG_GETCLKSPD 0x00030002

#define VCTAG_FBALLOC	0x00040001
#define VCTAG_FBFREE    0x00048001
#define VCTAG_GETRES	0x00040003
#define VCTAG_SETRES    0x00048003
#define VCTAG_SCRBLANK  0x00040002
#define VCTAG_GETVRES   0x00040004
#define VCTAG_SETVRES   0x00048004
#define VCTAG_GETDEPTH  0x00040005
#define VCTAG_SETDEPTH  0x00048005
#define VCTAG_GETRGB    0x00044006
#define VCTAG_SETRGB    0x00048006
#define VCTAG_GETPITCH  0x00040008
