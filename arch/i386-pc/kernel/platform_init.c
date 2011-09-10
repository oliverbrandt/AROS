#include <aros/symbolsets.h>
#include <asm/cpu.h>
#include <exec/execbase.h>
#include <proto/exec.h>

#include "kernel_base.h"
#include "kernel_debug.h"
#include "kernel_intern.h"
#include "traps.h"
#include "utils.h"
#include "xtpic.h"

#define D(x) x

static int PlatformInit(struct KernelBase *KernelBase)
{
    struct PlatformData *data;
    struct table_desc idtr;

    data = AllocMem(sizeof(struct PlatformData), MEMF_PUBLIC);
    if (!data)
	return FALSE;

    D(bug("[Kernel] Allocated platform data at 0x%p\n", data));
    KernelBase->kb_PlatformData = data;

    /*
     * Now we have a complete memory list and working AllocMem().
     * We can allocate space for IDT and TSS now and build them to make
     * interrupts working.
     */
    data->tss            = krnAllocMemAligned(sizeof(struct tss), 64);
    data->idt            = krnAllocMemAligned(sizeof(long long) * 256, 256);
    SysBase->SysStkLower = AllocMem(0x10000, MEMF_PUBLIC);  /* 64KB of system stack */

    if ((!data->tss) || (!data->idt) || (!SysBase->SysStkLower))
	return FALSE;

    data->tss->ssp_seg = KERNEL_DS; /* SSP segment descriptor */
    data->tss->cs      = USER_CS;
    data->tss->ds      = USER_DS;
    data->tss->es      = USER_DS;
    data->tss->ss      = USER_DS;
    data->tss->iomap   = 104;

    /* Set up system stack */
    SysBase->SysStkUpper = SysBase->SysStkLower + 0x10000;
    data->tss->ssp       = (IPTR)SysBase->SysStkUpper;

    /* Restore IDT structure */
    Init_Traps(data);

    /* Set correct TSS address in the GDT */
    GDT[6].base_low  = ((unsigned long)data->tss) & 0xffff;
    GDT[6].base_mid  = (((unsigned long)data->tss) >> 16) & 0xff;
    GDT[6].base_high = (((unsigned long)data->tss) >> 24) & 0xff;

    /*
     * As we prepared all necessary stuff, we can hopefully load IDT
     * into CPU. We may also play a bit with TSS
     */
    idtr.size = 0x07FF;
    idtr.base = (unsigned long)data->idt;
    asm
    (
	"lidt %0\n\t"
	"ltr %%ax\n\t"
	::"m"(idtr),"ax"(0x30)
    );

    D(bug("System restored\n"));
    
    /* Initialize our PIC */
    XTPIC_Init(&data->xtpic_mask);

    return TRUE;
}

ADD2INITLIB(PlatformInit, 10);
