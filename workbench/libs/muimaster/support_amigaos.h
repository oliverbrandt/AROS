/*
    Copyright � 2003, The AROS Development Team. All rights reserved.
    $Id$
*/

#ifndef _MUIMASTER_SUPPORT_AMIGAOS_H_
#define _MUIMASTER_SUPPORT_AMIGAOS_H_

#include <dos.h>

#define AROS_LONG2BE(x) (x)

char *StrDup(char *x);
int snprintf(char *buf, int size, const char *fmt, ...);
int strlcat(char *buf, char *src, int len);

/*** AROS Intuition extensions **********************************************/
#define DeinitRastPort(rp)      
#define CloneRastPort(rp) (rp)  
#define FreeRastPort(rp)        

/*** Miscellanous compiler supprot ******************************************/
#ifdef __MAXON__
#   define __asm
#   define __inline
#   define SAVEDS
#   define const
#else
#   define SAVEDS __saveds
#endif 

/*** Miscellanous AROS macros ***********************************************/
#define AROS_LIBFUNC_INIT
#define AROS_LIBBASE_EXT_DECL(a, b) extern a b;
#define AROS_LIBFUNC_EXIT

/*** AROS types *************************************************************/
#ifndef __AROS_TYPES_DEFINED__
#   define __AROS_TYPES_DEFINED__
    typedef unsigned long IPTR;
    typedef long          STACKLONG;
    typedef unsigned long STACKULONG;
#endif /* __AROS_TYPES_DEFINED__ */

/*** AROS register definitions **********************************************/
#define __REG_D0 __d0
#define __REG_D1 __d1
#define __REG_D2 __d2
#define __REG_D3 __d3
#define __REG_D4 __d4
#define __REG_D5 __d5
#define __REG_D6 __d6
#define __REG_D7 __d7
#define __REG_A0 __a0
#define __REG_A1 __a1
#define __REG_A2 __a2
#define __REG_A3 __a3
#define __REG_A4 __a4
#define __REG_A5 __a5
#define __REG_A6 __a6
#define __REG_A7 __a7

/*** AROS library function macros *******************************************/
#define AROS_LH0(rt, fn, bt, bn, lvo, p) \
    __asm rt fn (void)
#define AROS_LH1(rt, fn, a1, bt, bn, lvo, p) \
    __asm rt fn (a1)
#define AROS_LH2(rt, fn, a1, a2, bt, bn, lvo, p) \
    __asm rt fn (a1, a2)
#define AROS_LH3(rt, fn, a1, a2, a3, bt, bn, lvo, p) \
    __asm rt fn (a1, a2, a3)
#define AROS_LH4(rt, fn, a1, a2, a3, a4, bt, bn, lvo, p) \
    __asm rt fn (a1, a2, a3, a4)
#define AROS_LH5(rt, fn, a1, a2, a3, a4, a5, bt, bn, lvo, p) \
    __asm rt fn (a1, a2, a3, a4, a5)
#define AROS_LH6(rt, fn, a1, a2, a3, a4, a5, a6, bt, bn, lvo, p) \
    __asm rt fn (a1, a2, a3, a4, a5, a6)
#define AROS_LH7(rt, fn, a1, a2, a3, a4, a5, a6, a7, bt, bn, lvo, p) \
    __asm rt fn (a1, a2, a3, a4, a5, a6, a7)
#define AROS_LH8(rt, fn, a1, a2, a3, a4, a5, a6, a7, a8, bt, bn, lvo, p) \
    __asm rt fn (a1, a2, a3, a4, a5, a6, a7, a8)

#define AROS_LHA(type, name, reg) register __REG_##reg type name

/*** AROS user function macros **********************************************/
#define AROS_UFH0(rt, fn) \
    __asm rt fn (void)
#define AROS_UFH1(rt, fn, a1) \
    __asm rt fn (a1)
#define AROS_UFH2(rt, fn, a1, a2) \
    __asm rt fn (a1, a2)
#define AROS_UFH3(rt, fn, a1, a2, a3) \
    __asm rt fn (a1, a2, a3)
#define AROS_UFH4(rt, fn, a1, a2, a3, a4) \
    __asm rt fn (a1, a2, a3, a4)
#define AROS_UFH5(rt, fn, a1, a2, a3, a4, a5) \
    __asm rt fn (a1, a2, a3, a4, a5)
#define AROS_UFH6(rt, fn, a1, a2, a3, a4, a5, a6) \
    __asm rt fn (a1, a2, a3, a4, a5, a6)
#define AROS_UFH7(rt, fn, a1, a2, a3, a4, a5, a6, a7) \
    __asm rt fn (a1, a2, a3, a4, a5, a6, a7)
#define AROS_UFH8(rt, fn, a1, a2, a3, a4, a5, a6, a7, a8) \
    __asm rt fn (a1, a2, a3, a4, a5, a6, a7, a8)

#define AROS_UFH0S(rt, fn) \
    __asm static rt fn (void)
#define AROS_UFH1S(rt, fn, a1) \
    __asm static rt fn (a1)
#define AROS_UFH2S(rt, fn, a1, a2) \
    __asm static rt fn (a1, a2)
#define AROS_UFH3S(rt, fn, a1, a2, a3) \
    __asm static rt fn (a1, a2, a3)
#define AROS_UFH4S(rt, fn, a1, a2, a3, a4) \
    __asm static rt fn (a1, a2, a3, a4)
#define AROS_UFH5S(rt, fn, a1, a2, a3, a4, a5) \
    __asm static rt fn (a1, a2, a3, a4, a5)
#define AROS_UFH6S(rt, fn, a1, a2, a3, a4, a5, a6) \
    __asm static rt fn (a1, a2, a3, a4, a5, a6)
#define AROS_UFH7S(rt, fn, a1, a2, a3, a4, a5, a6, a7) \
    __asm static rt fn (a1, a2, a3, a4, a5, a6, a7)
#define AROS_UFH8S(rt, fn, a1, a2, a3, a4, a5, a6, a7, a8) \
    __asm static rt fn (a1, a2, a3, a4, a5, a6, a7, a8)

#define AROS_UFHA(type, name, reg) register __REG_##reg type name

#define AROS_UFP0 AROS_UFH0
#define AROS_UFP1 AROS_UFH1
#define AROS_UFP2 AROS_UFH2
#define AROS_UFP3 AROS_UFH3
#define AROS_UFP4 AROS_UFH4
#define AROS_UFP5 AROS_UFH5
#define AROS_UFP6 AROS_UFH6
#define AROS_UFP7 AROS_UFH7
#define AROS_UFP8 AROS_UFH8

#define AROS_UFPA AROS_UFHA

#endif /* _MUIMASTER_SUPPORT_AMIGAOS_H_ */
