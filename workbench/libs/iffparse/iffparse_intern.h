#ifndef IFFPARSE_INTERN_H
#define IFFPARSE_INTERN_H

/* Include files */

#ifndef EXEC_TYPES_H
#   include <exec/types.h>
#endif
#ifndef CLIB_ALIB_PROTOS_H
#   include <clib/alib_protos.h>
#endif
#ifndef PROTO_EXEC_H
#   include <proto/exec.h>
#endif
#ifndef PROTO_DOS_H
#   include <proto/dos.h>
#endif
#ifndef PROTO_UTILITY_H
#   include <proto/utility.h>
#endif
#ifndef PROTO_IFFPARSE_H
#   include <proto/iffparse.h>
#endif
#ifndef LIBRARIES_IFFPARSE_H
#   include <libraries/iffparse.h>
#endif
#ifndef EXEC_MEMORY_H
#   include <exec/memory.h>
#endif
#ifndef EXEC_LIBRARIES_H
#   include <exec/libraries.h>
#endif
#ifndef AROS_LIBCALL_H
#   include <aros/libcall.h>
#endif
#ifndef DOS_DOS_H
#   include <dos/dos.h>
#endif
#ifndef UTILITY_HOOKS_H
#   include <utility/hooks.h>
#endif
#include <stdlib.h>


/* Some external stuff (iffparse_init.c) */


struct IFFParseBase_intern; /* prerefrence */

/* Internal prototypes */
LONG ReadStream      (struct IFFHandle *, APTR, LONG, struct IFFParseBase_intern *);
LONG ReadStreamLong  (struct IFFHandle *, APTR, struct IFFParseBase_intern *);
LONG WriteStream     (struct IFFHandle *, APTR, LONG, struct IFFParseBase_intern *);
LONG WriteStreamLong (struct IFFHandle *, APTR, struct IFFParseBase_intern *);
VOID PurgeLCI	     (struct LocalContextItem *, struct IFFParseBase_intern *);

LONG PushContextNode (struct IFFHandle *, LONG ,LONG, LONG, LONG, struct IFFParseBase_intern *);
VOID PopContextNode  (struct IFFHandle *, struct IFFParseBase_intern *);
LONG GetChunkHeader  (struct IFFHandle *, struct IFFParseBase_intern *);
LONG InvokeHandlers  (struct IFFHandle *, LONG, LONG, struct IFFParseBase_intern *);
LONG SeekStream      (struct IFFHandle *, LONG, struct IFFParseBase_intern *);

/* Some system entry & exit handlers (hook funcs) �*/
LONG ExitContextFunc(struct Hook *, APTR, APTR);
LONG StopFunc	    (struct Hook *, APTR, APTR);
LONG PropFunc	    (struct Hook *, struct IFFHandle *, APTR);
LONG CollectionFunc (struct Hook *, struct IFFHandle *, APTR);

/* A system purge hook for purging the LCIs installed by PropChunk and CollectionChunk */
IPTR CollectionPurgeFunc (struct Hook *, struct LocalContextItem *, ULONG);
IPTR PropPurgeFunc	 (struct Hook *, struct LocalContextItem *, ULONG);

/* Buffer functions */
struct BufferList * AllocBuffer (ULONG, struct IFFParseBase_intern *);
VOID		    FreeBuffer	(struct BufferList *, struct IFFParseBase_intern *);

struct BufferNode * AllocBufferNode (struct BufferList *, struct IFFParseBase_intern *);

LONG WriteToBuffer  (struct BufferList *, UBYTE *, LONG, struct IFFParseBase_intern *);
BOOL SeekBuffer     (struct BufferList *, LONG);

BOOL BufferToStream (struct BufferList *, struct IFFHandle *, struct IFFParseBase_intern *);

/* StreamHandler hooks */

IPTR DOSStreamHandler  (struct Hook *, struct IFFHandle *, struct IFFStreamCmd *);
IPTR ClipStreamHandler (struct Hook *, struct IFFHandle *, struct IFFStreamCmd *);
IPTR BufStreamHandler  (struct Hook *, struct IFFHandle *, struct IFFStreamCmd *);

/* Message port help functions */

BOOL InitPort  (struct MsgPort *, struct Task *, struct IFFParseBase_intern *);
VOID ClosePort (struct MsgPort *, struct IFFParseBase_intern *);


/* Buffered stream install/deinstall */
LONG InitBufferedStream (struct IFFHandle*, struct IFFParseBase_intern *);
LONG ExitBufferedStream (struct IFFHandle*, struct IFFParseBase_intern *);

/* Endian conversion */
LONG SwitchIfLittleEndian(LONG);

/* Private flags */

#define IFFF_OPEN	    (1L << 19)

/*
    The different states the parser can be in.
    Chose values so that the compiler easily can create a jump table
*/
#define IFFSTATE_COMPOSITE  0
#define IFFSTATE_PUSHCHUNK  1
#define IFFSTATE_ATOMIC     2
#define IFFSTATE_SCANEXIT   3
#define IFFSTATE_EXIT	    4
#define IFFSTATE_POPCHUNK   5

/* Size of one tile of a buffer to use in buffering of chunks */
#define BUFFERNODESIZE	    10000

/* Size of buffer fake a forward seek with Read()s */
#define SEEKBUFSIZE	    10000


/************************/
/* Internal structures	*/
/************************/

/* Structure for maintaing a list of buffers for buffering */
struct BufferList
{
    struct MinList bl_ListHeader;

    /* The size of each buffer */
    ULONG bl_BufferNodeSize;

    /* The current position of the Write/Seek pointer */

    /* A buffer node */
    struct BufferNode * bl_CurrentNode;
    /* An offset pointer into that particular node. */
    ULONG bl_CurrentOffset;
    /*
	The number of the current buffernode (the number in the list).
	First buffenode is number 1 (not 0)
    */
    ULONG bl_CurrentNodeNum;

    /* Number of bytes written to the bufferlist */
    ULONG bl_BufferSize;

    /* Number of nodes in the list */

};

/*
    One node in the bufferlist.
    Contains a pointer to the buffer - tile itself
*/

struct BufferNode
{
    struct MinNode bn_Node;
    UBYTE	 * bn_BufferPtr;

};


/*
    Internal version of a ContextNode. Contains a list of
    LocalContextItems.
*/
struct IntContextNode
{
    struct ContextNode CN;
    struct MinList     cn_LCIList;

    /* True if this chunk is a FORM, LIST or CAT */
    BOOL	       cn_Composite;
};

#define GetIntCN(cn)   ((struct IntContextNode *)(cn))
#define GetCN(cn)      (&GetIntCN(cn)->CN))


/*
    Internal version of the IffHandle data structure. Contains quite a bit
    of useful information, eg the current context etc.
*/


struct IntIFFHandle
{
    struct IFFHandle IH;

    /* The default context node is built-in into the iffhandle */
    struct IntContextNode iff_DefaultCN;

    /* A linkedlist simulated stack with context nodes */
    struct MinList iff_CNStack;

    /* containd the state in which the parser currently is */
    ULONG iff_CurrentState;

    /*
	The context depth at which we have started buffering.
	0 if no buffering is needed.
    */
    ULONG iff_BufferStartDepth;

    /* Here InitIff store it's parameters */
    struct Hook * iff_StreamHandler;

    /*
	Entrys to preserve the StreamHandler, stream  and the stream flags
	whenever the buffering streamhandle is inserted
    */

    struct Hook * iff_PreservedHandler;
    LONG	  iff_PreservedFlags;
    IPTR	  iff_PreservedStream;
};
#define GetIntIH(ih) ((struct IntIFFHandle *)(ih))
#define GetIH(ih)    (&GetIntIH(ih)->IH)

/*
    Internal version of the LocalContextItem structure. Contains
    the dataSize and a Hook for purging...
*/

struct IntLocalContextItem
{
    struct LocalContextItem LCI;

    struct Hook * lci_PurgeHook;
    UBYTE	* lci_UserData;
    ULONG	  lci_UserDataSize;
};

#define GetIntLCI(lci) ((struct IntLocalContextItem *)(lci))
#define GetLCI(lci)    (&GetIntLCI(lci)->LCI)

/* LocalItem data in IFFLCI_ENTRY/EXITHANDLER LCIs */

struct HandlerInfo
{
    struct Hook * hi_Hook;
    APTR	  hi_Object;
};


/* LocalItem data inside IFFLCI_PROP LCIs */
struct CIPtr
{
    struct CollectionItem * FirstCI;
};



/* Usefull macros */
#define TopChunk( iff ) (struct ContextNode*)GetIntIH(iff)->iff_CNStack.mlh_Head
#define RootChunk(iff) (struct ContextNode*)GetIntIH(iff)->iff_CNStack.mlh_TailPred

struct IFFParseBase_intern
{
    struct Library    library;
    struct ExecBase * sysbase;
    BPTR	      seglist;
    struct Library  * dosbase;
    struct Library  * utilitybase;

    struct Hook       stophook;
    struct Hook       prophook;
    struct Hook       collectionhook;
    struct Hook       doshook;
    struct Hook       cliphook;
    struct Hook       bufhook;
    struct Hook       collectionpurgehook;
    struct Hook       proppurgehook;
    struct Hook       exitcontexthook;
};

#define IPB(ipb)        ((struct IFFParseBase_intern *)ipb)
#undef SysBase
#define SysBase (IPB(IFFParseBase)->sysbase)
#undef DOSBase
#define DOSBase (IPB(IFFParseBase)->dosbase)
#undef UtilityBase
#define UtilityBase (IPB(IFFParseBase)->utilitybase)

#define expunge() \
AROS_LC0(BPTR, expunge, struct IFFParseBase_intern *, IFFParseBase, 3, IFFParse)

#endif /* IFFPARSE_INTERN_H */
