/*
    Copyright � 2011-2017, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Intel 8259A "Legacy" PIC driver.
*/

#include <proto/exec.h>

#include <asm/io.h>
#include <inttypes.h>

#include "kernel_base.h"
#include "kernel_debug.h"
#include "kernel_debug.h"
#include "kernel_interrupts.h"
#include "i8259a.h"

#define D(x)

struct i8259a_Private
{
    ULONG       irq_mask[0];
};

icid_t i8259a_Register(struct KernelBase *KernelBase)
{
    D(bug("[Kernel:i8259a] %s()\n", __func__));

    /* if we have been disabled, fail to register */
    if (i8259a_IntrController.ic_Flags & ICF_DISABLED)
        return -1;

    i8259a_IntrController.ic_Flags |= ICF_ACKENABLE;

    return (icid_t)i8259a_IntrController.ic_Node.ln_Type;
}

BOOL i8259a_Init(struct KernelBase *KernelBase, icid_t instanceCount)
{
    ULONG *maskarray;
    int i;

    D(bug("[Kernel:i8259a] %s(%d)\n", __func__, instanceCount));

    /* sanity check .. */
    if (i8259a_IntrController.ic_Flags & ICF_DISABLED)
        return FALSE;

    maskarray = AllocMem(sizeof(ULONG) * instanceCount, MEMF_ANY);
    if ((i8259a_IntrController.ic_Private = maskarray) != NULL)
    {
        /* Take over the 8259a IRQ's */
        for (i = 0; i < IRQ_COUNT; i++)
        {
            if (KernelBase->kb_Interrupts[i].lh_Type == KBL_INTERNAL)
            {
                KernelBase->kb_Interrupts[i].lh_Type = i8259a_IntrController.ic_Node.ln_Type;
                KernelBase->kb_Interrupts[i].l_pad = 0;      /* Initially set to first instance */
            }
        }

        /*
         * After initialization everything is disabled except cascade interrupt (IRQ 2).
         * Be careful and don't miss this! Without IRQ2 enabled slave 8259 (and its IRQs)
         * will not work!
         */

        /* Setup Masks */
        for (i = 0; i < instanceCount; i++)
        {
            maskarray[i] = 0xFFFB;
        }

        /* Setup the 8259. Send four ICWs (see 8529 datasheet) */
        asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x11),"i"(MASTER8259_CMDREG)); /* Initialization sequence for 8259A-1 (edge-triggered, cascaded, ICW4 needed) */
        asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x11),"i"(SLAVE8259_CMDREG)); /* Initialization sequence for 8259A-2, the same as above */
        asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x20),"i"(MASTER8259_MASKREG)); /* IRQs for master at 0x20 - 0x27 */
        asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x28),"i"(SLAVE8259_MASKREG)); /* IRQs for slave at 0x28 - 0x2f */
        asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x04),"i"(MASTER8259_MASKREG)); /* 8259A-1 is master, slave at IRQ2 */
        asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x02),"i"(SLAVE8259_MASKREG)); /* 8259A-2 is slave, ID = 2 */
        asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x01),"i"(MASTER8259_MASKREG)); /* 8086 mode, non-buffered, nonspecial fully nested mode for both */
        asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0x01),"i"(SLAVE8259_MASKREG));
        
        /* Now initialize interrupt masks */
        asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0xfb),"i"(MASTER8259_MASKREG)); /* Enable cascade int */
        asm("outb   %b0,%b1\n\tcall delay"::"a"((char)0xff),"i"(SLAVE8259_MASKREG)); /* Mask all interrupts */

        return TRUE;
    }
    return FALSE;
}

/* Small delay routine used by i8259a_Init() initializer */
asm("\ndelay:\t.short   0x00eb\n\tret");

BOOL i8259a_DisableIRQ(APTR icPrivate, icid_t icInstance, icid_t intNum)
{
    ULONG *maskarray = (ULONG *)icPrivate;

    D(bug("[Kernel:i8259a] %s()\n", __func__));

    intNum &= 15;

    if (intNum == 2)
    {
    	/* IRQ2 must never be disabled. Doing so breaks communication between two 8259's */
    	return FALSE;
    }

    maskarray[icInstance] |= 1 << intNum;

    if (intNum >= 8)
        outb((maskarray[icInstance] >> 8) & 0xff, SLAVE8259_MASKREG);
    else
        outb(maskarray[icInstance] & 0xff, MASTER8259_MASKREG);

    return TRUE;
}

BOOL i8259a_EnableIRQ(APTR icPrivate, icid_t icInstance, icid_t intNum) // uint16_t *irqmask)
{
    ULONG *maskarray = (ULONG *)icPrivate;

    D(bug("[Kernel:i8259a] %s()\n", __func__));

    intNum &= 15;
    maskarray[icInstance] &= ~(1 << intNum);    

    if (intNum >= 8)
        outb((maskarray[icInstance] >> 8) & 0xff, SLAVE8259_MASKREG);
    else
        outb(maskarray[icInstance] & 0xff, MASTER8259_MASKREG);

    return TRUE;
}

/*
 * Careful! The 8259A is a fragile beast, it pretty much _has_ to be done exactly like this (mask it
 * first, _then_ send the EOI, and the order of EOI to the two 8259s is important!
 */
BOOL i8259a_AckIntr(APTR icPrivate, icid_t icInstance, icid_t intNum) // uint16_t *irqmask)
{
    ULONG *maskarray = (ULONG *)icPrivate;

    D(bug("[Kernel:i8259a] %s()\n", __func__));

    intNum &= 15;
    maskarray[icInstance] |= 1 << intNum;

    if (intNum >= 8)
    {
        outb((maskarray[icInstance] >> 8) & 0xff, SLAVE8259_MASKREG);
        outb(0x62, MASTER8259_CMDREG);
        outb(0x20, SLAVE8259_CMDREG);
    }
    else
    {
        outb(maskarray[icInstance] & 0xff, MASTER8259_MASKREG);
        outb(0x20, MASTER8259_CMDREG);
    }

    return TRUE;
}

struct IntrController i8259a_IntrController =
{
    {
        .ln_Name = "8259A PIC"
    },
    ICTYPE_I8259A,
    0,
    NULL,
    i8259a_Register,
    i8259a_Init,
    i8259a_EnableIRQ,
    i8259a_DisableIRQ,
    i8259a_AckIntr,
};

BOOL i8259a_Probe()
{
    UBYTE maskres;

    D(bug("[Kernel:i8259a] %s()\n", __func__));

    /* mask all of the interrupts except the cascade pin */
    outb(0xff, SLAVE8259_MASKREG);      
    outb(~(1 << 2), MASTER8259_MASKREG);
    maskres = inb(MASTER8259_MASKREG);
    if (maskres == ~(1 << 2))
        return TRUE;
    return FALSE;
}
