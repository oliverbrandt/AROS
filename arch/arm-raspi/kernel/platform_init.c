/*
    Copyright � 2013-2015, The AROS Development Team. All rights reserved.
    $Id$
*/

#define DEBUG 0

#include <aros/kernel.h>
#include <aros/symbolsets.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/alerts.h>
#include <exec/execbase.h>
#include <asm/io.h>
#include <proto/kernel.h>
#include <proto/exec.h>
#include <strings.h>

#include "etask.h"

#include "kernel_intern.h"
#include "kernel_arch.h"
#include "kernel_romtags.h"
#include "kernel_arm.h"

THIS_PROGRAM_HANDLES_SYMBOLSET(ARMPLATFORMS)
DEFINESET(ARMPLATFORMS)

void platform_Init(struct KernelBase *KernelBase)
{
    IPTR (*platprobe) (struct KernelBase *);
    int cur = 0;

    for ( ; ((long *)SETNAME(ARMPLATFORMS)[cur]) != NULL; cur++)
    {
        platprobe = SETNAME(ARMPLATFORMS)[cur];
        if (platprobe(KernelBase))
            break;
    }

    return;
}

void *(*__AllocMem)();

#define ExecAllocMem(bytesize, requirements)        \
AROS_CALL2(void *, __AllocMem,                      \
		AROS_LCA(ULONG, byteSize,     D0),			\
		AROS_LCA(ULONG, requirements, D1),			\
		struct ExecBase *, SysBase)

AROS_LH2(APTR, AllocMem,
	AROS_LHA(ULONG, byteSize,     D0),
	AROS_LHA(ULONG, requirements, D1),
	struct ExecBase *, SysBase, 33, Kernel)
{
    AROS_LIBFUNC_INIT

    if (requirements & MEMF_CHIP)
    {
        DALLOCMEM(bug("[Kernel] AllocMem: Ignoring MEMF_CHIP flag\n"));
        requirements &= ~MEMF_CHIP;
    }
    return ExecAllocMem(bytesize, requirements);

    AROS_LIBFUNC_EXIT
}

static int platform_PostInit(struct KernelBase *KernelBase)
{
    UBYTE *ptr;
    D(bug("[Kernel] platform_PostInit()\n"));

    D(bug("[Kernel] platform_PostInit: Patching in our AllocMem to ignore MEMF_CHIP..\n"));
    
    __AllocMem = SetFunction(SysBase, -33*LIB_VECTSIZE, AROS_SLIB_ENTRY(AllocMem, Kernel, 33));

    D(bug("[Kernel] platform_PostInit: Registering Heartbeat timer..\n"));

    KrnAddSysTimerHandler(KernelBase);

    D(bug("[Kernel] platform_PostInit: Done..\n"));

    return TRUE;
}

ADD2INITLIB(platform_PostInit, 0)

struct KernelBase *getKernelBase()
{
    return (struct KernelBase *)KernelBase;
}
