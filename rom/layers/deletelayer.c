/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <graphics/layers.h>
#include <graphics/regions.h>



/*****************************************************************************

    NAME */
#include <proto/layers.h>
#include "layers_intern.h"
#include <aros/libcall.h>

	AROS_LH2(LONG, DeleteLayer,

/*  SYNOPSIS */
	AROS_LHA(LONG          , dummy, A0),
	AROS_LHA(struct Layer *, LD   , A1),

/*  LOCATION */
	struct LayersBase *, LayersBase, 15, Layers)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS
      No call to any refresh functions of any layers.
      No support for simple layers!!!

    SEE ALSO

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    layers_lib.fd and clib/layers_protos.h

*****************************************************************************/
{
  AROS_LIBFUNC_INIT
  AROS_LIBBASE_EXT_DECL(struct LayersBase *,LayersBase)

  /*
    This is how  we do it:
      The layer to be deleted is called LD
      
    - All layers in front of the layer LD
      are touched in no way by this operation.

    - The ClipRects of the layer LD that are hidden by
      another layer can be deallocated immediately as
      no refresh has to be made there.
      These ClipRects can be recognized by lobs != NULL.
      A bitmap is also found there and must be deallocated. 

    - For the layer immediately behind the layer to be deleted:
      If a ClipRect->lobs of that layer is pointing to the
      layer to be destroyed then the bitmap found there can be 
      restored, the bitmap deallocated and the pointer lobs
      has to be zeroed. The restored ClipRect has to be cut
      out (splitting the rectangle) of the list of the 
      cliprects of the layer LD.
      
    - For all layers further behind:
      If a ClipRect is found with lobs pointing to layer LD
      ask in which layer (staring with the layer behind LD) the 
      upper left corner is located. If it is located in the layer
      the ClipRect belongs to, then restore the rectangle, cut
      it out of the ClipRect list of the layer LD. 
      Otherwise make lobs point to the result of the call to 
      internal_WhichLayer();
  
    - If we take out the very first layer then the new top layer
      doesn't have to have several cliprects. It just needs to have
      one single layer.
      !!!This is not implemented yet

  Ok, here we go: 
  */
  
  struct Layer_Info * LI = LD->LayerInfo;
  struct ClipRect * CR;
  struct Region * R;
  struct RegionRectangle * RR;

  /* no one may interrupt me */
  LockLayers(LI);

  /* take this Layer out of the list of layers */
  if (NULL != LD->front)
  {
    LD->front->back = LD->back;

    if (LD->back)
      LD->back->front = LD->front;
  }
  else /* take out the very first layer */
  {
    LI -> top_layer = LD -> back;

    if (NULL != LD->back)
      LD->back->front = NULL;
  }
 
  /* Let's delete the ClipRects of the layer LD that are
     hidden themselves. 
     The other ClipRects I add to the damage List for
     me to refroesh (clear) through at the end.
  */

  /* clear the region that is there */
  ClearRegion(LD->DamageList);

  CR = LD->ClipRect;
  while (NULL != CR)
  {
    BOOL status;
    struct ClipRect * _CR = CR->Next;
    if (NULL != CR->lobs)
    {
      /* 
         This ClipRect of the layer is hidden. So we can simply 
         free the bitmap data there.
       */
      FreeBitMap(CR->BitMap);
    }
    else
    {
      /* 
         Add this rectangle to the region which we might have
         to clear later on. Parts that do not have to be cleared
         later on will be taken out further below with a call
         to ClearRectRegion(). These parts are restored from
         other layers which were hidden by that layer.
      */
      status = OrRectRegion(LD->DamageList, &CR->bounds);
      if (FALSE == status)
      {
        /* 
           We ran out of memory. Hope this never happens, as
           some of the CR->BitMaps might already be gone. 
         */
        UnlockLayers(LI);
        return FALSE;
      }
    }
    FreeMem(CR, sizeof(struct ClipRect));      
    CR = _CR;
  }
  /* 
     just to make sure...
     Remember: The ClipRects list is now invalid!
  */
  
  

  /* there is a damagelist left and there is a layer behind */
  if (NULL != LD->DamageList->RegionRectangle && NULL != LD->back)
  {
    /* 
       Let's visit the layers that are behind this layer 
       start out with the one immediately behind the layer LD 
     */
     
    struct Layer * L_behind = LD->back;

    CR = L_behind -> ClipRect;

    while (NULL != L_behind)
    {
      CR = L_behind -> ClipRect;
      /* go through all ClipRects of the Layer L_behind. */
      while (NULL != CR)
      {
        /* 
           If the ClipRect of the layer L_behind was previously
           hidden by the deleted Layer LD then we might have to
           make it visible if it is not hidden by yet another
           layer.
         */
        if (LD == CR->lobs)
        {
          struct Layer * Ltmp = internal_WhichLayer(
            LD->back,
            CR->bounds.MinX,
            CR->bounds.MinY
          );
          /* 
              If this layer is now visible ... 
              (it was previously hidden by the layer ld, but not anymore) 
          */
          if (Ltmp == L_behind)
          {  
            if (0 == (L_behind->Flags & LAYERSUPER))
	    { 
              /* no SuperBitMap */
              BltBitMap(
                CR->BitMap,
                CR->bounds.MinX & 0x0f,
                0,
                LD->rp->BitMap,
                CR->bounds.MinX,
                CR->bounds.MinY,
                CR->bounds.MaxX - CR->bounds.MinX + 1,
                CR->bounds.MaxY - CR->bounds.MinY + 1,
                0x0c0, /* copy */
                0xff,
                NULL
               );
	    }
            else
	    {
              /* with SuperBitMap */
              BltBitMap(
                L_behind->SuperBitMap,
                CR->bounds.MinX - L_behind->bounds.MinX + L_behind->Scroll_X,
                CR->bounds.MinY - L_behind->bounds.MinY + L_behind->Scroll_Y,
                LD->rp->BitMap,
                CR->bounds.MinX,
                CR->bounds.MinY,
                CR->bounds.MaxX - CR->bounds.MinX + 1,
                CR->bounds.MaxY - CR->bounds.MinY + 1,
                0x0c0, /* copy */
                0xff,
                NULL
               );              
	    }
            /* ... restore the bitmap stuff found there */
            /* 
               Free the bitmap and clear the lobs entry 
            */
            FreeBitMap(CR->BitMap);
            CR->BitMap = NULL;
            CR->lobs   = NULL;
            /*
               Take this ClipRect out of the damagelist so that
               this part will not be cleared later on. 
             */
            ClearRectRegion(LD->DamageList, &CR->bounds);
          } /* if */
          else /* Ltmp != L_behind */
          {
            /* 
               The entry in lobs of the current ClipRect says that this
               ClipRect was previously hidden by the deleted layer, but
               it is still hidden by yet another layer. So I have to change
               the entry in lobs to the layer that is currently hiding this
               part.
             */
            CR -> lobs = Ltmp;
          }  
        } /* if */
        CR = CR->Next;
      } /* while */
      L_behind = L_behind -> back;
    }
  }
    
  /* 
     The List of the ClipRects of the layer LD should
     now only contain these parts that have to be cleared
     in the bitmap. 
  */
  /*
      The last thing we have to do now is clear those parts of
      the deleted layer that were not hiding anything.
      !!! Actually we should use EraseRect() here so the background
      can be restored with the installed backfill hook.
  */
  R = LD->DamageList;

  RR = R->RegionRectangle;
  /* check if a region is empty */
  while (NULL != RR)
  {
    BltBitMap(
      LD->rp->BitMap, /* don't need a source but mustn't give NULL!!!*/
      0,
      0,
      LD->rp->BitMap,
      RR->bounds.MinX + R->bounds.MinX,
      RR->bounds.MinY + R->bounds.MinY,
      RR->bounds.MaxX - RR->bounds.MinX + 1,
      RR->bounds.MaxY - RR->bounds.MinY + 1,
      0x000, /* clear destination */
      0xff,
      NULL
    );      
    RR = RR->Next;
  } /* while */
  
  /*
     Free 
     - rastport
     - damagelist 
     - layer structure itself
   */
 
   
  FreeMem(LD->rp, sizeof(struct RastPort));
  DisposeRegion(LD->DamageList);
  FreeMem(LD, sizeof(struct Layer));
  
  /* ok, I'm done */
  UnlockLayers(LI);
  return TRUE;
  
  AROS_LIBFUNC_EXIT
} /* DeleteLayer */
