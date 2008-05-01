#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <asm/amcc440.h>
#include "etask.h"
#include "exec_util.h"

#define DEBUG 0

#include <aros/libcall.h>
#include <aros/debug.h>

#define Regs(t) ((struct regs_t *)(GetIntETask(t)->iet_Context))

static UQUAD *PrepareContext_Common(struct Task *task, APTR entryPoint, APTR fallBack,
                                    struct TagItem *tagList, struct ExecBase *SysBase)
{
    context_t   *ctx;
    int         i;
    IPTR        *sp=(IPTR *)((IPTR)task->tc_SPReg & 0xfffffff0);
    IPTR        args[8] = {0};
    WORD        numargs = 0;
 
    while(tagList)
    {
        switch(tagList->ti_Tag)
        {
            case TAG_MORE:
                tagList = (struct TagItem *)tagList->ti_Data;
                continue;

            case TAG_SKIP:
                tagList += tagList->ti_Data;
                break;

            case TAG_DONE:
                tagList = NULL;
                break;
                
            #define HANDLEARG(x) \
            case TASKTAG_ARG ## x: \
                args[x - 1] = (IPTR)tagList->ti_Data; \
                if (x > numargs) numargs = x; \
                break;

            HANDLEARG(1)
            HANDLEARG(2)
            HANDLEARG(3)
            HANDLEARG(4)
            HANDLEARG(5)
            HANDLEARG(6)
            HANDLEARG(7)
            HANDLEARG(8)

            #undef HANDLEARG
            }

        if (tagList) tagList++;
    }

    if (!(task->tc_Flags & TF_ETASK) )
        return NULL;

    /* Get the memory for CPU context. Alloc it with MEMF_CLEAR flag */
    GetIntETask (task)->iet_Context = AllocTaskMem (task
        , SIZEOF_ALL_REGISTERS
        , MEMF_PUBLIC|MEMF_CLEAR
    );
    
    D(bug("[exec] PrepareContext: iet_Context = %012p\n", GetIntETask (task)->iet_Context));
    
    if (!(ctx = (context_t *)GetIntETask (task)->iet_Context))
        return NULL;

    if (numargs)
    {
        switch (numargs)
        {
            case 8:
                ctx->cpu.gpr[10] = args[7];
            case 7:
                ctx->cpu.gpr[9] = args[6];
            case 6:
                ctx->cpu.gpr[8] = args[5];
            case 5:
                ctx->cpu.gpr[7] = args[4];
            case 4:
                ctx->cpu.gpr[6] = args[3];
            case 3:
                ctx->cpu.gpr[5] = args[2];
            case 2:
                ctx->cpu.gpr[4] = args[1];
            case 1:
                ctx->cpu.gpr[3] = args[0];
                break;
        }
    }

    /* Push fallBack address */
    ctx->cpu.lr = fallBack;
    /* 
     * Task will be started upon interrupt resume. Push entrypoint into SRR0 
     * and the MSR register into SRR1. Enable FPU at the beginning
     */
    ctx->cpu.srr0 = (IPTR)entryPoint;
    ctx->cpu.srr1 = MSR_PR | MSR_EE | MSR_CE | MSR_ME;
    ctx->cpu.srr1 |= MSR_FP;
    ctx->cpu.gpr[1] = sp;
    
    task->tc_SPReg = sp;
   
    D(bug("[exec] New context:\n[exec] SRR0=%08x, SRR1=%08x\n",ctx->cpu.srr0, ctx->cpu.srr1));
    D(bug("[exec] GPR00=%08x GPR01=%08x GPR02=%08x GPR03=%08x\n",
             ctx->cpu.gpr[0],ctx->cpu.gpr[1],ctx->cpu.gpr[2],ctx->cpu.gpr[3]));
    D(bug("[exec] GPR04=%08x GPR05=%08x GPR06=%08x GPR07=%08x\n",
             ctx->cpu.gpr[4],ctx->cpu.gpr[5],ctx->cpu.gpr[6],ctx->cpu.gpr[7]));
    D(bug("[exec] GPR08=%08x GPR09=%08x GPR10=%08x GPR11=%08x\n",
             ctx->cpu.gpr[8],ctx->cpu.gpr[9],ctx->cpu.gpr[10],ctx->cpu.gpr[11]));
    D(bug("[exec] GPR12=%08x GPR13=%08x GPR14=%08x GPR15=%08x\n",
             ctx->cpu.gpr[12],ctx->cpu.gpr[13],ctx->cpu.gpr[14],ctx->cpu.gpr[15]));

    D(bug("[exec] GPR16=%08x GPR17=%08x GPR18=%08x GPR19=%08x\n",
             ctx->cpu.gpr[16],ctx->cpu.gpr[17],ctx->cpu.gpr[18],ctx->cpu.gpr[19]));
    D(bug("[exec] GPR20=%08x GPR21=%08x GPR22=%08x GPR23=%08x\n",
             ctx->cpu.gpr[20],ctx->cpu.gpr[21],ctx->cpu.gpr[22],ctx->cpu.gpr[23]));
    D(bug("[exec] GPR24=%08x GPR25=%08x GPR26=%08x GPR27=%08x\n",
             ctx->cpu.gpr[24],ctx->cpu.gpr[25],ctx->cpu.gpr[26],ctx->cpu.gpr[27]));
    D(bug("[exec] GPR28=%08x GPR29=%08x GPR30=%08x GPR31=%08x\n",
             ctx->cpu.gpr[28],ctx->cpu.gpr[29],ctx->cpu.gpr[30],ctx->cpu.gpr[31]));
    return sp;
}

AROS_LH4(BOOL, PrepareContext,
    AROS_LHA(struct Task *, task, A0),
    AROS_LHA(APTR, entryPoint, A1),
    AROS_LHA(APTR, fallBack, A2),
    AROS_LHA(struct TagItem *, tagList, A3),
    struct ExecBase *, SysBase, 6, Exec)
{
    AROS_LIBFUNC_INIT

    return PrepareContext_Common(task, entryPoint, fallBack, tagList, SysBase) ? TRUE : FALSE;

    AROS_LIBFUNC_EXIT
}
