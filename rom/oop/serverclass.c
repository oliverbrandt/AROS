/*
    Copyright 1995-1997 AROS - The Amiga Replacement OS
    $Id$

    Desc: Class for server objects.
    Lang: english
*/

#define AROS_ALMOST_COMPATIBLE 1
#include <proto/exec.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <proto/oop.h>
#include <oop/oop.h>
#include <oop/root.h>
#include <oop/meta.h>
#include <oop/server.h>
#include <oop/proxy.h>
#include <oop/ifmeta.h>
#include <string.h>

#include "intern.h"

#undef DEBUG
#define DEBUG 0
#include <aros/debug.h>

/* The server class, allows you to make objects public,
   obtain proxy objects for them from other processes.
   Method calls on the proxies will be transferred
   to the server using IPC, and will then be executed
   on the proxied object. This means method invocation
   will be threadsafe.
*/
   
   
/* Instance data */
struct ServerData
{
    /* List of public objects */
    struct List ObjectList;
    
    /* The MsgPort from which the server recieves invocation requests */
    struct MsgPort *ReceivePort;
    
    /* The public object list needs protection */
    struct SignalSemaphore ObjectListLock;

};

/* structure for making a list of objects */
struct  ServerObjectNode
{
    struct Node so_Node;
    Object *so_Object;
    
};

#define OOPBase ((struct Library *)cl->UserData)

static Object *_Root_New(Class *cl, Object *o, struct P_Root_New *msg)
{
    EnterFunc(bug("Server::New(cl=%s, o=%p, msg=%p)\n",
    		cl->ClassNode.ln_Name, o, msg));
    
    D(bug("DoSuperMethod: %p\n", cl->DoSuperMethod));
    o = (Object *)DoSuperMethod(cl, o, (Msg)msg);
    D(bug("got obj\n"));

    if (o)
    {
    	struct ServerData *data;
	ULONG disp_mid;
    	D(bug("getting instdata\n"));
	
	data = INST_DATA(cl, o);
    	D(bug("got instdata\n"));

	disp_mid = GetMethodID(IID_Root, MO_Root_Dispose);
    	D(bug("got dispmid\n"));
	/* Clear so we can test what resources are allocated in Dispose() */
	D(bug("Object created, o=%p, data=%p\n", o, data));
	memset(data, 0, sizeof (struct ServerData));

	D(bug("Object instdata cleared\n"));	
	NEWLIST(&data->ObjectList);
	InitSemaphore(&data->ObjectListLock);
	
	/* Set up port for receiving incoming invocation requests */
	D(bug("Creating receive msgport\n"));	

	data->ReceivePort = CreateMsgPort();
	if (data->ReceivePort)
	{
	
	    ReturnPtr("Server::New", Object *, o);
	}
	CoerceMethod(cl, o, (Msg)&disp_mid);
	
    }
    ReturnPtr ("Server::New", Object *, NULL);
}

static VOID _Root_Dispose(Class *cl, Object *o, Msg msg)
{
    struct Node *node, *nextnode;
    struct ServerData *data = INST_DATA(cl, o);
    
    EnterFunc(bug("Server::Dispose()\n"));
    
    node = data->ObjectList.lh_Head;
    
    /* Free all nodes in the list */
    while ((nextnode = node->ln_Succ))
    {
    	Remove(node);
    	FreeVec(node->ln_Name);
	FreeMem(node, sizeof (struct ServerObjectNode));
	
	node = nextnode;
    }
    
    /* Free the invocation request port */
    if(data->ReceivePort)
    	DeleteMsgPort(data->ReceivePort);
    
    ReturnVoid("Server::Dispose");
}


/*****************************
**  IServer implementation  **
*****************************/

/* Add an object to the list of public objects. */

static BOOL _Server_AddObject(Class *cl, Object *o, struct P_Server_AddObject *msg)
{
    struct ServerData *data = INST_DATA(cl, o);
     
    struct ServerObjectNode *so;
    
    EnterFunc(bug("Server::AddObject(obj=%p, id=%s)\n", msg->Object, msg->ObjectID));

    /* Allocate list node */     
    so = AllocMem(sizeof (struct ServerObjectNode), MEMF_ANY);
    if (so)
    {
    	/* Copy the object ID */
        so->so_Node.ln_Name = AllocVec(strlen(msg->ObjectID) + 1, MEMF_ANY);
	if (so->so_Node.ln_Name)
	{
	    strcpy(so->so_Node.ln_Name, msg->ObjectID);
	    so->so_Object = msg->Object;
	    
	    /* Add it to the list */
	    ObtainSemaphore(&data->ObjectListLock);
	    AddTail(&data->ObjectList, (struct Node *)so);
	    ReleaseSemaphore(&data->ObjectListLock);
	    
	    ReturnBool ("Server::AddObject", TRUE);
	}
	FreeMem(so, sizeof (struct ServerObjectNode));
    }
    ReturnBool ("Server::AddObject", FALSE);
}

/* Remove a previosly added object */
static VOID _Server_RemoveObject(Class *cl, Object *o, struct P_Server_RemoveObject *msg)
{
    struct ServerData *data = INST_DATA(cl, o);
   
    struct ServerObjectNode *so;
   
    EnterFunc(bug("Server::RemoveObject(objid=%s)\n", msg->ObjectID));

    ObtainSemaphore(&data->ObjectListLock);
    
    /* Find the object */
    so = (struct ServerObjectNode *)FindName(&data->ObjectList, msg->ObjectID);
    if (so)
    {
    	/* ..and remove it */
   	Remove((struct Node *)so);
   	FreeVec(so->so_Node.ln_Name);
	FreeMem(so, sizeof (struct ServerObjectNode) );
    }
    ReleaseSemaphore(&data->ObjectListLock);

    ReturnVoid("Server::RemoveObject");
}

/* Find a public object */
static Object * _Server_FindObject(Class *cl, Object *o, struct P_Server_FindObject *msg)
{
    struct ServerObjectNode *so;
    struct ServerData *data = INST_DATA(cl, o);
    
    EnterFunc(bug("Server::FindObject(objid=%s)\n", msg->ObjectID));
    
    /* See if the server knows about the object */
    ObtainSemaphoreShared(&data->ObjectListLock);
    /* Search for object */
    so = (struct ServerObjectNode *)FindName(&data->ObjectList, msg->ObjectID);
    ReleaseSemaphore(&data->ObjectListLock);

    if (so)
    {
    	/* If found, create a proxy for the object */
	struct TagItem proxy_tags[] =
	{
	    {A_Proxy_RealObject,	(IPTR)so->so_Object},
	    {A_Proxy_Port,		(IPTR)data->ReceivePort},
	    {TAG_DONE,	0UL}
	};
	
	Object *proxy;
	
	proxy = NewObject(NULL, CLID_Proxy, proxy_tags);
	if (proxy)
	{
	    ReturnPtr("Server::FindObject", Object *, proxy);
	}
    }
    
    ReturnPtr("Server::FindObject", Object *, NULL);
}
/* Let the server wait forevere, executing and replying
   answering to all incoming method invocation requests
*/   
static VOID _Server_Run(Class *cl, Object *o, Msg msg)
{
     struct ServerData *data = INST_DATA(cl, o);
     
     for (;;)
     {
     	struct ProxyMsg *pm;
	/* Wait for incoming invocation request */
     	WaitPort(data->ReceivePort);
	
	while ( (pm = (struct ProxyMsg *)GetMsg(data->ReceivePort)) )
	{
	    /* Execute method */
	    pm->pm_RetVal = DoMethod(pm->pm_Object, pm->pm_ObjMsg);
	    
	    /* Reply back to task invoking */
	    ReplyMsg((struct Message *)pm);
	}
	
     }
     
     return;
}

#undef OOPBase

Class *init_serverclass(struct Library *OOPBase)
{

    struct MethodDescr root_methods[] =
    {
	{(IPTR (*)())_Root_New,			MO_Root_New},
	{(IPTR (*)())_Root_Dispose,		MO_Root_Dispose},
	{ NULL, 0UL }
    };
    
    struct MethodDescr server_methods[] =
    {
	{(IPTR (*)())_Server_AddObject,		MO_Server_AddObject},
	{(IPTR (*)())_Server_RemoveObject,	MO_Server_RemoveObject},
	{(IPTR (*)())_Server_FindObject,	MO_Server_FindObject},
	{(IPTR (*)())_Server_Run,		MO_Server_Run},
	{ NULL, 0UL }
    };
    
    struct InterfaceDescr ifdescr[] =
    {
    	{ root_methods,		IID_Root, 2},
    	{ server_methods,	IID_Server, 4},
	{ NULL, 0UL, 0UL}
    };
    
    struct TagItem tags[] =
    {
        {A_Meta_SuperID,		(IPTR)CLID_Root},
	{A_Meta_InterfaceDescr,		(IPTR)ifdescr},
	{A_Meta_ID,			(IPTR)CLID_Server},
	{A_Meta_InstSize,		(IPTR)sizeof (struct ServerData)},
	{TAG_DONE, 0UL}
    };

    
    Class *cl;
    
    EnterFunc(bug("InitServerClass()\n"));
    
    cl = (Class *)NewObject(NULL, CLID_IFMeta, tags);
    if (cl)
    {
        cl->UserData = OOPBase;
    	AddClass(cl);
    }
    
    ReturnPtr ("InitServerClass", Class *, cl);
}
