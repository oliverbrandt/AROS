/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Driver for using gfxhidd for gfx output
    Lang: english
*/

#define AROS_ALMOST_COMPATIBLE 1


#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/arossupport.h>
#include <proto/utility.h>
#include <proto/oop.h>

#include <exec/memory.h>
#include <exec/semaphores.h>
#include <clib/macros.h>

#include <graphics/rastport.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>
#include <graphics/view.h>
#include <graphics/layers.h>
#include <graphics/clip.h>
#include <graphics/gfxmacros.h>
#include <graphics/regions.h>
#include <graphics/scale.h>

#include <oop/oop.h>
#include <utility/tagitem.h>
#include <aros/asmcall.h>

#include <intuition/intuition.h>

#include <hidd/graphics.h>

#include <cybergraphx/cybergraphics.h>

#include <stdio.h>
#include <string.h>

#include "graphics_intern.h"
#include "graphics_internal.h"
#include "intregions.h"
#include "dispinfo.h"
#include "gfxfuncsupport.h"

#define SDEBUG 0
#define DEBUG 0
#include <aros/debug.h>

static OOP_Object *fontbm_to_hiddbm(struct TextFont *font, struct GfxBase *GfxBase);
	





static BOOL init_cursor(struct GfxBase *GfxBase);
static VOID cleanup_cursor(struct GfxBase *GfxBase);

struct pix_render_data {
    HIDDT_Pixel pixel;
};

static LONG pix_write(APTR pr_data
	, OOP_Object *bm, OOP_Object *gc
	, LONG x, LONG y
	, struct GfxBase *GfxBase);

#if 0
OOP_AttrBase HiddBitMapAttrBase	= 0;
OOP_AttrBase HiddGCAttrBase		= 0;
OOP_AttrBase HiddSyncAttrBase	= 0;
OOP_AttrBase HiddPixFmtAttrBase	= 0;
OOP_AttrBase HiddPlanarBMAttrBase	= 0; 
OOP_AttrBase HiddGfxAttrBase	= 0; 
OOP_AttrBase HiddFakeGfxHiddAttrBase	= 0;

static struct OOP_ABDescr attrbases[] = {
    { IID_Hidd_BitMap,		&HiddBitMapAttrBase	},
    { IID_Hidd_GC,		&HiddGCAttrBase		},
    { IID_Hidd_Sync,		&HiddSyncAttrBase	},
    { IID_Hidd_PixFmt,		&HiddPixFmtAttrBase	},
    { IID_Hidd_PlanarBM,	&HiddPlanarBMAttrBase	},
    { IID_Hidd_Gfx,		&HiddGfxAttrBase	},
    { IID_Hidd_FakeGfxHidd,	&HiddFakeGfxHiddAttrBase	},
    { NULL, 0UL }
};

#endif


struct ETextFont
{
    struct TextFont	etf_Font;
};


/* InitDriverData() just allocates memory for the struct. To use e.g.   */
/* AreaPtrns, UpdateAreaPtrn() has to allocate the memory for the       */
/* Pattern itself (and free previously used memory!)                    */

struct gfx_driverdata * InitDriverData (struct RastPort * rp, struct GfxBase * GfxBase)
{
    struct gfx_driverdata * dd;
    EnterFunc(bug("InitDriverData(rp=%p)\n", rp));

    /* Does this rastport have a bitmap ? */
    if (rp->BitMap)
    {
        D(bug("Got bitmap\n"));
        /* Displayable ? (== rastport of a screen) */
	//if (IS_HIDD_BM(rp->BitMap))
	{
            D(bug("Has HIDD bitmap (displayable)\n"));

	    /* We can use this rastport. Allocate driverdata */
    	    dd = AllocMem (sizeof(struct gfx_driverdata), MEMF_CLEAR);
    	    if (dd)
    	    {
	        struct shared_driverdata *sdd;
		struct TagItem gc_tags[] = {
		    { TAG_DONE, 	0UL}
		};
		
		
		D(bug("Got driverdata\n"));
		sdd = SDD(GfxBase);
		
		dd->dd_GC = HIDD_Gfx_NewGC(sdd->gfxhidd, gc_tags);
		if (dd->dd_GC)
		{

		    D(bug("Got GC HIDD object\n"));
    		    dd->dd_RastPort = rp;
    		    SetDriverData(rp, dd);
    		    rp->Flags |= RPF_DRIVER_INITED;

		    ReturnPtr("InitDriverData", struct gfx_driverdata *, dd);
	        }

		FreeMem(dd, sizeof (struct gfx_driverdata));
	
    	    }
	}
    }

    ReturnPtr("InitDriverData", struct gfx_driverdata *, NULL);
}

void DeinitDriverData (struct RastPort * rp, struct GfxBase * GfxBase)
{
    struct gfx_driverdata * dd;
    struct shared_driverdata *sdd;
    
    EnterFunc(bug("DeInitDriverData(rp=%p)\n", rp));
		
    sdd = SDD(GfxBase);

    dd = (struct gfx_driverdata *) rp->longreserved[0];
    rp->longreserved[0] = 0;

    HIDD_Gfx_DisposeGC(sdd->gfxhidd, dd->dd_GC);

    FreeMem (dd, sizeof(struct gfx_driverdata));
    rp->Flags &= ~RPF_DRIVER_INITED;

    ReturnVoid("DeinitDriverData");
}

BOOL CorrectDriverData (struct RastPort * rp, struct GfxBase * GfxBase)
{
    BOOL retval = TRUE;
    struct gfx_driverdata * dd, * old;

    if (!rp)
    {
	retval = FALSE;
    }
    else
    {
	old = GetDriverData(rp);
	if (!old)
	{
	    old = InitDriverData(rp, GfxBase);

/* stegerg: ???? would have returned TRUE even if old == NULL
	    if (old)
	    	retval = TRUE;
*/
	    if (!old) retval = FALSE;
	}
	else if (rp != old->dd_RastPort)
	{
	    /* stegerg: cloned rastport?	    
	    ** Make sure to clear driverdata pointer and flag
	    ** in case InitDriverData fail
	    */
	    rp->longreserved[0] = 0;
	    rp->Flags &= ~RPF_DRIVER_INITED;

	    dd = InitDriverData(rp, GfxBase);

/* stegerg: ???? would have returned TRUE even if dd = NULL
	    if (dd)
	   	 retval = TRUE;
*/

	    if (!dd) retval = FALSE;

	}
    }
    
    return retval;
}

int driver_init(struct GfxBase * GfxBase)
{

    EnterFunc(bug("driver_init()\n"));
    
    /* Initialize the semaphore used for the chunky buffer */
    InitSemaphore(&(PrivGBase(GfxBase)->pixbuf_sema));
    InitSemaphore(&(PrivGBase(GfxBase)->blit_sema));
    
    /* Init the needed attrbases */
#ifndef CREATE_ROM
    __IHidd_BitMap  	= OOP_ObtainAttrBase(IID_Hidd_BitMap);
    __IHidd_GC      	= OOP_ObtainAttrBase(IID_Hidd_GC);
    __IHidd_Sync    	= OOP_ObtainAttrBase(IID_Hidd_Sync);
    __IHidd_PixFmt  	= OOP_ObtainAttrBase(IID_Hidd_PixFmt);
    __IHidd_PlanarBM 	= OOP_ObtainAttrBase(IID_Hidd_PlanarBM);
    __IHidd_Gfx     	= OOP_ObtainAttrBase(IID_Hidd_Gfx);
    __IHidd_FakeGfxHidd = OOP_ObtainAttrBase(IID_Hidd_FakeGfxHidd);
    

    if (__IHidd_BitMap   &&
        __IHidd_GC       &&
	__IHidd_Sync     &&
	__IHidd_PixFmt   &&
	__IHidd_PlanarBM &&
	__IHidd_Gfx      &&
	__IHidd_FakeGfxHidd)
#else
#warning Activate me again!
#endif
    {
	PrivGBase(GfxBase)->pixel_buf=AllocMem(PIXELBUF_SIZE,MEMF_ANY);
	if (PrivGBase(GfxBase)->pixel_buf) {

	    ReturnInt("driver_init", int, TRUE);

	    FreeMem(PrivGBase(GfxBase)->pixel_buf, PIXELBUF_SIZE);
	}
	
    }
	
    ReturnInt("driver_init", int, FALSE);
}

int driver_open (struct GfxBase * GfxBase)
{
    return TRUE;
}

void driver_close (struct GfxBase * GfxBase)
{
    return;
}

void driver_expunge (struct GfxBase * GfxBase)
{
    
    /* Try to free some other stuff */
    cleanup_cursor(GfxBase);

    if (SDD(GfxBase)->framebuffer)
	OOP_DisposeObject(SDD(GfxBase)->framebuffer);

#if 0
    if (SDD(GfxBase)->activescreen_inited)
	cleanup_activescreen_stuff(GfxBase);
#endif
    if (SDD(GfxBase)->dispinfo_db)
	destroy_dispinfo_db(SDD(GfxBase)->dispinfo_db, GfxBase);

    if ( SDD(GfxBase)->planarbm_cache )
	delete_object_cache( SDD(GfxBase)->planarbm_cache, GfxBase );

    if ( SDD(GfxBase)->gc_cache )
	delete_object_cache( SDD(GfxBase)->gc_cache, GfxBase );

    if ( SDD(GfxBase)->fakegfx_inited )
	cleanup_fakegfxhidd( &SDD(GfxBase)->fakegfx_staticdata, GfxBase);

    if ( SDD(GfxBase)->gfxhidd_orig )
	OOP_DisposeObject( SDD(GfxBase)->gfxhidd_orig );
	     
    return;
}

/* Called after DOS is up & running */
static OOP_Object *create_framebuffer(struct GfxBase *GfxBase)
{
    struct TagItem fbtags[] = {
    	{ aHidd_BitMap_FrameBuffer,	TRUE	},
	{ aHidd_BitMap_ModeID,		0	},
	{ TAG_DONE, 0 }
    };

    HIDDT_ModeID hiddmode;
    OOP_Object *fb = NULL;

    kprintf("-- 3 --\n");
    /* Get the highest available resolution at the best possible depth */
    hiddmode = get_best_resolution_and_depth(GfxBase);
    if (vHidd_ModeID_Invalid == hiddmode) {
    	D(bug("!!! create_framebuffer(): COULD NOT GET HIDD MODEID !!!\n"));
    } else {
    	/* Create the framebuffer object */
	fbtags[1].ti_Data = hiddmode;
	fb = HIDD_Gfx_NewBitMap(SDD(GfxBase)->gfxhidd, fbtags);

        kprintf("--- %p\n", fb);
    }

    return fb;
}
const UBYTE def_pointer_shape[] =
{
    06,02,00,00,00,00,00,00,00,00,00,
    01,06,02,02,00,00,00,00,00,00,00,
    00,01,06,06,02,02,00,00,00,00,00,
    00,01,06,06,06,06,02,02,00,00,00,
    00,00,01,06,06,06,06,06,02,02,00,
    00,00,01,06,06,06,06,06,06,06,00,
    00,00,00,01,06,06,06,02,00,00,00,
    00,00,00,01,06,06,01,06,02,00,00,
    00,00,00,00,01,06,00,01,06,02,00,
    00,00,00,00,01,06,00,00,01,06,02,
    00,00,00,00,00,00,00,00,00,01,06
};

#define DEF_POINTER_WIDTH	11
#define DEF_POINTER_HEIGHT	11
#define DEF_POINTER_DEPTH	7


static BOOL init_cursor(struct GfxBase *GfxBase)
{
    /* Create the pointer bitmap */
    struct TagItem pbmtags[] = {
    	{ aHidd_BitMap_Width,		DEF_POINTER_WIDTH		},
	{ aHidd_BitMap_Height,		DEF_POINTER_HEIGHT		},
	{ aHidd_BitMap_StdPixFmt,	vHidd_StdPixFmt_LUT8		},
	{ TAG_DONE, 0UL }
    };
    SDD(GfxBase)->pointerbm = HIDD_Gfx_NewBitMap(SDD(GfxBase)->gfxhidd, pbmtags);
    if (NULL != SDD(GfxBase)->pointerbm) {
	OOP_Object *gc;
	    
	gc = obtain_cache_object(SDD(GfxBase)->gc_cache, GfxBase);
	if (NULL != gc) {
	    /* Copy the default pointer image into the created pointer bitmap */
	    ULONG i;
	    struct TagItem gc_tags[] = {
		{ aHidd_GC_DrawMode,	vHidd_GC_DrawMode_Copy	},
		{ TAG_DONE, 0UL }
	    };
	    
	    HIDDT_Color col[DEF_POINTER_DEPTH];
	    
	    col[0].red		= 0xFFFF;
	    col[0].green	= 0xFFFF;
	    col[0].blue		= 0xFFFF;
	    col[0].alpha	= 0x0000;

	    for (i = 1; i < DEF_POINTER_DEPTH; i ++) {
	    	col[i].red	= 0x0000;
		col[i].green	= 0x0000;
	    	col[i].blue	= 0x0000;
	    	col[i].alpha	= 0x0000;
	    }
	    
	    HIDD_BM_SetColors(SDD(GfxBase)->pointerbm, col, 0, DEF_POINTER_DEPTH);

	    OOP_SetAttrs(gc, gc_tags);
#if 0
	    /* PutImageLUT not yet implemented in gfx baseclass */	    
	    HIDD_BM_PutImageLUT(SDD(GfxBase)->pointerbm, gc
			, (UBYTE *)def_pointer_shape
			, DEF_POINTER_WIDTH
			, 0, 0
			, DEF_POINTER_WIDTH, DEF_POINTER_HEIGHT
			, &plut
	    );
#else
	    HIDD_BM_PutImage(SDD(GfxBase)->pointerbm, gc
			, (UBYTE *)def_pointer_shape
			, DEF_POINTER_WIDTH
			, 0, 0
			, DEF_POINTER_WIDTH, DEF_POINTER_HEIGHT
			, vHidd_StdPixFmt_LUT8
	    );

#endif		
	    release_cache_object(SDD(GfxBase)->gc_cache, gc, GfxBase);
	    
	    if (HIDD_Gfx_SetCursorShape(SDD(GfxBase)->gfxhidd, SDD(GfxBase)->pointerbm)) {
D(bug("CURSOR SHAPE SET\n"));
		/* Make it visible */
		HIDD_Gfx_SetCursorVisible(SDD(GfxBase)->gfxhidd, TRUE);
		
	    	return TRUE;
	    }
	}
    }
    
    cleanup_cursor(GfxBase);

    return FALSE;
}

static VOID cleanup_cursor(struct GfxBase *GfxBase)
{
    if (NULL != SDD(GfxBase)->pointerbm) {
   	OOP_DisposeObject(SDD(GfxBase)->pointerbm);
	SDD(GfxBase)->pointerbm = NULL;
    }
}

BOOL driver_LateGfxInit (APTR data, struct GfxBase *GfxBase)
{

    /* Supplied data is really the librarybase of a HIDD */
    STRPTR gfxhiddname = (STRPTR)data;
    struct TagItem tags[] = {
    	{ TAG_DONE, 0UL },
    };
    EnterFunc(bug("driver_LateGfxInit(gfxhiddname=%s)\n", gfxhiddname));

    /* Create a new GfxHidd object */

    SDD(GfxBase)->gfxhidd = SDD(GfxBase)->gfxhidd_orig = OOP_NewObject(NULL, gfxhiddname, tags);
    D(bug("driver_LateGfxInit: gfxhidd=%p\n", SDD(GfxBase)->gfxhidd));

    if (NULL != SDD(GfxBase)->gfxhidd) {
	IPTR hwcursor;
	BOOL ok = TRUE;

	OOP_GetAttr(SDD(GfxBase)->gfxhidd, aHidd_Gfx_SupportsHWCursor, &hwcursor);
	SDD(GfxBase)->has_hw_cursor = (BOOL)hwcursor;
	if (!hwcursor) {
	    OOP_Object *fgh;
	    D(bug("There's no hardware cursor\n"));

	    fgh = init_fakegfxhidd(SDD(GfxBase)->gfxhidd
	    	, &SDD(GfxBase)->fakegfx_staticdata
		, GfxBase);

	    if (NULL != fgh) {
	    	SDD(GfxBase)->gfxhidd = fgh;
		SDD(GfxBase)->fakegfx_inited = TRUE;
	    } else {
	    	ok = FALSE;
	    }
	}

	if (ok) {
	    struct TagItem gc_create_tags[] = { { TAG_DONE, 0UL } };
	    D(bug("Ok\n"));
	    SDD(GfxBase)->gc_cache = create_object_cache(NULL, IID_Hidd_GC, gc_create_tags, GfxBase);
	    if (NULL != SDD(GfxBase)->gc_cache) {

		struct TagItem bm_create_tags[] = {
#warning Maybe make this class private and create the object through the graphicshidd
			{ aHidd_BitMap_GfxHidd,		(IPTR)SDD(GfxBase)->gfxhidd_orig },
			{ aHidd_BitMap_Displayable,	FALSE	},
			{ aHidd_PlanarBM_AllocPlanes,	FALSE },
			{ TAG_DONE, 0UL }
		};

		D(bug("Cache created\n"));
		SDD(GfxBase)->planarbm_cache = create_object_cache(NULL, CLID_Hidd_PlanarBM, bm_create_tags, GfxBase);

		if (NULL != SDD(GfxBase)->planarbm_cache) {

		    kprintf("-- 1 --\n");
		    /* Move the modes into the displayinfo DB */
		    SDD(GfxBase)->dispinfo_db = build_dispinfo_db(GfxBase);
		    if (NULL != SDD(GfxBase)->dispinfo_db) {

			    kprintf("-- 2 --\n");
			SDD(GfxBase)->framebuffer = create_framebuffer(GfxBase);
			if (NULL != SDD(GfxBase)->framebuffer) {
D(bug("FRAMEBUFFER OK: %p\n", SDD(GfxBase)->framebuffer));
			    if (init_cursor(GfxBase)) {

D(bug("MOUSE INITED\n"));
		            	ReturnBool("driver_LateGfxInit", TRUE);
			    }
			    OOP_DisposeObject(SDD(GfxBase)->framebuffer);
		    	} /* if (framebuffer inited) */
		    	destroy_dispinfo_db(SDD(GfxBase)->dispinfo_db, GfxBase);
		    	SDD(GfxBase)->dispinfo_db = NULL;
		    } /* if (displayinfo db inited) */
		    delete_object_cache(SDD(GfxBase)->planarbm_cache, GfxBase);
	            SDD(GfxBase)->planarbm_cache = NULL;
	    	} /* if (planarbm cache created) */
	    	delete_object_cache(SDD(GfxBase)->gc_cache, GfxBase);
		SDD(GfxBase)->gc_cache = NULL;
	    } /* if (gc object cache ok) */
	    
	} /* if (fake gfx stuff ok) */

	if (SDD(GfxBase)->fakegfx_inited) {
	    cleanup_fakegfxhidd(&SDD(GfxBase)->fakegfx_staticdata, GfxBase);
	    SDD(GfxBase)->fakegfx_inited = FALSE;
	}
	OOP_DisposeObject(SDD(GfxBase)->gfxhidd_orig);
	SDD(GfxBase)->gfxhidd_orig = NULL;
	    
    }
    
    ReturnBool("driver_LateGfxInit", FALSE);

}


#if 0 /* Unused? */

static ULONG getbitmappixel(struct BitMap *bm
	, LONG x
	, LONG y
	, UBYTE depth
	, UBYTE plane_mask)
{
    UBYTE i;
    ULONG idx;

    ULONG mask;
    ULONG pen = 0L;

    idx = COORD_TO_BYTEIDX(x, y, bm->BytesPerRow);
    mask = XCOORD_TO_MASK( x );
    
    for (i = depth - 1; depth; i -- , depth -- )
    {
        pen <<= 1; /* stegerg: moved to here, was inside if!? */

        if ((1L << i) & plane_mask)
	{
	    UBYTE *plane = bm->Planes[i];
	
	    if (plane == (PLANEPTR)-1)
	    {
	        pen |= 1;
	    }
	    else if (plane != NULL)
	    {
		if ((plane[idx] & mask) != 0)
		    pen |= 1;
	    }
	}

    }
    return pen;
}

#endif

struct bitmap_render_data {
    struct render_special_info rsi;
    ULONG minterm;
    struct BitMap *srcbm;
    OOP_Object *srcbm_obj;

};

static ULONG bitmap_render(APTR bitmap_rd, LONG srcx, LONG srcy
	, OOP_Object *dstbm_obj, OOP_Object *dst_gc
	, LONG x1, LONG y1, LONG x2, LONG y2
	, struct GfxBase *GfxBase)
{
    struct bitmap_render_data *brd;
    ULONG width, height;

    width  = x2 - x1 + 1;
    height = y2 - y1 + 1;

    brd = (struct bitmap_render_data *)bitmap_rd;

//bug("bitmap_render(%p, %d, %d, %p, %p, %d, %d, %d, %d, %p)\n"
//	, bitmap_rd, srcx, srcy, dstbm_obj, dst_gc, x1, y1, x2, y2, GfxBase);


    /* Get some info on the colormaps. We have to make sure
       that we have the apropriate mapping tables set.
    */

    if (!int_bltbitmap(brd->srcbm
    	, brd->srcbm_obj
	, srcx, srcy
	, brd->rsi.curbm
	, dstbm_obj
	, x1, y1
	, x2 - x1 + 1, y2 - y1 + 1
	, brd->minterm
	, dst_gc
	, GfxBase
    ))
    	return 0;

   return width * height;
}

void driver_BltBitMapRastPort (struct BitMap   * srcBitMap,
	LONG xSrc, LONG ySrc,
	struct RastPort * destRP,
	LONG xDest, LONG yDest,
	LONG xSize, LONG ySize,
	ULONG minterm, struct GfxBase *GfxBase
)
{
    struct bitmap_render_data brd;
    struct Rectangle rr;
    HIDDT_DrawMode old_drmd;
    OOP_Object *gc;

    Point src;

    struct TagItem gc_tags[] = {
    	{ aHidd_GC_DrawMode,	0UL },
	{ TAG_DONE, 0UL }
    };

    EnterFunc(bug("driver_BltBitMapRastPort(%d %d %d, %d, %d, %d)\n"
    	, xSrc, ySrc, xDest, yDest, xSize, ySize));

    if (!CorrectDriverData(destRP, GfxBase))
    	return;

    brd.minterm	= minterm;
    brd.srcbm_obj = OBTAIN_HIDD_BM(srcBitMap);
    if (NULL == brd.srcbm_obj)
    	return;

    brd.srcbm = srcBitMap;

    gc = GetDriverData(destRP)->dd_GC;
    OOP_GetAttr(gc, aHidd_GC_DrawMode, &old_drmd);

    gc_tags[0].ti_Data = MINTERM_TO_GCDRMD(minterm);
    OOP_SetAttrs(gc, gc_tags);

    rr.MinX = xDest;
    rr.MinY = yDest;
    rr.MaxX = xDest + xSize - 1;
    rr.MaxY = yDest + ySize - 1;

    src.x = xSrc;
    src.y = ySrc;


    do_render_func(destRP, &src, &rr, bitmap_render, &brd, TRUE, GfxBase);

    RELEASE_HIDD_BM(brd.srcbm_obj, srcBitMap);

    gc_tags[0].ti_Data = old_drmd;
    OOP_SetAttrs(gc, gc_tags);


    ReturnVoid("driver_BltBitMapRastPort");
}

#define dumprect(rect) \
        kprintf(#rect " : (%d,%d) - (%d,%d)\n",(rect)->MinX,(rect)->MinY,(rect)->MaxX,(rect)->MaxY)
BOOL driver_MoveRaster (struct RastPort * rp, LONG dx, LONG dy,
	LONG x1, LONG y1, LONG x2, LONG y2, BOOL UpdateDamageList,
	BOOL hasClipRegion,
	struct GfxBase * GfxBase)
{
    struct Layer     *L       = rp->Layer;
    struct Rectangle  ScrollRect;
    struct Rectangle  Rect;

    if (!CorrectDriverData (rp, GfxBase))
	return FALSE;

    if (0 == dx && 0 == dy)
    	return TRUE;

    ScrollRect.MinX = x1;
    ScrollRect.MinY = y1;
    ScrollRect.MaxX = x2;
    ScrollRect.MaxY = y2;

    if (!L)
    {
        Rect = ScrollRect;
	TranslateRect(&Rect, -dx, -dy);
        if (_AndRectRect(&ScrollRect, &Rect, &Rect))
        {
            BltBitMap(rp->BitMap,
                      Rect.MinX + dx,
                      Rect.MinY + dy,
	              rp->BitMap,
                      Rect.MinX,
                      Rect.MinY,
                      Rect.MaxX - Rect.MinX + 1,
                      Rect.MaxY - Rect.MinY + 1,
		      0xc0, /* copy */
                      0xff,
                      NULL );
	}
    }
    else
    {
    	struct ClipRect *CR, *LastHiddenCR;
        struct Region    R;

	LockLayerRom(L);

	TranslateRect(&ScrollRect, L->bounds.MinX, L->bounds.MinY);

	InitRegion(&R);

	if ((L->Flags & LAYERSIMPLE) && UpdateDamageList)
	{
	    if (!SetRegion(L->DamageList, &R))
	        goto failexit;

	    /* The damage list is relative to the layer */
	    TranslateRect(&R.bounds, L->bounds.MinX, L->bounds.MinY);
	}

        #define LayersBase (struct LayersBase *)(GfxBase->gb_LayersBase)
	SortLayerCR(L, dx, dy);
	#undef LayersBase

	for (LastHiddenCR = NULL, CR = L->ClipRect; CR; CR = CR->Next)
    	{
	    CR->_p1 = LastHiddenCR;

	    if (CR->lobs)
	    {
		if (LastHiddenCR)
		    LastHiddenCR->_p2 = CR;

		LastHiddenCR = CR;

		if ((L->Flags & LAYERSIMPLE) && UpdateDamageList)
		{
		    if (!OrRectRegion(&R, &CR->bounds))
		        goto failexit;
		}
	    }

	    CR->_p2 = NULL;
 	}


	if ((L->Flags & LAYERSIMPLE) && UpdateDamageList)
	{
	    TranslateRect(&R.bounds, -dx, -dy);

	    AndRectRegion(&R, &ScrollRect);
	}

	for (CR = L->ClipRect; CR; CR = CR->Next)
    	{
 	    int cando = 0;

	    if (CR->lobs && (L->Flags & LAYERSIMPLE) && UpdateDamageList)
	    {
	        ClearRectRegion(&R, &CR->bounds);
	    }
	    else
	    if (_AndRectRect(&ScrollRect, &CR->bounds, &Rect))
	    {
		TranslateRect(&Rect, -dx, -dy);

		if (_AndRectRect(&ScrollRect, &Rect, &Rect))
		    cando = 1;
	    }

	    if (cando)
	    {
		/* Rect.Min(X|Y) are the coordinates to wich the rectangle has to be moved
		   Rect.Max(X|Y) - Rect.Max(X|Y) - 1 are the dimensions of this rectangle */
		if (!CR->_p1 && !CR->lobs)
		{
		    /* there are no hidden/obscured rectangles with which this recrtangle has to deal */
		    BltBitMap(rp->BitMap,
                              Rect.MinX + dx,
        		      Rect.MinY + dy,
	          	      rp->BitMap,
                    	      Rect.MinX,
                   	      Rect.MinY,
                  	      Rect.MaxX - Rect.MinX + 1,
                  	      Rect.MaxY - Rect.MinY + 1,
			      0xc0, /* copy */
         		      0xff,
                 	      NULL );
		}
		else
		{
		    struct BitMap          *srcbm;
		    struct RegionRectangle *rr;
                    struct Region          *RectRegion;
		    struct Rectangle        Tmp;
		    struct ClipRect        *HiddCR;
		    WORD                    corrsrcx, corrsrcy;
		    BOOL   dosrcsrc;

		    RectRegion = NewRectRegion(Rect.MinX, Rect.MinY, Rect.MaxX, Rect.MaxY);
		    if (!RectRegion)
		        goto failexit;

 		    if (CR->lobs)
		    {
			if (L->Flags & LAYERSUPER)
		        {
   		            corrsrcx = - L->bounds.MinX - L->Scroll_X;
          	            corrsrcy = - L->bounds.MinY - L->Scroll_Y;
		        }
			else
			{
		            corrsrcx = - CR->bounds.MinX + ALIGN_OFFSET(CR->bounds.MinX);
		            corrsrcy = - CR->bounds.MinY;
		        }
			srcbm = CR->BitMap;
		    }
		    else
		    {
		        corrsrcx  = 0;
		        corrsrcy  = 0;
		        srcbm     = rp->BitMap;
		    }

		    for (HiddCR = CR->_p1; HiddCR; HiddCR = HiddCR->_p1)
		    {
			if (_AndRectRect(&RectRegion->bounds, &HiddCR->bounds, &Tmp))
			{

			    if (!(L->Flags & LAYERSIMPLE))
			    {
    			        WORD corrdstx, corrdsty;

				if (L->Flags & LAYERSUPER)
				{
	                            corrdstx =  - L->bounds.MinX - L->Scroll_X;
                        	    corrdsty =  - L->bounds.MinY - L->Scroll_Y;
				}
				else
				{
				    /* Smart layer */
				    corrdstx =  - HiddCR->bounds.MinX + ALIGN_OFFSET(HiddCR->bounds.MinX);
				    corrdsty =  - HiddCR->bounds.MinY;
				}


				BltBitMap(srcbm,
				          Tmp.MinX + corrsrcx + dx,
					  Tmp.MinY + corrsrcy + dy,
					  HiddCR->BitMap,
					  Tmp.MinX + corrdstx,
					  Tmp.MinY + corrdsty,
					  Tmp.MaxX - Tmp.MinX + 1,
                	      	          Tmp.MaxY - Tmp.MinY + 1,
			      	          0xc0, /* copy */
         		      	          0xff,
                 	   	          NULL );
			    }

			    if (!ClearRectRegion(RectRegion, &Tmp))
			    {
			        DisposeRegion(RectRegion);
				goto failexit;
			    }
			}
		    }

		    if ((dosrcsrc = _AndRectRect(&CR->bounds, &Rect, &Tmp)))
		    {
			if (!ClearRectRegion(RectRegion, &Tmp))
			{
			    DisposeRegion(RectRegion);
			    goto failexit;
			}
		    }

		    for (rr = RectRegion->RegionRectangle; rr; rr = rr->Next)
		    {

			BltBitMap(srcbm,
			          rr->bounds.MinX + RectRegion->bounds.MinX + corrsrcx + dx,
                	          rr->bounds.MinY + RectRegion->bounds.MinY + corrsrcy + dy,
	          	          rp->BitMap,
                	  	  rr->bounds.MinX + RectRegion->bounds.MinX,
          			  rr->bounds.MinY + RectRegion->bounds.MinY,
                		  rr->bounds.MaxX - rr->bounds.MinX + 1,
                	  	  rr->bounds.MaxY - rr->bounds.MinY + 1,
			      	  0xc0, /* copy */
         		      	  0xff,
                 	   	  NULL );
		    }

		    if (dosrcsrc)
		    {
			BltBitMap(srcbm,
			          Tmp.MinX + corrsrcx + dx,
                	          Tmp.MinY + corrsrcy + dy,
	          		  srcbm,
			          Tmp.MinX + corrsrcx,
                	          Tmp.MinY + corrsrcy,
                		  Tmp.MaxX - Tmp.MinX + 1,
                	  	  Tmp.MaxY - Tmp.MinY + 1,
			      	  0xc0, /* copy */
         		      	  0xff,
                 	   	  NULL );

		    }

		    DisposeRegion(RectRegion);
		}
	    }
        }

        if ((L->Flags & LAYERSIMPLE) && UpdateDamageList)
        {
            /* Add the damagelist to the layer's damage list and set the
               LAYERREFRESH flag, but of course only if it's necessary */

            /* first clear the damage lists of the scrolled area */

	    TranslateRect(&ScrollRect, -L->bounds.MinX, -L->bounds.MinY);
	    ClearRectRegion(L->DamageList, &ScrollRect);

            if (R.RegionRectangle)
            {
		TranslateRect(&R.bounds, -L->bounds.MinX, -L->bounds.MinY);

		OrRegionRegion(&R, L->DamageList);
                L->Flags |= LAYERREFRESH;
            }
        }

failexit:
        ClearRegion(&R);
        UnlockLayerRom(L);
    }

    return TRUE;

}
void driver_Draw( struct RastPort *rp, LONG x, LONG y, struct GfxBase  *GfxBase)
{

    struct Rectangle rr;
    OOP_Object *gc;
    struct Layer *L = rp->Layer;
    struct BitMap *bm = rp->BitMap;

    if (!CorrectDriverData (rp, GfxBase))
	return;
	
    gc = GetDriverData(rp)->dd_GC;

    if (rp->cp_x > x) {
	rr.MinX = x;
	rr.MaxX = rp->cp_x;
    } else {
    	rr.MinX = rp->cp_x;
	rr.MaxX = x;
    }
    
    if (rp->cp_y > y) {
	rr.MinY = y;
	rr.MaxY = rp->cp_y;
    } else {
    	rr.MinY = rp->cp_y;
	rr.MaxY = y;
    }
    
    if (NULL == L)
    {
        /* No layer, probably a screen, but may be a user inited bitmap */
	OOP_Object *bm_obj;


	bm_obj = OBTAIN_HIDD_BM(bm);
	if (NULL == bm_obj)
	    return;
	    
	/* No need for clipping */
	HIDD_BM_DrawLine(bm_obj, gc, rp->cp_x, rp->cp_y, x, y);  

	
	RELEASE_HIDD_BM(bm_obj, bm);
	    
    }
    else
    {
        struct ClipRect *CR;
	WORD xrel;
        WORD yrel;
	struct Rectangle torender, intersect;

	LockLayerRom(L);
	
	xrel = L->bounds.MinX;
	yrel = L->bounds.MinY;
	
	
	torender.MinX = rr.MinX + xrel;
	torender.MinY = rr.MinY + yrel;
	torender.MaxX = rr.MaxX + xrel;
	torender.MaxY = rr.MaxY + yrel;
	
	
	CR = L->ClipRect;
	
	for (;NULL != CR; CR = CR->Next)
	{
	    D(bug("Cliprect (%d, %d, %d, %d), lobs=%p\n",
	    	CR->bounds.MinX, CR->bounds.MinY, CR->bounds.MaxX, CR->bounds.MaxY,
		CR->lobs));
		
	    /* Does this cliprect intersect with area to rectfill ? */
	    if (_AndRectRect(&CR->bounds, &torender, &intersect))
	    {
	    	LONG xoffset, yoffset;
		LONG layer_rel_x, layer_rel_y;
		
		xoffset = intersect.MinX - torender.MinX;
		yoffset = intersect.MinY - torender.MinY;
		
		layer_rel_x = intersect.MinX - L->bounds.MinX;
		layer_rel_y = intersect.MinY - L->bounds.MinY;
		
		
		
	        if (NULL == CR->lobs)
		{
		
		    /* Set clip rectangle */
		    HIDD_GC_SetClipRect(gc
		    	, intersect.MinX
			, intersect.MinY
			, intersect.MaxX
			, intersect.MaxY
		    );
		    
		    HIDD_BM_DrawLine(HIDD_BM_OBJ(bm)
		    	, gc
			, rp->cp_x + xrel
			, rp->cp_y + yrel
			, x + xrel
			, y + yrel
		    );
		    
		    HIDD_GC_UnsetClipRect(gc);
		
		}
		else
		{
		    /* Render into offscreen cliprect bitmap */
		    if (L->Flags & LAYERSIMPLE)
		    	continue;
		    else if (L->Flags & LAYERSUPER)
		    {
		    	D(bug("do_render_func(): Superbitmap not handled yet\n"));
		    }
		    else
		    {
		    	LONG bm_rel_minx, bm_rel_miny, bm_rel_maxx, bm_rel_maxy;
			LONG layer_rel_x, layer_rel_y;

			layer_rel_x = intersect.MinX - xrel;
			layer_rel_y = intersect.MinY - yrel;
			
			bm_rel_minx = intersect.MinX - CR->bounds.MinX;
			bm_rel_miny = intersect.MinY - CR->bounds.MinY;
			bm_rel_maxx = intersect.MaxX - CR->bounds.MinX;
			bm_rel_maxy = intersect.MaxY - CR->bounds.MinY;

		    	HIDD_GC_SetClipRect(gc
		    		, bm_rel_minx + ALIGN_OFFSET(CR->bounds.MinX)
				, bm_rel_miny
				, bm_rel_maxx + ALIGN_OFFSET(CR->bounds.MinX) 
				, bm_rel_maxy
			);
			
			HIDD_BM_DrawLine(HIDD_BM_OBJ(CR->BitMap)
				, gc
				, bm_rel_minx - (layer_rel_x - rp->cp_x) + ALIGN_OFFSET(CR->bounds.MinX)
				, bm_rel_miny - (layer_rel_y - rp->cp_y)
				, bm_rel_minx - (layer_rel_x - x) + ALIGN_OFFSET(CR->bounds.MinX)
				, bm_rel_miny - (layer_rel_y - y)
			);
				
			HIDD_GC_UnsetClipRect(gc);
		    }
		    
		} /* if (CR->lobs == NULL) */
		
	    } /* if (cliprect intersects with area to render into) */
	    
	} /* for (each cliprect in the layer) */
	
        UnlockLayerRom(L);
    } /* if (rp->Layer) */
    
    rp->cp_x = x;
    rp->cp_y = y;
    
    return;
}

void driver_DrawEllipse (struct RastPort * rp, LONG center_x, LONG center_y, LONG rx, LONG ry,
		struct GfxBase * GfxBase)
{

    struct Rectangle rr;
    OOP_Object *gc;
    struct Layer *L = rp->Layer;
    struct BitMap *bm = rp->BitMap;
    
    if (!CorrectDriverData (rp, GfxBase))
	return;
/* bug("driver_DrawEllipse(%d %d %d %d)\n", center_x, center_y, rx, ry);	
*/    gc = GetDriverData(rp)->dd_GC;
    
    rr.MinX = center_x - rx;
    rr.MinY = center_y - ry;
    rr.MaxX = center_x + rx;
    rr.MaxY = center_y + ry;

    
    if (NULL == L)
    {
        /* No layer, probably a screen, but may be a user inited bitmap */
	OOP_Object *bm_obj;
	
	bm_obj = OBTAIN_HIDD_BM(bm);
	if (NULL == bm_obj)
	    return;
	    
	/* No need for clipping */
	HIDD_BM_DrawEllipse(bm_obj, gc
		, center_x, center_y
		, rx, ry
	);
	
	RELEASE_HIDD_BM(bm_obj, bm);
	    
    } else {
        struct ClipRect *CR;
	WORD xrel;
        WORD yrel;
	struct Rectangle torender, intersect;
	
	LockLayerRom(L);
	
	CR = L->ClipRect;
	
	xrel = L->bounds.MinX;
	yrel = L->bounds.MinY;
	
	torender.MinX = rr.MinX + xrel;
	torender.MinY = rr.MinY + yrel;
	torender.MaxX = rr.MaxX + xrel;
	torender.MaxY = rr.MaxY + yrel;
	
	
	for (;NULL != CR; CR = CR->Next)
	{
	    D(bug("Cliprect (%d, %d, %d, %d), lobs=%p\n",
	    	CR->bounds.MinX, CR->bounds.MinY, CR->bounds.MaxX, CR->bounds.MaxY,
		CR->lobs));
		
	    /* Does this cliprect intersect with area to rectfill ? */
	    if (_AndRectRect(&CR->bounds, &torender, &intersect))
	    {
	    	LONG xoffset, yoffset;
		LONG layer_rel_x, layer_rel_y;
		
		xoffset = intersect.MinX - torender.MinX;
		yoffset = intersect.MinY - torender.MinY;
		
		layer_rel_x = intersect.MinX - L->bounds.MinX;
		layer_rel_y = intersect.MinY - L->bounds.MinY;

	        if (NULL == CR->lobs)
		{
		
		    /* Set clip rectangle */
/* bug("Setting cliprect: %d %d %d %d : layerrel: %d %d %d %d\n"
		    	, intersect.MinX
			, intersect.MinY
			, intersect.MaxX
			, intersect.MaxY
			
		    	, intersect.MinX - xrel
			, intersect.MinY - yrel
			, intersect.MaxX - xrel
			, intersect.MaxY - yrel
		    );
*/		    
		    HIDD_GC_SetClipRect(gc
		    	, intersect.MinX
			, intersect.MinY
			, intersect.MaxX
			, intersect.MaxY
		    );
		    
		    HIDD_BM_DrawEllipse(HIDD_BM_OBJ(bm)
		    	, gc
			, center_x + xrel
			, center_y + yrel
			, rx
			, ry
		    );
		    
		    HIDD_GC_UnsetClipRect(gc);
		
		
		}
		else
		{
		    /* Render into offscreen cliprect bitmap */
		    if (L->Flags & LAYERSIMPLE)
		    	continue;
		    else if (L->Flags & LAYERSUPER)
		    {
		    	D(bug("do_render_func(): Superbitmap not handled yet\n"));
		    }
		    else
		    {
		    	LONG bm_rel_minx, bm_rel_miny, bm_rel_maxx, bm_rel_maxy;
			LONG layer_rel_x, layer_rel_y;
			
			layer_rel_x = intersect.MinX - xrel;
			layer_rel_y = intersect.MinY - yrel;
			
			bm_rel_minx = intersect.MinX - CR->bounds.MinX;
			bm_rel_miny = intersect.MinY - CR->bounds.MinY;
			bm_rel_maxx = intersect.MaxX - CR->bounds.MinX;
			bm_rel_maxy = intersect.MaxY - CR->bounds.MinY;
			
		    	HIDD_GC_SetClipRect(gc
		    		, bm_rel_minx + ALIGN_OFFSET(CR->bounds.MinX)
				, bm_rel_miny
				, bm_rel_maxx + ALIGN_OFFSET(CR->bounds.MinX) 
				, bm_rel_maxy
			);
			
			HIDD_BM_DrawEllipse(HIDD_BM_OBJ(CR->BitMap)
				, gc
				, bm_rel_minx - (layer_rel_x - center_x) + ALIGN_OFFSET(CR->bounds.MinX)
				, bm_rel_miny - (layer_rel_y - center_y)
				, rx
				, ry
			);
				
				
			HIDD_GC_UnsetClipRect(gc);
		    }
		    
		} /* if (CR->lobs == NULL) */
		
	    } /* if (cliprect intersects with area to render into) */
	    
	} /* for (each cliprect in the layer) */
	
        UnlockLayerRom(L);
    } /* if (rp->Layer) */
    return;

}

struct bgf_render_data {
    WORD fbm_xsrc;
    OOP_Object *fbm;
};

static ULONG bgf_render(APTR bgfr_data
	, LONG srcx, LONG srcy
	, OOP_Object *dstbm_obj, OOP_Object *dst_gc
	, LONG x1, LONG y1, LONG x2, LONG y2
	, struct GfxBase *GfxBase)
{
    struct bgf_render_data *bgfrd;
    ULONG width, height;
    
    width  = x2 - x1 + 1;
    height = y2 - y1 + 1;
    
    bgfrd = (struct bgf_render_data *)bgfr_data;
    
    HIDD_BM_BlitColorExpansion( dstbm_obj
    	, dst_gc
	, bgfrd->fbm
	, srcx + bgfrd->fbm_xsrc, srcy /* stegerg: instead of srcy there was a 0 */
	, x1, y1
	, width, height
     );
     
     return width * height;
    
}
    
void blit_glyph_fast(struct RastPort *rp, OOP_Object *fontbm, WORD xsrc
	, WORD destx, WORD desty, UWORD width, UWORD height
	, struct GfxBase * GfxBase)
{
    
    struct Rectangle rr;
    struct bgf_render_data bgfrd;

    EnterFunc(bug("blit_glyph_fast(%d, %d, %d, %d, %d)\n"
    	, xsrc, destx, desty, width, height));
	
    
    bgfrd.fbm_xsrc = xsrc;
    bgfrd.fbm	   = fontbm;
    
    rr.MinX = destx;
    rr.MinY = desty;
    rr.MaxX = destx + width  - 1;
    rr.MaxY = desty + height - 1;
	
    if (!CorrectDriverData(rp, GfxBase))
    	ReturnVoid("blit_glyph_fast");
	
    do_render_func(rp, NULL, &rr, bgf_render, &bgfrd, FALSE, GfxBase);
	
    ReturnVoid("blit_glyph_fast");
}

    

#define NUMCHARS(tf) ((tf->tf_HiChar - tf->tf_LoChar) + 2)
#define CTF(x) ((struct ColorTextFont *)x)

void driver_Text (struct RastPort * rp, STRPTR string, LONG len,
		  struct GfxBase * GfxBase)
{

#warning Does not handle color textfonts
    WORD  render_y;
    struct TextFont *tf;
    WORD current_x;
    struct tfe_hashnode *hn;
    OOP_Object *fontbm = NULL;
    
    if (!CorrectDriverData (rp, GfxBase))
    	return;

    if ((rp->DrawMode & ~INVERSVID) == JAM2)
    {
    	struct TextExtent te;
    	UBYTE 	    	  old_apen = (UBYTE)rp->FgPen;

    	/* This is actually needed, because below only the
	   part of the glyph which contains data is rendered:
	   
	   ...1100...
	   ...1100...
	   ...1100...
	   ...1111...
	   
	   '.' at left side can be there because of kerning.
	   '.' at the right side can be there because of
	   CharSpace being bigger than glyph bitmap data
	   width.
	*/
	
	TextExtent(rp, string, len, &te);
	SetAPen(rp, (UBYTE)rp->BgPen);
	RectFill(rp, rp->cp_x + te.te_Extent.MinX,
	    	     rp->cp_y + te.te_Extent.MinY,
		     rp->cp_x + te.te_Extent.MaxX,
		     rp->cp_y + te.te_Extent.MaxY);
	SetAPen(rp, old_apen);
    }
    
    /* does this rastport have a layer. If yes, lock the layer it.*/
    if (NULL != rp->Layer)
      LockLayerRom(rp->Layer);	
    
    tf = rp->Font;
    
    /* Check if font has character data as a HIDD bitmap */

    ObtainSemaphore(&PrivGBase(GfxBase)->fontsem);

    hn = tfe_hashlookup(tf, GfxBase);
    if (NULL != hn)
    {
	if (NULL == hn->font_bitmap)
	{
	    hn->font_bitmap = fontbm_to_hiddbm(tf, GfxBase);
	}
    }
    else
    {
    	hn = tfe_hashnode_create(GfxBase);
	if (NULL != hn)
	{
	    
	    hn->font_bitmap = fontbm_to_hiddbm(tf, GfxBase);
	    tfe_hashadd(hn, tf, NULL, GfxBase);
	}
    }

    ReleaseSemaphore(&PrivGBase(GfxBase)->fontsem);

    if (NULL != hn)
	fontbm = hn->font_bitmap;
    
    if (NULL == fontbm)
    {
    	D(bug("FONT HAS NO HIDD BITMAP ! Won't render text\n"));
	return;
    }


    /* Render along font's baseline */
    render_y = rp->cp_y - tf->tf_Baseline;
    current_x = rp->cp_x;
    
    while ( len -- )
    {
	ULONG charloc;
	WORD render_x;
	ULONG idx;
	
	if (*string < tf->tf_LoChar || *string > tf->tf_HiChar )
	{
	     /* A character which there is no glyph for. We just
	        draw the last glyph in the font
	     */
	     idx = tf->tf_HiChar - tf->tf_LoChar;
	}
	else
	{
	    idx = *string - tf->tf_LoChar;
	}
	
	charloc = ((ULONG *)tf->tf_CharLoc)[idx];
	
	if (tf->tf_Flags & FPF_PROPORTIONAL)
	{
//	    render_x = current_x + ((WORD *)tf->tf_CharKern)[idx];
    	    current_x += ((WORD *)tf->tf_CharKern)[idx];
	}
//	else
//	    render_x = current_x;	/* Monospace */
	    
	if (tf->tf_Style & FSF_COLORFONT)
	{
#warning Handle color fonts	
	}
	else
	{
	    WORD xoffset;
	    xoffset = charloc >> 16;
	
	    blit_glyph_fast(rp
		, fontbm
		, xoffset
		, current_x // render_x
		, render_y
		, charloc & 0xFFFF
		, tf->tf_YSize
		, GfxBase
	    );
	}
	
	if (tf->tf_Flags & FPF_PROPORTIONAL)
	    current_x += ((WORD *)tf->tf_CharSpace)[idx];
	else
	    current_x += tf->tf_XSize; /* Add glyph width */
	
	string ++;
    } /* for (each character to render) */
    
    Move(rp, current_x, rp->cp_y);
    
    if (NULL != rp->Layer)
      UnlockLayerRom(rp->Layer);
    
    return;

}

struct prlut8_render_data {
    ULONG pen;
    HIDDT_PixelLUT *pixlut;
};

static LONG pix_read_lut8(APTR prlr_data
	, OOP_Object *bm, OOP_Object *gc
	, LONG x, LONG y
	, struct GfxBase *GfxBase)
{
    struct prlut8_render_data *prlrd;
    
    
    prlrd = (struct prlut8_render_data *)prlr_data;
    
    if (NULL != prlrd->pixlut) {
	HIDD_BM_GetImageLUT(bm, (UBYTE *)&prlrd->pen, 1, x, y, 1, 1, prlrd->pixlut);
    } else {
    	prlrd->pen = HIDD_BM_GetPixel(bm, x, y);
    }

    return 0;
}


ULONG driver_ReadPixel (struct RastPort * rp, LONG x, LONG y,
		    struct GfxBase * GfxBase)
{
    struct prlut8_render_data prlrd;
    LONG ret;
    
    HIDDT_PixelLUT pixlut = { AROS_PALETTE_SIZE, HIDD_BM_PIXTAB(rp->BitMap) };
  
    if(!CorrectDriverData (rp, GfxBase))
	return ((ULONG)-1L);
	
    if (IS_HIDD_BM(rp->BitMap))
    	prlrd.pixlut = &pixlut;
    else
    	prlrd.pixlut = NULL;
	
    prlrd.pen = -1;

    ret = do_pixel_func(rp, x, y, pix_read_lut8, &prlrd, GfxBase);
    if (-1 == ret || -1 == prlrd.pen) {
        D(bug("ReadPixel(), COULD NOT GET PEN. TRYING TO READ FROM SimpleRefresh cliprect ??"));
    	return (ULONG)-1;
    }
	
    return prlrd.pen;
}

LONG driver_WritePixel (struct RastPort * rp, LONG x, LONG y,
		    struct GfxBase * GfxBase)
{

    struct pix_render_data prd;

    if(!CorrectDriverData (rp, GfxBase))
	return  -1L;
	
    prd.pixel = BM_PIXEL(rp->BitMap, (UBYTE)rp->FgPen);

    return do_pixel_func(rp, x, y, pix_write, &prd, GfxBase);
}

/******** SetRast() ************************************/


void driver_SetRast (struct RastPort * rp, ULONG color,
		    struct GfxBase * GfxBase)
{

    
    /* We have to use layers to perform clipping */
    struct BitMap *bm = rp->BitMap;
    HIDDT_Pixel pixval;
    
    ULONG width, height;
    
    width  = GetBitMapAttr(bm, BMA_WIDTH);
    height = GetBitMapAttr(bm, BMA_HEIGHT);
    pixval = BM_PIXEL(bm, color);
    
    
    fillrect_pendrmd(rp
    	, 0, 0
	, width  - 1
	, height - 1
	, pixval
	, vHidd_GC_DrawMode_Copy
	, GfxBase
    );
    

    ReturnVoid("driver_SetRast");

}


BOOL driver_ExtendFont(struct TextFont *tf, struct tfe_hashnode *hn, struct GfxBase *GfxBase)
{
    if (NULL != hn->font_bitmap)
    	return TRUE;
	
    hn->font_bitmap = fontbm_to_hiddbm(tf, GfxBase);
    if (NULL != hn->font_bitmap)
	return TRUE;
	    
    return FALSE;
}

void driver_StripFont(struct TextFont *tf, struct tfe_hashnode *hn, struct GfxBase *GfxBase)
{
    if (NULL != hn->font_bitmap)
    {
    	OOP_DisposeObject(hn->font_bitmap);
    }
    return;
}

int driver_CloneRastPort (struct RastPort * newRP, struct RastPort * oldRP,
			struct GfxBase * GfxBase)
{
    /* Let CorrectDriverData() have a bitmap to use for the GC */
    newRP->BitMap = oldRP->BitMap;
    
    /* Creates a new GC. Hmmm, a general Copy method would've been nice */
    
    if (!CorrectDriverData (newRP, GfxBase))
    	return FALSE;
	
    /* copy rastports attributes */
    SetFont(newRP, oldRP->Font);
    SetABPenDrMd(newRP, GetAPen(oldRP), GetBPen(oldRP), GetDrMd(oldRP));
    Move(newRP, oldRP->cp_x, oldRP->cp_y);
    
#warning Some attributes not copied    
    return TRUE;
}

void driver_DeinitRastPort (struct RastPort * rp, struct GfxBase * GfxBase)
{
    D(bug("driver_DeInitRP()\n"));

    if ( rp->Flags & RPF_DRIVER_INITED )
    {
    	D(bug("RP inited, rp=%p, %flags=%d=\n", rp, rp->Flags));
		 
        if (GetDriverData(rp)->dd_RastPort == rp) 
	{
	    D(bug("Calling DeInitDriverData\n"));
	    DeinitDriverData (rp, GfxBase);
	}
    }
    return;
}


/*********** BltMaskBitMapRastPort() ***************************/

struct bltmask_info
{
    PLANEPTR mask;
    LONG mask_xmin;
    LONG mask_ymin;
    ULONG mask_bpr;
    struct BitMap *srcbm;
    OOP_Object *srcbmobj;
    struct GfxBase *GfxBase;
};


static VOID bltmask_to_buf(struct bltmask_info *bmi
	, LONG x_src, LONG y_src
	, LONG x_dest, LONG y_dest
	, ULONG xsize, ULONG ysize
	, ULONG *buf
	, OOP_Object *dest_bm
	, HIDDT_Pixel *coltab
	, struct GfxBase * GfxBase
)
{	
    /* x_src, y_src is the coordinates int the layer. */
    LONG y;
    UBYTE src_depth;

    EnterFunc(bug("bltmask_to_buf(%p, %d, %d, %d, %d, %p)\n"
    			, bmi, x_src, y_src, xsize, ysize, buf ));

    src_depth = GetBitMapAttr(bmi->srcbm, BMA_DEPTH);
    
    /* We must get the data from the destination bitmap */
    HIDD_BM_GetImage(dest_bm
    	, (UBYTE *)buf
	, xsize * sizeof (HIDDT_Pixel)
	, x_dest, y_dest
	, xsize, ysize
	, vHidd_StdPixFmt_Native32
    );
			
    
    for (y = 0; y < ysize; y ++)
    {
        LONG x;
	
	for (x = 0; x < xsize; x ++)
	{
	    ULONG set_pixel;
	    
	    ULONG idx, mask;
	    idx = COORD_TO_BYTEIDX(x + bmi->mask_xmin, y + bmi->mask_ymin, bmi->mask_bpr);
	    mask = XCOORD_TO_MASK(x + bmi->mask_xmin);
		 
	    set_pixel = bmi->mask[idx] & mask;
		
	    if (set_pixel)
	    {
		  ULONG pen;
		  
#if 1
    	    	  pen = HIDD_BM_GetPixel(bmi->srcbmobj, x + x_src, y + y_src);    	    	
#else
		  pen = getbitmappixel(bmi->srcbm
		  	, x + x_src
			, y + y_src
			, src_depth
			, 0xFF
		   );
#endif
		   
		  *buf = pen; // (coltab != NULL) ? coltab[pen] : pen;
	    }
	    
	    buf ++;
	    
	} /* for (each column) */
	
    } /* for (each row) */

    
    ReturnVoid("bltmask_to_buf");
}


#define APPLY_MINTERM(pen, src, dest, minterm) \
	pen = 0;	\
	if ((minterm) & 0x0010)	pen = (~(src) & ~(dest));	\
	if ((minterm) & 0x0020)	pen = (~(src) &  (dest));	\
	if ((minterm) & 0x0040)	pen = ( (src) & ~(dest));	\
	if ((minterm) & 0x0080)	pen = ( (src) &  (dest));
	

VOID driver_BltMaskBitMapRastPort(struct BitMap *srcBitMap
    		, LONG xSrc, LONG ySrc
		, struct RastPort *destRP
		, LONG xDest, LONG yDest
		, ULONG xSize, ULONG ySize
		, ULONG minterm
		, PLANEPTR bltMask
		, struct GfxBase *GfxBase )
{
    ULONG width, height;
    struct Layer *L = destRP->Layer;
    struct BitMap *bm = destRP->BitMap;
    struct bltmask_info bmi;
    
    HIDDT_DrawMode old_drmd;
    OOP_Object *gc;
    
    struct TagItem gc_tags[] =
    {
	{ aHidd_GC_DrawMode, 0UL },
	{ TAG_DONE, 0UL }
    };
    
    
    EnterFunc(bug("driver_BltMaskBitMapRastPort(%d, %d, %d, %d, %d, %d)\n"
    		, xSrc, ySrc, xDest, yDest, xSize, ySize));

    if (!CorrectDriverData(destRP, GfxBase))
    	ReturnVoid("driver_BltMaskBitMapRastPort");
    
    gc = GetDriverData(destRP)->dd_GC;
    
    gc_tags[0].ti_Data = MINTERM_TO_GCDRMD(minterm);
    
    OOP_GetAttr(gc, aHidd_GC_DrawMode, &old_drmd);
    OOP_SetAttrs(gc, gc_tags);

    bmi.mask	 = bltMask;
    bmi.srcbm	 = srcBitMap;
    bmi.srcbmobj = OBTAIN_HIDD_BM(srcBitMap);
    bmi.GfxBase	 = GfxBase;
    
    if (!bmi.srcbmobj) return;
    
    /* The mask has always the same size as the source bitmap */
    
    bmi.mask_bpr = 2 * WIDTH_TO_WORDS(GetBitMapAttr(srcBitMap, BMA_WIDTH));
    
    /* Set minterm bitmap object */
    
    if (NULL == L)
    {
        /* No layer, probably a screen */
	
	bmi.mask_xmin = 0;
	bmi.mask_ymin = 0;
	
	amiga2hidd_fast( (APTR) &bmi
		, gc
		, xSrc, ySrc
		, bm
		, xDest, yDest
		, xSize
		, ySize
		, bltmask_to_buf
		, GfxBase
	);
	
    }
    else
    {
        struct ClipRect *CR;
	WORD xrel;
        WORD yrel;
	struct Rectangle toblit, intersect;
	
	LockLayerRom( L );
	
	CR = L->ClipRect;
	xrel = L->bounds.MinX;
	yrel = L->bounds.MinY;
	
	toblit.MinX = xDest + xrel;
	toblit.MinY = yDest + yrel;
	toblit.MaxX = (xDest + xSize - 1) + xrel;
	toblit.MaxY = (yDest + ySize - 1) + yrel;
	
	
	for (;NULL != CR; CR = CR->Next)
	{
	    D(bug("Cliprect (%d, %d, %d, %d), lobs=%p\n",
	    	CR->bounds.MinX, CR->bounds.MinY, CR->bounds.MaxX, CR->bounds.MaxY,
		CR->lobs));
		
	    /* Does this cliprect intersect with area to blit ? */
	    if (_AndRectRect(&CR->bounds, &toblit, &intersect))
	    {
	        ULONG xoffset = intersect.MinX - toblit.MinX;
		ULONG yoffset = intersect.MinY - toblit.MinY;
		
		width  = intersect.MaxX - intersect.MinX + 1;
		height = intersect.MaxY - intersect.MinY + 1;

		bmi.mask_xmin = xoffset;
		bmi.mask_ymin = yoffset;
	    
	        if (NULL == CR->lobs)
		{
		    
		    /* Cliprect not obscured, so we may render directly into the display */
		    amiga2hidd_fast( (APTR) &bmi
		    	, gc
			, xSrc + xoffset, ySrc + yoffset
			, bm
			, intersect.MinX, intersect.MinY
			, width
			, height
			, bltmask_to_buf
			, GfxBase
		     );
		}
		else
		{
		    /* Render into offscreen cliprect bitmap */
		    if (L->Flags & LAYERSIMPLE)
		    	continue;
		    else if (L->Flags & LAYERSUPER)
		    {
		    	D(bug("driver_BltMaskBitMapRastPort(): Superbitmap not handled yet\n"));
		    }
		    else
		    {
		    
		    
		    
		    	amiga2hidd_fast( (APTR) &bmi
				, gc
				, xSrc + xoffset, ySrc + yoffset
				, CR->BitMap
		    		, intersect.MinX - CR->bounds.MinX + ALIGN_OFFSET(CR->bounds.MinX)
				, intersect.MinY - CR->bounds.MinY
				, width
				, height
				, bltmask_to_buf
				, GfxBase
		     	);

		    }
		}
	    }
	}
	UnlockLayerRom( L );
	
    }
    
    RELEASE_HIDD_BM(bmi.srcbmobj, bmi.srcbm);
    
    /* Reset to told drawmode value */
    gc_tags[0].ti_Data = (IPTR)old_drmd;
    OOP_SetAttrs(gc, gc_tags);
	

    ReturnVoid("driver_BltMaskBitMapRastPort");
}


static OOP_Object *fontbm_to_hiddbm(struct TextFont *font, struct GfxBase *GfxBase)
{
    ULONG width, height;
    OOP_Object *bm_obj;
    OOP_Object *tmp_gc;
    /* Caclulate sizes for the font bitmap */
    struct TagItem bm_tags[] = {
	{ aHidd_BitMap_Width,		0	},
	{ aHidd_BitMap_Height,		0	},
	{ aHidd_BitMap_StdPixFmt,	vHidd_StdPixFmt_Plane	},
	{ TAG_DONE,	0UL }
    };
    
    tmp_gc = obtain_cache_object(SDD(GfxBase)->gc_cache, GfxBase);
    if (NULL == tmp_gc)
    	return NULL;

    width  = font->tf_Modulo * 8;
    height = font->tf_YSize;
    
    bm_tags[0].ti_Data = width;
    bm_tags[1].ti_Data = height;
	    
#warning Handle color textfonts
    bm_obj = HIDD_Gfx_NewBitMap(SDD(GfxBase)->gfxhidd, bm_tags);
    if (NULL != bm_obj)
    {
    	struct template_info ti;
    	struct BitMap bm;
	struct TagItem gc_tags[] = {
	    { aHidd_GC_DrawMode,	vHidd_GC_DrawMode_Copy },
	    { TAG_DONE, 0UL }
	};

	
	HIDD_BM_OBJ(&bm)	= bm_obj;
	HIDD_BM_COLMAP(&bm)	= NULL;
	HIDD_BM_COLMOD(&bm)	= vHidd_ColorModel_Palette;
	
	bm.Rows		= height;
	bm.BytesPerRow	= WIDTH_TO_BYTES(width);
	bm.Depth	= 1;
	bm.Flags	= BMF_AROS_HIDD;
	
	ti.source	= font->tf_CharData;
	ti.x_src	= 0;
	ti.modulo	= font->tf_Modulo;
	ti.invertsrc	= FALSE;
		
    	/* Copy the character data into the bitmap */
	OOP_SetAttrs(tmp_gc, gc_tags);
	
	amiga2hidd_fast((APTR)&ti
		, tmp_gc
		, 0, 0
		, &bm
		, 0, 0
		, width, height
		, template_to_buf
		, GfxBase
	);
		
    }
    
    release_cache_object(SDD(GfxBase)->gc_cache, tmp_gc, GfxBase);
    
    return bm_obj;
}

/***********************************************/
/* CYBERGFX CALLS                            ***/


#include <proto/cybergraphics.h>

struct wpa_render_data {
    UBYTE *array;
    HIDDT_StdPixFmt pixfmt;
    ULONG modulo;
    ULONG bppix;
};

static ULONG wpa_render(APTR wpar_data
	, LONG srcx, LONG srcy
	, OOP_Object *dstbm_obj
	, OOP_Object *dst_gc
	, LONG x1, LONG y1, LONG x2, LONG y2
	, struct GfxBase *GfxBase)
{
    struct wpa_render_data *wpard;
    ULONG width, height;
    UBYTE *array;
    
    width  = x2 - x1 + 1;
    height = y2 - y1 + 1;
    
    wpard = (struct wpa_render_data *)wpar_data;
    
    array = wpard->array + wpard->modulo * srcy + wpard->bppix * srcx;
    
    HIDD_BM_PutImage(dstbm_obj
    	, dst_gc, array
	, wpard->modulo
	, x1, y1
	, width, height
	, wpard->pixfmt
    );
    
    return width * height;
}


struct rpa_render_data {
    UBYTE *array;
    HIDDT_StdPixFmt pixfmt;
    ULONG modulo;
    ULONG bppix;
};

static ULONG rpa_render(APTR rpar_data
	, LONG srcx, LONG srcy
	, OOP_Object *dstbm_obj
	, OOP_Object *dst_gc
	, LONG x1, LONG y1, LONG x2, LONG y2
	, struct GfxBase *GfxBase)
{
    struct rpa_render_data *rpard;
    ULONG width, height;
    UBYTE *array;
    
    width  = x2 - x1 + 1;
    height = y2 - y1 + 1;
    
    rpard = (struct rpa_render_data *)rpar_data;
    
    array = rpard->array + rpard->modulo * srcy + rpard->bppix * srcx;
    
    HIDD_BM_GetImage(dstbm_obj
    	, array
	, rpard->modulo
	, x1, y1
	, width, height
	, rpard->pixfmt
    );
    
    return width * height;
}

static LONG pix_write(APTR pr_data
	, OOP_Object *bm, OOP_Object *gc
	, LONG x, LONG y
	, struct GfxBase *GfxBase)
{
    struct pix_render_data *prd;
    prd = (struct pix_render_data *)pr_data;
    
    HIDD_BM_PutPixel(bm, x, y, prd->pixel);
    
    return 0;
}

static LONG pix_read(APTR pr_data
	, OOP_Object *bm, OOP_Object *gc
	, LONG x, LONG y
	, struct GfxBase *GfxBase)
{
    struct pix_render_data *prd;
    
    prd = (struct pix_render_data *)pr_data;
    
    prd->pixel = HIDD_BM_GetPixel(bm, x, y);

    
    return 0;
}    


struct extcol_render_data {
    struct render_special_info rsi;
    struct BitMap *destbm;
    HIDDT_Pixel pixel;
    
};

static VOID buf_to_extcol(struct extcol_render_data *ecrd
	, LONG srcx, LONG srcy
	, LONG dstx, LONG dsty
	, ULONG width, ULONG height
	, HIDDT_Pixel *pixbuf
	, OOP_Object *bm_obj
	, HIDDT_Pixel *pixtab)
{
    LONG y;
    struct BitMap *bm;
    bm = ecrd->destbm;
    for (y = 0; y < height; y ++) {
    	LONG x;
	
    	for (x = 0; x < width; x ++) {
	    if (*pixbuf ++ == ecrd->pixel) {
	    	
	    	UBYTE *plane;
		ULONG i;
	    	/* Set the according bit in the bitmap */
		for (i = 0; i < bm->Depth; i ++) {
		    plane = bm->Planes[i];
		    if (NULL != plane) {
		    	UBYTE mask;
			
			plane += COORD_TO_BYTEIDX(x + dstx, y + dsty, bm->BytesPerRow);
			mask = XCOORD_TO_MASK(x + dstx);
			
			/* Set the pixel */
			*plane |= mask;
		    
		    } /* if (plane allocated) */
		} /* for (plane) */
	    } /* if (color match) */
	} /* for (x) */
    } /* for (y) */
    
    return;
}
	
	

static ULONG extcol_render(APTR funcdata
	, LONG dstx, LONG dsty
	, OOP_Object *dstbm_obj
	, OOP_Object *dst_gc
	, LONG x1, LONG y1, LONG x2, LONG y2
	, struct GfxBase *GfxBase)
{
    /* Get the info from the hidd */
    struct extcol_render_data *ecrd;
     
    ecrd = (struct extcol_render_data *)funcdata;
     
    hidd2buf_fast(ecrd->rsi.curbm
     	, x1, y1
	, (APTR)ecrd
	, dstx, dsty
	, x2 - x1 + 1
	, y2 - y1 + 1
	, buf_to_extcol
	, GfxBase
    );
		
    return (x2 - x1 + 1) * (y2 - y1 + 1);
}


struct dm_message {
    APTR memptr;
    ULONG offsetx;
    ULONG offsety;
    ULONG xsize;
    ULONG ysize;
    UWORD bytesperrow;
    UWORD bytesperpix;
    UWORD colormodel;
    
};

struct dm_render_data {
    struct dm_message msg;
    OOP_Object *pf;
    struct Hook *hook;
    struct RastPort *rp;
    HIDDT_StdPixFmt stdpf;
    OOP_Object *gc;
};


static ULONG dm_render(APTR dmr_data
	, LONG srcx, LONG srcy
	, OOP_Object *dstbm_obj
	, OOP_Object *dst_gc
	, LONG x1, LONG y1, LONG x2, LONG y2
	, struct GfxBase *GfxBase)
{
    struct dm_render_data *dmrd;
    UBYTE *addr;
    struct dm_message *msg;
    ULONG bytesperpixel;
    ULONG width, height, fb_width, fb_height;
    ULONG banksize, memsize;
    
    dmrd = (struct dm_render_data *)dmr_data;
    width  = x2 - x1 + 1;
    height = y2 - y1 + 1;;
    msg = &dmrd->msg;
#warning Not sure about this one . Set it to 0 since we adjust msg->memptr to x1/y1 lower down
    msg->offsetx = 0; // x1;
    msg->offsety = 0; // y1;
    msg->xsize = width;
    msg->ysize = height;
    
    /* Get the baseadress from where to render */
    if (HIDD_BM_ObtainDirectAccess(dstbm_obj
    	, &addr
	, &fb_height, &fb_width
	, &banksize, &memsize)) {

	OOP_GetAttr(dmrd->pf, aHidd_PixFmt_BytesPerPixel, &bytesperpixel);
	msg->bytesperpix = (UWORD)bytesperpixel;
    
	/* Colormodel allready set */
    
	/* Compute the adress for the start pixel */
	#warning We should maybe use something else than the BytesPerLine method since we may have alignment
	msg->bytesperrow = HIDD_BM_BytesPerLine(dstbm_obj, dmrd->stdpf, width);
	msg->memptr = addr + (msg->bytesperrow * y1) + (bytesperpixel * x1);
	
	HIDD_BM_ReleaseDirectAccess(dstbm_obj);
	
	CallHookPkt(dmrd->hook, dmrd->rp, msg);
	
    } else {
    	/* We are unable to gain direct access to the framebuffer,
	   so we have to emulate it
	*/
	ULONG bytesperrow;
	ULONG tocopy_h, max_tocopy_h;
	ULONG lines_todo;
    
	lines_todo = height;
    
	/* The HIDD bm does not have a base adress so we have to render into
	   it using a temporary buffer
	*/
   	OOP_GetAttr(dmrd->pf, aHidd_PixFmt_BytesPerPixel, &bytesperpixel);
	bytesperrow = HIDD_BM_BytesPerLine(dstbm_obj, dmrd->stdpf, width);
    
	if (PIXELBUF_SIZE < bytesperrow) {
	    D(bug("!!! NOT ENOUGH SPACE IN TEMP BUFFER FOR A SINGLE LINE IN DoCDrawMethodTagList() !!!\n"));
	    return 0;
    	}
    
    	/* Calculate number of lines we might copy */
    	max_tocopy_h = PIXELBUF_SIZE / bytesperrow;
    
    	/* Get the maximum number of lines */
    	while (lines_todo != 0) {
	
            struct TagItem gc_tags[] = {
	    	{ aHidd_GC_DrawMode, vHidd_GC_DrawMode_Copy },
	    	{ TAG_DONE, 0UL }
	    };
	    
	    HIDDT_DrawMode old_drmd;

    	    tocopy_h = MIN(lines_todo, max_tocopy_h);
    	    msg->memptr = PrivGBase(GfxBase)->pixel_buf;
	    msg->bytesperrow = bytesperrow;
    
	    msg->bytesperpix = (UWORD)bytesperpixel;

LOCK_PIXBUF
	    /* Use the hook to set some pixels */
	    CallHookPkt(dmrd->hook, dmrd->rp, msg);
	
	    OOP_GetAttr(dmrd->gc, aHidd_GC_DrawMode, &old_drmd);
	    OOP_SetAttrs(dmrd->gc, gc_tags);
	    HIDD_BM_PutImage(dstbm_obj, dmrd->gc
		, (UBYTE *)PrivGBase(GfxBase)->pixel_buf
		, bytesperrow
		, x1, y1, width, height
		, dmrd->stdpf
	    );
	    gc_tags[0].ti_Data = (IPTR)old_drmd;
	    OOP_SetAttrs(dmrd->gc, gc_tags);
	
ULOCK_PIXBUF
	}

    }
    
    return width * height;
}

#include "cybergraphics_intern.h"



LONG driver_WriteLUTPixelArray(APTR srcrect,
	UWORD srcx, UWORD srcy,
	UWORD srcmod, struct RastPort *rp, APTR ctable,
	UWORD destx, UWORD desty,
	UWORD sizex, UWORD sizey,
	UBYTE ctabformat,
	struct Library *CyberGfxBase)
{
    ULONG depth;
    
    HIDDT_PixelLUT pixlut;
    HIDDT_Pixel pixtab[256];
    
    HIDDT_Color col;
    ULONG i;
    
    LONG pixwritten = 0;
    UBYTE *buf;
    
    /* This is cybergraphx. We only work wih HIDD bitmaps */
    if (!IS_HIDD_BM(rp->BitMap)) {
    	D(bug("!!!!! Trying to use CGFX call on non-hidd bitmap in WriteLUTPixelArray()!!!\n"));
    	return 0;
    }
    
    pixlut.entries	= 256;
    pixlut.pixels	= pixtab;
    
    depth = GetBitMapAttr(rp->BitMap, BMA_DEPTH);
    
    /* This call does only support bitmaps with depth > 8. Use WritePixelArray8
       for other bitmaps
    */
    
    if (depth <= 8) {
    	D(bug("!!! TRYING TO USE WriteLUTPixelArray() ON BITMAP WITH DEPTH < 8\n"));
    	return 0;
    }
	
    /* Curently only one format is supported */
    if (CTABFMT_XRGB8 != ctabformat) {
    	D(bug("!!! WriteLUTPixelArray() CALLED WITH UNSUPPORTED CTAB FORMAT %d\n"
		, ctabformat));
    	return 0;
    }
    col.alpha	= 0;
	
    /* Convert the coltab into native pixels */
    for (i = 0; i < 256; i ++) {
    	register ULONG rgb = ((ULONG *)ctable)[i];
    	col.red		= (HIDDT_ColComp)((rgb & 0x00FF0000) >> 8);
	col.green	= (HIDDT_ColComp)(rgb & 0x0000FF00);
	col.blue	= (HIDDT_ColComp)((rgb & 0x000000FF) << 8);
	
	pixtab[i] = HIDD_BM_MapColor(HIDD_BM_OBJ(rp->BitMap), &col);
    }
    
    buf = (UBYTE *)srcrect;
    
    buf += CHUNKY8_COORD_TO_BYTEIDX(srcx, srcy, srcmod);
    
    pixwritten = write_pixels_8(rp
    	, buf
	, srcmod
	, destx, desty
	, destx + sizex - 1, desty + sizey - 1
	, &pixlut
	, GfxBase
    );
    
    
    /* Now blit the colors onto the screen */
    
    return pixwritten;
}


LONG driver_WritePixelArray(APTR src, UWORD srcx, UWORD srcy
	, UWORD srcmod, struct RastPort *rp, UWORD destx, UWORD desty
	, UWORD width, UWORD height, UBYTE srcformat, struct Library *CyberGfxBase)
{
     
    OOP_Object *pf;
    HIDDT_StdPixFmt srcfmt_hidd;
    ULONG start_offset, bppix;
    
    LONG pixwritten = 0;
    
    struct wpa_render_data wpard;
    struct Rectangle rr;

    /* This is cybergraphx. We only work wih HIDD bitmaps */
    if (!IS_HIDD_BM(rp->BitMap)) {
    	D(bug("!!!!! Trying to use CGFX call on non-hidd bitmap in WritePixelArray() !!!\n"));
    	return 0;
    }
    
    if (!CorrectDriverData (rp, GfxBase))
	return 0;
	
    if (RECTFMT_LUT8 == srcformat) {
    
	HIDDT_PixelLUT pixlut = { 256, HIDD_BM_PIXTAB(rp->BitMap) };
	UBYTE * array = (UBYTE *)src;
	
	if (rp->BitMap->Flags & BMF_SPECIALFMT) {
	    D(bug("!!! No CLUT in driver_WritePixelArray\n"));
	    return 0;
	}
	
	array += CHUNKY8_COORD_TO_BYTEIDX(srcx, srcy, srcmod);
	
    	pixwritten = write_pixels_8(rp
		, array, srcmod
		, destx, desty
		, destx + width - 1, desty + height - 1
		, &pixlut
		, GfxBase);
		
	return pixwritten;
    }
    
    if (RECTFMT_GREY8 == srcformat) {
    	D(bug("!!! RECTFMT_GREY8 not yet handled in driver_WritePixelArray\n"));
	return 0;
    }
    
    switch (srcformat) {
	case RECTFMT_RGB  : srcfmt_hidd = vHidd_StdPixFmt_RGB24;  break;
	case RECTFMT_RGBA : srcfmt_hidd = vHidd_StdPixFmt_RGBA32; break;
	case RECTFMT_ARGB : srcfmt_hidd = vHidd_StdPixFmt_ARGB32; break;
    }

    /* Compute the start of the array */

#warning Get rid of the below code ?
/* This can be done by passing the srcx and srcy parameters on to
   the HIDD bitmap and let it take care of it itself.
   This means that HIDD_BM_PutImage() gets a lot of parameters,
   which may not be necessary in real life.
   
   Compromise: convert from *CyberGfx* pixfmt to bppix using a table lookup.
   This is faster
*/
    pf = HIDD_Gfx_GetPixFmt(SDD(GfxBase)->gfxhidd, srcfmt_hidd);
    OOP_GetAttr(pf, aHidd_PixFmt_BytesPerPixel, &bppix);
    
    start_offset = ((ULONG)srcy) * srcmod + srcx * bppix;
        
    wpard.array	 = ((UBYTE *)src) + start_offset;
    wpard.pixfmt = srcfmt_hidd;
    wpard.modulo = srcmod;
    wpard.bppix	 = bppix;
    
    rr.MinX = destx;
    rr.MinY = desty;
    rr.MaxX = destx + width  - 1;
    rr.MaxY = desty + height - 1;
    
    pixwritten = do_render_func(rp, NULL, &rr, wpa_render, &wpard, FALSE, GfxBase);
    
    return pixwritten;
}

LONG driver_ReadPixelArray(APTR dst, UWORD destx, UWORD desty
	, UWORD dstmod, struct RastPort *rp, UWORD srcx, UWORD srcy
	, UWORD width, UWORD height, UBYTE dstformat, struct Library *CyberGfxBase)
{
     
    OOP_Object *pf;    
    HIDDT_StdPixFmt dstfmt_hidd;
    
    ULONG start_offset, bppix;
    
    LONG pixread = 0;
    HIDDT_DrawMode old_drmd;
    OOP_Object *gc;
    
    struct Rectangle rr;
    struct rpa_render_data rpard;

    struct TagItem gc_tags[] = {
	{ aHidd_GC_DrawMode, vHidd_GC_DrawMode_Copy},
	{ TAG_DONE, 0}
    };
    
    /* This is cybergraphx. We only work wih HIDD bitmaps */
    if (!IS_HIDD_BM(rp->BitMap)) {
    	D(bug("!!!!! Trying to use CGFX call on non-hidd bitmap in ReadPixelArray() !!!\n"));
    	return 0;
    }
    
    if (!CorrectDriverData (rp, GfxBase))
	return 0;
	
    gc = GetDriverData(rp)->dd_GC;

   /* Preserve old drawmode */
    OOP_GetAttr(gc, aHidd_GC_DrawMode, &old_drmd);
    OOP_SetAttrs(gc, gc_tags);
    
    
    switch (dstformat) {
	case RECTFMT_RGB  : dstfmt_hidd = vHidd_StdPixFmt_RGB24;  break;
	case RECTFMT_RGBA : dstfmt_hidd = vHidd_StdPixFmt_RGBA32; break;
	case RECTFMT_ARGB : dstfmt_hidd = vHidd_StdPixFmt_ARGB32; break;
    }

#warning Get rid of the below code ?
/* This can be done by passing the srcx and srcy parameters on to
   the HIDD bitmap and let it take care of it itself.
   This means that HIDD_BM_PutImage() gets a lot of parameters,
   which may not be necessary in real life.
   
   Compromise: convert from *CyberGfx* pixfmt to bppix using a table lookup.
   This is faster
*/
    pf = HIDD_Gfx_GetPixFmt(SDD(GfxBase)->gfxhidd, dstfmt_hidd);
    OOP_GetAttr(pf, aHidd_PixFmt_BytesPerPixel, &bppix);
    
    start_offset = ((ULONG)srcy) * dstmod + srcx * bppix;
        
    rpard.array	 = ((UBYTE *)dst) + start_offset;
    rpard.pixfmt = dstfmt_hidd;
    rpard.modulo = dstmod;
    rpard.bppix	 = bppix;
    
    rr.MinX = srcx;
    rr.MinY = srcy;
    rr.MaxX = srcx + width  - 1;
    rr.MaxY = srcy + height - 1;
    
    pixread = do_render_func(rp, NULL, &rr, rpa_render, &rpard, FALSE, GfxBase);
    
    /* restore old gc values */
    gc_tags[0].ti_Data = (IPTR)old_drmd;
    OOP_SetAttrs(gc, gc_tags);
    
    return pixread;
}

LONG driver_InvertPixelArray(struct RastPort *rp
	, UWORD destx, UWORD desty, UWORD width, UWORD height
	, struct Library *CyberGfxBase)
{

    /* This is cybergraphx. We only work wih HIDD bitmaps */
    if (!IS_HIDD_BM(rp->BitMap)) {
    	D(bug("!!!!! Trying to use CGFX call on non-hidd bitmap InvertPixelArray() !!!\n"));
    	return 0;
    }

   return (LONG)fillrect_pendrmd(rp
   	, destx, desty
	, destx + width  - 1
	, desty + height - 1
	, 0	/* Source pen does not matter */
	, vHidd_GC_DrawMode_Invert
	, GfxBase);
}

LONG driver_FillPixelArray(struct RastPort *rp
	, UWORD destx, UWORD desty, UWORD width, UWORD height
	, ULONG pixel, struct Library *CyberGfxBase) 
{
    HIDDT_Color col;
    HIDDT_Pixel pix;
    
    /* HIDDT_ColComp are 16 Bit */
    col.alpha	= (HIDDT_ColComp)((pixel >> 16) & 0x0000FF00);
    col.red	= (HIDDT_ColComp)((pixel >> 8) & 0x0000FF00);
    col.green	= (HIDDT_ColComp)(pixel & 0x0000FF00);
    col.blue	= (HIDDT_ColComp)((pixel << 8) & 0x0000FF00);
    
    pix = HIDD_BM_MapColor(HIDD_BM_OBJ(rp->BitMap), &col);

    return (LONG)fillrect_pendrmd(rp
	, destx, desty
	, destx + width  - 1
	, desty + height - 1
	, pix
	, vHidd_GC_DrawMode_Copy
	, GfxBase
    );
}

ULONG driver_MovePixelArray(UWORD srcx, UWORD srcy, struct RastPort *rp
	, UWORD destx, UWORD desty, UWORD width, UWORD height
	, struct Library *CyberGfxBase)
{

    if (!CorrectDriverData(rp, GfxBase))
    	return 0;

    ClipBlit(rp
		, srcx, srcy
		, rp
		, destx, desty
		, width, height
		, 0x00C0 /* Copy */
    );
    return width * height;
}



LONG driver_WriteRGBPixel(struct RastPort *rp, UWORD x, UWORD y
	, ULONG pixel, struct Library *CyberGfxBase)
{
    
    struct pix_render_data prd;
    
    /* Get the HIDD pixel val */
    HIDDT_Color col;
    
    /* This is cybergraphx. We only work wih HIDD bitmaps */
    if (!IS_HIDD_BM(rp->BitMap)) {
    	D(bug("!!!!! Trying to use CGFX call on non-hidd bitmap in WriteRGBPixel() !!!\n"));
    	return 0;
    }

    /* HIDDT_ColComp are 16 Bit */
    
    col.alpha	= (HIDDT_ColComp)((pixel >> 16) & 0x0000FF00);
    col.red	= (HIDDT_ColComp)((pixel >> 8) & 0x0000FF00);
    col.green	= (HIDDT_ColComp)(pixel & 0x0000FF00);
    col.blue	= (HIDDT_ColComp)((pixel << 8) & 0x0000FF00);
    
    prd.pixel = HIDD_BM_MapColor(HIDD_BM_OBJ(rp->BitMap), &col);
    
    return do_pixel_func(rp, x, y, pix_write, &prd, GfxBase);
   
}


ULONG driver_ReadRGBPixel(struct RastPort *rp, UWORD x, UWORD y
	, struct Library *CyberGfxBase)
{
    struct pix_render_data prd;
    
    /* Get the HIDD pixel val */
    HIDDT_Color col;
    HIDDT_Pixel pix;
    LONG ret;
    
    /* This is cybergraphx. We only work wih HIDD bitmaps */
    if (!IS_HIDD_BM(rp->BitMap)) {
    	D(bug("!!!!! Trying to use CGFX call on non-hidd bitmap in ReadRGBPixel()!!!\n"));
    	return (ULONG)-1;
    }
    
    ret = do_pixel_func(rp, x, y, pix_read, &prd, GfxBase);
    if (-1 == ret)
    	return (ULONG)-1;

    HIDD_BM_UnmapPixel(HIDD_BM_OBJ(rp->BitMap), prd.pixel, &col);

    /* HIDDT_ColComp are 16 Bit */
    
    pix =	  ((col.alpha & 0xFF00) << 16)
    		| ((col.red & 0xFF00) << 8)
		| (col.green & 0xFF00)
		| ((col.blue & 0xFF00) >> 8);
    
    return pix;
}



ULONG driver_GetCyberMapAttr(struct BitMap *bitMap, ULONG attribute, struct Library *CyberGfxBase)
{
    OOP_Object *bm_obj;
    OOP_Object *pf;
    
    IPTR retval;
    
    /* This is cybergraphx. We only work wih HIDD bitmaps */
    if (!IS_HIDD_BM(bitMap)) {
    	D(bug("!!!!! Trying to use CGFX call on non-hidd bitmap in GetCyberMapAttr() !!!\n"));
    	return 0;
    }
	
    bm_obj = HIDD_BM_OBJ(bitMap);
    
    OOP_GetAttr(bm_obj, aHidd_BitMap_PixFmt, (IPTR *)&pf);
    
    switch (attribute) {
   	case CYBRMATTR_XMOD:
	    OOP_GetAttr(bm_obj, aHidd_BitMap_BytesPerRow, &retval);
	    break;
	
   	case CYBRMATTR_BPPIX:
	    OOP_GetAttr(pf, aHidd_PixFmt_BytesPerPixel, &retval);
	    break;
	
   	case CYBRMATTR_PIXFMT: {
	    HIDDT_StdPixFmt stdpf;
	    UWORD cpf;
	    OOP_GetAttr(pf, aHidd_PixFmt_StdPixFmt, (IPTR *)&stdpf);
	    
	    /* Convert to cybergfx */
	    cpf = hidd2cyber_pixfmt(stdpf, GfxBase);
	    
	    if (cpf == (UWORD)-1) {
	    	D(bug("!!! UNKNOWN PIXEL FORMAT IN GetCyberMapAttr()\n"));
	    }
	    
	    retval = (IPTR)cpf;
	    break;
	    
	}
	
   	case CYBRMATTR_WIDTH:
	    OOP_GetAttr(bm_obj, aHidd_BitMap_Width, &retval);
	    break;
	
   	case CYBRMATTR_HEIGHT:
	    OOP_GetAttr(bm_obj, aHidd_BitMap_Height, &retval);
	    break;
	
   	case CYBRMATTR_DEPTH:
	    OOP_GetAttr(pf, aHidd_PixFmt_Depth, &retval);
	    break;
	
   	case CYBRMATTR_ISCYBERGFX: {
	    IPTR depth;
	    
	    OOP_GetAttr(pf, aHidd_PixFmt_Depth, &depth);
	    
	    if (depth < 8) {
	    	retval = FALSE;
	    } else {
	    /* We allways have a HIDD bitmap */
	    	retval = TRUE;
	    }
	    break; }
	
   	case CYBRMATTR_ISLINEARMEM:
	    OOP_GetAttr(bm_obj, aHidd_BitMap_IsLinearMem, &retval);
	    break;
	
	default:
	    D(bug("!!! UNKNOWN ATTRIBUTE PASSED TO GetCyberMapAttr()\n"));
	    break;
	
	
    } /* switch (attribute) */
    
    return retval;
}


VOID driver_CVideoCtrlTagList(struct ViewPort *vp, struct TagItem *tags, struct Library *CyberGfxBase)
{
    struct TagItem *tag, *tstate;
    ULONG dpmslevel;
    
    struct TagItem htags[] = {
	{ aHidd_Gfx_DPMSLevel,	0UL	},
	{ TAG_DONE, 0UL }    
    };
    
    BOOL dpms_found = FALSE;
    
    HIDDT_DPMSLevel hdpms;
    
    for (tstate = tags; (tag = NextTagItem((const struct TagItem **)&tstate)); ) {
    	switch (tag->ti_Tag) {
	    case SETVC_DPMSLevel:
	    	dpmslevel = tag->ti_Data;
		dpms_found = TRUE;
	    	break;
	    
	    default:
	    	D(bug("!!! UNKNOWN TAG IN CVideoCtrlTagList(): %x !!!\n"
			, tag->ti_Tag));
		break;
	    
	} /* switch() */
	
    } /* for (each tagitem) */
    
   
    if (dpms_found) {  
    
	/* Convert to hidd dpms level */
	switch (dpmslevel) {
	    case DPMS_ON:
	    	hdpms = vHidd_Gfx_DPMSLevel_On;
	    	break;

	    case DPMS_STANDBY:
	    	hdpms = vHidd_Gfx_DPMSLevel_Standby;
	    	break;

	    case DPMS_SUSPEND:
	    	hdpms = vHidd_Gfx_DPMSLevel_Suspend;
	    	break;

	    case DPMS_OFF:
	    	hdpms = vHidd_Gfx_DPMSLevel_Off;
	    	break;
	
	    default:
	    	D(bug("!!! UNKNOWN DPMS LEVEL IN CVideoCtrlTagList(): %x !!!\n"
	    	    , dpmslevel));
		    
		dpms_found = FALSE;
		break;
	
	}
    }
    
    if (dpms_found) {
	htags[0].ti_Data = hdpms;
    } else {
    	htags[0].ti_Tag = TAG_IGNORE;
    }
    
    OOP_SetAttrs(SDD(GfxBase)->gfxhidd, htags);
    
    return;
}


ULONG driver_ExtractColor(struct RastPort *rp, struct BitMap *bm
	, ULONG color, ULONG srcx, ULONG srcy, ULONG width, ULONG height
	, struct Library *CyberGfxBase)
{
    struct Rectangle rr;
    LONG pixread = 0;
    struct extcol_render_data ecrd;
    OOP_Object *pf;
    HIDDT_ColorModel colmod;
    
    if (!CorrectDriverData(rp, GfxBase))
    	return FALSE;
	
    if (!IS_HIDD_BM(rp->BitMap)) {
    	D(bug("!!! CALLING ExtractColor() ON NO-HIDD BITMAP !!!\n"));
	return FALSE;
    }
    
    rr.MinX = srcx;
    rr.MinY = srcy;
    rr.MaxX = srcx + width  - 1;
    rr.MaxY = srcy + height - 1;
    
    OOP_GetAttr(HIDD_BM_OBJ(rp->BitMap), aHidd_BitMap_PixFmt, (IPTR *)&pf);
    
    OOP_GetAttr(pf, aHidd_PixFmt_ColorModel, (IPTR *)&colmod);
    
    if (vHidd_ColorModel_Palette == colmod) {
        ecrd.pixel = color;
    } else {
	HIDDT_Color col;
	
	col.alpha = (color >> 16) & 0x0000FF00;
	col.red	  = (color >> 8 ) & 0x0000FF00;
	col.green = color & 0x0000FF00;
	col.blue  = (color << 8) & 0x0000FF00;
	
	ecrd.pixel = HIDD_BM_MapColor(HIDD_BM_OBJ(rp->BitMap), &col);
    
    }
    
    ecrd.destbm = bm;
    
    pixread = do_render_func(rp, NULL, &rr, extcol_render, NULL, TRUE, GfxBase);
	
    if (pixread != (width * height))
    	return FALSE;
	
    return TRUE;
}


VOID driver_DoCDrawMethodTagList(struct Hook *hook, struct RastPort *rp, struct TagItem *tags, struct Library *CyberGfxBase)
{

    struct dm_render_data dmrd;
    struct Rectangle rr;
    struct Layer *L;
    
    if (!CorrectDriverData(rp, GfxBase))
    	return;
	
	
    if (!IS_HIDD_BM(rp->BitMap)) {
    	D(bug("!!! NO HIDD BITMAP IN CALL TO DoCDrawMethodTagList() !!!\n"));
	return;
    }

    /* Get the bitmap std pixfmt */    
    OOP_GetAttr(HIDD_BM_OBJ(rp->BitMap), aHidd_BitMap_PixFmt, (IPTR *)&dmrd.pf);
    OOP_GetAttr(dmrd.pf, aHidd_PixFmt_StdPixFmt, &dmrd.stdpf);
    dmrd.msg.colormodel = hidd2cyber_pixfmt(dmrd.stdpf, GfxBase);
    dmrd.hook = hook;
    dmrd.rp = rp;
    
    if (((ULONG)-1) == dmrd.msg.colormodel) {
    	D(bug("!!! UNKNOWN HIDD PIXFMT IN DoCDrawMethodTagList() !!!\n"));
	return;
    }
    
    
    L = rp->Layer;

    rr.MinX = 0;
    rr.MinY = 0;
    
    if (NULL == L) {
	rr.MaxX = GetBitMapAttr(rp->BitMap, BMA_WIDTH)  - 1;
	rr.MaxY = GetBitMapAttr(rp->BitMap, BMA_HEIGHT) - 1;
    } else {
    	/* Lock the layer */
	LockLayerRom(L);
    
    	rr.MaxX = rr.MinX + (L->bounds.MaxX - L->bounds.MinX) - 1;
	rr.MaxY = rr.MinY + (L->bounds.MaxY - L->bounds.MinY) - 1;
    }
    
    dmrd.gc = GetDriverData(rp)->dd_GC;
    do_render_func(rp, NULL, &rr, dm_render, &dmrd, FALSE, GfxBase);
    
    if (NULL != L) {
	UnlockLayerRom(L);
    }
    return;
}

APTR driver_LockBitMapTagList(struct BitMap *bm, struct TagItem *tags, struct Library *CyberGfxBase)
{
    struct TagItem *tag;
    UBYTE *baseaddress;
    ULONG width, height, banksize, memsize;
    OOP_Object *pf;
    HIDDT_StdPixFmt stdpf;
    UWORD cpf;
    
    if (!IS_HIDD_BM(bm)) {
    	D(bug("!!! TRYING TO CALL LockBitMapTagList() ON NON-HIDD BM !!!\n"));
	return NULL;
    }

    OOP_GetAttr(HIDD_BM_OBJ(bm), aHidd_BitMap_PixFmt, (IPTR *)&pf);
    
    OOP_GetAttr(pf, aHidd_PixFmt_StdPixFmt, &stdpf);
    cpf = hidd2cyber_pixfmt(stdpf, GfxBase);
    if (((UWORD)-1) == cpf) {
    	D(bug("!!! TRYING TO CALL LockBitMapTagList() ON NON-CYBER PIXFMT BITMAP !!!\n"));
	return NULL;
    }
    
    /* Get some info from the bitmap object */
    if (!HIDD_BM_ObtainDirectAccess(HIDD_BM_OBJ(bm), &baseaddress, &width, &height, &banksize, &memsize))
    	return NULL;
    
    
    while ((tag = NextTagItem((const struct TagItem **)&tags))) {
    	switch (tag->ti_Tag) {
	    case LBMI_BASEADDRESS:
	    	*((ULONG **)tag->ti_Data) = (ULONG *)baseaddress;
	    	break;
		
	    case LBMI_BYTESPERROW:
	    	*((ULONG *)tag->ti_Data) = 
			(ULONG)HIDD_BM_BytesPerLine(HIDD_BM_OBJ(bm), stdpf, width);
	    	break;
	    
	    case LBMI_BYTESPERPIX:
	    	OOP_GetAttr(pf, aHidd_PixFmt_BytesPerPixel, (IPTR *)tag->ti_Data);
	    	break;
	    
	    case LBMI_PIXFMT: 
		*((ULONG *)tag->ti_Data) = (ULONG)cpf;
	    	break;
		
	    case LBMI_DEPTH:
	    	OOP_GetAttr(pf, aHidd_PixFmt_Depth, (IPTR *)tag->ti_Data);
		break;
	    
	    case LBMI_WIDTH:
	    	OOP_GetAttr(HIDD_BM_OBJ(bm), aHidd_BitMap_Width, (IPTR *)tag->ti_Data);
	    	break;
	    
	    case LBMI_HEIGHT:
	    	OOP_GetAttr(HIDD_BM_OBJ(bm), aHidd_BitMap_Height, (IPTR *)tag->ti_Data);
	    	break;
		
	    default:
	    	D(bug("!!! UNKNOWN TAG PASSED TO LockBitMapTagList() !!!\n"));
		break;
	}
    }
    
    return HIDD_BM_OBJ(bm);
}

VOID driver_UnLockBitMap(APTR handle, struct Library *CyberGfxBase)
{
    HIDD_BM_ReleaseDirectAccess((OOP_Object *)handle);
}

VOID driver_UnLockBitMapTagList(APTR handle, struct TagItem *tags, struct Library *CyberGfxBase)
{
    struct TagItem *tag;
    BOOL reallyunlock = TRUE;
    
    while ((tag = NextTagItem((const struct TagItem **)&tags))) {
    	switch (tag->ti_Tag) {
	    case UBMI_REALLYUNLOCK:
	    	reallyunlock = (BOOL)tag->ti_Data;
		break;
		
	    case UBMI_UPDATERECTS: {
	    	struct RectList *rl;
		
		rl = (struct RectList *)tag->ti_Data;
		
#warning Dunno what to do with this
		
	    	break; }
	
	    default:
	    	D(bug("!!! UNKNOWN TAG PASSED TO UnLockBitMapTagList() !!!\n"));
		break;
	}
    }
    
    if (reallyunlock) {
	HIDD_BM_ReleaseDirectAccess((OOP_Object *)handle);
    }
}
/******************************************/
/* Support stuff for cybergfx             */
/******************************************/

#undef GfxBase

