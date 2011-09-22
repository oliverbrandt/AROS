#ifndef ___EXITFUNC_H
#define ___EXITFUNC_H

/*
    Copyright � 1995-2011, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <exec/lists.h>
#include <exec/nodes.h>
#include <aros/relbase.h>

struct AtExitNode
{
    struct Node node;
    union
    {
        void (*fvoid)(void);
        void (*fptr)(int, void *);
    } func;
    void *ptr, *relbase;
};

#define AEN_VOID 0
#define AEN_PTR 1

int __addexitfunc(struct AtExitNode *aen);
void __callexitfuncs(void);

#endif
