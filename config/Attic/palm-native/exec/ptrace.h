#ifndef ASM_PTRACE_H
#define ASM_PTRACE_H

/* this struct defines the way the registers are stored on the 
   stack during a system call. */

struct pt_regs {
	long            usp;
	long            d0;
	long            d1;
	long            d2;
	long            d3;
	long            d4;
	long            d5;
	long            d6;
	long            d7;
	long            a0;
	long            a1;
	long            a2;
	long            a3;
	long            a4;
	long            a5;
	long            a6;
	unsigned short  sr;
	long            pc;
};

#define user_mode(regs) (0 == ((1 << 13) & (regs)->sr))

#endif
