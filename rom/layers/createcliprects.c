/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/

#include <exec/types.h>
#include <graphics/layers.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/regions.h>
#include <proto/graphics.h>

#include "layers_intern.h"
#include "basicfuncs.h"


void CreateClipRects(struct Layer_Info * li, struct Layer * L)
{
  struct Layer * L_other;
  L_other = li -> top_layer;

  SetLayerPriorities(li);

  while (NULL != L_other)
  {
    /* let the new layer split all other layers except for the top_layer */
    if (L_other != L && 
        L_other != li->top_layer)
      LayerSplitsLayer(L, L_other, FALSE);

    /*   
       If the new layer is not the topmost layer let this one
       also be split by the other layer. 
     */
    if (L != li->top_layer && L != L_other)
      LayerSplitsLayer(L_other, L, TRUE);

    L_other = L_other -> back;
  } /* while */
}

/*
   Let all other layers that are in the list with the layer L split
   the cliprects of the layer L
 */
void CreateClipRectsSelf(struct Layer * L, BOOL notdisplayed)
{
  struct Layer_Info * li = L->LayerInfo;
  struct Layer * L_other = li -> top_layer;

  SetLayerPriorities(li);

  if (li->top_layer == L)
    return;

  while (NULL != L_other)
  {
    /* The Layer L is split by all other Layers */
    if (L_other != L)
      LayerSplitsLayer(L_other, L, notdisplayed);

    L_other = L_other -> back;
  }
}

/*
 *  Let all other layers that are in the list with the top layer split
 *  the cliprects of the top layer
 */
void CreateClipRectsTop(struct Layer_Info * li, BOOL notdisplayed)
{
  struct Layer * L_top   = li->top_layer;
  struct Layer * L_other = L_top->back;

  SetLayerPriorities(li);

  while (NULL != L_other)
  {
    /* The Layer L_top is split by all other Layers */
    LayerSplitsLayer(L_other, L_top, notdisplayed);

    L_other = L_other -> back;
  }
}

/*
 *  The Layer L splits all other layers.
 */

void CreateClipRectsOther(struct Layer * L)
{
  struct Layer_Info * li = L->LayerInfo;
  struct Layer * L_other = li -> top_layer;

  SetLayerPriorities(li);

  while (NULL != L_other)
  {
    /* The Layer L is split by all other Layers */
    if (L_other != L)
      LayerSplitsLayer(L, L_other, FALSE);

    L_other = L_other -> back; 
  }
}

void CreateClipRectsAll(struct Layer_Info * li, struct Layer * L)
{
  struct Layer * L_other;

  L_other = li -> top_layer;

  SetLayerPriorities(li);

  while (NULL != L_other)
  {
    /* let the layer L split all other layers except for the top_layer */
    if (L_other != L)
    { 
      if (L_other != li->top_layer)
        LayerSplitsLayer(L, L_other, TRUE);
    /*   
       Even if the new layer is the topmost layer let this one
       also be split by the other layer. 
     */
      LayerSplitsLayer(L_other, L, TRUE);
    }
    L_other = L_other -> back;
  } /* while */
}


/*
   Inputs:
      L_active:  Layer that splits L_passive;
      L_passive: Layer that is being split
      notdisplayed: TRUE  if Layer L_passive is not displayed yet.
                          (= just being added(created) to the list of layers.
                    FALSE if Layer L_passive is already displayed.
 */
static  struct ClipRect *(* FunctionArray[])() = {
                	(void *)&Case_0,
                 	(void *)&Case_1,
                 	(void *)&Case_2,
                 	(void *)&Case_3,
                 	(void *)&Case_4,
                 	(void *)&Case_5,
                 	(void *)&Case_6,
                 	(void *)&Case_7,
                 	(void *)&Case_8,
                 	(void *)&Case_9,
                 	(void *)&Case_10,
                 	(void *)&Case_11,
                 	(void *)&Case_12,
                 	(void *)&Case_13,
                 	(void *)&Case_14,
                 	(void *)&Case_15};

void LayerSplitsLayer(struct Layer * L_active, 
                      struct Layer * L_passive,  
                      BOOL notdisplayed)
{
  LONG x0 = L_active->bounds.MinX;
  LONG y0 = L_active->bounds.MinY;
  LONG x1 = L_active->bounds.MaxX;
  LONG y1 = L_active->bounds.MaxY;
  struct BitMap * bm = L_active->rp->BitMap;

  /* first check whether L_active overlaps L_passive at all */

  if (!(x0 > L_passive -> bounds.MaxX ||
        x1 < L_passive -> bounds.MinX ||
        y0 > L_passive -> bounds.MaxY ||
        y1 < L_passive -> bounds.MinY   ))
  {
    /* they overlap!! */
    /* Now split Layer L_passive's cliprects with L_active */
    struct ClipRect * CR = L_passive -> ClipRect;

    /* 
     * Examine all cliprects of the behind-layer whether they are
     * overlapped somehow by the new layer. Only treat those
     * layers that were not previously hidden by some other layer. 
     */
     
    while (NULL != CR)
    {
      if (!(x0 > CR->bounds.MaxX || 
            x1 < CR->bounds.MinX ||
            y0 > CR->bounds.MaxY ||
            y1 < CR->bounds.MinY   ))
      {
        /* hm, it can also only be partially overlapped */
        /* 
           This is the really difficult case. If the Layer L_active partially
           overlaps a cliprect of the Layer L_passive it will cause the
           cliprect of that layer to be split up into several other cliprects.
           Depending on how it is overlapping the cliprect is split up into 
           up to 5 cliprects.
           See further documentation for this in cliprectfunctions.c.
        */
        /* 
           Now I know that the new layer is overlapping the other
           cliprect somehow. If the other cliprect was overlapped
           before by some layer we may only change the lobs-entry,
           no bitmaps may be copied! (Wrong results otherwise!)
           The lobs entry will be pointing to the new layer.
           This is handled in case_0!! 
	*/
        int OverlapIndex = 0;

        /* one special case to consider: total overlap of newlayer over
           cliprect. treated like case 0 */
        if (~(x0 <= CR->bounds.MinX &&
              x1 >= CR->bounds.MaxX &&
              y0 <= CR->bounds.MinY &&
              y1 >= CR->bounds.MaxY))
	{
          if (x0 >  CR->bounds.MinX &&
              x0 <= CR->bounds.MaxX )
            OverlapIndex |= 8;

          if (y0 >  CR->bounds.MinY &&
              y0 <= CR->bounds.MaxY )
            OverlapIndex |= 4;

          if (x1 >= CR->bounds.MinX &&
              x1 <  CR->bounds.MaxX )
            OverlapIndex |= 2;

          if (y1 >= CR->bounds.MinY &&
              y1 <  CR->bounds.MaxY )
            OverlapIndex |= 1;
        }

        /* 
           Let's call the routine that treats that particular case 
           The chain of ClipRects now looks like this:
           CR->A->B->...
        */

        if (TRUE == notdisplayed)
          CR->Flags |= CR_NEEDS_NO_LAYERBLIT_DAMAGE;
        else
          CR->Flags = 0; /* I had a problem when I didn't do this, so I do this now*/

        CR = (struct ClipRect *)
            FunctionArray[OverlapIndex](&L_active->bounds, 
                                        CR, 
                                        bm, 
                                        L_active, 
                                        L_passive);

        CR->Flags &= ~CR_NEEDS_NO_LAYERBLIT_DAMAGE;
        /* 
           CR should point to the cliprect *BEFORE* the
           next ClipRect that existed before the call to the
           FunctionArray-Function.
           If the chain of cliprects now looks like this
           CR(old one)->New1->New2->A->B
           CR should point to New2 now.
        */
      } /* else */
      /* visit the next cliprect of this layer */
      CR = CR -> Next;   
    } /* while */    
  }
  return;
}

/* 
 *  From a given list of cliprects make a 1:1 copy for all those
 *  parts that are within a region. Also copy the BitMaps if 
 *  necessary.
 */
 
struct ClipRect * CopyClipRectsInRegion(struct Layer * L,
                                        struct ClipRect * CR,
                                        struct Region * ClipRegion)
{
  struct ClipRect * CR_new = NULL, * _CR;
  BOOL isSmart;

  if (LAYERSMART == (L->Flags & (LAYERSMART | LAYERSUPER)))
    isSmart = TRUE;
  else
    isSmart = FALSE;
  
  /* walk through all ClipRects */
  while (NULL != CR)
  {
    /* 
    ** if this is a simple layer and the cliprect is hidden then I
    ** don't even bother with that cliprect 
    */
    if (!(   (0 != (L ->Flags & LAYERSIMPLE)) && NULL != CR->lobs))
    { 
      struct RegionRectangle * RR = ClipRegion->RegionRectangle;
      struct Rectangle Rect = CR->bounds;
      int area;
      /*
      ** Usually I would have to add the ClipRegion's coordinate
      ** to all its RegionRectangles. To prevent this I subtract it
      ** from the Rect's coordinates. 
      */
      Rect.MinX -= (L->bounds.MinX + ClipRegion->bounds.MinX);
      Rect.MinY -= (L->bounds.MinY + ClipRegion->bounds.MinY);
      Rect.MaxX -= (L->bounds.MinX + ClipRegion->bounds.MinX);
      Rect.MaxY -= (L->bounds.MinY + ClipRegion->bounds.MinY);
      area = (Rect.MaxX - Rect.MinX + 1) *
             (Rect.MaxY - Rect.MinY + 1);
    
      /* compare it with all RegionRectangles */
    
      while (NULL != RR && area > 0)
      {
        if (!(Rect.MinX > RR->bounds.MaxX ||
              Rect.MinY > RR->bounds.MaxY ||
              Rect.MaxX < RR->bounds.MinX ||
              Rect.MaxY < RR->bounds.MinY   ))
        {
          /* this is at least partly matching */
          if (NULL == CR_new)
          {
            CR_new = _AllocClipRect(L);
            _CR    = CR_new;
          }
          else
          {
            _CR->Next  = _AllocClipRect(L);
            _CR        = _CR->Next; 
          }

          /* That's what we need in any case */
          _CR->bounds.MinX = ClipRegion->bounds.MinX;
          _CR->bounds.MinY = ClipRegion->bounds.MinY;
          _CR->bounds.MaxX = ClipRegion->bounds.MinX;
          _CR->bounds.MaxY = ClipRegion->bounds.MinY;

          if (RR->bounds.MinX > Rect.MinX)
            _CR->bounds.MinX += RR->bounds.MinX + L->bounds.MinX;
          else
            _CR->bounds.MinX +=       Rect.MinX + L->bounds.MinX;

          if (RR->bounds.MinY > Rect.MinY)
            _CR->bounds.MinY += RR->bounds.MinY + L->bounds.MinY;
          else
            _CR->bounds.MinY +=       Rect.MinY + L->bounds.MinY;

          if (RR->bounds.MaxX < Rect.MaxX)
            _CR->bounds.MaxX += RR->bounds.MaxX + L->bounds.MinX;
          else
            _CR->bounds.MaxX +=       Rect.MaxX + L->bounds.MinX;

          if (RR->bounds.MaxY < Rect.MaxY)
            _CR->bounds.MaxY += RR->bounds.MaxY + L->bounds.MinY;
          else
            _CR->bounds.MaxY +=       Rect.MaxY + L->bounds.MinY;

          area -= (_CR->bounds.MaxX - _CR->bounds.MinX + 1) *
                  (_CR->bounds.MaxY - _CR->bounds.MinY + 1);

          /* copy important data from the original */
          _CR -> lobs   = CR -> lobs;
        
          /* copy parts of/ the whole bitmap in case of a SMART LAYER */
          if (TRUE == isSmart && NULL != CR->BitMap)
          {
            _CR->BitMap = AllocBitMap(_CR->bounds.MaxX - _CR->bounds.MinX + 1 + 15,
                                      _CR->bounds.MaxY - _CR->bounds.MinY + 1,
                                      GetBitMapAttr(CR->BitMap, BMA_DEPTH),
                                      0,
                                      CR->BitMap);
            if (NULL == _CR->BitMap)
              goto failexit;

            BltBitMap(CR->BitMap,
                      _CR->bounds.MinX - CR->bounds.MinX + ALIGN_OFFSET( CR->bounds.MinX),
                      _CR->bounds.MinY - CR->bounds.MinY,
                      _CR->BitMap,
                      ALIGN_OFFSET(_CR->bounds.MinX),
                      0,
                      _CR->bounds.MaxX - _CR->bounds.MinX + 1,
                      _CR->bounds.MaxY - _CR->bounds.MinY + 1,
                      0x0c0,/* copy */
                      0xff,
                      NULL);
          }
          else
          {
            _CR -> BitMap = CR -> BitMap;
          }
        }
        /* visit next RegionRectanlge */
        RR = RR->Next;
      }
    }
    
    /* check next layer */
    CR = CR->Next;
  }
  
  return CR_new;
  



/* out of memory failure */
failexit:
  
  if (NULL == CR_new)
    return NULL;

  CR = CR_new;

  while (TRUE)
  {
    if (TRUE == isSmart)
      FreeBitMap(CR->BitMap);
    
    if (NULL == CR->Next)
      break;
      
    CR = CR->Next;
    
  }

  /* concat the two lists of cliprects */
  CR->Next = L->SuperSaveClipRects;
  L->SuperSaveClipRects = CR_new;
  
  return NULL;
}

