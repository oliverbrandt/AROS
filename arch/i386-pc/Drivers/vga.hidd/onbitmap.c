/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc: Bitmap class for VGA hidd.
    Lang: English.
*/

#define AROS_ALMOST_COMPATIBLE 1

#include <proto/oop.h>
#include <proto/utility.h>

#include <exec/memory.h>
#include <exec/lists.h>

#include <graphics/rastport.h>
#include <graphics/gfx.h>
#include <oop/oop.h>

#include <hidd/graphics.h>

#include "vgahw.h"
#include "vga.h"
#include "vgaclass.h"

#include "bitmap.h"

static AttrBase HiddBitMapAttrBase = 0;
static AttrBase HiddVGAGfxAB = 0;
static AttrBase HiddVGABitMapAB = 0;

static struct ABDescr attrbases[] = 
{
    { IID_Hidd_BitMap,		&HiddBitMapAttrBase },
    /* Private bases */
    { IID_Hidd_VGAgfx,		&HiddVGAGfxAB	},
    { IID_Hidd_VGABitMap,	&HiddVGABitMapAB },
    { NULL, NULL }
};

void vgaRestore(struct vgaHWRec *);
void * vgaSave(struct vgaHWRec *);
int vgaInitMode(struct vgaModeDesc *, struct vgaHWRec *);
void vgaLoadPalette(struct vgaHWRec *, unsigned char *);
void vgaRefreshArea(struct bitmap_data *, int , struct Box *);

void free_onbmclass(struct vga_staticdata *);

extern unsigned long __draw_enable;

#define MNAME(x) onbitmap_ ## x

#define SDEBUG 0
#define DEBUG 0
#include <aros/debug.h>

#define OnBitmap 1
#include "bitmap_common.c"

/*********** BitMap::New() *************************************/

static Object *onbitmap_new(Class *cl, Object *o, struct pRoot_New *msg)
{
    EnterFunc(bug("VGAGfx.BitMap::New()\n"));

    o = (Object *)DoSuperMethod(cl, o, (Msg) msg);
    if (o)
    {
    	struct bitmap_data *data;
	
	Object *pf;

#if 0
	
	struct TagItem depth_tags[] = {
	    { aHidd_BitMap_Depth, 0 },
	    { TAG_DONE, 0 }
	};
#endif	
        IPTR width, height, depth;
	
        data = INST_DATA(cl, o);

	/* clear all data  */
        memset(data, 0, sizeof(struct bitmap_data));
	
	/* Get attr values */
	GetAttr(o, aHidd_BitMap_Width,		&width);
	GetAttr(o, aHidd_BitMap_Height, 	&height);
#if 0
/* nlorentz: The aHidd_BitMap_Depth attribute no loner exist,, so we must
	get the depth in two steps: First get pixel format, then get depth */
	GetAttr(o, aHidd_BitMap_Depth,		&depth);
#else
	GetAttr(o,  aHidd_BitMap_PixFmt,	(IPTR *)&pf);
	GetAttr(pf, aHidd_PixFmt_Depth,		&depth);
#endif
	
	
	assert (width != 0 && height != 0 && depth != 0);
	
	/* 
	   We must only create depths that are supported by the friend drawable
	   Currently we only support the default depth
	*/
/* nlorentz: This test is really not necessary with the new
    design because the user can only create modes that you supplied
    in Gfx::New() in vgaclass.c. */
	if (depth != 4)
	{
	    depth = 4;
	}

	/* Update the depth to the one we use */
#if 0
    /* nlorentz: No longer necessary nor possible */
	depth_tags[0].ti_Data = depth;
	SetAttrs(o, depth_tags);
#endif

	data->width = width;
	data->height = height;
	data->bpp = depth;
	data->Regs = AllocVec(sizeof(struct vgaHWRec),MEMF_PUBLIC|MEMF_CLEAR);
	data->disp = -1;
	width=(width+15) & ~15;

	if (data->Regs)
	{
	    data->VideoData = AllocVec(width*height,MEMF_PUBLIC|MEMF_CLEAR);
	    if (data->VideoData)
	    {
		struct vgaModeEntry *mode,*sel = NULL;
		/* Find out the best video mode */
		ForeachNode(&XSD(cl)->modelist,mode)
		{
		    if ((mode->Desc->Width == data->width) &&
			(mode->Desc->Height == data->height) &&
			(mode->Desc->Depth == data->bpp))
			sel = mode;
		}

		if (sel)
		{
		    struct Box box = {0, 0, width-1, height-1};
		    ObtainSemaphore(&XSD(cl)->HW_acc);

		    /* Now, when the best display mode is chosen, we can build it */
		    vgaInitMode(sel->Desc, data->Regs);
		    vgaLoadPalette(data->Regs,(unsigned char *)NULL);

		    /* Lock the lowlevel vga driver so it cannot mess anything */
		    __draw_enable = 0;

		    /*
		       Because of not defined BitMap_Show method show 
		       bitmap immediately
		    */
		
		    vgaRestore(data->Regs);
		    vgaRefreshArea(data, 1, &box);

		    ReleaseSemaphore(&XSD(cl)->HW_acc);

#if 0
    /* nlorentz: No longer necessary nor possible */
		    set_pixelformat(o);
#endif

#if 0
    /* nlorentz: Should only be needed for HIDDs built on top of
       window environments
    */
		    if (XSD(cl)->activecallback)
			XSD(cl)->activecallback(XSD(cl)->callbackdata, o, TRUE);
#endif

		    XSD(cl)->visible = data;	/* Set created object as visible */

		    ReturnPtr("VGAGfx.BitMap::New()", Object *, o);
		}
		
	    } /* if got data->VideoData */
	    
	    FreeMem(data->Regs,sizeof(struct vgaHWRec));
	} /* if got data->Regs */

	{
	    MethodID disp_mid = GetMethodID(IID_Root, moRoot_Dispose);
    	    CoerceMethod(cl, o, (Msg) &disp_mid);
	}
	
	o = NULL;
    } /* if created object */

    ReturnPtr("VGAGfx.BitMap::New()", Object *, o);
}

/**********  Bitmap::Dispose()  ***********************************/

static VOID onbitmap_dispose(Class *cl, Object *o, Msg msg)
{
    struct bitmap_data *data = INST_DATA(cl, o);
    EnterFunc(bug("VGAGfx.BitMap::Dispose()\n"));
    
    if (data->VideoData)
	FreeVec(data->VideoData);
    if (data->Regs)
	FreeVec(data->Regs);
    
    DoSuperMethod(cl, o, msg);
    
    ReturnVoid("VGAGfx.BitMap::Dispose");
}

/*** init_onbmclass *********************************************************/

#undef XSD
#define XSD(cl) xsd

#define NUM_ROOT_METHODS   3
#define NUM_BITMAP_METHODS 12

Class *init_onbmclass(struct vga_staticdata *xsd)
{
    struct MethodDescr root_descr[NUM_ROOT_METHODS + 1] =
    {
        {(IPTR (*)())MNAME(new)    , moRoot_New    },
        {(IPTR (*)())MNAME(dispose), moRoot_Dispose},
#if 0
        {(IPTR (*)())MNAME(set)	   , moRoot_Set},
#endif
        {(IPTR (*)())MNAME(get)	   , moRoot_Get},
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
    	{(IPTR (*)())MNAME(putimagelut),	moHidd_BitMap_PutImageLUT},
    	{(IPTR (*)())MNAME(getimagelut),	moHidd_BitMap_GetImageLUT},
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
            xsd->onbmclass = cl;
            cl->UserData     = (APTR) xsd;
           
            /* Get attrbase for the BitMap interface */
	    if (ObtainAttrBases(attrbases))
            {
                AddClass(cl);
            }
            else
            {
                free_onbmclass( xsd );
                cl = NULL;
            }
        }
	
	/* We don't need this anymore */
	ReleaseAttrBase(IID_Meta);
    } /* if(MetaAttrBase) */

    ReturnPtr("init_onbmclass", Class *,  cl);
}

/*** free_bitmapclass *********************************************************/

void free_onbmclass(struct vga_staticdata *xsd)
{
    EnterFunc(bug("free_onbmclass(xsd=%p)\n", xsd));

    if(xsd)
    {
        RemoveClass(xsd->onbmclass);
        if(xsd->onbmclass) DisposeObject((Object *) xsd->onbmclass);
        xsd->onbmclass = NULL;
	
	ReleaseAttrBases(attrbases);
    }

    ReturnVoid("free_onbmclass");
}
