/*
    Copyright � 2017, The AROS Development Team. All rights reserved.
    $Id$
*/

#define DEBUG 0
#include <aros/debug.h>

#include <aros/asmcall.h>
#include <exec/execbase.h>
#include <exec/lists.h>

#define __AROS_KERNEL__

#include "exec_intern.h"

#include "kernel_intern.h"

#include "intservers.h"

/*
 * Unlike the VBlankServer, we might not run at a fixed 60Hz.
 */
void APICHeartbeatServer(struct ExecBase *SysBase, void *unused)
{
    struct KernelBase *KernelBase = __kernelBase;
    struct PlatformData *pdata = KernelBase->kb_PlatformData;
    struct APICData *apicData = pdata->kb_APIC;
    IPTR __LAPICBase = apicData->lapicBase;
#if defined(__AROSEXEC_SMP__)
    struct X86SchedulerPrivate  *apicScheduleData;
    tls_t *apicTLS;
#endif
    UWORD current;

    if (apicData->flags & APF_TIMER)
    {
#if defined(__AROSEXEC_SMP__)
        apicid_t cpuNum = core_APIC_GetNumber(apicData);

        // Update LAPIC tick
        apicData->cores[cpuNum].cpu_LAPICTick += APIC_REG(__LAPICBase, APIC_TIMER_ICR);

        // Relaunch LAPIC timer
        APIC_REG(__LAPICBase, APIC_TIMER_ICR) = (apicData->cores[cpuNum].cpu_TimerFreq + 50) / 100;
#if 0
        // Every second update CPU usage statistics...
        if (apicData->cores[cpuNum].cpu_LAPICTick - apicData->cores[cpuNum].cpu_LastCPULoadTime > apicData->cores[cpuNum].cpu_TimerFreq)
        {
            D(bug("[Kernel:APIC.%03u] %s() updating CPU load statistics...\n", cpuNum, __func__));
            D(bug("[Kernel:APIC.%03u] %s() sleep time %d\n", cpuNum, __func__, (ULONG)apicData->cores[cpuNum].cpu_SleepTime));
            
            apicData->cores[cpuNum].cpu_Load = (UBYTE)((100 * (apicData->cores[cpuNum].cpu_TimerFreq - apicData->cores[cpuNum].cpu_SleepTime) / apicData->cores[cpuNum].cpu_TimerFreq));

            (bug("[Kernel:APIC.%03u] %s() cpu load %d\n", cpuNum, __func__, (ULONG)apicData->cores[cpuNum].cpu_Load));

            apicData->cores[cpuNum].cpu_LastCPULoadTime = apicData->cores[cpuNum].cpu_LAPICTick;
            apicData->cores[cpuNum].cpu_SleepTime = 0;
        }
#endif

        D(bug("[Kernel:APIC.%03u] %s(), tick=%08x:%08x\n", cpuNum, __func__, (ULONG)(apicData->cores[cpuNum].cpu_LAPICTick >> 32),
            (ULONG)(apicData->cores[cpuNum].cpu_LAPICTick & 0xffffffff)));

        apicTLS = apicData->cores[cpuNum].cpu_TLS;
        if ((apicTLS) && ((apicScheduleData = apicTLS->ScheduleData) != NULL))
        {
            if ((current = apicScheduleData->Elapsed))
            {
                if ((apicScheduleData->Elapsed = (current - apicScheduleData->Granularity)) == 0)
                {
                    __AROS_ATOMIC_OR_L(apicScheduleData->ScheduleFlags, TLSSF_Quantum);
                    __AROS_ATOMIC_OR_L(apicScheduleData->ScheduleFlags, TLSSF_Switch);
                }
            }
        }
#else
        D(bug("[Kernel:APIC] %s()\n", __func__));

        current = SCHEDELAPSED_GET;
        if (current)
            SCHEDELAPSED_SET(--current);

        if (current == 0)
        {
            FLAG_SCHEDQUANTUM_SET;
            FLAG_SCHEDSWITCH_SET;
        }
#endif
    }
}
