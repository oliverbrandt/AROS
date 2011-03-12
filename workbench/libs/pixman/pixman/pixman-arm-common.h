/*
 * Copyright © 2010 Nokia Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author:  Siarhei Siamashka (siarhei.siamashka@nokia.com)
 */

#ifndef PIXMAN_ARM_COMMON_H
#define PIXMAN_ARM_COMMON_H

#include "pixman-fast-path.h"

/* Define some macros which can expand into proxy functions between
 * ARM assembly optimized functions and the rest of pixman fast path API.
 *
 * All the low level ARM assembly functions have to use ARM EABI
 * calling convention and take up to 8 arguments:
 *    width, height, dst, dst_stride, src, src_stride, mask, mask_stride
 *
 * The arguments are ordered with the most important coming first (the
 * first 4 arguments are passed to function in registers, the rest are
 * on stack). The last arguments are optional, for example if the
 * function is not using mask, then 'mask' and 'mask_stride' can be
 * omitted when doing a function call.
 *
 * Arguments 'src' and 'mask' contain either a pointer to the top left
 * pixel of the composited rectangle or a pixel color value depending
 * on the function type. In the case of just a color value (solid source
 * or mask), the corresponding stride argument is unused.
 */

#define SKIP_ZERO_SRC  1
#define SKIP_ZERO_MASK 2

#define PIXMAN_ARM_BIND_FAST_PATH_SRC_DST(cputype, name,                \
                                          src_type, src_cnt,            \
                                          dst_type, dst_cnt)            \
void                                                                    \
pixman_composite_##name##_asm_##cputype (int32_t   w,                   \
                                         int32_t   h,                   \
                                         dst_type *dst,                 \
                                         int32_t   dst_stride,          \
                                         src_type *src,                 \
                                         int32_t   src_stride);         \
                                                                        \
static void                                                             \
cputype##_composite_##name (pixman_implementation_t *imp,               \
                            pixman_op_t              op,                \
                            pixman_image_t *         src_image,         \
                            pixman_image_t *         mask_image,        \
                            pixman_image_t *         dst_image,         \
                            int32_t                  src_x,             \
                            int32_t                  src_y,             \
                            int32_t                  mask_x,            \
                            int32_t                  mask_y,            \
                            int32_t                  dest_x,            \
                            int32_t                  dest_y,            \
                            int32_t                  width,             \
                            int32_t                  height)            \
{                                                                       \
    dst_type *dst_line;                                                 \
    src_type *src_line;                                                 \
    int32_t dst_stride, src_stride;                                     \
                                                                        \
    PIXMAN_IMAGE_GET_LINE (src_image, src_x, src_y, src_type,           \
                           src_stride, src_line, src_cnt);              \
    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, dst_type,         \
                           dst_stride, dst_line, dst_cnt);              \
                                                                        \
    pixman_composite_##name##_asm_##cputype (width, height,             \
                                             dst_line, dst_stride,      \
                                             src_line, src_stride);     \
}

#define PIXMAN_ARM_BIND_FAST_PATH_N_DST(flags, cputype, name,           \
                                        dst_type, dst_cnt)              \
void                                                                    \
pixman_composite_##name##_asm_##cputype (int32_t    w,                  \
                                         int32_t    h,                  \
                                         dst_type  *dst,                \
                                         int32_t    dst_stride,         \
                                         uint32_t   src);               \
                                                                        \
static void                                                             \
cputype##_composite_##name (pixman_implementation_t *imp,               \
                            pixman_op_t              op,                \
                            pixman_image_t *         src_image,         \
                            pixman_image_t *         mask_image,        \
                            pixman_image_t *         dst_image,         \
                            int32_t                  src_x,             \
                            int32_t                  src_y,             \
                            int32_t                  mask_x,            \
                            int32_t                  mask_y,            \
                            int32_t                  dest_x,            \
                            int32_t                  dest_y,            \
                            int32_t                  width,             \
                            int32_t                  height)            \
{                                                                       \
    dst_type  *dst_line;                                                \
    int32_t    dst_stride;                                              \
    uint32_t   src;                                                     \
                                                                        \
    src = _pixman_image_get_solid (					\
	imp, src_image, dst_image->bits.format);			\
                                                                        \
    if ((flags & SKIP_ZERO_SRC) && src == 0)                            \
	return;                                                         \
                                                                        \
    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, dst_type,         \
                           dst_stride, dst_line, dst_cnt);              \
                                                                        \
    pixman_composite_##name##_asm_##cputype (width, height,             \
                                             dst_line, dst_stride,      \
                                             src);                      \
}

#define PIXMAN_ARM_BIND_FAST_PATH_N_MASK_DST(flags, cputype, name,      \
                                             mask_type, mask_cnt,       \
                                             dst_type, dst_cnt)         \
void                                                                    \
pixman_composite_##name##_asm_##cputype (int32_t    w,                  \
                                         int32_t    h,                  \
                                         dst_type  *dst,                \
                                         int32_t    dst_stride,         \
                                         uint32_t   src,                \
                                         int32_t    unused,             \
                                         mask_type *mask,               \
                                         int32_t    mask_stride);       \
                                                                        \
static void                                                             \
cputype##_composite_##name (pixman_implementation_t *imp,               \
                            pixman_op_t              op,                \
                            pixman_image_t *         src_image,         \
                            pixman_image_t *         mask_image,        \
                            pixman_image_t *         dst_image,         \
                            int32_t                  src_x,             \
                            int32_t                  src_y,             \
                            int32_t                  mask_x,            \
                            int32_t                  mask_y,            \
                            int32_t                  dest_x,            \
                            int32_t                  dest_y,            \
                            int32_t                  width,             \
                            int32_t                  height)            \
{                                                                       \
    dst_type  *dst_line;                                                \
    mask_type *mask_line;                                               \
    int32_t    dst_stride, mask_stride;                                 \
    uint32_t   src;                                                     \
                                                                        \
    src = _pixman_image_get_solid (					\
	imp, src_image, dst_image->bits.format);			\
                                                                        \
    if ((flags & SKIP_ZERO_SRC) && src == 0)                            \
	return;                                                         \
                                                                        \
    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, dst_type,         \
                           dst_stride, dst_line, dst_cnt);              \
    PIXMAN_IMAGE_GET_LINE (mask_image, mask_x, mask_y, mask_type,       \
                           mask_stride, mask_line, mask_cnt);           \
                                                                        \
    pixman_composite_##name##_asm_##cputype (width, height,             \
                                             dst_line, dst_stride,      \
                                             src, 0,                    \
                                             mask_line, mask_stride);   \
}

#define PIXMAN_ARM_BIND_FAST_PATH_SRC_N_DST(flags, cputype, name,       \
                                            src_type, src_cnt,          \
                                            dst_type, dst_cnt)          \
void                                                                    \
pixman_composite_##name##_asm_##cputype (int32_t    w,                  \
                                         int32_t    h,                  \
                                         dst_type  *dst,                \
                                         int32_t    dst_stride,         \
                                         src_type  *src,                \
                                         int32_t    src_stride,         \
                                         uint32_t   mask);              \
                                                                        \
static void                                                             \
cputype##_composite_##name (pixman_implementation_t *imp,               \
                            pixman_op_t              op,                \
                            pixman_image_t *         src_image,         \
                            pixman_image_t *         mask_image,        \
                            pixman_image_t *         dst_image,         \
                            int32_t                  src_x,             \
                            int32_t                  src_y,             \
                            int32_t                  mask_x,            \
                            int32_t                  mask_y,            \
                            int32_t                  dest_x,            \
                            int32_t                  dest_y,            \
                            int32_t                  width,             \
                            int32_t                  height)            \
{                                                                       \
    dst_type  *dst_line;                                                \
    src_type  *src_line;                                                \
    int32_t    dst_stride, src_stride;                                  \
    uint32_t   mask;                                                    \
                                                                        \
    mask = _pixman_image_get_solid (					\
	imp, mask_image, dst_image->bits.format);			\
                                                                        \
    if ((flags & SKIP_ZERO_MASK) && mask == 0)                          \
	return;                                                         \
                                                                        \
    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, dst_type,         \
                           dst_stride, dst_line, dst_cnt);              \
    PIXMAN_IMAGE_GET_LINE (src_image, src_x, src_y, src_type,           \
                           src_stride, src_line, src_cnt);              \
                                                                        \
    pixman_composite_##name##_asm_##cputype (width, height,             \
                                             dst_line, dst_stride,      \
                                             src_line, src_stride,      \
                                             mask);                     \
}

#define PIXMAN_ARM_BIND_FAST_PATH_SRC_MASK_DST(cputype, name,           \
                                               src_type, src_cnt,       \
                                               mask_type, mask_cnt,     \
                                               dst_type, dst_cnt)       \
void                                                                    \
pixman_composite_##name##_asm_##cputype (int32_t    w,                  \
                                         int32_t    h,                  \
                                         dst_type  *dst,                \
                                         int32_t    dst_stride,         \
                                         src_type  *src,                \
                                         int32_t    src_stride,         \
                                         mask_type *mask,               \
                                         int32_t    mask_stride);       \
                                                                        \
static void                                                             \
cputype##_composite_##name (pixman_implementation_t *imp,               \
                            pixman_op_t              op,                \
                            pixman_image_t *         src_image,         \
                            pixman_image_t *         mask_image,        \
                            pixman_image_t *         dst_image,         \
                            int32_t                  src_x,             \
                            int32_t                  src_y,             \
                            int32_t                  mask_x,            \
                            int32_t                  mask_y,            \
                            int32_t                  dest_x,            \
                            int32_t                  dest_y,            \
                            int32_t                  width,             \
                            int32_t                  height)            \
{                                                                       \
    dst_type  *dst_line;                                                \
    src_type  *src_line;                                                \
    mask_type *mask_line;                                               \
    int32_t    dst_stride, src_stride, mask_stride;                     \
                                                                        \
    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, dst_type,         \
                           dst_stride, dst_line, dst_cnt);              \
    PIXMAN_IMAGE_GET_LINE (src_image, src_x, src_y, src_type,           \
                           src_stride, src_line, src_cnt);              \
    PIXMAN_IMAGE_GET_LINE (mask_image, mask_x, mask_y, mask_type,       \
                           mask_stride, mask_line, mask_cnt);           \
                                                                        \
    pixman_composite_##name##_asm_##cputype (width, height,             \
                                             dst_line, dst_stride,      \
                                             src_line, src_stride,      \
                                             mask_line, mask_stride);   \
}

#define PIXMAN_ARM_BIND_SCALED_NEAREST_SRC_DST(cputype, name, op,             \
                                               src_type, dst_type)            \
void                                                                          \
pixman_scaled_nearest_scanline_##name##_##op##_asm_##cputype (                \
                                                       int32_t        w,      \
                                                       dst_type *     dst,    \
                                                       src_type *     src,    \
                                                       pixman_fixed_t vx,     \
                                                       pixman_fixed_t unit_x);\
                                                                              \
static force_inline void                                                      \
scaled_nearest_scanline_##cputype##_##name##_##op (dst_type *       pd,       \
                                                   src_type *       ps,       \
                                                   int32_t          w,        \
                                                   pixman_fixed_t   vx,       \
                                                   pixman_fixed_t   unit_x,   \
                                                   pixman_fixed_t   max_vx)   \
{                                                                             \
    pixman_scaled_nearest_scanline_##name##_##op##_asm_##cputype (w, pd, ps,  \
                                                                  vx, unit_x);\
}                                                                             \
                                                                              \
FAST_NEAREST_MAINLOOP (cputype##_##name##_cover_##op,                         \
                       scaled_nearest_scanline_##cputype##_##name##_##op,     \
                       src_type, dst_type, COVER)                             \
FAST_NEAREST_MAINLOOP (cputype##_##name##_none_##op,                          \
                       scaled_nearest_scanline_##cputype##_##name##_##op,     \
                       src_type, dst_type, NONE)                              \
FAST_NEAREST_MAINLOOP (cputype##_##name##_pad_##op,                           \
                       scaled_nearest_scanline_##cputype##_##name##_##op,     \
                       src_type, dst_type, PAD)

/* Provide entries for the fast path table */
#define PIXMAN_ARM_SIMPLE_NEAREST_FAST_PATH(op,s,d,func)                      \
    SIMPLE_NEAREST_FAST_PATH_COVER (op,s,d,func),                             \
    SIMPLE_NEAREST_FAST_PATH_NONE (op,s,d,func),                              \
    SIMPLE_NEAREST_FAST_PATH_PAD (op,s,d,func)

#endif
