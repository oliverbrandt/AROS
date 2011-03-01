#include <devices/timer.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/timer.h>

#include <stdio.h>

int __nocommandline = 1;

int main(void)
{
    struct MsgPort *mp;
    struct timerequest *req;

    printf("exec.library v%u.%u\n", SysBase->LibNode.lib_Version, SysBase->LibNode.lib_Revision);
    printf("VBlank frequency: %u\n", SysBase->VBlankFrequency);
    printf("PSU    frequency: %u\n", SysBase->PowerSupplyFrequency);
    printf("EClock frequency: %u\n\n", SysBase->ex_EClockFrequency);
    
    mp = CreateMsgPort();
    if (mp) {
	req = (struct timerequest *)CreateIORequest(mp, sizeof(struct timerequest));
	if (req) {
	    if (!OpenDevice("timer.device", UNIT_VBLANK, &req->tr_node, 0))
	    {
		struct Device *TimerBase = req->tr_node.io_Device;
		struct EClockVal clock, oclock;
		struct timeval tv, otv;
		ULONG seconds;
	    
		printf("timer.device v%u.%u\n", TimerBase->dd_Library.lib_Version, TimerBase->dd_Library.lib_Revision);
		printf("EClock frequency reported: %u\n\n", ReadEClock(&clock));
		ReadEClock(&clock);
		GetSysTime(&tv);
		seconds = tv.tv_secs + 10;
		while (!(SetSignal(0, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) && tv.tv_secs < seconds) {
		    oclock.ev_hi = clock.ev_hi;
		    oclock.ev_lo = clock.ev_lo;
		    otv.tv_secs = tv.tv_secs;
		    otv.tv_micro = tv.tv_micro;
		    ReadEClock(&clock);
		    GetSysTime(&tv);
		    if (oclock.ev_hi > clock.ev_hi || (oclock.ev_hi == clock.ev_hi && oclock.ev_lo > clock.ev_lo)) {
		    	printf("\n\nFAIL: EClock old: %08x.%08x new: %08x.%08x\n", oclock.ev_hi, oclock.ev_lo, clock.ev_hi, clock.ev_lo); 
			break;
		    }
		    if (otv.tv_secs > tv.tv_secs || (otv.tv_secs == tv.tv_secs && otv.tv_micro >= tv.tv_micro) || tv.tv_micro >= 1000000) {
		    	printf("\n\nFAIL: SysTime old: %u.%06u new: %u.%06u\n", otv.tv_secs, otv.tv_micro, tv.tv_secs, tv.tv_micro); 
			break;
		    }
		    printf("%08x.%08x %u.%06u\r", clock.ev_hi, clock.ev_lo, tv.tv_secs, tv.tv_micro);
		}
		printf("\n\n");

		CloseDevice(&req->tr_node);
	    }
	    else
		printf("Failed to open timer.device!\n");

	    DeleteIORequest(req);
	} else
	    printf("Failed to create IORequest!\n");

	DeleteMsgPort(mp);
    } else
	printf("Failed to create message port!\n");
    
    return 0;
}
