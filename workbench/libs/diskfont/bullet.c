/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Functions for readin .font files
    Lang: English.
*/

/****************************************************************************************/

#include <dos/dos.h>
#include <diskfont/diskfont.h>
#include <diskfont/diskfonttag.h>
#include <diskfont/oterrors.h>
#include <diskfont/glyph.h>
#include <aros/macros.h>
#include <aros/debug.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/bullet.h>
#include <proto/graphics.h>

#include <string.h>

#include "diskfont_intern.h"

/****************************************************************************************/

STRPTR OTAG_MakeFileName(STRPTR filename, struct DiskfontBase_intern *DiskfontBase)
{
    STRPTR  retval;
    LONG    l;
    
    l = strlen(filename) + 1;
    if (l < 7) return NULL;
    
    retval = AllocVec(l, MEMF_ANY);
    if (retval)
    {
    	memcpy(retval, filename, l - 5);
	strcpy(retval + l - 5, "otag");   	
    }
    
    return retval;
}

/****************************************************************************************/

VOID OTAG_FreeFileName(STRPTR filename, struct DiskfontBase_intern *DiskfontBase)
{
    if (filename) FreeVec(filename);
}

/****************************************************************************************/

#ifdef _AROS
#warning This needs to be reworked for systems where sizeof(TagItem) is > 8!
#endif

/****************************************************************************************/

struct OTagList *OTAG_GetFile(STRPTR filename, struct DiskfontBase_intern *DiskfontBase)
{
    struct FileInfoBlock    *fib;
    struct OTagList 	    *otaglist;
    struct TagItem  	    *ti;
    STRPTR  	    	     otagfilename;
    BPTR    	    	     otagfile;
    LONG    	    	     l;
    BOOL    	    	     ok;
    
    otagfilename = OTAG_MakeFileName(filename, DiskfontBase);
    if (!otagfilename) return NULL;
    
    otagfile = Open(otagfilename, MODE_OLDFILE);
    if (!otagfile)
    {
    	OTAG_FreeFileName(otagfilename, DiskfontBase);
    	return NULL;
    }
    
    fib = AllocDosObject(DOS_FIB, NULL);
    if (!fib)
    {
        OTAG_FreeFileName(otagfilename, DiskfontBase);
    	Close(otagfile);
	return NULL;
    }
    
    ok = ExamineFH(otagfile, fib);
    l = fib->fib_Size;
    FreeDosObject(DOS_FIB, fib);
    
    if (!ok)
    {
    	OTAG_FreeFileName(otagfilename, DiskfontBase);
    	Close(otagfile);
	return NULL;
    }

    otaglist = AllocVec(sizeof(struct OTagList) + l, MEMF_PUBLIC | MEMF_CLEAR);
    if (!otaglist)
    {
    	OTAG_FreeFileName(otagfilename, DiskfontBase);
    	Close(otagfile);
	return NULL;
    }
    
    ok = (Read(otagfile, otaglist->tags, l) == l);
    Close(otagfile);
    
    if (AROS_LONG2BE(otaglist->tags[0].ti_Tag)  != OT_FileIdent) ok = FALSE;
    if (AROS_LONG2BE(otaglist->tags[0].ti_Data) != l) ok = FALSE;
    
    if (!ok)
    {
    	OTAG_FreeFileName(otagfilename, DiskfontBase);
    	FreeVec(otaglist);
	return NULL;
    }
    
    ti = otaglist->tags;

    do
    {
    	ti->ti_Tag = AROS_LONG2BE(ti->ti_Tag);
	ti->ti_Data = AROS_LONG2BE(ti->ti_Data);
	
	if (ti->ti_Tag & OT_Indirect)
	{
	    ti->ti_Data = (IPTR)otaglist->tags + ti->ti_Data;
	}
		
    } while ((ti++)->ti_Tag != TAG_DONE);
    
    otaglist->filename = otagfilename;
    
    return otaglist;
    
}

/****************************************************************************************/

VOID OTAG_KillFile(struct OTagList *otaglist, struct DiskfontBase_intern *DiskfontBase)
{
    if (otaglist)
    {
    	if (otaglist->filename) OTAG_FreeFileName(otaglist->filename, DiskfontBase);
    	FreeVec(otaglist);
    }
}

/****************************************************************************************/

UBYTE OTAG_GetFontStyle(struct OTagList *otaglist, struct DiskfontBase_intern *DiskfontBase)
{
    UBYTE style = 0;

    /* A font gets FSF_BOLD if OT_StemWeight >= 0x90 */
    
    if (GetTagData(OT_StemWeight, 0, otaglist->tags) >= 0x90)
    {
    	style |= FSF_BOLD;
    }

    /* A font gets FSF_ITALIC if OT_SlantStyle != OTS_Upright */
    
    if (GetTagData(OT_SlantStyle, OTS_Upright, otaglist->tags) != OTS_Upright)
    {
    	style |= FSF_ITALIC;
    }

    /* A font gets FSF_EXTENDED if OT_HorizStyle >= 0xA0 */
    
    if (GetTagData(OT_HorizStyle, 0, otaglist->tags) >= 0xA0)
    {
    	style |= FSF_EXTENDED;
    }
    
    return style;
}

/****************************************************************************************/

UBYTE OTAG_GetFontFlags(struct OTagList *otaglist, struct DiskfontBase_intern *DiskfontBase)
{
    UBYTE flags;
    
    flags = FONTTYPE_OUTLINEFONT;
    if (GetTagData(OT_IsFixed, FALSE, otaglist->tags) == FALSE)
    {
    	flags |= FPF_PROPORTIONAL;
    }
    
    return flags;
}

/****************************************************************************************/

static BOOL OTAG_SetupFontEngine(struct TTextAttr *ta,
    	    	    	    	 struct TTextAttr *ra,
    	    	    	    	 struct OTagList *otag,
    	    	    	    	 struct GlyphEngine *ge,
				 LONG	*xdpi_ptr,
				 LONG	*ydpi_ptr,
    	    	    	    	 struct Library *BulletBase,
				 struct DiskfontBase_intern *DiskfontBase)
{
    struct TagItem maintags[] =
    {
    	{OT_OTagList, (IPTR)otag->tags	    },
	{OT_OTagPath, (IPTR)otag->filename  },
	{TAG_DONE   	    	    	    }
    };
    struct TagItem sizetags[] =
    {
    	{OT_PointHeight, 0  },
	{OT_DeviceDPI  , 0  },
	{OT_DotSize    , 0  },
	{TAG_DONE   	    }
    };
    LONG pointheight, xdot, ydot;
    LONG xdpi, ydpi, taxdpi, taydpi;
    LONG ysizefactor, ysizefactor_low, ysizefactor_high;
    
    ysizefactor      = GetTagData(OT_YSizeFactor, 0x10001, otag->tags);
    ysizefactor_low  = ysizefactor & 0xFFFF;
    ysizefactor_high = (ysizefactor >> 16) & 0xFFFF;
    
    if (!ysizefactor_low)
    {
    	ysizefactor_low = ysizefactor_high = 1;
    }
    
    if ((ra->tta_Style & FSF_TAGGED) && ra->tta_Tags)
    {
    	ULONG devicedpi;
	
    	devicedpi = GetTagData(TA_DeviceDPI, 0x10001, ra->tta_Tags);
	taxdpi    = (devicedpi >> 16) & 0xFFFF;
	taydpi    = devicedpi & 0xFFFF;
	
	if (!taydpi)
	{
	    taxdpi = taydpi = 1;
	}
    }
    else
    {
    	taxdpi = taydpi = 1;
    }
    
    xdpi = 72 * ysizefactor_high / ysizefactor_low * taxdpi / taydpi;
    ydpi = 72 * ysizefactor_high / ysizefactor_low;
    
    *xdpi_ptr = xdpi;
    *ydpi_ptr = ydpi;
    
    pointheight = ta->tta_YSize << 16;
    xdot = ydot = 100;
    
    sizetags[0].ti_Data = pointheight;
    sizetags[1].ti_Data = (xdpi << 16) | ydpi;
    sizetags[2].ti_Data = (xdot << 16) | ydot;
    
    if (SetInfoA(ge, maintags) != OTERR_Success)
    {
    	return FALSE;
    }
    
    return (SetInfoA(ge, sizetags) == OTERR_Success);
    
}
			 
/****************************************************************************************/

static VOID OTAG_FreeGlyphMaps(struct GlyphEngine *ge,
    	    	    	       struct GlyphMap **gm,
			       struct Library *BulletBase,
			       struct DiskfontBase_intern *DiskfontBase)
{
    UWORD i;
    
    for(i = 0; i < 257; i++)
    {
    	if (gm[i])
	{
	    struct TagItem releasetags[] =
	    {
	    	{OT_GlyphMap, (IPTR)gm[i]},
		{TAG_DONE   	    	 }
	    };
	    
	    ReleaseInfoA(ge, releasetags);
	    
	    gm[i] = NULL;
	}
    }
}

/****************************************************************************************/

static BOOL OTAG_GetGlyphMaps(struct GlyphEngine *ge,
    	    	    	      struct GlyphMap **gm,
    	    	    	      UWORD fontheight,
    	    	    	      WORD *lochar,
			      WORD *hichar,
			      WORD *baseline,
			      LONG *gfxwidth, 		      
    	    	    	      struct Library *BulletBase,
    	    	    	      struct DiskfontBase_intern *DiskfontBase)
{
    UWORD i;
    
    *lochar   = -1;
    *hichar   = 0;
    *baseline = 0;
    *gfxwidth = 0;
    
    for(i = 0; i < 257; i++)
    {
    	struct TagItem settags[] =
	{
	    {OT_GlyphCode, (i < 256) ? i : 0x25A1},
	    {TAG_DONE	    	    	    	 }
	};
	struct TagItem obtaintags[] =
	{
	    {OT_GlyphMap, (IPTR)&gm[i]	    	},
	    {TAG_DONE	    	    	    	}
	};
	
	SetInfoA(ge, settags);
	ObtainInfoA(ge, obtaintags);
	
	if (gm[i])
	{
	    if (i < 256)
	    {
	    	if (*lochar == -1) *lochar = i;
	    	*hichar = i;
	    }
	    
	    if (gm[i]->glm_Y0 - (WORD)gm[i]->glm_BlackTop > *baseline)
	    {
	    	*baseline = gm[i]->glm_Y0 - (WORD)gm[i]->glm_BlackTop;
	    }
	    
	    *gfxwidth += gm[i]->glm_BlackWidth;
	}
    }
        
    if (*baseline >= fontheight) *baseline = fontheight;
    if (*lochar > 32) *lochar = 32;
    
    return (*lochar == -1) ? FALSE : TRUE;
}
			      
/****************************************************************************************/

struct DiskFontHeader *OTAG_AllocFontStruct(STRPTR name, UWORD numchars, LONG gfxwidth,
    	    	    	    	    	    LONG fontheight, struct DiskfontBase_intern *DiskfontBase)
{
    struct DiskFontHeader *dfh;
    APTR    	    	   charkern;
    APTR    	    	   charspace;
    APTR   	    	   charloc;
    APTR    	    	   chardata;
    APTR    	    	   prevsegment = NULL;
    BOOL    	    	   ok = FALSE;
    
    dfh = prevsegment = AllocSegment(prevsegment,
    	    	    	    	     sizeof(struct DiskFontHeader) + sizeof(struct TagItem) * 5,
				     MEMF_ANY | MEMF_CLEAR,
				     DiskfontBase);

    if (dfh)
    {
    	charkern = prevsegment = AllocSegment(prevsegment,
	    	    	    	    	      numchars * sizeof(WORD),
					      MEMF_ANY | MEMF_CLEAR,
					      DiskfontBase);
	if (charkern)
    	{
    	    charspace = prevsegment = AllocSegment(prevsegment,
	    	    	    	    		   numchars * sizeof(WORD),
						   MEMF_ANY | MEMF_CLEAR,
						   DiskfontBase);
						   
	    if (charspace)
	    {
	    	charloc = prevsegment = AllocSegment(prevsegment,
		    	    	    	    	     numchars * sizeof(LONG),
						     MEMF_ANY | MEMF_CLEAR,
						     DiskfontBase);
		
		if (charloc)
		{
		    gfxwidth = (gfxwidth + 15) & ~15;
		    
		    chardata = prevsegment = AllocSegment(prevsegment,	
		    	    	    	    	    	  gfxwidth / 8 * fontheight,
							  MEMF_ANY | MEMF_CLEAR,
							  DiskfontBase);
							  
		    if (chardata)
		    {
		    	WORD i;
			
		    	dfh->dfh_FileID     = DFH_ID;
			dfh->dfh_DF.ln_Name = dfh->dfh_Name;
			dfh->dfh_Segment    = MAKE_REAL_SEGMENT(dfh);
			
			i = strlen(FilePart(name)) + 1;
			if (i >= sizeof(dfh->dfh_Name)) i = sizeof(dfh->dfh_Name) - 1;
 			CopyMem(FilePart(name), dfh->dfh_Name, i);
			
			dfh->dfh_TF.tf_Message.mn_Node.ln_Name = dfh->dfh_Name;
			dfh->dfh_TF.tf_Message.mn_Node.ln_Type = NT_FONT;
			
			dfh->dfh_TF.tf_YSize 	    = fontheight;
			dfh->dfh_TF.tf_CharKern     = charkern;
			dfh->dfh_TF.tf_CharSpace    = charspace;
			dfh->dfh_TF.tf_CharLoc      = charloc;
			dfh->dfh_TF.tf_CharData     = chardata;
			dfh->dfh_TF.tf_Modulo 	    = gfxwidth / 8;
			dfh->dfh_TF.tf_BoldSmear    = 1;
			
			ok = TRUE;
		    }
		    
		} /* if (charloc) */
						     
	    } /* if (charspace) */
	    
	} /* if (charkern) */
	
    } /* if (dfh) */
    
    if (!ok)
    {
    	if (dfh) UnLoadSeg(MAKE_REAL_SEGMENT(dfh));
	dfh = NULL;
    }
    
    return dfh;
}

/****************************************************************************************/

static VOID OTAG_CalcMetrics(struct GlyphMap **gm, struct TextFont *tf)
{
    WORD *charspace = (UWORD *)tf->tf_CharSpace;
    WORD *charkern = (UWORD *)tf->tf_CharKern;
    UWORD lochar = tf->tf_LoChar;
    UWORD hichar = tf->tf_HiChar;
    UWORD i;
    
    for(i = lochar; i <= hichar + 1; i++)
    {
    	struct GlyphMap *g;
	WORD 	    	 index;
	
	index = (i <= hichar) ? i : 256;
	
	g = gm[index];
	if (!g) g = gm[256];
	
	if (g && (i != 32))
	{
	    charkern [i - lochar] = ((WORD)g->glm_BlackLeft) - g->glm_X0;	
	    charspace[i - lochar] = g->glm_X1 - (WORD)g->glm_BlackLeft;
	    
	    if ((tf->tf_Flags & FPF_PROPORTIONAL) == 0)
	    {
		/*
		    In a fixed font (charkern + charspace) must always equal
	            calculated fontwidth.

		    x = propspace - propkern
		    fixedkern = (fontwidth - x + 1) / 2
		    fixedspace = fontwidth - fixedkern
	     	*/
 
	    	LONG w = charspace[i - lochar] - charkern[i - lochar];
		
	    	charkern[i - lochar]  = ((LONG)tf->tf_XSize - w + 1) / 2;
		charspace[i - lochar] = (LONG)tf->tf_XSize - charkern[i - lochar];
	    	
	    }
	    
	}
	else if ((i == 32) || ((tf->tf_Flags & FPF_PROPORTIONAL) == 0))
	{
	    charkern[i - lochar] = 0;
	    charspace[i - lochar] = tf->tf_XSize;
	}
	
    } /* for(i = lochar; i <= hichar + 1; i++) */
}

/****************************************************************************************/

static VOID OTAG_BlitGlyph(struct GlyphMap *gm, struct TextFont *tf, LONG xpos,
    	    	    	   LONG ypos, struct DiskfontBase_intern *DiskfontBase)
{
    UBYTE *src, *dest;
    LONG x, y, width, height;
    LONG srcx, destx, srcy;
           
    srcx   = gm->glm_BlackLeft & 7;
    srcy   = gm->glm_BlackTop;
    destx  = xpos & 7;
    width  = gm->glm_BlackWidth;
    height = gm->glm_BlackHeight;

    if ((width < 1) || (height < 1)) return;
    
    #warning whats the best thing to do here

    /* Check if glyph is bigger/elsewhere than expected,
       ie. extends outside the bounding box vertically:
    
       possible cases:


                                                        ****       
                                                       **  **       
                                                       **  **       
                                                       **  **       
           *****                         *****         **  **
          **   **                       **   **         ****
       +--**---**--+  +-----------+  +--**---**--+  +----------+  +----------+
       |  **   **  |  |           |  |  **   **  |  |          |  |          |
       |  **   **  |  |           |  |  **   **  |  |          |  |          |  
       |   *****   |  |           |  |  **   **  |  |          |  |          |  
       |           |  |           |  |  **   **  |  |          |  |          |  
       |           |  |   *****   |  |  **   **  |  |          |  |          |  
       |           |  |  **   **  |  |  **   **  |  |          |  |          |  
       |           |  |  **   **  |  |  **   **  |  |          |  |          |  
       +-----------+  +--**---**--+  +--**---**--+  +----------+  +----------+
                         **   **        **   **                       ****
			  *****          *****                       **  **
			                                             **  **
			                                             **  **
			                                             **  **
			                                              ****
								     
    */
        
    if (ypos < 0)
    {
    	if (height <= tf->tf_YSize)
	{
	    ypos = 0;
	}
	else
	{
	    srcy   += (height - tf->tf_YSize) / 2;
	    height =  tf->tf_YSize;
	    ypos   =  0;
	}
    }
    else if (ypos + height > tf->tf_YSize)
    {
    	if (height <= tf->tf_YSize)
	{
	    ypos = tf->tf_YSize - height;
	}
	else
	{
	    srcy += (height - tf->tf_YSize) / 2;
	    height = tf->tf_YSize;
	    ypos = 0;
	}
    }
    
    src = gm->glm_BitMap +
          gm->glm_BMModulo * srcy +
	  gm->glm_BlackLeft / 8;
	  
    dest = (UBYTE *)tf->tf_CharData + 
    	    	    ypos * tf->tf_Modulo +
		    xpos / 8;
		       
    for(y = 0; y < height; y++)
    {
    	for(x = 0; x < width; x++)
	{
	    LONG sx = x + srcx;
	    LONG dx = x + destx;
	    
	    if (src[sx / 8] & (0x80 >> (sx & 7)))
	    {
	    	dest[dx / 8] |= (0x80 >> (dx & 7));
	    }
	    else
	    {
	    	dest[dx / 8] &= ~(0x80 >> (dx & 7));
	    }
	}
	src  += gm->glm_BMModulo;
	dest += tf->tf_Modulo;
    }
};

/****************************************************************************************/

static VOID OTAG_MakeCharData(struct GlyphMap **gm, struct TextFont *tf,
    	    	    	      struct DiskfontBase_intern *DiskfontBase)
{
    ULONG *charloc = (ULONG *)tf->tf_CharLoc;
    LONG   xpos = 0;
    UWORD  lochar = tf->tf_LoChar;
    UWORD  hichar = tf->tf_HiChar;
    UWORD  i;
    
    for(i = lochar; i <= hichar + 1; i++)
    {
    	struct GlyphMap *g;
	WORD	    	 index;
	
	index = (i <= hichar) ? i : 256;
	g = gm[index];
	if (g)
	{
	    LONG ypos;
	    
	    ypos = (LONG)tf->tf_Baseline + 1 - (g->glm_Y0 - (WORD)g->glm_BlackTop);
	    
	    OTAG_BlitGlyph(g, tf, xpos, ypos, DiskfontBase);

	    charloc[i - lochar] = (xpos << 16) + g->glm_BlackWidth;
	    xpos += g->glm_BlackWidth;
	}
	
    }
}

/****************************************************************************************/

struct TextFont *OTAG_ReadOutlineFont(struct TTextAttr *attr, struct TTextAttr *reqattr,
    	    	    	    	      struct OTagList *otag, struct DiskfontBase_intern *DiskfontBase)
{
    struct Library  	    *BulletBase;
    struct GlyphEngine      *ge;
    struct GlyphMap 	    **gm;
    struct DiskFontHeader   *dfh = NULL;
    STRPTR  	    	    enginename, enginenamebuf;
    
    LONG    	    	    gfxwidth, spacewidth, xdpi, ydpi;
    WORD    	    	    lochar, hichar, baseline;
    
    enginename = (STRPTR)GetTagData(OT_Engine, NULL, otag->tags);
    if (!enginename) return NULL;
   
    enginenamebuf = AllocVec(strlen(enginename) + sizeof(".library") + 1, MEMF_ANY);
    if (!enginenamebuf) return NULL;
    
    strcpy(enginenamebuf, enginename);
    strcat(enginenamebuf, ".library");
    
    BulletBase = OpenLibrary(enginenamebuf, 0);
    FreeVec(enginenamebuf);
    
    if (!BulletBase) return NULL;
    
    ge = OpenEngine();
    if (!ge)
    {
    	CloseLibrary(BulletBase);
	return NULL;
    }
    
    if (!OTAG_SetupFontEngine(attr, reqattr, otag, ge, &xdpi, &ydpi, BulletBase, DiskfontBase))
    {
    	CloseEngine(ge);
    	CloseLibrary(BulletBase);
    	return NULL;
    }
    
    gm = (struct GlyphMap **)AllocVec(sizeof(struct GlyhpMap *) * 257, MEMF_ANY | MEMF_CLEAR);
    if (!gm)
    {
    	CloseEngine(ge);
    	CloseLibrary(BulletBase);
    	return NULL;
    }
    
    if (!OTAG_GetGlyphMaps(ge,
    	    	    	   gm,
			   attr->tta_YSize,
			   &lochar,
			   &hichar,
			   &baseline,
			   &gfxwidth,
			   BulletBase,
			   DiskfontBase))
    {
    	FreeVec(gm);
    	CloseEngine(ge);
    	CloseLibrary(BulletBase);
	return NULL;
    }
    
    dfh = OTAG_AllocFontStruct(reqattr->tta_Name,
    	    	    	       hichar - lochar + 2,
			       gfxwidth,
			       attr->tta_YSize,
			       DiskfontBase);
    if (!dfh)
    {
    	OTAG_FreeGlyphMaps(ge, gm, BulletBase, DiskfontBase);
    	FreeVec(gm);
    	CloseEngine(ge);
    	CloseLibrary(BulletBase);
	return NULL;
    }
    
    spacewidth = GetTagData(OT_SpaceWidth, 3000, otag->tags);


    /*
       OT_SpaceWidth     pointsize 
       -------------- *  --------- * xdpi
	   2540            250
    */

    #warning maybe should do 64 bit calculations (long long)
    spacewidth = spacewidth * attr->tta_YSize / 250 * xdpi / 2540;

    dfh->dfh_TF.tf_Style    = OTAG_GetFontStyle(otag, DiskfontBase);
    dfh->dfh_TF.tf_Flags    = OTAG_GetFontFlags(otag, DiskfontBase) & ~FPF_ROMFONT;
    dfh->dfh_TF.tf_LoChar   = lochar;
    dfh->dfh_TF.tf_HiChar   = hichar;
    dfh->dfh_TF.tf_Baseline = baseline - 1; /* CHECKME */
    dfh->dfh_TF.tf_XSize    = spacewidth;
    
    OTAG_CalcMetrics(gm, &dfh->dfh_TF);
    OTAG_MakeCharData(gm, &dfh->dfh_TF, DiskfontBase);
    
    OTAG_FreeGlyphMaps(ge, gm, BulletBase, DiskfontBase);   
    FreeVec(gm);
    CloseEngine(ge);
    CloseLibrary(BulletBase);
    
    {
    	/* TagItems were allocated in OTAG_AllocFontStruct */
	
    	struct TagItem *tags = (struct TagItem *)(dfh + 1);
	
	tags[0].ti_Tag  = OT_PointHeight;
	tags[0].ti_Data = attr->tta_YSize << 16;
	tags[1].ti_Tag  = OT_DeviceDPI;
	tags[1].ti_Data = (xdpi << 16) | ydpi;
	tags[2].ti_Tag  = OT_DotSize;
	tags[2].ti_Data = (100 << 16) | 100;
	tags[3].ti_Tag  = TAG_DONE;
	tags[3].ti_Data = 0;
	
    	ExtendFont(&dfh->dfh_TF, tags);
    }
    
    return &dfh->dfh_TF;
}


/****************************************************************************************/
