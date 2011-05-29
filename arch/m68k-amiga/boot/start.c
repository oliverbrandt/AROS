/*
    Copyright © 1995-2010, The AROS Development Team. All rights reserved.
    $Id$

    Desc: m68k-amiga bootstrap to exec.
    Lang: english
 */

#include <aros/kernel.h>
#include <aros/debug.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <exec/execbase.h>
#include <proto/exec.h>

#include "memory.h"

#include "kernel_romtags.h"
#include "kernel_base.h"

#include "exec_intern.h"

#include "amiga_hwreg.h"
#include "amiga_irq.h"

#include "early.h"
#include "debug.h"

#define SS_STACK_SIZE	0x04000

extern const struct Resident Exec_resident;

extern void __clear_bss(const struct KernelBSS *bss);

extern void __attribute__((interrupt)) Exec_Supervisor_Trap (void);

/* detect CPU and FPU type, enable code cache (if supported)
 * does not enable data caches, it is bad idea on real 68030 hardware
 * without correct MMU tables (for example chip ram must be non-cacheable)
 * 68040/060 data caches will be enabled later (transparent translation
 * registers can be used to disable data caching in low ram, even on EC models)
 * ram testing requires disabled data caches.
 */

void __attribute__((interrupt)) cpu_detect_fpu_asm(void);
asm (".chip 68060\n"
	"	.text\n"
	"	.globl cpu_detect_fpu_asm\n"
	"cpu_detect_fpu_asm:\n"
	"	move.l %sp,%a1\n"
	"	lea %sp@(-60),%sp\n"
	"	move.l %sp,%a0\n"
	"	clr.b (%a0)\n"
	"	fsave (%a0)\n"
	"	move.w #0x8000,%d0\n"
	"	move.b (%a0),%d0\n"
	"	move.l %a1,%sp\n"
	"	rts\n" /* return to cpu_detect() */
);

void __attribute__((interrupt)) fpu_detect_trap_f(void);
asm (
	"	.text\n"
	"	.globl fpu_detect_trap_f\n"
	"fpu_detect_trap_f:\n"
	"	move.l %a1,%sp\n"
	"	moveq #0,%d0\n"
	"	rts\n" /* return to cpu_detect() */
);

/* detect CPU addressing capability (24 or 32-bit) */
ULONG __attribute__((interrupt)) cpu_detect_32bit(void);
asm (	"	.text\n"
	"	.globl cpu_detect_32bit\n"
	"cpu_detect_32bit:\n"
	"	lea	0x08000000,%a0\n"
	"	move.l	0.w,%d0\n"	/* save old */
	"	move.l	#0xfecaf00d,%d1\n"
	"	move.l	%d1,0.w\n"
	"	nop\n"			/* force 68040/060 bus cycle to finish */
	"	cmp.l	%a0@,%d1\n"
	"	bne.s	2f\n"		/* different? no mirror */
	"	not.l	%d1\n"
	"	move.l	%d1,0.w\n"
	"	nop\n"
	"	cmp.l	%a0@,%d1\n"
	"	bne.s	2f\n"		/* check again, maybe 0 already had our test value */
	"	moveq	#0,%d1\n"
	"2:	move.l	%d0,0.w\n"	/* restore saved value */
	"	move.l	%d1,%d0\n"
	"	rts\n"
);

void __attribute__((interrupt)) cpu_detect_asm(void);
asm (".chip 68060\n"
	"	.text\n"
	"	.globl cpu_detect_asm\n"
	"cpu_detect_asm:\n"
	"	move.l	%sp,%a1\n"
	"	moveq	#0,%d0\n"
		/* VBR is 68010+ */
	"	movec	%vbr,%d1\n"
	"	move.w	#0x0001,%d0\n"
 		/* CACR is 68020+ */
	"	dc.l 0x4e7a0002\n" // movec	%cacr,%d0\n"
		/* 68020+ or better */
	"       move.l	#0x00008000,%d0\n"
 		/* enable 68040/060 code cache */
	"	dc.l 0x4e7b0002\n" // movec	%d0,%cacr\n"
	"	dc.l 0x4e7a0002\n" // movec	%cacr,%d0\n"
 		/* bit 15 still set? */
	"	tst.w	%d0\n"
 		/* yes, it is 68040 or 68060 */
	"	bmi.s	0f\n"
 		/* enable 68020/030 code cache and 68030 data cache */
 	"	move.w	#0x0101,%d0\n"
	"	dc.l 0x4e7b0002\n" // movec	%d0,%cacr\n"
	"	dc.l 0x4e7a0002\n" // movec	%cacr,%d0\n"
 		/* data cache bit still set? */
	"	btst	#8,%d0\n"
	"	bne.s	1f\n" /* yes, it is 68030 */
		/* 68020 */
	"	move.w	#0x0003,%d0\n"
	"	bra cpu_detect_trap_illg\n"
		/* 68030 */
	"1:	move.w	#0x0001,%d0\n"
		/* disable data cache, bad idea without correct MMU tables */
	"	dc.l 0x4e7b0002\n" // movec	%d0,%cacr\n"
	"	move.w	#0x0007,%d0\n"
	"	bra cpu_detect_trap_illg\n"
		/* 68040 or 68060 */
	"0:	moveq	#0,%d0\n"
		/* set transparent translation registers,
		 * allow data caching only in 32-bit fast,
		 * code caching allowed everywhere */
	"	movec	%d0,%itt1\n"
	"	move.l	#0x0000e040,%d0\n"
	"	movec	%d0,%dtt0\n"
	"	move.l	#0x00ffe000,%d0\n"
	"	movec	%d0,%dtt1\n"
	"	movec	%d0,%itt0\n"
	"	move.w	#0x000f,%d0\n"
 		/* PCR is 68060 only */
	"	dc.l 0x4e7a0808\n" // movec	%pcr,%d0\n"
		/* 68060 */
	"	move.l	%d0,%a0@\n" // save PCR
	"	move.w	#0x0001,%d0\n"
 		/* enable supercalar, enable FPU */
 	"	dc.l 0x4e7b0808\n" // movec	%d0,%pcr\n"
		/* enable code cache, store buffer and branch cache */
	"	move.l	#0x0080a000,%d0\n"
	"	dc.l 0x4e7b0002\n" // movec	%d0,%cacr\n"
	"	move.w	#0x008f,%d0\n"
	"	bra cpu_detect_trap_illg\n"
);

void __attribute__((interrupt)) cpu_detect_trap_illg(void);
asm (
	"	.text\n"
	"	.globl cpu_detect_trap_illg\n"
	"cpu_detect_trap_illg:\n"
	"	move.l %a1,%sp\n" /* remove exception stack frame */
	"	rts\n" /* return to cpu_detect() */
);

/* Detect CPU and FPU model */
static ULONG cpu_detect(ULONG *pcr)
{
	volatile APTR *trap = NULL;
	APTR old_trap4, old_trap11;
	UWORD cpuret, fpuret;

	old_trap4 = trap[4];
	trap[4] = cpu_detect_trap_illg;

	*pcr = 0;
	asm volatile (
		"move.l %1,%%a0\n"
		"bsr cpu_detect_asm\n"
		"move.w	%%d0,%0\n"
		: "=m" (cpuret) : "m" (pcr) : "%d0", "%d1", "%a0", "%a1" );

	old_trap11 = trap[11];
	trap[11] = fpu_detect_trap_f;
	asm volatile (
		"bsr cpu_detect_fpu_asm\n"
		"move.w	%%d0,%0\n"
		: "=m" (fpuret) : : "%d0", "%a0", "%a1" );

	trap[4] = old_trap4;
	trap[11] = old_trap11;

	cpuret &= 0xff;
	if (cpu_detect_32bit())
		cpuret |= AFF_ADDR32;

	if (fpuret) {
		cpuret |= AFF_FPU;
		if (cpuret & (AFF_68040 | AFF_68060))
			cpuret |= AFF_FPU40;
			// AFF_68881 | AFF_68882 set only if 040/060 math emulation running
		else if ((fpuret & 0x00ff) <= 0x1f)
			cpuret |= AFF_68881;
		else
			cpuret |= AFF_68881 | AFF_68882;
	}
	return cpuret;
}

#define _AS_STRING(x)	#x
#define AS_STRING(x)	_AS_STRING(x)
	
/* Create a sign extending call stub:
 * foo:
 *   jsr AROS_SLIB_ENTRY(funcname, libname)
 *     0x4eb9 .... ....
 *   ext.w %d0	// EXT_BYTE only
 *     0x4880	
 *   ext.l %d0	// EXT_BYTE and EXT_WORD
 *     0x48c0
 *   rts
 *     0x4e75
 */
#define EXT_BYTE(lib, libname, funcname, funcid) \
	do { \
		void libname##_##funcname##_Wrapper(void) \
		{ asm volatile ( \
			"jsr " AS_STRING(AROS_SLIB_ENTRY(funcname, libname)) "\n" \
			"ext.w %d0\n" \
			"ext.l %d0\n" \
			"rts\n"); } \
		/* Insert into the library's jumptable */ \
		__AROS_SETVECADDR(lib, funcid, libname##_##funcname##_Wrapper); \
	} while (0)
#define EXT_WORD(lib, libname, funcname, funcid) \
	do { \
		void libname##_##funcname##_Wrapper(void) \
		{ asm volatile ( \
			"jsr " AS_STRING(AROS_SLIB_ENTRY(funcname, libname)) "\n" \
			"ext.l %d0\n" \
			"rts\n"); } \
		/* Insert into the library's jumptable */ \
		__AROS_SETVECADDR(lib, funcid, libname##_##funcname##_Wrapper); \
	} while (0)
/*
 * Create a register preserving call stub:
 * foo:
 *   movem.l %d0-%d1/%a0-%a1,%sp@-
 *     0x48e7 0xc0c0
 *   jsr AROS_SLIB_ENTRY(funcname, libname)
 *     0x4eb9 .... ....
 *   movem.l %sp@+,%d0-%d1/%d0-%a1
 *     0x4cdf 0x0303
 *   rts
 *     0x4e75
 */
#define PRESERVE_ALL(lib, libname, funcname, funcid) \
	do { \
		void libname##_##funcname##_Wrapper(void) \
	        { asm volatile ( \
	        	"movem.l %d0-%d1/%a0-%a1,%sp@-\n" \
	        	"jsr " AS_STRING(AROS_SLIB_ENTRY(funcname, libname)) "\n" \
	        	"movem.l %sp@+,%d0-%d1/%a0-%a1\n" \
	        	"rts\n" ); } \
		/* Insert into the library's jumptable */ \
		__AROS_SETVECADDR(lib, funcid, libname##_##funcname##_Wrapper); \
	} while (0)
/* Inject arbitrary code into the jump table
 * Used for GetCC and nano-stubs
 */
#define FAKE_IT(lib, libname, funcname, funcid, ...) \
	do { \
		UWORD *asmcall = (UWORD *)__AROS_GETJUMPVEC(lib, funcid); \
		const UWORD code[] = { __VA_ARGS__ }; \
		asmcall[0] = code[0]; \
		asmcall[1] = code[1]; \
		asmcall[2] = code[2]; \
	} while (0)
/* Inject a 'move.w #value,%d0; rts" sequence into the
 * jump table, to fake an private syscall.
 */
#define FAKE_ID(lib, libname, funcname, funcid, value) \
	FAKE_IT(lib, libname, funcname, funcid, 0x303c, value, 0x4e75)

extern void SuperstackSwap(void);
/* This calls the register-ABI library
 * routine Exec/InitCode, for use in NewStackSwap()
 */
static LONG doInitCode(void)
{
	/* Attempt to allocate a new supervisor stack */
	do {
	    APTR ss_stack;

	    ss_stack = AllocMem(SS_STACK_SIZE, MEMF_ANY | MEMF_CLEAR | MEMF_REVERSE);
	    DEBUGPUTHEX(("SS  lower", (ULONG)ss_stack));
	    DEBUGPUTHEX(("SS  upper", (ULONG)ss_stack + SS_STACK_SIZE - 1));
	    if (ss_stack == NULL) {
	    	DEBUGPUTS(("Strange. Can't allocate a new system stack\n"));
	    	Early_Alert(CODE_ALLOC_FAIL);
	    	break;
	    }
            SysBase->SysStkLower    = ss_stack;
            SysBase->SysStkUpper    = ss_stack + SS_STACK_SIZE;
	    SetSysBaseChkSum();

	    Supervisor((ULONG_FUNC)SuperstackSwap);
	} while(0);

	InitCode(RTF_COLDSTART, 0);

	return 0;
}

extern BYTE _rom_start;
extern BYTE _rom_end;
extern BYTE _ext_start;
extern BYTE _ext_end;
extern BYTE _bss;
extern BYTE _bss_end;
extern BYTE _ss;
extern BYTE _ss_end;

/* Protect the 'ROM' if it is actually in RAM.
 */
static void protectROM(struct MemHeader *mh)
{
    APTR tmp;

    APTR ss_start = &_ss;
    ULONG ss_len = &_ss_end - &_ss;

    APTR bss_start = &_bss;
    ULONG bss_len = &_bss_end - &_bss;

    APTR rom_start = &_rom_start;
    ULONG rom_len = &_rom_end - &_rom_start;

    APTR ext_start = &_ext_start;
    ULONG ext_len = &_ext_end - &_ext_start;

    DEBUGPUTHEX(("Protect: ", (IPTR)mh));
    tmp = Early_AllocAbs(mh, ss_start, ss_len);
    DEBUGPUTHEX(("     ss: ", (IPTR)tmp));
    tmp = Early_AllocAbs(mh, bss_start, bss_len);
    DEBUGPUTHEX(("    bss: ", (IPTR)tmp));
    tmp = Early_AllocAbs(mh, rom_start, rom_len);
    DEBUGPUTHEX(("    rom: ", (IPTR)tmp));
    tmp = Early_AllocAbs(mh, ext_start, ext_len);
    DEBUGPUTHEX(("    ext: ", (IPTR)tmp));
    DEBUGPUTHEX(("  First: ", (IPTR)mh->mh_First));
    DEBUGPUTHEX(("  Bytes: ", (IPTR)mh->mh_First->mc_Bytes));
}

static struct MemHeader *addmemoryregion(ULONG startaddr, ULONG size)
{
	if (size < 65536)
		return NULL;
	if (startaddr < 0x00c00000) {
		krnCreateMemHeader("chip memory", -10,
			(APTR)startaddr, size,
			 MEMF_CHIP | MEMF_KICK | MEMF_PUBLIC | MEMF_LOCAL | MEMF_24BITDMA);
	} else {
		krnCreateMemHeader(startaddr < 0x01000000 ? "memory" : "expansion memory",
			startaddr < 0x01000000 ? -5 : (startaddr < 0x08000000 ? 30 : 40),
			(APTR)startaddr, size,
			MEMF_FAST | MEMF_KICK | MEMF_PUBLIC | MEMF_LOCAL | (startaddr < 0x01000000 ? MEMF_24BITDMA : 0));
	}

	protectROM((struct MemHeader*)startaddr);
	return (struct MemHeader*)startaddr;
}

static BOOL IsSysBaseValidNoVersion(struct ExecBase *sysbase)
{
    if (sysbase == NULL || (((ULONG)sysbase) & 0x80000001))
	return FALSE;
    if (sysbase->ChkBase != ~(IPTR)sysbase)
	return FALSE;
    return GetSysBaseChkSum(sysbase) == 0xffff;
}

static BOOL InitKickMem(struct ExecBase *SysBase)
{
    struct MemList *ml = SysBase->KickMemPtr;
    DEBUGPUTHEX(("KickMemPtr: ", (IPTR)ml));
    while (ml) {
	int i;
	DEBUGPUTHEX(("NumEntries: ", ml->ml_NumEntries));
	for (i = 0; i < ml->ml_NumEntries; i++) {
	    DEBUGPUTHEX(("      Addr: ", (IPTR)ml->ml_ME[i].me_Addr));
	    DEBUGPUTHEX(("       Len: ", ml->ml_ME[i].me_Length));
	    /* Use the non-mungwalling AllocAbs */
	    if (!InternalAllocAbs(ml->ml_ME[i].me_Addr, ml->ml_ME[i].me_Length, SysBase))
		return FALSE;
	}
	ml = (struct MemList*)ml->ml_Node.ln_Succ;
    }

    return TRUE;
}

void doColdCapture(void)
{
    APTR ColdCapture = SysBase->ColdCapture;
    if (ColdCapture == NULL)
    	return;

    if (((ULONG *)ColdCapture)[1] == AROS_MAKE_ID('F','A','K','E')) {
    	/* Fake SysBase installed by AROSBootstrap.
    	 *
    	 * In this case, ColdCapture is the trampoline executed
    	 * by the AOS ROM to get into AROS. We need to keep
    	 * ColdCapture around in AROS SysBase, but we don't
    	 * want to execute it (and cause an infinite loop).
    	 */
    	DEBUGPUTS(("[ColdCapture] Ignoring AOS->AROS trampoline\n"));
    	return;
    }

    SysBase->ColdCapture = NULL;
    /* ColdCapture calling method is a little
     * strange. It's in supervisor mode, requires
     * the return location in A5, and SysBase in A6.
     */
    asm volatile (
    	"move.l %0,%%a0\n"
    	"move.l %1,%%a6\n"
    	"move.l #0f,%%a5\n"
    	"jmp (%%a0)\n"
    	"0:\n"
    	:
    	: "m" (ColdCapture), "m" (SysBase)
    	: "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
    	  "a0", "a1", "a2", "a3", "a4", "a5", "a6");
}

void exec_boot(ULONG *membanks)
{
	struct TagItem bootmsg[] = {
#ifdef AROS_SERIAL_DEBUG
	    { KRN_CmdLine, (IPTR)"sysdebug=InitCode" },
#endif
	    { TAG_END },
	};
	volatile APTR *trap;
	int i;
	BOOL wasvalid;
	UWORD *kickrom[8];
	const struct KernelBSS kbss[2] = {
		{
			.addr = &_bss,
			.len = &_bss_end - &_bss,
		}, {
			.addr = 0,
			.len = 0,
		}
	};
	struct MemHeader *mh;
	LONG oldLastAlert[4];
	ULONG oldmem, pcr;
	APTR ColdCapture = NULL, CoolCapture = NULL, WarmCapture = NULL;
	APTR KickMemPtr = NULL, KickTagPtr = NULL, KickCheckSum = NULL;
	/* We can't use the global 'SysBase' symbol, since
	 * the compiler does not know that PrepareExecBase
	 * may change it out from under us.
	 */
	struct ExecBase *oldSysBase = *(APTR *)4;
#define SysBase CANNOT_USE_SYSBASE_SYMBOL_HERE

	trap = (APTR *)(NULL);

	/* Set all the exceptions to the Early_TrapHandler
	 */
	for (i = 2; i < 64; i++) {
	    if (i != 31)
	    	trap[i] = Early_TrapHandler;
	}

	/* Let the world know we exist
	 */
	DebugInit();
	DEBUGPUTS(("[reset]\n"));

	/* Zap out old SysBase if invalid */
	wasvalid = IsSysBaseValid(oldSysBase);
	if (wasvalid) {
	    DEBUGPUTHEX(("[SysBase] was at", (ULONG)oldSysBase));
	} else {
	    wasvalid = IsSysBaseValidNoVersion(oldSysBase);
	    if (wasvalid) {
	    	DEBUGPUTHEX(("[SysBase] fakebase at", (ULONG)oldSysBase));
	    	wasvalid = TRUE;
	    } else {
	    	DEBUGPUTHEX(("[SysBase] invalid at", (ULONG)oldSysBase));
	    	wasvalid = FALSE;
	    }
	}

	if (wasvalid) {
	    /* Save reset proof vectors */
	    ColdCapture  = oldSysBase->ColdCapture;
	    CoolCapture  = oldSysBase->CoolCapture;
	    WarmCapture  = oldSysBase->WarmCapture;
	    KickMemPtr   = oldSysBase->KickMemPtr; 
	    KickTagPtr   = oldSysBase->KickTagPtr;
	    KickCheckSum = oldSysBase->KickCheckSum;

	    /* Mark the oldSysBase as processed */
	    oldSysBase = NULL;
	}

	for (i = 0; membanks[i + 1]; i += 2) {
		ULONG addr = membanks[i + 0];
		ULONG size = membanks[i + 1];
		DEBUGPUTHEX(("RAM lower", addr));
		DEBUGPUTHEX(("RAM upper", addr + size - 1));
	}

	kickrom[0] = (UWORD*)&_rom_start;
	kickrom[1] = (UWORD*)&_rom_end;
	kickrom[2] = (UWORD*)0x00f00000;
	kickrom[3] = (UWORD*)0x00f80000;
	kickrom[4] = (UWORD*)&_ext_start;
	kickrom[5] = (UWORD*)&_ext_end;
	kickrom[6] = (UWORD*)~0;
	kickrom[7] = (UWORD*)~0;

	/* Look for 'HELP' at address 0 - we're recovering
	 * from a fatal alert
	 */
	if (trap[0] == (APTR)0x48454c50) {
	    for (i = 0; i < 4; i++)
	    	oldLastAlert[i] = (LONG)trap[64 + i];

	    DEBUGPUTHEX(("LastAlert Alert", oldLastAlert[0]));
	    DEBUGPUTHEX(("LastAlert  Task", oldLastAlert[1]));
	} else {
	    oldLastAlert[0] = (LONG)-1;
	    oldLastAlert[1] = 0;
	    oldLastAlert[2] = 0;
	    oldLastAlert[2] = 0;
	}

	/* Clear alert marker */
	trap[0] = 0;

	DEBUGPUTHEX(("BSS lower", (ULONG)&_bss));
	DEBUGPUTHEX(("BSS upper", (ULONG)&_bss_end - 1));
	DEBUGPUTHEX(("SS  lower", (ULONG)&_ss));
	DEBUGPUTHEX(("SS  upper", (ULONG)&_ss_end - 1));

	Early_ScreenCode(CODE_RAM_CHECK);

	mh = addmemoryregion(membanks[0], membanks[1]);
	if (mh == NULL) {
	    DEBUGPUTS(("Can't create initial memory header!\n"));
	    Early_Alert(AT_DeadEnd | AG_NoMemory);
	}

	/* Clear the BSS. */
	__clear_bss(&kbss[0]);

	/* NOTE: mh *must* have, as its first mc, a chunk
	 *       big enough for krnRomTagScanner, and at
	 *       least one other chunk big enough for the
	 *       initial SysBase.
	 */
	DEBUGPUTHEX(("[SysBase mh]", (ULONG)mh));
	if (mh->mh_First->mc_Bytes < 64*1024) {
	    DEBUGPUTHEX(("FATAL: SysBase mh's first chunk is too small", mh->mh_First->mc_Bytes));
	    Early_Alert(AT_DeadEnd | AG_NoMemory);
	}

	/*
	 * Call the SysBase initialization.
	 */
	Early_ScreenCode(CODE_EXEC_CHECK);
	if (!krnPrepareExecBase(kickrom, mh, bootmsg))
	    Early_Alert(AT_DeadEnd | AG_MakeLib | AO_ExecLib);

	/* From here on, we can reference SysBase */
#undef SysBase
	DEBUGPUTHEX(("[SysBase at]", (ULONG)SysBase));

    	if (wasvalid) {
    	    SysBase->ColdCapture = ColdCapture;
    	    SysBase->CoolCapture = CoolCapture;
    	    SysBase->WarmCapture = WarmCapture;
    	    SysBase->ChkSum = 0;
    	    SysBase->ChkSum = GetSysBaseChkSum(SysBase) ^ 0xffff; 
    	    SysBase->KickMemPtr = KickMemPtr;
    	    SysBase->KickTagPtr = KickTagPtr;
    	    SysBase->KickCheckSum = KickCheckSum;
    	}

        SysBase->SysStkUpper    = (APTR)&_ss_end;
        SysBase->SysStkLower    = (APTR)&_ss;

        /* Mark what the last alert was */
        for (i = 0; i < 4; i++)
            SysBase->LastAlert[i] = oldLastAlert[i];

	/* Determine CPU model */
	SysBase->AttnFlags |= cpu_detect(&pcr);

#ifdef AROS_SERIAL_DEBUG
	DEBUGPUTS(("CPU: "));
	if (SysBase->AttnFlags & AFF_68060)
		DEBUGPUTS(("68060"));
	else if (SysBase->AttnFlags & AFF_68040)
		DEBUGPUTS(("68040"));
	else if (SysBase->AttnFlags & AFF_68030)
		DEBUGPUTS(("68030"));
	else if (SysBase->AttnFlags & AFF_68020)
		DEBUGPUTS(("68020"));
	else if (SysBase->AttnFlags & AFF_68010)
		DEBUGPUTS(("68010"));
	else
		DEBUGPUTS(("68000"));
	if (SysBase->AttnFlags & AFF_ADDR32)
		DEBUGPUTS((" 32bit"));
	else
		DEBUGPUTS((" 24bit"));
	DEBUGPUTS((" FPU: "));
	if (SysBase->AttnFlags & AFF_FPU40) {
		if (SysBase->AttnFlags & AFF_68060)
			DEBUGPUTS(("68060"));
		else if (SysBase->AttnFlags & AFF_68040)
			DEBUGPUTS(("68040"));
		else
			DEBUGPUTS(("-"));
	} else if (SysBase->AttnFlags & AFF_68882)
		DEBUGPUTS(("68882"));
	else if (SysBase->AttnFlags & AFF_68881)
		DEBUGPUTS(("68881"));
	else
		DEBUGPUTS(("-"));
	DEBUGPUTS(("\n"));
	if (pcr)
		DEBUGPUTHEX(("PCR", pcr));
#endif

	/* Inject code for GetCC, depending on CPU model */
	if (SysBase->AttnFlags & AFF_68010) {
		/* move.w %ccr,%d0; rts; nop */
		FAKE_IT(SysBase, Exec, GetCC, 88, 0x42c0, 0x4e75, 0x4e71);
	} else {
		/* move.w %sr,%d0; rts; nop */
		FAKE_IT(SysBase, Exec, GetCC, 88, 0x40c0, 0x4e75, 0x4e71);
	}

#ifdef THESE_ARE_KNOWN_SAFE_ASM_ROUTINES
	PRESERVE_ALL(SysBase, Exec, Disable, 20);
	PRESERVE_ALL(SysBase, Exec, Enable, 21);
	PRESERVE_ALL(SysBase, Exec, Forbid, 22);
#endif
	PRESERVE_ALL(SysBase, Exec, Permit, 23);
	PRESERVE_ALL(SysBase, Exec, ObtainSemaphore, 94);
	PRESERVE_ALL(SysBase, Exec, ReleaseSemaphore, 95);
	PRESERVE_ALL(SysBase, Exec, ObtainSemaphoreShared, 113);

	/* Functions that need sign extension */
	EXT_BYTE(SysBase, Exec, SetTaskPri, 50);
	EXT_BYTE(SysBase, Exec, AllocSignal, 55);

	/* Only add the 2 standard ROM locations, since
	 * we may get memory at 0x00f00000, or when we
	 * are ReKicked, at the rom-in-ram locations.
	 */
	krnCreateROMHeader(mh, "Kickstart ROM", (APTR)0x00f80000, (APTR)0x01000000);
	krnCreateROMHeader(mh, "Kickstart ROM", (APTR)0x00e00000, (APTR)0x00e80000);

	/* Add remaining memory regions */
	for (i = 2; membanks[i + 1]; i += 2) {
		IPTR  addr = membanks[i];
		ULONG size = membanks[i + 1];

		DEBUGPUTHEX(("RAM Addr: ", addr));
		DEBUGPUTHEX(("RAM Size: ", size));
		mh = addmemoryregion(membanks[i], membanks[i + 1]);
		Enqueue(&SysBase->MemList, &mh->mh_Node);

		/* Adjust MaxLocMem and MaxExtMem as needed */
		if (addr < 0x00200000)
			SysBase->MaxLocMem = (size + 0xffff) & 0xffff0000;
		else if (addr < 0x00d00000)
			SysBase->MaxExtMem = size ? (APTR)(((0xc00000 + (size + 0xffff)) & 0xffff0000)) : 0;
	}

	/* Now that we have a valid SysBase,
	 * we can call ColdCapture
	 */
	if (wasvalid)
	    doColdCapture();

	/* Seal up SysBase's critical variables */
	SetSysBaseChkSum();

	/* Set privilege violation trap - we
	 * need this to support the Exec/Supervisor call
	 */
	trap[8] = Exec_Supervisor_Trap;

	oldmem = AvailMem(MEMF_FAST);

	/* Ok, let's start the system. We have to
	 * do this in Supervisor context, since some
	 * expansions ROMs (Cyperstorm PPC) expect it.
	 */
	DEBUGPUTS(("[start] InitCode(RTF_SINGLETASK, 0)\n"));
	InitCode(RTF_SINGLETASK, 0);

	/* Autoconfig ram expansions are now configured */

	/* If oldSysBase is not NULL, that means that it
	 * (a) wasn't valid before when we only had MEMF_LOCAL
	 * ram and (b) could possibly be in the MEMF_KICK memory
	 * we just got. Let's check it and find out if we
	 * can use it's capture vectors.
	 */
	if (oldSysBase && IsSysBaseValidNoVersion(oldSysBase)) {
	    /* Save reset proof vectors */
	    SysBase->ColdCapture  = oldSysBase->ColdCapture;
	    SysBase->CoolCapture  = oldSysBase->CoolCapture;
	    SysBase->WarmCapture  = oldSysBase->WarmCapture;
	    /* Save KickData */
	    SysBase->KickMemPtr   = oldSysBase->KickMemPtr; 
	    SysBase->KickTagPtr   = oldSysBase->KickTagPtr;
	    SysBase->KickCheckSum = oldSysBase->KickCheckSum;
	    doColdCapture();
	    /* Re-seal SysBase */
	    SetSysBaseChkSum();
	    wasvalid = TRUE;
	}

	/* Before we allocate anything else, we need to
	 * lock down the entries in KickMemPtr
	 *
	 * If we get a single failure, don't run any
	 * of the KickTags.
	 */
	if (SysBase->KickCheckSum == (APTR)SumKickData()) {
	    if (!InitKickMem(SysBase)) {
	    	DEBUGPUTS(("[KickMem] KickMem failed an allocation. Ignoring KickTags\n"));
	    	SysBase->KickTagPtr = NULL;
	    }
	    SysBase->KickMemPtr = NULL;
	    SysBase->KickCheckSum = (APTR)SumKickData();
	}

	if ((AvailMem(MEMF_FAST) > (oldmem + 256 * 1024)) &&
	    ((TypeOfMem(SysBase) & MEMF_CHIP) ||
	     ((ULONG)SysBase >= 0x00a00000ul && (ULONG)SysBase < 0x01000000ul))) {
	    /* Move execbase to real fast if available now */
	    SysBase = PrepareExecBaseMove(SysBase);
	    DEBUGPUTHEX(("[Sysbase] now at", (ULONG)SysBase));
	}
	
	/* Now that SysBase is in its final position, we can
	 * call CoolCapture and the KickTags.
	 */
	InitKickTags();

	/* Initialize IRQ subsystem */
	AmigaIRQInit(SysBase);

	/* Set privilege violation trap again.
	 * AmigaIRQInit may have blown it away.
	 */
	trap[8] = Exec_Supervisor_Trap;

	/* Attempt to allocate a real stack, and switch to it. */
	do {
	    const ULONG size = AROS_STACKSIZE;
	    IPTR *usp;
	
	    usp = AllocMem(size * sizeof(IPTR), MEMF_PUBLIC);
	    if (usp == NULL) {
		DEBUGPUTS(("Can't allocate a new stack for Exec... Strange.\n"));
	    	Early_Alert(CODE_ALLOC_FAIL);
		break;
	    }

	    SysBase->ThisTask->tc_SPUpper = &usp[size];
	    SysBase->ThisTask->tc_SPLower = usp;

	    /* Leave supervisor mode */
	    asm volatile (
	    	"move.l %0,%%usp\n"
	    	"move.w #0,%%sr\n"
	    	"jmp %1@\n"
	    	:
	    	: "a" (&usp[size-3]),
	    	  "a" (doInitCode)
	    	:);
	} while (0);

	/* We shouldn't get here */
	Early_Alert(CODE_EXEC_FAIL);
	for (;;);
}
