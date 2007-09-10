#include <inttypes.h>
#include <stdio.h>
#include <asm/cpu.h>
#include <asm/segments.h>
#include <aros/libcall.h>
#include <aros/asmcall.h>
#include <exec/execbase.h>
#include <hardware/intbits.h>
#include "kernel_intern.h"


AROS_LH2(void *, KrnAddIRQHandler,
         AROS_LHA(uint8_t, irq, D0),
         AROS_LHA(void *, handler, A0),
         struct KernelBase *, KernelBase, 7, Kernel)
{
    AROS_LIBFUNC_INIT
    
    void *handle = NULL;
    
    
    return handle;
    
    AROS_LIBFUNC_EXIT
}

AROS_LH1(void, KrnRemIRQHandler,
         AROS_LHA(void *, handle, A0),
         struct KernelBase *, KernelBase, 8, Kernel)
{
    AROS_LIBFUNC_INIT

    AROS_LIBFUNC_EXIT
}


static struct int_gate_64bit IGATES[256] __attribute__((used,aligned(256)));
const struct
{
    uint16_t size __attribute__((packed));
    uint64_t base __attribute__((packed));
} IDT_sel = {sizeof(IGATES)-1, (uint64_t)IGATES};

#define STR_(x) #x
#define STR(x) STR_(x)

#define IRQ_NAME_(nr) nr##_intr(void)
#define IRQ_NAME(nr) IRQ_NAME_(IRQ##nr)

#define BUILD_IRQ(nr) \
    void IRQ_NAME(nr); \
    asm(".balign 8  ,0x90\n\t" \
        ".globl IRQ" STR(nr) "_intr\n\t" \
        ".type IRQ" STR(nr) "_intr,@function\n" \
        "IRQ" STR(nr) "_intr: pushq $0; pushq $" #nr "\n\t" \
        "jmp core_EnterInterrupt\n\t" \
        ".size IRQ" STR(nr) "_intr, . - IRQ" STR(nr) "_intr" \
    );

#define BUILD_IRQ_ERR(nr) \
    void IRQ_NAME(nr); \
    asm(".balign 8  ,0x90\n\t" \
        ".globl IRQ" STR(nr) "_intr\n\t" \
        ".type IRQ" STR(nr) "_intr,@function\n" \
        "IRQ" STR(nr) "_intr: pushq $" #nr "\n\t" \
        "jmp core_EnterInterrupt\n\t" \
        ".size IRQ" STR(nr) "_intr, . - IRQ" STR(nr) "_intr" \
    );

BUILD_IRQ(0x00)         // Divide-By-Zero Exception
BUILD_IRQ(0x01)         // Debug Exception
BUILD_IRQ(0x02)         // NMI Exception
BUILD_IRQ(0x03)         // Breakpoint Exception
BUILD_IRQ(0x04)         // Overflow Exception
BUILD_IRQ(0x05)         // Bound-Range Exception
BUILD_IRQ(0x06)         // Invalid-Opcode Exception
BUILD_IRQ(0x07)         // Device-Not-Available Exception
BUILD_IRQ_ERR(0x08)     // Double-Fault Exception
BUILD_IRQ(0x09)         // Unused (used to be Coprocesor-Segment-Overrun)
BUILD_IRQ_ERR(0x0a)     // Invalid-TSS Exception
BUILD_IRQ_ERR(0x0b)     // Segment-Not-Present Exception
BUILD_IRQ_ERR(0x0c)     // Stack Exception
BUILD_IRQ_ERR(0x0d)     // General-Protection Exception
BUILD_IRQ_ERR(0x0e)     // Page-Fault Exception
BUILD_IRQ(0x0f)         // Reserved
BUILD_IRQ(0x10)         // Floating-Point Exception
BUILD_IRQ_ERR(0x11)     // Alignment-Check Exception
BUILD_IRQ(0x12)         // Machine-Check Exception
BUILD_IRQ(0x13)         // SIMD-Floating-Point Exception
BUILD_IRQ(0x14) BUILD_IRQ(0x15) BUILD_IRQ(0x16) BUILD_IRQ(0x17) 
BUILD_IRQ(0x18) BUILD_IRQ(0x19) BUILD_IRQ(0x1a) BUILD_IRQ(0x1b)
BUILD_IRQ(0x1c) BUILD_IRQ(0x1d) BUILD_IRQ(0x1e) BUILD_IRQ(0x1f)

#define B(x,y) BUILD_IRQ(x##y)
#define B16(x) \
    B(x,0) B(x,1) B(x,2) B(x,3) B(x,4) B(x,5) B(x,6) B(x,7) \
    B(x,8) B(x,9) B(x,a) B(x,b) B(x,c) B(x,d) B(x,e) B(x,f)

B16(0x2)
BUILD_IRQ(0x80)
BUILD_IRQ(0xfe) // APIC timer

#define SAVE_REGS        \
        "pushq %rax; pushq %rbp; pushq %rbx; pushq %rdi; pushq %rsi; pushq %rdx;"  \
        "pushq %rcx; pushq %r8; pushq %r9; pushq %r10; pushq %r11; pushq %r12;"  \
        "pushq %r13; pushq %r14; pushq %r15; mov %ds,%eax; pushq %rax;"
        
#define RESTORE_REGS    \
        "popq %rax; mov %ax,%ds; mov %ax,%es; popq %r15; popq %r14; popq %r13;"  \
        "popq %r12; popq %r11; popq %r10; popq %r9; popq %r8; popq %rcx;" \
        "popq %rdx; popq %rsi; popq %rdi; popq %rbx; popq %rbp; popq %rax"

asm(
"                .balign 32,0x90      \n"
"                .globl core_EnterInterrupt \n"
"                .type core_EnterInterrupt,@function \n"
"core_EnterInterrupt: \n\t" SAVE_REGS "\n"
"                movl    $" STR(KERNEL_DS) ",%eax \n"
"                mov     %ax,%ds \n"
"                mov     %ax,%es \n"
"                movq    %rsp,%rdi \n"
"                call    core_IRQHandle \n"
"                movq    %rsp,%rdi \n"
"                jmp     core_ExitInterrupt \n"
"                .size core_EnterInterrupt, .-core_EnterInterrupt"      
); 

asm(
"                .balign 32,0x90      \n"
"                .globl core_LeaveInterrupt \n"
"                .type core_LeaveInterrupt,@function \n"
"core_LeaveInterrupt: movq %rdi,%rsp \n\t" RESTORE_REGS "\n"                
"                addq $16,%rsp \n"
"                iretq \n"
"                .size core_LeaveInterrupt, .-core_LeaveInterrupt"
);

#define IRQ(x,y) \
    (const void (*)(void))IRQ##x##y##_intr

#define IRQLIST_16(x) \
    IRQ(x,0), IRQ(x,1), IRQ(x,2), IRQ(x,3), \
    IRQ(x,4), IRQ(x,5), IRQ(x,6), IRQ(x,7), \
    IRQ(x,8), IRQ(x,9), IRQ(x,a), IRQ(x,b), \
    IRQ(x,c), IRQ(x,d), IRQ(x,e), IRQ(x,f)

const void __attribute__((section(".text"))) (*interrupt[256])(void) = {
    IRQLIST_16(0x0), IRQLIST_16(0x1), IRQLIST_16(0x2)
};

void core_SetupIDT()
{
    int i;
    uintptr_t off;
    rkprintf("Setting all interrupt handlers to default value\n");

    for (i=0; i < 256; i++)
    {
        if (interrupt[i])
            off = (uintptr_t)interrupt[i];
        else if (i == 0x80)
            off = (uintptr_t)&IRQ0x80_intr;
        else if (i == 0xfe)
            off = (uintptr_t)&IRQ0xfe_intr;
        else
            off = (uintptr_t)&core_DefaultIRETQ;

        IGATES[i].offset_low = off & 0xffff;
        IGATES[i].offset_mid = (off >> 16) & 0xffff;
        IGATES[i].offset_high = (off >> 32) & 0xffffffff;
        IGATES[i].type = 0x0e;
        IGATES[i].dpl = 3;
        IGATES[i].p = 1;
        IGATES[i].selector = KERNEL_CS;
        IGATES[i].ist = 0;
    }

    asm volatile ("lidt %0"::"m"(IDT_sel));
}


void core_Cause(struct ExecBase *SysBase)
{
    struct IntVector *iv = &SysBase->IntVects[INTB_SOFTINT];

    /* If the SoftInt vector in SysBase is set, call it. It will do the rest for us */
    if (iv->iv_Code)
    {
        AROS_UFC5(void, iv->iv_Code,
            AROS_UFCA(ULONG, 0, D1),
            AROS_UFCA(ULONG, 0, A0),
            AROS_UFCA(APTR, 0, A1),
            AROS_UFCA(APTR, iv->iv_Code, A5),
            AROS_UFCA(struct ExecBase *, SysBase, A6)
        );
    }
}


void core_IRQHandle(regs_t regs)
{
    struct ExecBase *SysBase = TLS_GET(SysBase);        // *(struct ExecBase **)4;
    
    rkprintf("IRQ %02x:", regs.irq_number);
    
    if (regs.irq_number == 0xfe)
        while(1);
    
    if (regs.irq_number == 0x03)        /* Debug */
    {
        rkprintf("  INT3 debug fault!\n");

        rkprintf("  stack=%04x:%012x rflags=%016x ip=%04x:%012x err=%08x\n",
                 regs.return_ss, regs.return_rsp, regs.return_rflags, 
                 regs.return_cs, regs.return_rip, regs.error_code);

        rkprintf("  rax=%016lx rbx=%016lx rcx=%016lx rdx=%016lx\n", regs.rax, regs.rbx, regs.rcx, regs.rdx);
        rkprintf("  rsi=%016lx rdi=%016lx rbp=%016lx rsp=%016lx\n", regs.rsi, regs.rdi, regs.rbp, regs.return_rsp);
        rkprintf("  r08=%016lx r09=%016lx r10=%016lx r11=%016lx\n", regs.r8, regs.r9, regs.r10, regs.r11);
        rkprintf("  r12=%016lx r13=%016lx r14=%016lx r15=%016lx\n", regs.r12, regs.r13, regs.r14, regs.r15);
    }
    else if (regs.irq_number == 0x0D)        /* GPF */
    {
        rkprintf("  GENERAL PROTECTION FAULT!\n");

        rkprintf("  stack=%04x:%012x rflags=%016x ip=%04x:%012x err=%08x\n",
                 regs.return_ss, regs.return_rsp, regs.return_rflags, 
                 regs.return_cs, regs.return_rip, regs.error_code);

        rkprintf("  rax=%016lx rbx=%016lx rcx=%016lx rdx=%016lx\n", regs.rax, regs.rbx, regs.rcx, regs.rdx);
        rkprintf("  rsi=%016lx rdi=%016lx rbp=%016lx rsp=%016lx\n", regs.rsi, regs.rdi, regs.rbp, regs.return_rsp);
        rkprintf("  r08=%016lx r09=%016lx r10=%016lx r11=%016lx\n", regs.r8, regs.r9, regs.r10, regs.r11);
        rkprintf("  r12=%016lx r13=%016lx r14=%016lx r15=%016lx\n", regs.r12, regs.r13, regs.r14, regs.r15);
        while(1);
    }
    else if (regs.irq_number == 0x0e)        /* Page fault */
    {
        void *ptr = rdcr(cr2);

        rkprintf("  stack=%04x:%012x rflags=%016x ip=%04x:%012x err=%08x\n",
                 regs.return_ss, regs.return_rsp, regs.return_rflags, 
                 regs.return_cs, regs.return_rip, regs.error_code);

        rkprintf("  PAGE FAULT EXCEPTION! %016p\n",ptr);
        while(1);
    }
    else if (regs.irq_number == 0x80) /* Syscall? */
    {
        switch (regs.rax)
        {
            case SC_CAUSE:
                core_Cause(SysBase);
                break;

            case SC_DISPATCH:
                core_Dispatch(&regs);
                break;
                
            case SC_SWITCH:
                core_Switch(&regs);
                break;
            
            case SC_SCHEDULE:
                if (regs.ds != KERNEL_DS)
                    core_Schedule(&regs);
                break;
        }
    }
}

asm(".text\n\t"
    ".globl core_DefaultIRETQ\n\t"
    ".type core_DefaultIRETQ,@function\n"
"core_DefaultIRETQ: iretq");
