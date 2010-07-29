#include <aros/symbolsets.h>
#include <proto/exec.h>

#include "kernel_base.h"

static int Kernel_Init(struct KernelBase *KernelBase)
{
    int i;

    for (i=0; i < EXCEPTIONS_COUNT; i++)
	NEWLIST(&KernelBase->kb_Exceptions[i]);

    for (i=0; i < IRQ_COUNT; i++)
        NEWLIST(&KernelBase->kb_Interrupts[i]);

    NEWLIST(&KernelBase->kb_Modules);
    InitSemaphore(&KernelBase->kb_ModSem);

    return 1;
}

ADD2INITLIB(Kernel_Init, 0)
