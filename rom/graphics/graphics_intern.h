#ifndef GRAPHICS_INTERN_H
#define GRAPHICS_INTERN_H
/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$

    Desc: Internal header file for graphics.library
    Lang: english
*/
#ifndef AROS_LIBCALL_H
#   include <aros/libcall.h>
#endif
#ifndef EXEC_EXECBASE_H
#   include <exec/execbase.h>
#endif
#ifndef GRAPHICS_GFXBASE_H
#   include <graphics/gfxbase.h>
#endif
#ifndef GRAPHICS_TEXT_H
#   include <graphics/text.h>
#endif
#ifndef GRAPHICS_RASTPORT_H
#   include <graphics/rastport.h>
#endif
#ifndef OOP_OOP_H
#   include <oop/oop.h>
#endif


#include "fontsupport.h"


extern struct GfxBase * GfxBase;

/* Internal GFXBase struct */

struct GfxBase_intern
{
    struct GfxBase 	 gfxbase;
    
    /* Driver data shared between all rastports (allocated once) */
    APTR		*shared_driverdata;


#define TFE_HASHTABSIZE   16 /* This MUST be a power of two */

    struct tfe_hashnode   * tfe_hashtab[TFE_HASHTABSIZE];
    struct SignalSemaphore  tfe_hashtab_sema;
};

/* Macros */

#define WIDTH_TO_BYTES(width) ((( (width) - 1) >> 3) + 1)
#define WIDTH_TO_WORDS(width) ((( (width) - 1) >> 4) + 1)

#define XCOORD_TO_BYTEIDX( x ) (( x ) >> 3)
#define XCOORD_TO_WORDIDX( x ) (( x ) >> 4)

#define COORD_TO_BYTEIDX(x, y, bytes_per_row)	\
	( ((y) * (bytes_per_row)) + XCOORD_TO_BYTEIDX(x))

#define XCOORD_TO_MASK(x) (1L << (7 - ((x) & 0x07)))


/* Defines */
#define BMT_STANDARD	0x0000	/* Standard bitmap */
#define BMT_RGB 	0x1234	/* RTG Bitmap. 24bit RGB chunky */
#define BMT_RGBA	0x1238	/* RTG Bitmap. 32bit RGBA chunky */
#define BMT_DRIVER	0x8000	/* Special RTG bitmap.
				   Use this as an offset. */

#define TFE(tfe) ((struct TextFontExtension*)tfe)

/* Forward declaration */
struct ViewPort;

#ifdef SysBase
#undef SysBase
#endif
#define SysBase ((struct ExecBase *)(GfxBase->ExecBase))
#ifdef UtilityBase
#undef UtilityBase
#endif
#define UtilityBase ((struct Library *)(GfxBase->UtilBase))

/* Needed for close() */
#define expunge() \
    AROS_LC0(BPTR, expunge, struct GfxBase *, GfxBase, 3, Gfx)

/* a function needed by GfxAssocate(), GfxLookUp(), GfxFree() */
extern ULONG CalcHashIndex(ULONG n);

/* a function needed by Draw() */
BOOL CorrectDriverData (struct RastPort * rp, struct GfxBase * GfxBase);

/* a function needed by ClipBlit */
void internal_ClipBlit(struct RastPort * srcRP,
                       LONG xSrc,
                       LONG ySrc,
                       struct RastPort * destRP,
                       LONG xDest,
                       LONG yDest,
                       LONG xSize,
                       LONG ySize,
                       UBYTE minterm,
                       struct GfxBase * GfxBase);

/* Driver prototypes */
extern BOOL driver_LateGfxInit(APTR, struct GfxBase *GfxBase);

extern struct BitMap * driver_AllocBitMap (ULONG, ULONG, ULONG, ULONG,
			struct BitMap *, struct GfxBase *);
extern void driver_BltClear (void * memBlock, ULONG bytecount, ULONG flags,
			struct GfxBase * GfxBase);
extern LONG driver_BltBitMap ( struct BitMap * srcBitMap, LONG xSrc,
			LONG ySrc, struct BitMap * destBitMap, LONG xDest,
			LONG yDest, LONG xSize, LONG ySize, ULONG minterm,
			ULONG mask, PLANEPTR tempA, struct GfxBase *);
extern VOID driver_BltBitMapRastPort(struct BitMap *,  LONG, LONG,
			    struct RastPort *, LONG, LONG , LONG, LONG,
			    ULONG, struct GfxBase *);
extern VOID driver_BltMaskBitMapRastPort(struct BitMap *srcBitMap
    		, LONG xSrc, LONG ySrc
		, struct RastPort *destRP
		, LONG xDest, LONG yDest
		, ULONG xSize, ULONG ySize
		, ULONG minterm
		, PLANEPTR bltMask
		, struct GfxBase *GfxBase );
		
extern VOID driver_BltPattern(struct RastPort *rp, PLANEPTR mask, LONG xMin, LONG yMin,
		LONG xMax, LONG yMax, ULONG byteCnt, struct GfxBase *GfxBase);
		
extern VOID driver_BltTemplate(PLANEPTR source, LONG xSrc, LONG srcMod, struct RastPort * destRP,
	LONG xDest, LONG yDest, LONG xSize, LONG ySize, struct GfxBase *GfxBase);
extern int driver_CloneRastPort (struct RastPort *, struct RastPort *,
			struct GfxBase *);
extern void driver_CloseFont (struct TextFont *, struct GfxBase *);
extern void driver_DeinitRastPort (struct RastPort *, struct GfxBase *);
extern void driver_DrawEllipse (struct RastPort *, LONG x, LONG y,
			LONG rx, LONG ry, struct GfxBase *);
extern void driver_EraseRect (struct RastPort *, LONG, LONG, LONG, LONG,
			    struct GfxBase *);
extern void driver_FreeBitMap (struct BitMap *, struct GfxBase *);
extern int  driver_InitRastPort (struct RastPort *, struct GfxBase *);
extern void driver_InitView (struct View *, struct GfxBase *);
extern void driver_InitVPort (struct ViewPort *, struct GfxBase *);
extern void driver_LoadRGB4 (struct ViewPort * vp, UWORD * colors, LONG count, struct GfxBase *);
extern void driver_LoadRGB32 (struct ViewPort * vp, const ULONG * table, struct GfxBase *);
extern void driver_Move (struct RastPort *, LONG, LONG, struct GfxBase *);
extern struct TextFont * driver_OpenFont (struct TextAttr *,
			    struct GfxBase *);
extern void driver_PolyDraw (struct RastPort *, LONG, WORD *,
			    struct GfxBase *);
extern ULONG driver_ReadPixel (struct RastPort *, LONG, LONG,
			    struct GfxBase *);
extern LONG driver_ReadPixelArray8 (struct RastPort * rp, ULONG xstart,
			    ULONG ystart, ULONG xstop, ULONG ystop,
			    UBYTE * array, struct RastPort * temprp,
			    struct GfxBase *);
extern LONG driver_ReadPixelLine8 (struct RastPort * rp, ULONG xstart,
			    ULONG ystart, ULONG width,
			    UBYTE * array, struct RastPort * temprp,
			    struct GfxBase *);
extern void driver_RectFill (struct RastPort *, LONG, LONG, LONG, LONG,
			    struct GfxBase *);
extern BOOL driver_MoveRaster (struct RastPort *,
			    LONG, LONG, LONG, LONG, LONG, LONG, BOOL, BOOL,
			    struct GfxBase *);
extern void driver_SetABPenDrMd (struct RastPort *, ULONG, ULONG, ULONG,
			    struct GfxBase * GfxBase);
extern void driver_SetAPen (struct RastPort *, ULONG, struct GfxBase *);
extern void driver_SetBPen (struct RastPort *, ULONG, struct GfxBase *);
extern void driver_SetDrMd (struct RastPort *, ULONG, struct GfxBase *);
extern void driver_SetFont (struct RastPort *, struct TextFont *,
			    struct GfxBase *);
extern void driver_SetOutlinePen (struct RastPort *, ULONG, struct GfxBase *);
extern void driver_SetRast (struct RastPort *, ULONG, struct GfxBase *);
extern void driver_SetRGB32 (struct ViewPort *, ULONG, ULONG, ULONG, ULONG,
			    struct GfxBase *);
extern void driver_SetRGB4 (struct ViewPort *, ULONG, ULONG, ULONG, ULONG,
			    struct GfxBase *);
extern ULONG driver_SetWriteMask (struct RastPort *, ULONG, struct GfxBase *);
extern void driver_Text (struct RastPort *, STRPTR, LONG, struct GfxBase *);
extern WORD driver_TextLength (struct RastPort *, STRPTR, ULONG,
			    struct GfxBase *);
extern VOID driver_WriteChunkyPixels(struct RastPort *rp,
		ULONG, ULONG, ULONG, ULONG,
		UBYTE *, LONG, struct GfxBase *);
extern LONG driver_WritePixel (struct RastPort *, LONG, LONG,
			    struct GfxBase *);
extern LONG driver_WritePixelArray8 (struct RastPort * rp, ULONG xstart,
			    ULONG ystart, ULONG xstop, ULONG ystop,
			    UBYTE * array, struct RastPort * temprp,
			    struct GfxBase *);
extern LONG driver_WritePixelLine8 (struct RastPort * rp, ULONG xstart,
			    ULONG ystart, ULONG width,
			    UBYTE * array, struct RastPort * temprp,
			    struct GfxBase *);
			    
extern void driver_WaitTOF (struct GfxBase *);

extern BOOL driver_ExtendFont(struct TextFont *font, struct tfe_hashnode *hn, struct GfxBase *GfxBase);
extern void driver_StripFont(struct TextFont *font, struct tfe_hashnode *hn, struct GfxBase *GfxBase);


/* functions in support.c */
extern BOOL pattern_pen(struct RastPort *rp
	, LONG x, LONG y
	, ULONG apen, ULONG bpen
	, ULONG *pixval_ptr
	, struct GfxBase *GfxBase);

/* function for area opeartions */
BOOL areafillpolygon(struct RastPort  * rp,
                     struct Rectangle * bounds, 
                     UWORD              first_idx, 
                     UWORD              last_idx,
                     UWORD              bytesperrow,
                     struct GfxBase   * GfxBase);

void areafillellipse(struct RastPort   * rp,
		     struct Rectangle  * bounds,
                     UWORD              * CurVctr,
                     UWORD               BytesPerRow,
                     struct GfxBase    * GfxBase);
                     

#endif /* GRAPHICS_INTERN_H */



