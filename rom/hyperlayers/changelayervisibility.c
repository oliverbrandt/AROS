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
	AROS_LH2(LONG, ChangeLayerVisibility,

/*  SYNOPSIS */
	AROS_LHA(struct Layer *, l      , A0),
	AROS_LHA(int           , visible, D0),

/*  LOCATION */
	struct LayersBase *, LayersBase, 39, Layers)

/*  FUNCTION
       Makes the given layer visible or invisible.
       If it is a simple refresh layer it will loose all its
       cliprects and therefore rendering will go into the void.

    INPUTS
       L       - pointer to layer 
       visible - TRUE or FALSE

    RESULT
       TRUE  - layer was changed to new state
       FALSE - layer could not be changed to new state
  
    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
  AROS_LIBFUNC_INIT
  AROS_LIBBASE_EXT_DECL(struct LayersBase *,LayersBase)

  struct Layer * _l, * lparent;
  struct Region rtmp;
  rtmp.RegionRectangle = NULL;

  if (l->visible == visible)
    return TRUE;

  LockLayers(l->LayerInfo);

  l->visible = visible;
  
  if (TRUE == visible)
  {
    /*
     * Make the layer visible
     * Back up all layers behind this layer.
     */
    lparent = l->parent;
    _l = l->back;
    while (1)
    {
      if (IS_VISIBLE(l) && DO_OVERLAP(&l->shape->bounds, &_l->shape->bounds))
        _BackupPartsOfLayer(_l, l->shape, 0, FALSE, LayersBase);
      
      if (_l == lparent)
      {
        if (IS_VISIBLE(_l) || (NULL == lparent->parent))
          break;
        else
          lparent = lparent->parent;
      }
      _l = _l->back;
    }
    
    /*
     * For the layer to become visible I must recalculate its
     * visible area.
     */
    if (l->front)
    {
      _SetRegion(l->front->VisibleRegion, l->VisibleRegion);
      _SetRegion(l->front->shape, &rtmp);
      AndRegionRegion(l->front->parent->shape, &rtmp);
      ClearRegionRegion(&rtmp, l->VisibleRegion);
    }
    else
    {
      /*
       * This is the frontmost layer...
       */
      _SetRegion(l->LayerInfo->check_lp->shape, l->VisibleRegion);
    }
    /*
     * Let me show the layer in its full beauty...
     */
    _ShowPartsOfLayer(l, l->VisibleRegion, LayersBase);
  }
  else
  {
    struct Region r, clearr;
    r.RegionRectangle = NULL; // min. initialization
    _SetRegion(l->VisibleRegion, &r);
    
    clearr.RegionRectangle = NULL; // min. initialization
    _SetRegion(l->shape, &clearr);
    /*
     * Make the layer invisible
     */
    _BackupPartsOfLayer(l, l->shape, 0, FALSE, LayersBase);
    
    /*
     * Walk through all the layers behind this layer and
     * make them (more) visible...
     */
    lparent = l->parent;
    _l = l->back;
    while (1)
    {
      if (IS_VISIBLE(_l) || NULL == lparent->parent)
      {
        ClearRegion(l->VisibleRegion);
        _ShowPartsOfLayer(l, &r, LayersBase);
      }
      else
        _SetRegion(&r, l->VisibleRegion);

      if (IS_VISIBLE(_l) || IS_ROOTLAYER(_l))
        AndRegionRegion(_l->VisibleRegion, &clearr);

      if (_l == lparent)
      {
        if (IS_VISIBLE(_l) || (NULL == lparent->parent))
          break;
        else
          lparent = lparent->parent;
      }
      
      /*
       * Take the shape of the current layer out of
       * the visible region that will be applied to the
       * layer behind this one.
       */
      if (IS_VISIBLE(_l))
      {
        _SetRegion(_l->shape, &rtmp);
        AndRegionRegion(_l->parent->shape, &rtmp);
        ClearRegionRegion(&rtmp, &r);
      }
      _l = _l->back;
    }
    ClearRegion(&r);
  
    if (!IS_EMPTYREGION(&clearr))
    {
      if (lparent &&
          (IS_SIMPLEREFRESH(lparent) || IS_ROOTLAYER(lparent)))
        _BackFillRegion(lparent, &clearr, FALSE);
    }
    
    ClearRegion(&clearr);
  }

  ClearRegion(&rtmp);
  UnlockLayers(l->LayerInfo);

  return TRUE;

  AROS_LIBFUNC_EXIT
} /* ChangeLayerVisibility */
