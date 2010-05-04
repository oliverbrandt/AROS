/*
 * Copyright (c) 2007-2008 Tungsten Graphics, Inc., Cedar Park, Texas.
 * Copyright (c) 2007-2008 Dave Airlie <airlied@linux.ie>
 * Copyright (c) 2007-2008 Jakob Bornecrantz <wallbraker@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include <drm.h>

typedef struct _drmModeModeInfo {
    uint32_t clock;
    uint16_t hdisplay, hsync_start, hsync_end, htotal, hskew;
    uint16_t vdisplay, vsync_start, vsync_end, vtotal, vscan;

    uint32_t vrefresh; /* vertical refresh * 1000 */

    uint32_t flags;
    uint32_t type;
    char name[DRM_DISPLAY_MODE_LEN];
} drmModeModeInfo, *drmModeModeInfoPtr;

/* Add passed bo (handle) to list of frame buffers */
extern int drmModeAddFB(int fd, uint32_t width, uint32_t height, uint8_t depth,
            uint8_t bpp, uint32_t pitch, uint32_t bo_handle,
            uint32_t *buf_id);

/* Set a specified crtc and outputs to given mode using bufferId framebuffer */
extern int drmModeSetCrtc(int fd, uint32_t crtcId, uint32_t bufferId,
            uint32_t x, uint32_t y, uint32_t *connectors, int count,
            drmModeModeInfoPtr mode);


