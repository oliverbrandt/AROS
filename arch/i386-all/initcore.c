#include <hardware/intbits.h>
#include <exec/interrupts.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <aros/asmcall.h>
#include <signal.h>
#include <stdio.h>
#define timeval     sys_timeval
#include <sys/time.h>
#undef timeval

void os_disable(void);
static int sig2inttabl[NSIG];
int supervisor;

static void signals(int sig)
{
    struct IntVector *iv;
    supervisor++;
    if (supervisor != 1)
    {
	fprintf (stderr, "Illegal supervisor %d\n", supervisor);
	fflush (stderr);
    }
    iv=&SysBase->IntVects[sig2inttabl[sig]];
    if (iv->iv_Code)
    {
	AROS_UFC2(void,iv->iv_Code,
	    AROS_UFCA(APTR,iv->iv_Data,A1),
	    AROS_UFCA(struct ExecBase *,SysBase,A6)
	);
    }
    os_disable ();
    supervisor--;
    if(SysBase->AttnResched&0x8000)
    {
	SysBase->AttnResched&=~0x8000;
	Dispatch();
    }
}

void InitCore(void)
{
    static const int sig2int[][2]=
    {
	{   SIGALRM, INTB_VERTB   },
    };
    struct itimerval interval;
    int i;
    struct sigaction sa={ signals, ~0ul, SA_RESTART, NULL };

    for(i=0;i<sizeof(sig2int)/sizeof(sig2int[0]);i++)
    {
	sig2inttabl[sig2int[i][0]]=sig2int[i][1];
	sigaction(sig2int[i][0],&sa,NULL);
    }

    interval.it_interval.tv_sec = interval.it_value.tv_sec = 0;
    interval.it_interval.tv_usec = interval.it_value.tv_usec = 1000000/5;

    setitimer (ITIMER_REAL, &interval, NULL);
}
