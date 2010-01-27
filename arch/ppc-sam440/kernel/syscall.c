#include <asm/amcc440.h>
#include <asm/io.h>
#include "kernel_intern.h"
#include "syscall.h"

extern char * __text_start;
extern char * __text_end;

#define CFGADD(bus,dev,func,reg)    \
    ( 0x80000000 | ((bus)<<16) |    \
    ((dev)<<11) | ((func)<<8) | ((reg)&~3))

typedef union _pcicfg
{
    uint32_t	ul;
    uint16_t	uw[2];
    uint8_t		ub[4];
} pcicfg;

static uint32_t _read_config_long(int reg)
{
	uint32_t temp;
    outl_le(CFGADD(0, 0, 0, reg),(uint32_t *)PCIC0_CFGADDR);
    temp=inl_le(PCIC0_CFGDATA);
    return temp;
}

static uint16_t _read_config_word(int reg)
{
	pcicfg temp;

    temp.ul = _read_config_long(reg);
    return temp.uw[1 - ((reg&2)>>1)];
}

static void _write_config_long(int reg, uint32_t val)
{
	outl_le(CFGADD(0, 0, 0, reg),(uint32_t *)PCIC0_CFGADDR);
	outl_le(val,PCIC0_CFGDATA);
}

static void _write_config_word(int reg, uint16_t val)
{
	pcicfg temp;

	temp.ul = _read_config_long(reg);
	temp.uw[1 - ((reg&2)>>1)] = val;
	_write_config_long(reg, temp.ul);
}


void __attribute__((noreturn)) syscall_handler(regs_t *ctx, uint8_t exception, void *self)
{
    struct KernelBase *KernelBase = getKernelBase();
    int cause = 0;

//    D(bug("[KRN] SysCall handler. context @ %p SC=%d\n", ctx, ctx->gpr[3]));
    
    if ((char*)ctx->srr0 < &__text_start || (char*)ctx->srr0 >= &__text_end)
    {
        D(bug("[KRN] ERROR ERROR! SysCall issued directly outside kernel.resource!!!!!\n"));
        core_LeaveInterrupt(ctx);
    }
    
    switch (ctx->gpr[3])
    {
        case SC_CLI:
            ctx->srr1 &= ~MSR_EE;
            break;
        
        case SC_STI:
            ctx->srr1 |= MSR_EE;
            break;
        
        case SC_SUPERSTATE:
        	ctx->gpr[3] = ctx->srr1;
            ctx->srr1 &= ~MSR_PR;
            break;
        
        case SC_ISSUPERSTATE:
            if (ctx->srr1 & MSR_PR)
                ctx->gpr[3] = 0;
            else
                ctx->gpr[3] = 1;
            break;
        
        case SC_CAUSE:
        	cause = 1;
        	break;
        
        case SC_DISPATCH:
            core_Dispatch(ctx);
            break;
        
        case SC_SWITCH:
            core_Switch(ctx);
            break;
        
        case SC_SCHEDULE:
            core_Schedule(ctx);
            break;
        
        case SC_INVALIDATED:
        {
            char *start = (char*)((IPTR)ctx->gpr[4] & 0xffffffe0);
            char *end = (char*)(((IPTR)ctx->gpr[4] + ctx->gpr[5] + 31) & 0xffffffe0);
            char *ptr;
            
            for (ptr = start; ptr < end; ptr +=32)
            {
                asm volatile("dcbi 0,%0"::"r"(ptr));
            }
            asm volatile("sync");
        }

        case SC_REBOOT:
        {
        	/*
        	 * Hard case on Sam440. First of all, the CPU has to be found on PCI bus.
        	 * The PCI Bus reset signal will be issued there. Further, CPU returns from
        	 * exception to address 0xfffffffc.
        	 */

        	uint64_t newtbu = mftbu() + KernelBase->kb_OPBFreq;
        	D(bug("[KRN] REBOOT..."));
        	while(newtbu > mftbu());
        	D(bug("3..."));
        	newtbu = mftbu() + KernelBase->kb_OPBFreq;
        	while(newtbu > mftbu());
        	D(bug("2..."));
        	newtbu = mftbu() + KernelBase->kb_OPBFreq;
        	while(newtbu > mftbu());
        	D(bug("1..."));
        	newtbu = mftbu() + KernelBase->kb_OPBFreq;
        	while(newtbu > mftbu());
        	D(bug("\n\n\n"));

        	/* PCI Bridge options 2 register */
        	uint16_t val = _read_config_word(0x60);
        	/* Set the PCI reset signal */
        	_write_config_word(0x60, val | (1 << 12));
        	int i;
        	/* Let the PCI reset last as long as needed */
        	for (i=0; i < 100; i++)
        		asm volatile("sync");
        	/* De-assert the PCI reset */
        	_write_config_word(0x60, val & ~(1 << 12));

        	/* Restart CPU */
        	asm volatile("mtsrr0 %0; mtsrr1 %1; rfi"::"r"(0xfffffffc), "r"(1 << 6));
        	while(1);
        }
    }
    
    if (cause)
    	core_ExitInterrupt(ctx);
    else
    	core_LeaveInterrupt(ctx);
}
