/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$
*/

#ifndef SERIAL_HIDD_INTERN_H
#define SERIAL_HIDD_INTERN_H

/* Include files */

#include <sys/termios.h>

#ifndef EXEC_LIBRARIES_H
#   include <exec/libraries.h>
#endif
#ifndef OOP_OOP_H
#   include <oop/oop.h>
#endif
#ifndef HIDD_SERIAL_H
#   include <hidd/serial.h>
#endif
#include <dos/dos.h>

#define SER_MAX_UNITS	4

struct HIDDSerialData
{
    OOP_Class 		*SerialHIDDClass;

    OOP_Object		*SerialUnits[SER_MAX_UNITS];
    UBYTE		usedunits;
};


#define SER_UNIT_0_F	1
#define SER_UNIT_1_F	2
#define SER_UNIT_2_F	4
#define SER_UNIT_3_F	8


struct class_static_data
{
    struct ExecBase      * sysbase;
    struct Library       * utilitybase;
    struct Library       * oopbase;

    OOP_Class		 *serialhiddclass;
    OOP_Class		 *serialunitclass;
};

struct HIDDSerialUnitData
{
    VOID (*DataWriteCallBack)	(ULONG unitnum, APTR userdata);
    VOID (*DataReceivedCallBack)(UBYTE *buffer, ULONG len, ULONG unitnum, APTR userdata);
    VOID 		*DataWriteUserData;
    VOID		*DataReceivedUserData;
    
    ULONG 		unitnum;
    int			filedescriptor;
    ULONG		baudrate;
    UBYTE		datalength;
    BOOL		parity;
    UBYTE		paritytype;
    UBYTE		stopbits;
    BOOL		breakcontrol;
    
    struct MsgPort	*replyport_read;
    struct Interrupt 	*softint_read;
    HIDD		unixio_read;
    
    struct MsgPort	*replyport_write;
    struct Interrupt 	*softint_write;
    HIDD		unixio_write;
    
    struct termios	orig_termios;
};


#define SER_DEFAULT_BAUDRATE	57600


/* Library base */

struct IntHIDDSerialBase
{
    struct Library            hdg_LibNode;
    BPTR                      hdg_SegList;
    struct ExecBase          *hdg_SysBase;
    struct Library           *hdg_UtilityBase;

    struct class_static_data *hdg_csd;
};


#define CSD(x) ((struct class_static_data *)x)

#undef SysBase
#define SysBase (CSD(cl->UserData)->sysbase)

#undef UtilityBase
#define UtilityBase (CSD(cl->UserData)->utilitybase)

#undef OOPBase
#define OOPBase (CSD(cl->UserData)->oopbase)


/* pre declarations */

OOP_Class *init_serialhiddclass(struct class_static_data *csd);
void   free_serialhiddclass(struct class_static_data *csd);

OOP_Class *init_serialunitclass(struct class_static_data *csd);
void   free_serialunitclass(struct class_static_data *csd);


#endif /* SERIAL_HIDD_INTERN_H */
