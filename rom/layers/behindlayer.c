/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include <proto/exec.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <layers_intern.h>
#include <aros/libcall.h>
#include <proto/graphics.h>
#include "basicfuncs.h"

/*****************************************************************************

    NAME */
#include <proto/layers.h>
	AROS_LH2(LONG, BehindLayer,

/*  SYNOPSIS */
	AROS_LHA(LONG          , dummy, A0),
	AROS_LHA(struct Layer *, L, A1),

/*  LOCATION */
	struct LayersBase *, LayersBase, 9, Layers)

/*  FUNCTION
       If the layer is a backdrop layer it will be moved to the most
       behind position. If it is a non-backdrop layer it will be moved
       in front of the first backdrop layer.
       The areas of simple layers, that become visible by moving this
       layer, are added to the damagelist and the LAYERREFRESH flag
       is set.  

    INPUTS
       dummy - nothing
       L     - pointer to layer 

    RESULT
       TRUE  - layer was successfully moved
       FALSE - layer could not be moved (probably out of memory)
  
    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    layers_lib.fd and clib/layers_protos.h

*****************************************************************************/
{
  AROS_LIBFUNC_INIT
  AROS_LIBBASE_EXT_DECL(struct LayersBase *,LayersBase)

  struct Layer_Info * LI = L->LayerInfo;
  struct Layer * _L;
  struct ClipRect * CR, * _CR;
  struct Region * oldclipregion;
  ULONG area_vis, area_hid;

  /* 
     If it's a non-BACKDROP layer and behind it is either no
     layer or a BACKDROP layer then we're done already
     If it's a BACKDROP layer and there's no layer behind it
     then we're also done.
   */
  if (   L->back == NULL ||
      ( (L->back->Flags & LAYERBACKDROP) != 0 && 
        (L      ->Flags & LAYERBACKDROP) == 0    )  )
    return TRUE;

  /* 
     At this point I am sure that something has to be moved.
     As I am moving a layer further behind I can be sure that
     none of its parts will be displayed that are hidden now, but
     some of its parts might be hidden that are visible now. 
   */ 
  LockLayers(LI);

  /*
     Make sure that all ClipRects are split correctly so that parts
     that are visible now and must be hidden later can go behind
     the right ClipRects. 
   */

  SetLayerPriorities(LI);

  oldclipregion = InstallClipRegion(L, NULL);

  UninstallClipRegionClipRects(LI);

  if (LI->top_layer == L)
  {
    CreateClipRectsTop(LI, FALSE);
    CreateClipRectsAll(LI, L);
  }
  else
  {
    CreateClipRectsSelf(L, FALSE);
  }
  
  /*
   *  Take the layer out of the list and put it in at the approriate
   *  place in the new layer.
   */

  if (LI->top_layer == L)
    LI -> top_layer = L -> back;
  else
    L -> front -> back = L -> back;

  if (NULL != L->back)
    L -> back -> front = L -> front;

  /*  
     Now look for the new position to hang the layer into the list
     I start out with the layer itself and look for the layer before 
     where I want to insert the layer.
   */

  _L = L;

  if ((L -> Flags & LAYERBACKDROP) == 0)
  {
    while (_L -> back != NULL)
    {
      if ((_L -> back -> Flags & LAYERBACKDROP) == 0)
        _L = _L -> back;
      else
        break;    
    }
  }
  else
  {
    while (NULL != _L -> back)
      _L = _L -> back;
  }

  /* _L points to the layer where i have to insert the layer *AFTER* */

  if (NULL != _L->back)
    _L->back->front = L;

   L -> back = _L->back;
  _L -> back = L;
   L -> front = _L;

  /* The layer is linked into its new position. */

  /* 
   * I have to visit all the ClipRects of the layer and see whether
   * they are still visible and I might have to hide them now.
   */

  CR = L->ClipRect;

  while (NULL != CR)
  {
    /* Was this ClipRect visible before ??? */
    if (NULL == CR->lobs)
    {
      /* 
         Check which layer is now visible at this point. If its not
         the layer L then I will have to hide that part of the ClipRect.
       */
      _L = WhichLayer(LI, CR->bounds.MinX, CR->bounds.MinY);
      if (_L != L)
      {
        struct Layer * L_tmp = _L;
        
        /*
        ** The size of the visible cliprect
        */
        area_vis = (CR->bounds.MaxX - CR->bounds.MinX + 1) *
                   (CR->bounds.MaxY - CR->bounds.MinY + 1);
        /*
        ** Search for the first ClipRect in the Layer _L that
        ** somehow overlaps with this cliprect CR. That 
        ** ClipRect and possibly some more have to be made
        ** visible.
        */
        area_hid = 0;
        
        
        _CR = _L->ClipRect;
        while (NULL != _CR)
        {
          if (L == _CR->lobs &&
              !(_CR->bounds.MinX > CR->bounds.MaxX ||
                _CR->bounds.MinY > CR->bounds.MaxY ||
                _CR->bounds.MaxX < CR->bounds.MinX ||
                _CR->bounds.MaxY < CR->bounds.MinY))
          {
            area_hid = (_CR->bounds.MaxX - _CR->bounds.MinX + 1) *
                       (_CR->bounds.MaxY - _CR->bounds.MinY + 1);
            break;
          }
          _CR = _CR->Next;
        }
        
        /*
           It is possible that multiple of the layer L's Clipects
           have to become hidden to make the ClipRect of layer _L
           visible. CR is one of them.
           But it is possible that multiple of the layer _L's 
           ClipRects have to become visible to make the ClipRect
           of layer L visible. _CR is one of them.
        */

        if (area_vis >= area_hid)
        {
          /*
          **  More than one hidden ClipRect must be made visible
          */
          /*
              ClipRect(s) [_L] is/are now visible and 
               CR [ L] is now hidden
	   */

          /*
            There are several cases now:
            The CR  to be hidden can be a simple layer,
                                          superbitmap layer or
                                          smart layer
            The _CR(s) to become visible can be a simple layer(s),
                                                  superbitmap layer(s) or
                                                  smart layer(s) 
          */
          /* One special case :
             Both layers are smart layers (not superbitmap layers) so
             I can use SwapBitsRastPortClipRect
          */

          if (0 == ( L->Flags & LAYERSIMPLE))
          {
            /* the part to be hidden has to go into a bitmap. */
            if (0 == (L->Flags & LAYERSUPER))
            {
              CR->BitMap = 
                  AllocBitMap(CR->bounds.MaxX - CR->bounds.MinX + 1 + 16,
                              CR->bounds.MaxY - CR->bounds.MinY + 1,
                              L->rp->BitMap->Depth,
                              0,
                              L->rp->BitMap);
                              
              BltBitMap(L->rp->BitMap,
                        CR->bounds.MinX,
                        CR->bounds.MinY,
                        CR->BitMap,
                        CR->bounds.MinX & 0x0f,
                        0,
                        CR->bounds.MaxX - CR->bounds.MinX + 1,
                        CR->bounds.MaxY - CR->bounds.MinY + 1,
                        0x0c0,
                        0xff,
                        NULL);
            }
            else
            {
              /* a superbitmap layer */
              CR->BitMap = L->SuperBitMap;
              BltBitMap(L->rp->BitMap,
                        CR->bounds.MinX,
                        CR->bounds.MinY,
                        CR->BitMap,
                        CR->bounds.MinX - L->bounds.MinX + L->Scroll_X,
                        CR->bounds.MinY - L->bounds.MinY + L->Scroll_Y,
                        CR->bounds.MaxX - CR->bounds.MinX + 1,
                        CR->bounds.MaxY - CR->bounds.MinY + 1,
                        0x0c0,
                        0xff,
                        NULL);
            }
          }
          else
          {
            /* the part to be hidden belongs to a simple layer.
               I don't do anything here. */
          }

          CR->lobs = _L;

          while (NULL != _CR && 0 != area_vis)
          {
            /* There might not just be one ClipRect of the layer _L hidden. So
               I have to check them all  
             */
            if (L == _CR->lobs &&
                !(_CR->bounds.MinX > CR->bounds.MaxX ||
                  _CR->bounds.MaxX < CR->bounds.MinX ||
                  _CR->bounds.MinY > CR->bounds.MaxY ||
                  _CR->bounds.MaxY < CR->bounds.MinY))
            {
              /*
              ** Make this ClipRect visible
              */
              if (0 == (_L->Flags & LAYERSIMPLE))
              {
                if (0 == (_L->Flags & LAYERSUPER))
                {
                  BltBitMap(_CR->BitMap,
                            _CR->bounds.MinX & 0x0f,
                            0,
                            _L->rp->BitMap,
                            _CR->bounds.MinX,
                            _CR->bounds.MinY,
                            _CR->bounds.MaxX - _CR->bounds.MinX + 1,
                            _CR->bounds.MaxY - _CR->bounds.MinY + 1,
                            0x0c0,
                            0xff,
                            NULL);
                  FreeBitMap(_CR->BitMap);
                }
                else
                {
                  /* the part to become visible belongs to a superbitmap layer */
                  BltBitMap(_L->SuperBitMap,
                            _CR->bounds.MinX - _L->bounds.MinX + _L->Scroll_X,
                            _CR->bounds.MinY - _L->bounds.MinY + _L->Scroll_Y,
                            _L->rp->BitMap,
                            _CR->bounds.MinX,
                            _CR->bounds.MinY,
                            _CR->bounds.MaxX - _CR->bounds.MinX + 1,
                            _CR->bounds.MaxY - _CR->bounds.MinY + 1,
                            0x0c0,
                            0xff,
                            NULL);
                }
              }
              else
              {
              }
 
              /*
              ** Mark this ClipRect as visible
              */
              _CR -> lobs   = NULL;
              _CR -> BitMap = NULL;
              
              /* check whether the whole area has already been moved... */
              area_vis -= (_CR->bounds.MaxX - _CR->bounds.MinX + 1) *
                          (_CR->bounds.MaxY - _CR->bounds.MinY + 1);
            }
            else
            {
            }
             
            _CR = _CR->Next;
          } /* while (NULL != _CR && 0 != area_vis) */

          if (0 != (_L ->Flags & LAYERSIMPLE))
          {
            /* 
            ** The part to become visible belongs to a simple layer,
            ** I add that part to the damage list and clear that area. 
            */
            /* 
            ** The damagelist is relative to the window instead of the
            ** screen! 
            */
            struct Rectangle R = CR->bounds;
            R.MinX -= _L->bounds.MinX;
            R.MinY -= _L->bounds.MinY;
            R.MaxX -= _L->bounds.MinX;
            R.MaxY -= _L->bounds.MinY;
            
            OrRectRegion(_L->DamageList, &R);
            _L->Flags |= LAYERREFRESH;
 
            _CallLayerHook(_L->BackFill,
                           _L->rp,
                           _L,
                           &CR->bounds,
                           CR->bounds.MinX,
                           CR->bounds.MinY
                          );
          }
          
          if (0 != (L->Flags & LAYERSIMPLE))
          {
            /*
            ** If L is a simple layer than clear that part from
            ** the damagelist such that no mess happens on the screen.
            */
            struct Rectangle R = CR->bounds;
            R.MinX -= L->bounds.MinX;
            R.MinY -= L->bounds.MinY;
            R.MaxX -= L->bounds.MinX;
            R.MaxY -= L->bounds.MinY;
            ClearRectRegion(L->DamageList, &R);
          } 

          /*
             Now I have to change all lobs-entries in the layers
             behind the layer that became visible (_L) so they are
             pointing to the correct layer
           */
          while (NULL != L_tmp -> back)
          {
            L_tmp = internal_WhichLayer(L_tmp->back, 
                                        CR->bounds.MinX, 
                                        CR->bounds.MinY);
            if (NULL == L_tmp)
              break;

            _CR = L_tmp -> ClipRect;
            while (NULL != _CR)
            {
              if (!(CR->bounds.MinX > _CR->bounds.MaxX || 
                    CR->bounds.MinY > _CR->bounds.MaxY ||
                    CR->bounds.MaxX < _CR->bounds.MinX ||
                    CR->bounds.MaxY < _CR->bounds.MinY))
                _CR -> lobs = _L;
              
              _CR = _CR->Next;
            }
	  } /* while */
	}
	else
	{
	  struct ClipRect * CR2 = CR;
	  
	  /*
	    The cliprect to become visible is bigger than the one
	    that is visible right now -> More than one ClipRect needs
	    to be backed up.
	  */
	  
	  if (NULL == _CR)
	  {
	    kprintf("%s:ERROR!!\n",__FUNCTION__);
	    break;
	  }
	  
          /*
          ** First backup all relevant ClipRects and set their lobs
          ** entries to the new layer.
          */
          while (NULL != CR2)
          {
            if (NULL == CR2->lobs &&
                !(CR2->bounds.MinX > _CR->bounds.MaxX || 
                  CR2->bounds.MinY > _CR->bounds.MaxY ||
                  CR2->bounds.MaxX < _CR->bounds.MinX ||
                  CR2->bounds.MaxY < _CR->bounds.MinY))
            {
              if (0 == ( L->Flags & LAYERSIMPLE))
              {
                /* the part to be hidden has to go into a bitmap. */
                if (0 == (L->Flags & LAYERSUPER))
                {
                  CR2->BitMap = 
                      AllocBitMap(CR2->bounds.MaxX - CR2->bounds.MinX + 1 + 16,
                                  CR2->bounds.MaxY - CR2->bounds.MinY + 1,
                                  L->rp->BitMap->Depth,
                                  0,
                                  L->rp->BitMap);
                              
                  BltBitMap(L->rp->BitMap,
                            CR2->bounds.MinX,
                            CR2->bounds.MinY,
                            CR2->BitMap,
                            CR2->bounds.MinX & 0x0f,
                            0,
                            CR2->bounds.MaxX - CR2->bounds.MinX + 1,
                            CR2->bounds.MaxY - CR2->bounds.MinY + 1,
                            0x0c0,
                            0xff,
                            NULL);
                }
                else
                {
                  /* a superbitmap layer */
                  CR2->BitMap = L->SuperBitMap;
                  BltBitMap(L->rp->BitMap,
                            CR2->bounds.MinX,
                            CR2->bounds.MinY,
                            CR2->BitMap,
                            CR2->bounds.MinX - L->bounds.MinX + L->Scroll_X,
                            CR2->bounds.MinY - L->bounds.MinY + L->Scroll_Y,
                            CR2->bounds.MaxX - CR2->bounds.MinX + 1,
                            CR2->bounds.MaxY - CR2->bounds.MinY + 1,
                            0x0c0,
                            0xff,
                            NULL);
                }
              }
              else
              {
                struct Rectangle R = CR2->bounds;
                R.MinX -= L->bounds.MinX;
                R.MinY -= L->bounds.MinY;
                R.MaxX -= L->bounds.MinX;
                R.MaxY -= L->bounds.MinY;
                ClearRectRegion(L->DamageList, &R);
              }
            
              CR2->lobs = _L;
            
              area_hid -= (CR2->bounds.MaxX - CR2->bounds.MinX + 1) *
                          (CR2->bounds.MaxY - CR2->bounds.MinY + 1);
              if (0 == area_hid)
                break;
            
            } /* if */
            CR2 = CR2->Next;
          } /* while */
	
	  
	  /*
	  ** Now make the hidden part visible 
	  */
	  
	  if (0 == (_L->Flags & LAYERSIMPLE))
          {
            if (0 == (_L->Flags & LAYERSUPER))
            {
              BltBitMap(_CR->BitMap,
                        _CR->bounds.MinX & 0x0f,
                        0,
                        _L->rp->BitMap,
                        _CR->bounds.MinX,
                        _CR->bounds.MinY,
                        _CR->bounds.MaxX - _CR->bounds.MinX + 1,
                        _CR->bounds.MaxY - _CR->bounds.MinY + 1,
                        0x0c0,
                        0xff,
                        NULL);
              FreeBitMap(_CR->BitMap);
            }
            else
            {
              /* the part to become visible belongs to a superbitmap layer */
              BltBitMap(_L->SuperBitMap,
                        _CR->bounds.MinX - _L->bounds.MinX + _L->Scroll_X,
                        _CR->bounds.MinY - _L->bounds.MinY + _L->Scroll_Y,
                        _L->rp->BitMap,
                        _CR->bounds.MinX,
                        _CR->bounds.MinY,
                        _CR->bounds.MaxX - _CR->bounds.MinX + 1,
                        _CR->bounds.MaxY - _CR->bounds.MinY + 1,
                        0x0c0,
                        0xff,
                        NULL);
            }
          }
          else
          {
            /* the part to become visible belongs to a simple layer,
               I add that part to the damage list and clear the part. */
            /* The damagelist is relative to the window instead of the
               screen! */
            struct Rectangle R = _CR->bounds;
            R.MinX -= _L->bounds.MinX;
            R.MinY -= _L->bounds.MinY;
            R.MaxX -= _L->bounds.MinX;
            R.MaxY -= _L->bounds.MinY;
            OrRectRegion(_L->DamageList, &R);
            _L->Flags |= LAYERREFRESH;
 
            _CallLayerHook(_L->BackFill,
                           _L->rp,
                           _L,
                           &_CR->bounds,
                           _CR->bounds.MinX,
                           _CR->bounds.MinY
                           );
          }
 
          /*
          ** Mark that part as visible
          */
          _CR -> lobs   = NULL;
          _CR -> BitMap = NULL;

          /*
             Now I have to change all lobs-entries in the layers
             behind the layer that became visible (_L) so they are
             pointing to the correct layer
           */
          while (NULL != L_tmp -> back)
          {
            L_tmp = internal_WhichLayer(L_tmp->back, 
                                        CR->bounds.MinX, 
                                        CR->bounds.MinY);
            if (NULL == L_tmp)
              break;

            CR2 = L_tmp -> ClipRect;
            while (NULL != CR2)
            {
              if (!(CR2->bounds.MinX > _CR->bounds.MaxX || 
                    CR2->bounds.MinY > _CR->bounds.MaxY ||
                    CR2->bounds.MaxX < _CR->bounds.MinX ||
                    CR2->bounds.MaxY < _CR->bounds.MinY))
                CR2 -> lobs = _L;
              CR2 = CR2->Next;
            } /* while (all ClipRects) */
	  } /* while */

	}
      } /* if */
    } /* if */

    CR = CR -> Next;
  } /* while */

  CleanupLayers(LI);

  if (NULL != oldclipregion)
    InstallClipRegion(L, oldclipregion);
  
  InstallClipRegionClipRects(LI);

  UnlockLayers(LI);

  return TRUE;

  AROS_LIBFUNC_EXIT
} /* BehindLayer */
