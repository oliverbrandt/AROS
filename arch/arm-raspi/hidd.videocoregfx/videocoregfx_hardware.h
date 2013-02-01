#ifndef _VIDEOCOREGFX_HARDWARE_H
#define _VIDEOCOREGFX_HARDWARE_H

#include <exec/libraries.h>
#include <hidd/pci.h>
#include <oop/oop.h>

#include <asm/bcm2835.h>
#include <hardware/videocore.h>

#include "videocoregfx_class.h"
#include "videocoregfx_bitmap.h"
#include "videocoregfx_mouse.h"

#define VCMB_FBCHAN     8

struct HWData  {
	UWORD indexReg;
	UWORD valueReg;
	ULONG depth;
	ULONG redmask;
	ULONG greenmask;
	ULONG bluemask;
	ULONG redshift;
	ULONG greenshift;
	ULONG blueshift;
	ULONG bytesperpixel;
	ULONG bitsperpixel;
	ULONG bytesperline;
	ULONG vramsize;
	APTR vrambase;
	ULONG maxwidth;
	ULONG maxheight;
        ULONG displaycount;
	ULONG mmiosize;
	APTR mmiobase;
	ULONG fboffset; /* last byte in framebuffer of current screen mode */
	ULONG pseudocolor;
	ULONG capabilities;
};

#define VCDATA(x) ((struct HWData *)x->data)

BOOL initVideoCoreGfxHW(APTR);

#endif /* _VIDEOCOREGFX_HARDWARE_H */
