/*
    (C) 1999 AROS - The Amiga Research OS
    $Id$

    Desc: IRQ system for standalone i386 AROS
    Lang: english
*/

#define AROS_ALMOST_COMPATIBLE
#include <exec/types.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <proto/oop.h>
#include <oop/oop.h>
#include <utility/utility.h>

#include "irq.h"

#undef SysBase

/* Customize libheader.c */
#define LC_SYSBASE_FIELD(lib)   (((LIBBASETYPEPTR       )(lib))->sysbase)
#define LC_SEGLIST_FIELD(lib)   (((LIBBASETYPEPTR       )(lib))->seglist)
#define LC_RESIDENTNAME		irqHidd_resident
#define LC_RESIDENTFLAGS	RTF_AUTOINIT|RTF_COLDSTART
#define LC_RESIDENTPRI		90
#define LC_LIBBASESIZE          sizeof(LIBBASETYPE)
#define LC_LIBHEADERTYPEPTR     LIBBASETYPEPTR
#define LC_LIB_FIELD(lib)       (((LIBBASETYPEPTR)(lib))->library)

#define LC_NO_OPENLIB
#define LC_NO_EXPUNGELIB
#define LC_NO_CLOSELIB


#define NOEXPUNGE

struct irqbase
{
    struct Library 		library;
    struct ExecBase 		*sysbase;
    BPTR			seglist;
    struct irq_staticdata 	*isd;
};

#include <libcore/libheader.c>

#undef  SDEBUG
#undef  DEBUG
#define DEBUG 1
#include <aros/debug.h>

void timer_handler(int cpl, void *dev_id, struct pt_regs *regs) { }

struct irqaction timer_int = { timer_handler, 0, 0, "timer", NULL, NULL};
struct irqaction kbd_int = { timer_handler, 0, 0, "keyboard", NULL, NULL};
struct irqaction rtc_int = { timer_handler, 0, 0, "rtc", NULL, NULL};
struct irqaction ide0_int = { timer_handler, 0, 0, "ide0", NULL, NULL};
struct irqaction ide1_int = { timer_handler, 0, 0, "ide1", NULL, NULL};

#undef SysBase

ULONG SAVEDS STDARGS LC_BUILDNAME(L_InitLib) (LC_LIBHEADERTYPEPTR lh)
{
    struct irq_staticdata *isd;

    D(bug("IRQ: Initializing\n"));

    isd = AllocMem( sizeof (struct irq_staticdata), MEMF_CLEAR|MEMF_PUBLIC );
    lh->isd = isd;
    if (isd)
    {
        isd->sysbase = SysBase;
        isd->oopbase = OpenLibrary(AROSOOP_NAME, 0);
	if (isd->oopbase)
	{
	    isd->utilitybase = OpenLibrary(UTILITYNAME, 37);
	    if (isd->utilitybase)
	    {
		init_IRQ();
		D(bug("    Adding timer interrupt\n"));
		setup_x86_irq(0, &timer_int);
		setup_x86_irq(1, &kbd_int);
		setup_x86_irq(8, &rtc_int);
		setup_x86_irq(14, &ide0_int);
		setup_x86_irq(15, &ide1_int);
		D(bug("    Init OK\n"));
		return TRUE;
	    }
	    CloseLibrary(isd->oopbase);
	}
	FreeMem(isd, sizeof (struct irq_staticdata));
    }
    return FALSE;
}
