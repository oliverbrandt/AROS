/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc: Offscreen bitmap class for X11 hidd.
    Lang: English.
*/

#define AROS_ALMOST_COMPATIBLE 1

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <string.h>

#include <proto/oop.h>
#include <proto/utility.h>

#include <exec/memory.h>
#include <exec/lists.h>

#include <graphics/rastport.h>
#include <graphics/gfx.h>
#include <oop/oop.h>
#include <exec/alerts.h>

#include <hidd/graphics.h>

#include "x11gfx_intern.h"
#include "x11.h"

#define SDEBUG 0
#define DEBUG 0
#include <aros/debug.h>

#include "bitmap.h"

static AttrBase HiddBitMapAttrBase = 0;
static AttrBase HiddX11GfxAB = 0;
static AttrBase HiddX11BitMapAB = 0;

static struct abdescr attrbases[] = 
{
    { IID_Hidd_BitMap,		&HiddBitMapAttrBase },
    /* Private bases */
    { IID_Hidd_X11Gfx,		&HiddX11GfxAB	},
    { IID_Hidd_X11BitMap,	&HiddX11BitMapAB },
    { NULL, NULL }
};



/* Macro trick to reuse code between offscreen and onscreen bitmap hidd
(bitmap_common.c) */
#define DRAWABLE(data) (data)->drawable.pixmap

#define MNAME(x) offbitmap_ ## x

/* !!! Include methods whose implementation is eqaul for windows and pixmaps
 (except the DRAWABLE) */


 
#include "bitmap_common.c"


 

/*********** BitMap::New() *************************************/



static Object *offbitmap_new(Class *cl, Object *o, struct pRoot_New *msg)
{
    BOOL ok = TRUE;
    
    EnterFunc(bug("X11Gfx.BitMap::New()\n"));
    
    o = (Object *)DoSuperMethod(cl, o, (Msg) msg);
    if (o)
    {
    	struct bitmap_data *data;
	
        IPTR width, height, depth;
	
	Object *friend;
	Drawable friend_drawable;
	
        data = INST_DATA(cl, o);
	
	/* clear all data  */
        memset(data, 0, sizeof(struct bitmap_data));
	
	/* Get some info passed to us by the x11gfxhidd class */
	data->display = (Display *)GetTagData(aHidd_X11Gfx_SysDisplay, 0, msg->attrList);
	data->screen  = GetTagData(aHidd_X11Gfx_SysScreen, 0, msg->attrList);
	data->hidd2x11cmap = (long *)GetTagData(aHidd_X11Gfx_Hidd2X11CMap, 0, msg->attrList);
	data->cursor = (Cursor)GetTagData(aHidd_X11Gfx_SysCursor, 0, msg->attrList);
	data->colmap = (Colormap)GetTagData(aHidd_X11Gfx_ColorMap, 0, msg->attrList);
		
	
	/* Get attr values */
	GetAttr(o, aHidd_BitMap_Width,		&width);
	GetAttr(o, aHidd_BitMap_Height, 	&height);
	GetAttr(o, aHidd_BitMap_Depth,		&depth);
	
	/* Get the friend bitmap. This should be a displayable bitmap */
	GetAttr(o, aHidd_BitMap_Friend,	(IPTR *)&friend);
	
	/* Get the X11 window from the friend window */
	if (NULL == friend) {
		kprintf("ALERT!!! NO FRIEND BITMAP in config/x11/hidd/offbitmap.c\n");
		Alert(AT_DeadEnd);
	}
	
	GetAttr(friend, aHidd_X11BitMap_Drawable, &friend_drawable);
	
	if (0 == friend_drawable) {
		kprintf("ALERT!!! FRIEND BITMAP HAS NO DRAWABLE in config/x11/hidd/offbitmap.c\n");
		Alert(AT_DeadEnd);
	}
	
	    
	D(bug("Creating X Pixmap, %p, %d, %d, %d\n"
		, friend_drawable
		, width
		, height
		, depth
	));
	
LX11	
	DRAWABLE(data) = XCreatePixmap( data->display
		, friend_drawable
		, width
		, height
		, DefaultDepth (GetSysDisplay(), GetSysScreen())
	);
	
	XFlush(data->display);
UX11	    
	D(bug("X Pixmap : %p\n", DRAWABLE(data) ));
	if (DRAWABLE(data))
	{
	
	    XGCValues gcval;
		    
LX11	    

	    /* Create X11 GC */
	    D(bug("Creating GC\n"));
	 
	    gcval.plane_mask = 0xFFFFFFFF; /*BlackPixel(data->display, data->screen); */ /* bm_data->sysplanemask; */
	    gcval.graphics_exposures = True;
	 
	    data->gc = XCreateGC( data->display
	 		, DefaultRootWindow( data->display )
			, GCPlaneMask | GCGraphicsExposures
			, &gcval
		    );

UX11		
	    if (data->gc)
	    {
LX11	    
	    	XFlush(data->display);
UX11		
	    }
	    else
	    {
	    	ok = FALSE;
	    }
	
	}
	else
	{
	    ok = FALSE;
	} /* if (Xwindow created) */
		
    	if (!ok)
    	{
    
            MethodID disp_mid = GetMethodID(IID_Root, moRoot_Dispose);
    	    CoerceMethod(cl, o, (Msg) &disp_mid);
	
	    o = NULL;
    	}


    } /* if (object allocated by superclass) */
    
    ReturnPtr("X11Gfx.BitMap::New()", Object *, o);
}


/**********  Bitmap::Dispose()  ***********************************/

static VOID offbitmap_dispose(Class *cl, Object *o, Msg msg)
{
    struct bitmap_data *data = INST_DATA(cl, o);
    EnterFunc(bug("X11Gfx.BitMap::Dispose()\n"));
    
    if (data->gc)
    {
LX11
    	XFreeGC(data->display, data->gc);
UX11	
    }
    if (DRAWABLE(data))
    {
LX11	
    	XFreePixmap( GetSysDisplay(), DRAWABLE(data));
	XFlush( GetSysDisplay() );
UX11
	
    }
    
    DoSuperMethod(cl, o, msg);
    
    ReturnVoid("X11Gfx.BitMap::Dispose");
}





/*********  BitMap::Clear()  *************************************/
static VOID offbitmap_clear(Class *cl, Object *o, struct pHidd_BitMap_Clear *msg)
{
    ULONG width, height, bg, old_fg;
    struct bitmap_data *data = INST_DATA(cl, o);
    
    GetAttr(o, aHidd_BitMap_Background, &bg);
    GetAttr(o, aHidd_BitMap_Foreground, &old_fg);
    
    
    /* Get width & height from bitmap superclass */
  
    GetAttr(o, aHidd_BitMap_Width,  &width);
    GetAttr(o, aHidd_BitMap_Height, &height);
    

LX11 
    XSetForeground(data->display, data->gc, data->hidd2x11cmap[bg]);
    XFillRectangle(data->display, DRAWABLE(data), data->gc
    	, 0 , 0
	, width, height
    );
    
    XSetForeground(data->display, data->gc, data->hidd2x11cmap[old_fg]);
    
    XFlush(data->display);
UX11
    
    return;
    
}


#undef SDEBUG
#undef DEBUG
#define SDEBUG 0
#define DEBUG 0
#include <aros/debug.h>



/*** init_bmclass *********************************************************/

#undef XSD
#define XSD(cl) xsd

#define NUM_ROOT_METHODS   4
#define NUM_BITMAP_METHODS 10


Class *init_offbmclass(struct x11_staticdata *xsd)
{
    struct MethodDescr root_descr[NUM_ROOT_METHODS + 1] =
    {
        {(IPTR (*)())MNAME(new), 	moRoot_New    },
        {(IPTR (*)())MNAME(dispose),	moRoot_Dispose},
        {(IPTR (*)())MNAME(set),	moRoot_Set},
        {(IPTR (*)())MNAME(get),	moRoot_Get},
        {NULL, 0UL}
    };

    struct MethodDescr bitMap_descr[NUM_BITMAP_METHODS + 1] =
    {
        {(IPTR (*)())MNAME(setcolors),		moHidd_BitMap_SetColors},
    	{(IPTR (*)())MNAME(putpixel),		moHidd_BitMap_PutPixel},
    	{(IPTR (*)())MNAME(clear),		moHidd_BitMap_Clear},
    	{(IPTR (*)())MNAME(getpixel),		moHidd_BitMap_GetPixel},
    	{(IPTR (*)())MNAME(drawpixel),		moHidd_BitMap_DrawPixel},
    	{(IPTR (*)())MNAME(fillrect),		moHidd_BitMap_FillRect},
    	{(IPTR (*)())MNAME(copybox),		moHidd_BitMap_CopyBox},
    	{(IPTR (*)())MNAME(getimage),		moHidd_BitMap_GetImage},
    	{(IPTR (*)())MNAME(putimage),		moHidd_BitMap_PutImage},
    	{(IPTR (*)())MNAME(blitcolorexpansion),	moHidd_BitMap_BlitColorExpansion},
        {NULL, 0UL}
    };
    
    struct InterfaceDescr ifdescr[] =
    {
        {root_descr,    IID_Root       , NUM_ROOT_METHODS},
        {bitMap_descr,  IID_Hidd_BitMap, NUM_BITMAP_METHODS},
        {NULL, NULL, 0}
    };

    AttrBase MetaAttrBase = ObtainAttrBase(IID_Meta);

    struct TagItem tags[] =
    {
        {aMeta_SuperID,        (IPTR) CLID_Hidd_BitMap},
        {aMeta_InterfaceDescr, (IPTR) ifdescr},
        {aMeta_InstSize,       (IPTR) sizeof(struct bitmap_data)},
        {TAG_DONE, 0UL}
    };
    
    Class *cl = NULL;

    EnterFunc(bug("init_bitmapclass(xsd=%p)\n", xsd));
    
    
    D(bug("Metattrbase: %x\n", MetaAttrBase));


    if(MetaAttrBase)
    {
       D(bug("Got attrbase\n"));
       
/*    for (;;) {cl = cl; } */
        cl = NewObject(NULL, CLID_HiddMeta, tags);
        if(cl)
        {
            D(bug("BitMap class ok\n"));
            xsd->offbmclass = cl;
            cl->UserData     = (APTR) xsd;
           
            /* Get attrbase for the BitMap interface */
	    if (obtainattrbases(attrbases, OOPBase))
            {
	    
                AddClass(cl);
            }
            else
            {
                free_offbmclass( xsd );
                cl = NULL;
            }
        }
	
	/* We don't need this anymore */
	ReleaseAttrBase(IID_Meta);
    } /* if(MetaAttrBase) */

    ReturnPtr("init_bmclass", Class *,  cl);
}


/*** free_offbitmapclass *********************************************************/

void free_offbmclass(struct x11_staticdata *xsd)
{
    EnterFunc(bug("free_bmclass(xsd=%p)\n", xsd));

    if(xsd)
    {
        RemoveClass(xsd->offbmclass);
        if(xsd->offbmclass) DisposeObject((Object *) xsd->offbmclass);
        xsd->offbmclass = NULL;
	
	releaseattrbases(attrbases, OOPBase);
	
    }

    ReturnVoid("free_bmclass");
}
