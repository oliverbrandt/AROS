/*
    Copyright � 1995-2002, The AROS Development Team. All rights reserved.
    $Id$
 
    Desc: vesa "hardware" functions
    Lang: English
*/


#define DEBUG 1 /* no SysBase */
#include <aros/asmcall.h>
#include <aros/debug.h>
#include <aros/macros.h>
#include <aros/bootloader.h>
#include <proto/bootloader.h>
#include <proto/oop.h>
#include <utility/hooks.h>
#include <utility/tagitem.h>
#include <oop/oop.h>
#include <hidd/pci.h>

#include "bitmap.h"
#include "vesagfxclass.h"
#include "hardware.h"

#include <string.h>

#undef SysBase
extern struct ExecBase *SysBase;
OOP_AttrBase HiddPCIDeviceAttrBase;
static void Find_PCI_Card(struct HWData *sd);

BOOL initVesaGfxHW(struct HWData *data)
{
    struct BootLoaderBase *BootLoaderBase;
    struct VesaInfo *vi;

    if ((BootLoaderBase = OpenResource("bootloader.resource")))
    {
	D(bug("[Vesa] Init: Bootloader.resource opened\n"));
	if ((vi = (struct VesaInfo *)GetBootInfo(BL_Video)))
	{
	    D(bug("[Vesa] Init: Got Vesa structure from resource\n"));
	    if (vi->ModeNumber == 3)
	    {
		D(bug("[Vesa] Init: Textmode was specified. Aborting\n"));
		return FALSE;
	    }
	    data->width = vi->XSize; data->height = vi->YSize;
	    data->bitsperpixel = data->depth = vi->BitsPerPixel;
	    data->bytesperline = vi->BytesPerLine;
	    data->redmask = vi->Masks[VI_Red];
	    data->greenmask = vi->Masks[VI_Green];
	    data->bluemask = vi->Masks[VI_Blue];
	    data->redshift = vi->Shifts[VI_Red];
	    data->greenshift = vi->Shifts[VI_Green];
	    data->blueshift = vi->Shifts[VI_Blue];
	    data->framebuffer = vi->FrameBuffer;

	    if (!data->framebuffer)
		Find_PCI_Card(data);
	    if (!data->framebuffer) {
		D(bug("[Vesa] HwInit: Framebuffer not found, your card is not PCI\n"));
		return FALSE;
	    }
	    
	    if (data->depth > 24)
	    {
	    	data->bytesperpixel = 4;
	    }
	    else if (data->depth > 16)
	    {
	    	data->bytesperpixel = 3;
	    }
	    else if (data->depth > 8)
	    {
	    	data->bytesperpixel = 2;
	    }
	    else
	    {
	    	data->bytesperpixel = 1;
	    }
	    
	    D(bug("[Vesa] HwInit: Clearing framebuffer at 0x%08x size %d KB\n",data->framebuffer, vi->FrameBufferSize));
	    memset(data->framebuffer, 0, vi->FrameBufferSize * 1024);
	    D(bug("[Vesa] HwInit: Linear framebuffer at 0x%08x\n",data->framebuffer));
	    D(bug("[Vesa] HwInit: Screenmode %dx%dx%d\n",data->width,data->height,data->depth));
	    D(bug("[Vesa] HwInit: Masks R %08x<<%2d G %08x<<%2d B %08x<<%2d\n",
			data->redmask, data->redshift,
			data->greenmask, data->greenshift,
			data->bluemask, data->blueshift));
	    D(bug("[vesa] HwInit: BytesPerPixel %d\n", data->bytesperpixel));
	    return TRUE;
	}
    }

    bug("[Vesa] HwInit: No Vesa information from the bootloader. Failing\n");
    return FALSE;
}


#if BUFFERED_VRAM
void vesaRefreshArea(struct BitmapData *data, LONG x1, LONG y1, LONG x2, LONG y2)
{
    UBYTE *src, *dst;
    ULONG srcmod, dstmod;
    LONG x, y, w, h;

    x1 *= data->bytesperpix;
    x2 *= data->bytesperpix; x2 += data->bytesperpix - 1;
    
    x1 &= ~3;
    x2 = (x2 & ~3) + 3;
    w = (x2 - x1) + 1;
    h = (y2 - y1) + 1;
    
    srcmod = (data->bytesperline - w);
    dstmod = (data->data->bytesperline - w);
   
    src = data->VideoData + y1 * data->bytesperline + x1;
    dst = data->data->framebuffer + y1 * data->data->bytesperline + x1;
    
    for(y = 0; y < h; y++)
    {
    	for(x = 0; x < w / 4; x++)
	{
	    *((ULONG *)dst) = *((ULONG *)src);
	    dst += sizeof(ULONG);
	    src += sizeof(ULONG);
	}
	src += srcmod;
	dst += dstmod;
    }
    
}
#endif

AROS_UFH3(void, Enumerator,
    AROS_UFHA(struct Hook *,	hook,	    A0),
    AROS_UFHA(OOP_Object *,	pciDevice,  A2),
    AROS_UFHA(APTR,		message,    A1))
{
    AROS_USERFUNC_INIT

    APTR buf;
    ULONG size;
    OOP_Object *driver;
    struct pHidd_PCIDriver_MapPCI mappci,*msg = &mappci;
    struct HWData *sd = hook->h_Data;

    D(bug("[VESA] Enumerator: Found deivce\n"));

    OOP_GetAttr(pciDevice, aHidd_PCIDevice_Driver, (APTR)&driver);
    OOP_GetAttr(pciDevice, aHidd_PCIDevice_Base0, (APTR)&buf);
    OOP_GetAttr(pciDevice, aHidd_PCIDevice_Size0, (APTR)&size);

    mappci.mID = OOP_GetMethodID(IID_Hidd_PCIDriver, moHidd_PCIDriver_MapPCI);
    mappci.PCIAddress = buf;
    mappci.Length = size;
    sd->framebuffer = (APTR)OOP_DoMethod(driver, (OOP_Msg)msg);

    D(bug("[VESA] Got framebuffer @ %x (size=%x)\n", sd->framebuffer, size));

    AROS_USERFUNC_EXIT
}

#undef sd
static void Find_PCI_Card(struct HWData *sd)
{
    OOP_Object *pci;

    D(bug("[VESA] Find_PCI_Card\n"));

    if (HiddPCIDeviceAttrBase)
    {
	pci = OOP_NewObject(NULL, CLID_Hidd_PCI, NULL);
	
	D(bug("[VESA] Creating PCI object\n"));

	if (pci)
	{
	    struct Hook FindHook = {
		h_Entry:    (IPTR (*)())Enumerator,
		h_Data:	    sd,
	    };

	    struct TagItem Requirements[] = {
		{ tHidd_PCI_Interface,	0x00 },
		{ tHidd_PCI_Class,	0x03 },
		{ tHidd_PCI_SubClass,	0x00 },
		{ TAG_DONE, 0UL }
	    };
	
	    struct pHidd_PCI_EnumDevices enummsg = {
		mID:		OOP_GetMethodID(IID_Hidd_PCI, moHidd_PCI_EnumDevices),
		callback:	&FindHook,
		requirements:	(struct TagItem*)&Requirements,
	    }, *msg = &enummsg;
	    D(bug("[VESA] Calling search Hook\n"));
	    OOP_DoMethod(pci, (OOP_Msg)msg);
	    OOP_DisposeObject(pci);
	}
    }
}
