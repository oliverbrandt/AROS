/* $Id$ */

#include <stdio.h>
#include <sys/time.h>

#ifndef __AROS__
/* 
   This is a trick to make the timer.device's timeval struct not clash with the
   sys/time.h's one. I'm using gettymeofday 'cause I'm lazy and don't wanna
   bother with timer.device myself :)
   
   On AROS this is handled automatically (go AROS!)
*/
#    define timeval timeval_aos
#endif

#include <exec/tasks.h>
#include <dos/dostags.h>
#include <proto/exec.h>
#include <clib/alib_protos.h>
#include <proto/dos.h>

#ifndef __AROS__
/* The trick is over... */
#undef timeval
#endif

#ifndef  __typedef_STACKIPTR
/* Normal AmigaOS environments don't have this defined */
typedef ULONG STACKIPTR;
#endif

/* Give some prettier names to the standard signals */
#define SIGF_STOP  SIGBREAKF_CTRL_C
#define SIGF_HELLO SIGBREAKF_CTRL_D
#define SIGF_BYE   SIGBREAKF_CTRL_E
#define SIGF_START SIGBREAKF_CTRL_F

unsigned long counter = 0;
struct Task *task1, *task2;

struct timeval start_tv, end_tv;

void Task1Entry()
{
    for (;;)
    {
        if (Wait(SIGF_HELLO | SIGF_STOP) == SIGF_HELLO)
            Signal(task2, SIGF_HELLO);
        else
    	{
     	    Wait(SIGF_BYE);
            return;
        }
    }
}

void Task2Entry()
{
    Wait(SIGF_START);

    for (;;)
    {
        Signal(task1, SIGF_HELLO);
        if (Wait(SIGF_HELLO | SIGF_STOP) == SIGF_HELLO)
            counter++;
        else
        {
            Signal(task1, SIGF_BYE);
            return;
        }
    }
}

int __nocommandline = 1;

/* 
   define this to non-zero if you want the benchmark to end automatically
   when it realizes that there's no need to continue.
   
   Beware that it can introduce some overhead (although very little).
*/
#define SELF_TIMED_TEST 1

int main(void)
{
    double elapsed = 0;

    /*
       Since I'm SO lazy, I'm using processes here, even if tasks would work too, 'cause
       it's easier to create processes than tasks...

       Ok ok... that's a lie, I could use CreateTask() from libamiga, but it seems to be
       buggy under AROS?
    */
    task1 = (struct Task *)CreateNewProcTags(NP_Entry, (STACKIPTR)Task1Entry, TAG_DONE);
    task2 = (struct Task *)CreateNewProcTags(NP_Entry, (STACKIPTR)Task2Entry, TAG_DONE);

    printf
    (
        "The test is starting.\n"
        #if !SELF_TIMED_TEST
        "Press CTRL-C to stop the test and get the results.\n\n"
        #endif
    );

    gettimeofday(&start_tv, NULL);

    Signal(task2, SIGF_START);

    #if SELF_TIMED_TEST
    unsigned long i;
    double oldratio = 0;

    for (i=4;;i+=2)
    {
        double ratio;
        unsigned long int counter2 = counter;

        Delay(200);

        ratio = (double)i/counter2;
        if ((ratio - oldratio) <= .000000001) break;

        oldratio = ratio;
    }

    #else

    Wait(SIGF_STOP);

    #endif

    gettimeofday(&end_tv, NULL);

    Signal(task1, SIGF_STOP);
    Signal(task2, SIGF_STOP);

    elapsed =  ((double)(((end_tv.tv_sec * 1000000) + end_tv.tv_usec) - ((start_tv.tv_sec * 1000000) + start_tv.tv_usec)))/1000000.;

    printf
    (
        "Elapsed time:               %f seconds\n"
        "Number of context switches: %ld\n"
        "Signal roundtrip time:      %.8f\n"
        "Context switch time:     <= %.8f seconds\n",
        elapsed,
        counter * 2,
        elapsed / (double)counter,
        elapsed / (double)(counter * 2)
    );

    return 0;
}
