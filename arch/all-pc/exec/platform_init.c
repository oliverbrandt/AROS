/*
    Copyright � 2017, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <aros/config.h>

#define DEBUG 0
#include <aros/debug.h>

#include <proto/exec.h>

#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/interrupts.h>
#include <exec/rawfmt.h>

#define __KERNEL_NOLIBBASE__
#include <proto/kernel.h>

#include "kernel_base.h"
#include "kernel_arch.h"
#include "kernel_intr.h"

#include "etask.h"

#define __AROS_KERNEL__

#include "exec_intern.h"

extern void IdleTask(struct ExecBase *);
extern AROS_INTP(Exec_X86ShutdownHandler);
extern AROS_INTP(Exec_X86WarmResetHandler);
extern AROS_INTP(Exec_X86ColdResetHandler);

#if (__WORDSIZE==64)
#define EXCX_REGA    regs->rax
#define EXCX_REGB    regs->rbx
#else
#define EXCX_REGA    regs->eax
#define EXCX_REGB    regs->ebx
#endif

#if defined(__AROSEXEC_SMP__)
struct Hook Exec_TaskSpinLockFailHook;
struct Hook Exec_TaskSpinLockForbidHook;
struct Hook Exec_TaskSpinLockDisableHook;

AROS_UFH3(void, Exec_TaskSpinLockFailFunc,
    AROS_UFHA(struct Hook *, h, A0),
    AROS_UFHA(spinlock_t *, spinLock, A1),
    AROS_UFHA(void *, unused, A2))
{
    AROS_USERFUNC_INIT

    struct Task *spinTask = GET_THIS_TASK;
    struct IntETask *thisET;

    D(bug("[Exec:X86] %s()\n", __func__));
    thisET = GetIntETask(spinTask);
    if (thisET)
    {
        D(bug("[Exec:X86] %s: Setting task @ 0x%p to spinning...\n", __func__, spinTask));

        /* tell the scheduler that the task is waiting on a spinlock */
        spinTask->tc_State = TS_SPIN;

        thisET->iet_SpinLock = spinLock;

    }
    D(bug("[Exec:X86] %s: Forcing Reschedule...\n", __func__));

    /* schedule us away for now .. */
    Reschedule();

    AROS_USERFUNC_EXIT
}

AROS_UFH3(void, Exec_TaskSpinLockForbidFunc,
    AROS_UFHA(struct Hook *, h, A0),
    AROS_UFHA(spinlock_t *, spinLock, A1),
    AROS_UFHA(void *, unused, A2))
{
    AROS_USERFUNC_INIT

    D(
        struct Task *spinTask = GET_THIS_TASK;
        bug("[Exec:X86] %s(0x%p)\n", __func__, spinTask);
    )
 
    Forbid();

    D(bug("[Exec:X86] %s: done\n", __func__));

    AROS_USERFUNC_EXIT
}

AROS_UFH3(void, Exec_TaskSpinLockDisableFunc,
    AROS_UFHA(struct Hook *, h, A0),
    AROS_UFHA(spinlock_t *, spinLock, A1),
    AROS_UFHA(void *, unused, A2))
{
    AROS_USERFUNC_INIT

    D(
        struct Task *spinTask = GET_THIS_TASK;
        bug("[Exec:X86] %s(0x%p)\n", __func__, spinTask);
    )
 
    Disable();

    D(bug("[Exec:X86] %s: done\n", __func__));

    AROS_USERFUNC_EXIT
}

void Exec_TaskSpinUnlock(spinlock_t *spinLock)
{
#if (0)
    struct Task *spinTask, *nxtTask;
    struct IntETask *thisET;
#endif
    D(bug("\n[Exec:X86] %s(0x%p)\n", __func__, spinLock));

#if (0)
    EXEC_SPINLOCK_LOCK(&PrivExecBase(SysBase)->TaskSpinningLock, SPINLOCK_MODE_WRITE);
    ForeachNodeSafe(&PrivExecBase(SysBase)->TaskSpinning, spinTask, nxtTask)
    {
        thisET = GetIntETask(spinTask);
        if ((thisET) && (thisET->iet_SpinLock == spinLock))
        {
            EXEC_SPINLOCK_LOCK(&PrivExecBase(SysBase)->TaskReadySpinLock, SPINLOCK_MODE_WRITE);
            Disable();
            Remove(&spinTask->tc_Node);
            Enqueue(&SysBase->TaskReady, &spinTask->tc_Node);
            EXEC_SPINLOCK_UNLOCK(&PrivExecBase(SysBase)->TaskReadySpinLock);
            Enable();
        }
    }
    EXEC_SPINLOCK_UNLOCK(&PrivExecBase(SysBase)->TaskSpinningLock);
#endif

    D(bug("[Exec:X86] %s: done\n\n", __func__));
}

void X86_HandleSpinLock(struct ExceptionContext *regs)
{
    struct ExecSpinSCData *spinData = (struct ExecSpinSCData *)EXCX_REGB;

    D(bug("[Exec:X86] %s(0x%p, 0x%p, %08x)\n", __func__, spinData->lock_ptr, spinData->lock_failhook, spinData->lock_mode));

    Kernel_43_KrnSpinLock(spinData->lock_ptr, spinData->lock_failhook, spinData->lock_mode, NULL);

    if (spinData->lock_obtainhook)
    {
        D(bug("[Exec:X86] %s: calling lock-obtained hook @ 0x%p ...\n", __func__, spinData->lock_obtainhook);)
        CALLHOOKPKT(spinData->lock_obtainhook, (APTR)spinData->lock_ptr, 0);
    }

    EXCX_REGA = (IPTR)spinData->lock_ptr;

    D(bug("[Exec:X86] %s: done\n", __func__));

    return;
}

struct syscallx86_Handler x86_SCSpinLockHandler =
{
    {
        .ln_Name = (APTR)SC_X86CPUSPINLOCK
    },
    (APTR)X86_HandleSpinLock
};

spinlock_t *ExecSpinLockCall(spinlock_t *spinLock, struct Hook *hookObtained, struct Hook *hookFailed, ULONG spinMode)
{
    struct ExecSpinSCData __spinData =
    {
        spinLock,
        hookObtained,
        hookFailed,
        spinMode
    };
    spinlock_t *__retval;
    D(bug("\n[Exec:X86] %s: attempting to lock 0x%p\n", __func__, spinLock));
    __retval = krnSysCallSpinLock(&__spinData);
    D(bug("[Exec:X86] %s: returning 0x%p\n\n", __func__, __retval));
    return __retval;
}
#endif

void X86_HandleSwitch(struct ExceptionContext *regs)
{
    D(bug("[Exec:X86] %s()\n", __func__));

    cpu_Switch(regs);

    return;
}

struct syscallx86_Handler x86_SCSwitchHandler =
{
    {
        .ln_Name = (APTR)SC_X86SWITCH
    },
    (APTR)X86_HandleSwitch
};

void X86_HandleReschedTask(struct ExceptionContext *regs)
{
#if defined(__AROSEXEC_SMP__)
    struct Task *reschTask = (struct Task *)EXCX_REGB;
    spinlock_t *task_listlock = NULL;
#endif
    
    D(bug("[Exec:X86] %s()\n", __func__));

    /* Move to the ready list... */
#if defined(__AROSEXEC_SMP__)
    switch (reschTask->tc_State)
    {
        case TS_WAIT:
            task_listlock = &PrivExecBase(SysBase)->TaskRunningSpinLock;
            break;
        case TS_SPIN:
            task_listlock = &PrivExecBase(SysBase)->TaskSpinningLock;
            break;
        default:
            break;
    }
    if (task_listlock)
    {
        Kernel_43_KrnSpinLock(task_listlock, &Exec_TaskSpinLockFailHook, SPINLOCK_MODE_WRITE, NULL);
        Remove(&reschTask->tc_Node);
        Kernel_44_KrnSpinUnLock(task_listlock, NULL);
    }
    reschTask->tc_State = TS_READY;
    Kernel_43_KrnSpinLock(&PrivExecBase(SysBase)->TaskReadySpinLock, &Exec_TaskSpinLockFailHook, SPINLOCK_MODE_WRITE, NULL);
    Enqueue(&SysBase->TaskReady, &reschTask->tc_Node);
    Kernel_44_KrnSpinUnLock(&PrivExecBase(SysBase)->TaskReadySpinLock, NULL);
#endif

    return;
}

struct syscallx86_Handler x86_SCReschedTaskHandler =
{
    {
        .ln_Name = (APTR)SC_X86RESCHEDTASK
    },
    (APTR)X86_HandleReschedTask
};


struct Task *Exec_X86CreateIdleTask(APTR sysBase)
{
    struct ExecBase *SysBase = (struct ExecBase *)sysBase;
    struct Task *CPUIdleTask;
    char *taskName;
#if defined(__AROSEXEC_SMP__)
    struct KernelBase *KernelBase = __kernelBase;
    int cpuNo = KrnGetCPUNumber();
    IPTR idleNameArg[] = 
    {
        cpuNo
    };
    struct MemList *ml;

    if ((ml = AllocMem(sizeof(struct MemList), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
    {
        bug("[Exec:X86.%03u] FATAL : Failed to allocate memory for idle task name info", cpuNo);
        return NULL;
    }

    ml->ml_NumEntries      = 1;

    ml->ml_ME[0].me_Length = 15;
    if ((ml->ml_ME[0].me_Addr = AllocMem(15, MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
    {
        bug("[Exec:X86.%03u] FATAL : Failed to allocate memory for task idle name", cpuNo);
        FreeMem(ml, sizeof(struct MemList));
        return NULL;
    }
    taskName = ml->ml_ME[0].me_Addr;
    RawDoFmt("CPU #%03u Idle", (RAWARG)idleNameArg, RAWFMTFUNC_STRING, taskName);
#else
    taskName = "CPU Idle";
#endif
    CPUIdleTask = NewCreateTask(TASKTAG_NAME   , taskName,
#if defined(__AROSEXEC_SMP__)
                                TASKTAG_AFFINITY   , KrnGetCPUMask(cpuNo),
#endif
                                TASKTAG_PRI        , -127,
                                TASKTAG_PC         , IdleTask,
                                TASKTAG_ARG1       , SysBase,
                                TAG_DONE);

    if (CPUIdleTask)
    {
        D(
            bug("[Exec:X86] %s: '%s' Task created @ 0x%p\n", __func__, CPUIdleTask->tc_Node.ln_Name, CPUIdleTask);
#if defined(__AROSEXEC_SMP__)
            bug("[Exec:X86] %s:      CPU Affinity : %08x\n", __func__, IntETask(CPUIdleTask->tc_UnionETask.tc_ETask)->iet_CpuAffinity);
#endif
        )
#if defined(__AROSEXEC_SMP__)
        AddTail(&CPUIdleTask->tc_MemEntry, &ml->ml_Node);
#endif
    }
#if defined(__AROSEXEC_SMP__)
    else
    {
        FreeMem(ml->ml_ME[0].me_Addr, 15);
        FreeMem(ml, sizeof(struct MemList));
    }
#endif
    return CPUIdleTask;
}

int Exec_X86Init(struct ExecBase *SysBase)
{
    struct IntExecBase *sysBase = (struct IntExecBase *)SysBase;
    struct KernelBase *KernelBase = __kernelBase;

    D(bug("[Exec:X86] %s()\n", __func__));
    D(bug("[Exec:X86] %s: PlatformData @ 0x%p\n", __func__, &sysBase->PlatformData));

    /* Install The default Power Management handlers */
    sysBase->ColdResetHandler.is_Node.ln_Pri = -64;
    sysBase->ColdResetHandler.is_Node.ln_Name = "Keyboard Controller Reset";
    sysBase->ColdResetHandler.is_Code = (VOID_FUNC)Exec_X86ColdResetHandler;
    sysBase->ColdResetHandler.is_Data = &sysBase->ColdResetHandler;
    AddResetCallback(&sysBase->ColdResetHandler);

    sysBase->WarmResetHandler.is_Node.ln_Pri = -64;
    sysBase->WarmResetHandler.is_Node.ln_Name = "System Reset";
    sysBase->WarmResetHandler.is_Code = (VOID_FUNC)Exec_X86WarmResetHandler;
    sysBase->WarmResetHandler.is_Data = &sysBase->WarmResetHandler;
    AddResetCallback(&sysBase->WarmResetHandler);

    sysBase->ShutdownHandler.is_Node.ln_Pri = -128;
    sysBase->ShutdownHandler.is_Node.ln_Name = "System Shutdown";
    sysBase->ShutdownHandler.is_Code = (VOID_FUNC)Exec_X86ShutdownHandler;
    sysBase->ShutdownHandler.is_Data = &sysBase->ShutdownHandler;
    AddResetCallback(&sysBase->ShutdownHandler);

    D(bug("[Exec:X86] %s: Default Handlers Registered\n", __func__));

    krnAddSysCallHandler(KernelBase->kb_PlatformData, &x86_SCSwitchHandler, TRUE, TRUE);
    krnAddSysCallHandler(KernelBase->kb_PlatformData, &x86_SCReschedTaskHandler, TRUE, TRUE);

#if defined(__AROSEXEC_SMP__)
    /* register the task spinlock syscall */
    krnAddSysCallHandler(KernelBase->kb_PlatformData, &x86_SCSpinLockHandler, TRUE, TRUE);
    sysBase->PlatformData.SpinLockCall = ExecSpinLockCall;

    /* set up the task spinning hooks */
    Exec_TaskSpinLockForbidHook.h_Entry = (HOOKFUNC)Exec_TaskSpinLockForbidFunc;
    Exec_TaskSpinLockDisableHook.h_Entry = (HOOKFUNC)Exec_TaskSpinLockDisableFunc;
    Exec_TaskSpinLockFailHook.h_Entry = (HOOKFUNC)Exec_TaskSpinLockFailFunc;
    D(bug("[Exec:X86] %s: SpinLock Fail Hook @ 0x%p, Handler @ 0x%p\n", __func__, &Exec_TaskSpinLockFailHook, Exec_TaskSpinLockFailFunc));
    D(bug("[Exec:X86] %s: SpinLock Forbid Hook @ 0x%p, Handler @ 0x%p\n", __func__, &Exec_TaskSpinLockForbidHook, Exec_TaskSpinLockForbidFunc));
#endif

    Exec_X86CreateIdleTask(SysBase);

    return TRUE;
}

ADD2INITLIB(Exec_X86Init, 0)
