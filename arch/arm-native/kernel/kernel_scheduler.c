/*
    Copyright � 2015, The AROS Development Team. All rights reserved.
    $Id$

*/

#include <exec/alerts.h>
#include <exec/execbase.h>
#include <exec/lists.h>
#include <proto/exec.h>

#include <kernel_base.h>
#include <kernel_debug.h>
#include <kernel_scheduler.h>
#include "tls.h"
#include "etask.h"

#define D(x)

/*
 * Schedule the currently running task away. Put it into the TaskReady list
 * in some smart way. This function is subject of change and it will be probably replaced
 * by some plugin system in the future
 */
BOOL core_Schedule(void)
{
    struct Task *task = TLS_GET(ThisTask);

    D(bug("[KRN:BCM2708] core_Schedule()\n"));

    SysBase->AttnResched &= ~ARF_AttnSwitch;

    /* If task has pending exception, reschedule it so that the dispatcher may handle the exception */
    if (!(task->tc_Flags & TF_EXCEPT))
    {
        BYTE pri;

        /* Is the TaskReady empty? If yes, then the running task is the only one. Let it work */
        if (IsListEmpty(&SysBase->TaskReady))
            return FALSE;

//## TODO : Lock TaskReady access (READ)
        /* Does the TaskReady list contains tasks with priority equal or lower than current task?
         * If so, then check further... */
        pri = ((struct Task*)GetHead(&SysBase->TaskReady))->tc_Node.ln_Pri;
        if (pri <= task->tc_Node.ln_Pri)
        {
            /* If the running task did not used it's whole quantum yet, let it work */
            if (!(SysBase->SysFlags & SFF_QuantumOver))
            {
//TODO: Unlock TaskReady access
                return FALSE;
            }
        }
//## TODO: Unlock TaskReady access
    }

    /*
     * If we got here, then the rescheduling is necessary.
     * Put the task into the TaskReady list.
     */
    D(bug("[KRN:BCM2708] Setting task 0x%p (%s) to READY\n", task, task->tc_Node.ln_Name));
    task->tc_State = TS_READY;
//## TODO : Lock TaskReady access (WRITE)
    Enqueue(&SysBase->TaskReady, &task->tc_Node);
//## TODO: Unlock TaskReady access

    /* Select new task to run */
    return TRUE;
}

/* Actually switch away from the task */
void core_Switch(void)
{
    struct Task *task = TLS_GET(ThisTask);

    D(bug("[KRN:BCM2708] core_Switch(): Old task = %p (%s)\n", task, task->tc_Node.ln_Name));

    if (task->tc_SPReg <= task->tc_SPLower || task->tc_SPReg > task->tc_SPUpper)
    {
        bug("[KRN:BCM2708] Task %s went out of stack limits\n", task->tc_Node.ln_Name);
        bug("[KRN:BCM2708] Lower %p, upper %p, SP %p\n", task->tc_SPLower, task->tc_SPUpper, task->tc_SPReg);
        /*
         * Suspend the task to stop it from causing more harm. In some rare cases, if the task is holding
         * lock on some global/library semaphore it will most likelly mean immenent freeze. In most cases
         * however, user will be shown an alert.
         */
        Remove(&task->tc_Node);
        task->tc_SigWait    = 0;
        task->tc_State      = TS_WAIT;
        Enqueue(&SysBase->TaskWait, &task->tc_Node);

        Alert(AN_StackProbe);
    }

    task->tc_IDNestCnt = SysBase->IDNestCnt;

    if (task->tc_Flags & TF_SWITCH)
        AROS_UFC1NR(void, task->tc_Switch, AROS_UFCA(struct ExecBase *, SysBase, A6));
}

/*
 * Task dispatcher. Basically it may be the same one no matter
 * what scheduling algorithm is used (except SysBase->Elapsed reloading)
 */
struct Task *core_Dispatch(void)
{
    struct Task *task;
    uint32_t cpumask;
    uint32_t tmp;

    D(bug("[KRN:BCM2708] core_Dispatch()\n"));

    asm volatile (" mrc p15, 0, %0, c0, c0, 5 " : "=r" (tmp));
    cpumask =  (1 << (tmp & 3));

// ## TODO: Lock TaskReady access READ
    for (task = (struct Task *)GetHead(&SysBase->TaskReady); task != NULL; task = (struct Task *)GetSucc(task))
    {
        if ((GetIntETask(task)->iet_CpuAffinity  & cpumask) == cpumask)
        {
// ## TODO: switch TaskReady Lock to  WRITE
            Remove(&task->tc_Node);
            break;
        }
    }
// ## TODO: Unlock TaskReady access

    if (!task)
    {
        /* Is the list of ready tasks empty? Well, go idle. */
        D(bug("[KRN:BCM2708] No ready tasks, entering sleep mode\n"));

        /*
         * Idle counter is incremented every time when we enter here,
         * not only once. This is correct.
         */
        SysBase->IdleCount++;
        SysBase->AttnResched |= ARF_AttnSwitch;

        return NULL;
    }

    SysBase->DispCount++;
    SysBase->IDNestCnt = task->tc_IDNestCnt;
    TLS_SET(ThisTask, task);
    SysBase->Elapsed   = SysBase->Quantum;
    SysBase->SysFlags &= ~SFF_QuantumOver;
    task->tc_State     = TS_RUN;

    D(bug("[KRN:BCM2708] New task = %p (%s)\n", task, task->tc_Node.ln_Name));

    /* Check the stack of the task we are about to launch. */

    if (task->tc_SPReg <= task->tc_SPLower || task->tc_SPReg > task->tc_SPUpper)
    {
        /* Don't let the task run, switch it away (raising Alert) and dispatch another task */
        core_Switch();
        return core_Dispatch();
    }

    if (task->tc_Flags & TF_LAUNCH)
        AROS_UFC1NR(void, task->tc_Launch, AROS_UFCA(struct ExecBase *, SysBase, A6));

    /* Leave interrupt and jump to the new task */
    return task;
}
