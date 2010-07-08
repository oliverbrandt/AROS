/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Windows-hosted version of Disable()
    Lang: english
*/

#include <exec/tasks.h>
#include <exec/execbase.h>
#include <aros/libcall.h>
#include <aros/atomic.h>
#include <proto/exec.h>
#include <proto/kernel.h>

#undef  Exec
#ifdef UseExecstubs
#    define Exec _Exec
#endif

AROS_LH0(void, Disable,
    struct ExecBase *, SysBase, 20, Exec)
{
#undef Exec
    AROS_LIBFUNC_INIT
    
    /* Georg Steger */
    KrnCli();

    AROS_ATOMIC_INC(SysBase->IDNestCnt);

    AROS_LIBFUNC_EXIT
}
