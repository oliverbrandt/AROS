/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <string.h>

#include <hidd/graphics.h>
#include "graphics_intern.h"
#include <aros/machine.h>

#define DEBUG 0
#include <aros/debug.h>

/****************************************************************************************/

VOID bitmap_fillmemrect8(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_FillMemRect8 *msg)
{
    UBYTE *start;
    LONG phase, width, height, w, p;
    ULONG fill32;
    ULONG start_add;
    
    start = msg->dstBuf + msg->minY * msg->dstMod + msg->minX;
    width = msg->maxX - msg->minX + 1;
    height = msg->maxY - msg->minY + 1;
    start_add = msg->dstMod - width;
        
    if ((phase = (IPTR)start & 3L))
    {
    	phase = 4 - phase;
	if (phase > width) phase = width;
	width -= phase;
    }
    
    fill32 = msg->fill;
    fill32 |= (fill32 << 8);
    fill32 |= (fill32 << 16);
    
    while(height--)
    {
    	w = width;
	p = phase;
	
	while(p--)
	{
	    *start++ = (UBYTE)fill32;
	}
	while(w >= 4)
	{
	    *((ULONG *)start)++ = fill32;
	    w -= 4;
	}
	while(w--)
	{
	    *start++ = (UBYTE)fill32;
	}
	start += start_add;
    }
    
};

/****************************************************************************************/

VOID bitmap_fillmemrect16(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_FillMemRect16 *msg)
{
    UBYTE *start;
    LONG phase, width, height, w, p;
    ULONG fill32;
    ULONG start_add;
    
    start = msg->dstBuf + msg->minY * msg->dstMod + msg->minX * 2;
    width = msg->maxX - msg->minX + 1;
    height = msg->maxY - msg->minY + 1;
    start_add = msg->dstMod - width * 2;
        
    if ((phase = (IPTR)start & 1L))
    {
    	phase = 2 - phase;
	if (phase > width) phase = width;
	width -= phase;
    }
    
    fill32 = msg->fill;
    fill32 |= (fill32 << 16);
    
    while(height--)
    {
    	w = width;
	p = phase;
	
	while(p--)
	{
	    *((UWORD *)start)++ = (UWORD)fill32;
	}
	while(w >= 2)
	{
	    *((ULONG *)start)++ = fill32;
	    w -= 2;
	}
	while(w--)
	{
	    *((UWORD *)start)++ = (UWORD)fill32;
	}
	start += start_add;
    }
    
}

/****************************************************************************************/

VOID bitmap_fillmemrect32(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_FillMemRect32 *msg)
{
    UBYTE *start;
    LONG width, height, w;
    ULONG fill32;
    ULONG start_add;
    
    start = msg->dstBuf + msg->minY * msg->dstMod + msg->minX * 4;
    width = msg->maxX - msg->minX + 1;
    height = msg->maxY - msg->minY + 1;
    start_add = msg->dstMod - width * 4;
        
    fill32 = msg->fill;
        
    while(height--)
    {
    	w = width;
	
	while(w--)
	{
	    *((ULONG *)start)++ = fill32;
	}

	start += start_add;
    }
}

/****************************************************************************************/

VOID bitmap_invertmemrect(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_InvertMemRect *msg)
{
    UBYTE *start;
    LONG phase, width, height, w, p;
    ULONG start_add;
    
    start = msg->dstBuf + msg->minY * msg->dstMod + msg->minX;
    width = msg->maxX - msg->minX + 1;
    height = msg->maxY - msg->minY + 1;
    start_add = msg->dstMod - width;
        
    if ((phase = (IPTR)start & 3L))
    {
    	phase = 4 - phase;
	if (phase > width) phase = width;
	width -= phase;
    }
    
    while(height--)
    {
    	UBYTE bg;
    	ULONG bg32;
	
    	w = width;
	p = phase;
	
	while(p--)
	{
	    bg = *start;
	    *start++ = ~bg;
	}
	while(w >= 4)
	{
	    bg32 = *(ULONG *)start;
	    *((ULONG *)start)++ = ~bg32;
	    w -= 4;
	}
	while(w--)
	{
	    bg = *start;
	    *start++ = ~bg;
	}
	start += start_add;
    }
        
}

/****************************************************************************************/

VOID bitmap_copymembox8(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyMemBox8 *msg)
{
    UBYTE *src_start, *dst_start;
    LONG phase, width, height, w, p;
    ULONG src_start_add, dst_start_add;
    BOOL descending;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX;
    src_start_add = msg->srcMod - width;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX;
    dst_start_add = msg->dstMod - width;
        
    if ((IPTR)src_start > (IPTR)dst_start)
    {
	if ((phase = (IPTR)src_start & 3L))
	{
    	    phase = 4 - phase;
	    if (phase > width) phase = width;
	    width -= phase;
	}
    	descending = FALSE;
    }
    else
    {
    	src_start += (height - 1) * msg->srcMod + width;
	dst_start += (height - 1) * msg->dstMod + width;
	
	phase = ((IPTR)src_start & 3L);
	if (phase > width) phase = width;
	width -= phase;
	
	descending = TRUE;
    }
 
    /* NOTE: This can write LONGs to odd addresses, which might not work
       on some CPUs (MC68000) */

    if (!descending)
    {
	while(height--)
	{
    	    w = width;
	    p = phase;

	    while(p--)
	    {
		*dst_start++ = *src_start++;
	    }
	    while(w >= 4)
	    {
		*((ULONG *)dst_start)++ = *((ULONG *)src_start)++;
		w -= 4;
	    }
	    while(w--)
	    {
		*dst_start++ = *src_start++;
	    }
	    src_start += src_start_add;
	    dst_start += dst_start_add;
	}
    }
    else
    {
	while(height--)
	{
    	    w = width;
	    p = phase;

	    while(p--)
	    {
		*--dst_start = *--src_start;
	    }
	    while(w >= 4)
	    {
		*--((ULONG *)dst_start) = *--((ULONG *)src_start);
		w -= 4;
	    }
	    while(w--)
	    {
		*--dst_start = *--src_start;
	    }
	    src_start -= src_start_add;
	    dst_start -= dst_start_add;
	}
     }
    
}

/****************************************************************************************/

VOID bitmap_copymembox16(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyMemBox16 *msg)
{
    UBYTE *src_start, *dst_start;
    LONG phase, width, height, w, p;
    ULONG src_start_add, dst_start_add;
    BOOL descending;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX * 2;
    src_start_add = msg->srcMod - width * 2;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 2;
    dst_start_add = msg->dstMod - width * 2;
        
    if ((IPTR)src_start > (IPTR)dst_start)
    {
	if ((phase = (IPTR)src_start & 1L))
	{
    	    phase = 2 - phase;
	    if (phase > width) phase = width;
	    width -= phase;
	}
    	descending = FALSE;
    }
    else
    {
    	src_start += (height - 1) * msg->srcMod + width * 2;
	dst_start += (height - 1) * msg->dstMod + width * 2;
	
	phase = ((IPTR)src_start & 1L);
	if (phase > width) phase = width;
	width -= phase;
	
	descending = TRUE;
    }
 
    if (!descending)
    {
	while(height--)
	{
    	    w = width;
	    p = phase;

	    while(p--)
	    {
		*((UWORD *)dst_start)++ = *((UWORD *)src_start)++;
	    }
	    while(w >= 2)
	    {
		*((ULONG *)dst_start)++ = *((ULONG *)src_start)++;
		w -= 2;
	    }
	    while(w--)
	    {
		*((UWORD *)dst_start)++ = *((UWORD *)src_start)++;
	    }
	    src_start += src_start_add;
	    dst_start += dst_start_add;
	}
    }
    else
    {
	while(height--)
	{
    	    w = width;
	    p = phase;

	    while(p--)
	    {
		*--((UWORD *)dst_start) = *--((UWORD *)src_start);
	    }
	    while(w >= 2)
	    {
		*--((ULONG *)dst_start) = *--((ULONG *)src_start);
		w -= 2;
	    }
	    while(w--)
	    {
		*--((UWORD *)dst_start) = *--((UWORD *)src_start);
	    }
	    src_start -= src_start_add;
	    dst_start -= dst_start_add;
	}
     }
    
}

/****************************************************************************************/

VOID bitmap_copymembox32(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyMemBox32 *msg)
{
    UBYTE *src_start, *dst_start;
    LONG width, height, w;
    ULONG src_start_add, dst_start_add;
    BOOL descending;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX * 4;
    src_start_add = msg->srcMod - width * 4;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 4;
    dst_start_add = msg->dstMod - width * 4;
        
    if ((IPTR)src_start > (IPTR)dst_start)
    {
    	descending = FALSE;
    }
    else
    {
    	src_start += (height - 1) * msg->srcMod + width * 4;
	dst_start += (height - 1) * msg->dstMod + width * 4;
	
	descending = TRUE;
    }
 
    if (!descending)
    {
	while(height--)
	{
    	    w = width;

	    while(w--)
	    {
		*((ULONG *)dst_start)++ = *((ULONG *)src_start)++;
	    }

	    src_start += src_start_add;
	    dst_start += dst_start_add;
	}
    }
    else
    {
	while(height--)
	{
    	    w = width;

	    while(w--)
	    {
		*--((ULONG *)dst_start) = *--((ULONG *)src_start);
	    }
	    
	    src_start -= src_start_add;
	    dst_start -= dst_start_add;
	}
     }
    
}

/****************************************************************************************/

VOID bitmap_copylutmembox16(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyLUTMemBox16 *msg)
{
    HIDDT_Pixel *pixlut = msg->pixlut->pixels;
    UBYTE *src_start, *dst_start;
    LONG width, height, w;
    ULONG src_start_add, dst_start_add;
    
    if (!pixlut) return;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX;
    src_start_add = msg->srcMod - width;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 2;
    dst_start_add = msg->dstMod - width * 2;
        
    while(height--)
    {
    	w = width;

	while(w--)
	{
	    *((UWORD *)dst_start)++ = (UWORD)(pixlut[*((UBYTE *)src_start)++]);
	}
	src_start += src_start_add;
	dst_start += dst_start_add;
    }
}

/****************************************************************************************/

VOID bitmap_copylutmembox32(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyLUTMemBox32 *msg)
{
    HIDDT_Pixel *pixlut = msg->pixlut->pixels;
    UBYTE *src_start, *dst_start;
    LONG width, height, w;
    ULONG src_start_add, dst_start_add;

    if (!pixlut) return;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX;
    src_start_add = msg->srcMod - width;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 4;
    dst_start_add = msg->dstMod - width * 4;
        
    while(height--)
    {
    	w = width;

	while(w--)
	{
	    *((ULONG *)dst_start)++ = (ULONG)(pixlut[*((UBYTE *)src_start)++]);
	}
	src_start += src_start_add;
	dst_start += dst_start_add;
    }
}

/****************************************************************************************/
