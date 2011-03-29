#ifndef EXEC_EXECBASE_H
#define EXEC_EXECBASE_H

/*
    Copyright � 1995-2011, The AROS Development Team. All rights reserved.
    $Id$

    Desc: structure of exec.library
    Lang: english
*/

#ifndef EXEC_INTERRUPTS_H
#   include <exec/interrupts.h>
#endif
#ifndef EXEC_LIBRARIES_H
#   include <exec/libraries.h>
#endif
#ifndef EXEC_LISTS_H
#   include <exec/lists.h>
#endif
#ifndef EXEC_TASKS_H
#   include <exec/tasks.h>
#endif

/* Most fields are PRIVATE */
struct ExecBase
{
/* Standard Library Structure */
    struct Library LibNode;

/* System Constants */
    UWORD SoftVer;      /* OBSOLETE */
    WORD  LowMemChkSum;
    IPTR  ChkBase;
    APTR  ColdCapture;
    APTR  CoolCapture;
    APTR  WarmCapture;
    APTR  SysStkUpper;  /* System Stack Bounds */
    APTR  SysStkLower;
    IPTR  MaxLocMem;    /* Chip Memory Pointer */
    APTR  DebugEntry;
    APTR  DebugData;
    APTR  AlertData;
    APTR  MaxExtMem;    /* Extended Memory Pointer (may be NULL) */
    UWORD ChkSum;       /* SoftVer to MaxExtMem */

/* Interrupts */
    struct IntVector IntVects[16];

/* System Variables */
    struct Task * ThisTask;       /* Pointer to currently running task
                                     (readable) */
    ULONG        IdleCount;
    ULONG        DispCount;
    UWORD        Quantum;        /* # of ticks, a task may run */
    UWORD        Elapsed;        /* # of ticks, the current task has run */
    UWORD        SysFlags;
    BYTE         IDNestCnt;
    BYTE         TDNestCnt;
    UWORD        AttnFlags;      /* Attention Flags (see below) (readable) */
    UWORD        AttnResched;
    APTR         ResModules;
    APTR         TaskTrapCode;
    APTR         TaskExceptCode;
    APTR         TaskExitCode;
    ULONG        TaskSigAlloc;
    UWORD        TaskTrapAlloc;

/* PRIVATE Lists */
    struct List        MemList;
    struct List        ResourceList;
    struct List        DeviceList;
    struct List        IntrList;
    struct List        LibList;
    struct List        PortList;
    struct List        TaskReady;      /* Tasks that are ready to run */
    struct List        TaskWait;       /* Tasks that wait for some event */
    struct SoftIntList SoftInts[5];

/* Miscellaneous Stuff */
    LONG               LastAlert[4];

    UBYTE              VBlankFrequency;      /* (readable) */
    UBYTE              PowerSupplyFrequency; /* (readable) */
    	    	    	    	    	     /* AROS PRIVATE: VBlankFreq * PowerSupplyFreq = Timer Tick Rate */
    struct List        SemaphoreList;

/* Kickstart */
    APTR KickMemPtr;
    APTR KickTagPtr;
    APTR KickCheckSum;

/* Miscellaneous Stuff */
    UWORD          ex_Pad0;            /* PRIVATE */
    IPTR           ex_LaunchPoint;     /* PRIVATE */
    APTR           ex_RamLibPrivate;
    ULONG          ex_EClockFrequency; /* (readable) */
    ULONG          ex_CacheControl;    /* PRIVATE */
    ULONG          ex_TaskID;
    ULONG          ex_Reserved1[5];
    APTR           ex_MMULock;         /* PRIVATE */
    ULONG          ex_Reserved2[2];
    ULONG	   ex_DebugFlags;
    struct MinList ex_MemHandlers;
    APTR           ex_MemHandler;      /* PRIVATE */

/* Additional field for AROS */
    struct Library      * DebugAROSBase;
};

/* AttnFlags */
/* Processors */
#define AFB_68010        0
#define AFF_68010   (1L<<0)
#define AFB_68020        1
#define AFF_68020   (1L<<1)
#define AFB_68030        2
#define AFF_68030   (1L<<2)
#define AFB_68040        3
#define AFF_68040   (1L<<3)
#define AFB_68060        7
#define AFF_68060   (1L<<7)
/* Co-Processors */
#define AFB_68881        4
#define AFF_68881   (1L<<4)
#define AFB_68882        5
#define AFF_68882   (1L<<5)
#define AFB_FPU40        6
#define AFF_FPU40   (1L<<6)
#define AFB_PRIVATE      15 /* PRIVATE */
#define AFF_PRIVATE (1L<<15)

/* Cache */
#define CACRF_EnableI       (1L<<0)
#define CACRF_FreezeI       (1L<<1)
#define CACRF_ClearI        (1L<<3)
#define CACRF_IBE           (1L<<4)
#define CACRF_EnableD       (1L<<8)
#define CACRF_FreezeD       (1L<<9)
#define CACRF_ClearD        (1L<<11)
#define CACRF_DBE           (1L<<12)
#define CACRF_WriteAllocate (1L<<13)
#define CACRF_InvalidateD   (1L<<15)
#define CACRF_EnableE       (1L<<30)
#define CACRF_CopyBack      (1L<<31)

/* DMA */
#define DMA_Continue    (1L<<1)
#define DMA_NoModify    (1L<<2)
#define DMA_ReadFromRAM (1L<<3)

/*
 * Runtime debug output flags, MorphOS-compatible.
 * Most of them are reserved for now.
 */
#define EXECDEBUGF_INITRESIDENT     0x00000001	/* Single resident initialization	*/
#define EXECDEBUGF_INITCODE         0x00000002	/* Kickstart initialization		*/
#define EXECDEBUGF_FINDRESIDENT     0x00000004	/* Resident search			*/
#define EXECDEBUGF_CREATELIBRARY    0x00000010	/* Library creation			*/
#define EXECDEBUGF_SETFUNCTION      0x00000020	/* Library function patching		*/
#define EXECDEBUGF_NEWSETFUNCTION   0x00000040
#define EXECDEBUGF_CHIPRAM          0x00000080
#define EXECDEBUGF_ADDTASK          0x00000100	/* Task creation			*/
#define EXECDEBUGF_REMTASK          0x00000200	/* Task removal				*/
#define EXECDEBUGF_GETTASKATTR      0x00000400
#define EXECDEBUGF_SETTASKATTR      0x00000800
#define EXECDEBUGF_EXCEPTHANDLER    0x00001000
#define EXECDEBUGF_ADDDOSNODE       0x00002000
#define EXECDEBUGF_PCI              0x00004000
#define EXECDEBUGF_RAMLIB           0x00008000
#define EXECDEBUGF_NOLOGSERVER      0x00010000
#define EXECDEBUGF_NOLOGWINDOW      0x00020000
#define EXECDEBUGF_LOGFILE          0x00040000
#define EXECDEBUGF_LOGKPRINTF       0x00080000
#define EXECDEBUGF_PERMMEMTRACK     0x00100000
#define EXECDEBUGF_MEMTRACK         0x00200000
#define EXECDEBUGF_CYBERGUARDDEADLY 0x00400000
#define EXECDEBUGF_LOGEXTENDED      0x00800000
#define EXECDEBUGF_LOADSEG          0x01000000
#define EXECDEBUGF_UNLOADSEG        0x02000000
#define EXECDEBUGF_PPCSTART         0x04000000
#define EXECDEBUGF_CGXDEBUG         0x08000000
#define EXECDEBUGF_INVZEROPAGE      0x10000000
#define EXECDEBUGF_INIT             0x40000000	/* Generic system startup		*/
#define EXECDEBUGF_LOG              0x80000000

/*
 * The following definitions are private!
 */

/* SysFlags */
#define SFF_SoftInt         (1L<<5)  /* There is a software interrupt */
#define SFF_QuantumOver     (1L<<13) /* Task's time slice is over     */

/* AttnFlags */
#define ARF_AttnSwitch      (1L<<7)  /* Delayed Switch() pending   */
#define ARF_AttnDispatch    (1L<<15) /* Delayed Dispatch() pending */

#endif /* EXEC_EXECBASE_H */
