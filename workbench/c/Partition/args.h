#ifndef _ARGS_H_
#define _ARGS_H_

/*
    Copyright � 2004, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <exec/types.h>

/*** Structures *************************************************************/
enum Argument
{
    DEVICE,
    UNIT,
    SYSSIZE, /* Size of System partition in MBs */
    WORKSIZE, /* Size of Work partition in MBs */
    MAXWORK, /* Create maximum-sized Work partition */
    WIPE,   /* Destroy all existing partitions */
    FORCE,  /* Don't ask the user for permission */
    QUIET,  /* Don't print anything */
    COUNT   /* Number of arguments */
};

/*** Prototypes *************************************************************/
BOOL ReadArguments(VOID);
VOID FreeArguments(VOID);
IPTR GetArgument(enum Argument arg);

/*** Macros *****************************************************************/
#define ARG(a) GetArgument((a))

#endif /* _ARGS_H_ */
