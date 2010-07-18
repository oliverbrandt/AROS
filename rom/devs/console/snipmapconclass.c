/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Code for CONU_SNIPMAP console units.
*/
#include <string.h>

#include <proto/graphics.h>
#include <proto/intuition.h>
#include <intuition/intuition.h>

#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>

#include <graphics/rastport.h>
#include <aros/asmcall.h>

#define SDEBUG 0
//#define DEBUG 1
#define DEBUG 0
#include <aros/debug.h>

#include "console_gcc.h"
#include "consoleif.h"

struct snipmapcondata
{
  ULONG start_selection_x;
  ULONG start_selection_y;
};

#undef ConsoleDevice
#define ConsoleDevice ((struct ConsoleBase *)cl->cl_UserData)

/***********  SnipMapCon::New()  **********************/

static Object *snipmapcon_new(Class *cl, Object *o, struct opSet *msg)
{
    EnterFunc(bug("SnipMapCon::New()\n"));
    o = (Object *)DoSuperMethodA(cl, o, (Msg)msg);
    if (o)
    {
    	struct snipmapdata *data = INST_DATA(cl, o);
		memset(data, 0, sizeof (struct snipmapcondata));
	}
    ReturnPtr("SnipMapCon::New", Object *, o);
}

/***********  SnipMapCon::Dispose()  **************************/

static VOID snipmapcon_dispose(Class *cl, Object *o, Msg msg)
{
    struct snipmapcondata *data= INST_DATA(cl, o);

    DoSuperMethodA(cl, o, msg);
}

AROS_UFH3S(IPTR, dispatch_snipmapconclass,
    AROS_UFHA(Class *,  cl,  A0),
    AROS_UFHA(Object *, o,   A2),
    AROS_UFHA(Msg,      msg, A1)
)
{
    AROS_USERFUNC_INIT

    IPTR retval = 0UL;

    switch (msg->MethodID)
    {
    case OM_NEW:
        retval = (IPTR)snipmapcon_new(cl, o, (struct opSet *)msg);
	break;

    case OM_DISPOSE:
    	snipmapcon_dispose(cl, o, msg);
	break;

    default:
    	retval = DoSuperMethodA(cl, o, msg);
	break;
    }

    return retval;

    AROS_USERFUNC_EXIT
}

#undef ConsoleDevice

Class *makeSnipMapConClass(struct ConsoleBase *ConsoleDevice)
{

   Class *cl;

   cl = MakeClass(NULL, NULL ,CHARMAPCLASSPTR , sizeof(struct snipmapcondata), 0UL);
   if (cl)
   {
    	cl->cl_Dispatcher.h_Entry = (APTR)dispatch_snipmapconclass;
    	cl->cl_Dispatcher.h_SubEntry = NULL;

    	cl->cl_UserData = (IPTR)ConsoleDevice;

   	return (cl);
    }
    return (NULL);
}

