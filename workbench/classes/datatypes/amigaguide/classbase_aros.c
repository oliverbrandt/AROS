/*
** $PROJECT: class library template
**
** $VER: classbase.c 40.12 (22.08.03)
**
** $AUTHOR: Stefan Ruppert <stefan@ruppert-it.de>
**
** $COPYRIGHT: (C) Copyright 1996-2003 by Stefan Ruppert. All Rights Reserved!
**
** $HISTORY:
**
** 22.08.03 : 040.012 : added MorphOS support
** 12.02.00 : 040.011 : added two more system libraries
** 08.11.98 : 040.010 : added beta version support, removed BOOPSI-Debugger stuff
** 20.06.98 : 040.009 : added support for BOOPSI-Debugger
** 17.06.98 : 040.008 : added support for more system libraries
** 11.09.97 : 040.007 : added global __UtilityBase and SysBase for GNU/C link libraries
** 19.08.97 : 040.006 : UserClassBaseOpen() and UserClassBaseClose() added
** 04.08.97 : 040.005 : added USE_VERSTAG support
** 05.07.97 : 040.004 : added missing entry() function to avoid crashes,
**                      if the library is executed
** 20.06.97 : 040.003 : added gcc support from Gunther Nikl
** 09.11.96 : 040.002 : changed to new ClassLibrary structure from the DevCD
** 15.09.96 : 040.001 : + now uses semaphore for shared data as in AIFFdtc
**                        from Olaf Barthel
**                      + OpenLibrary() now in LibOpen() (suggestion from
**                        Stefan Becker)
**                      + added ROMTag and removed last asm file.
*/

/* disable debug output */
#undef DEBUG

/* ------------------------------- includes ------------------------------- */

#include "classbase.h"

#include <exec/initializers.h>
#include <exec/resident.h>

/* ------------------------------ prototypes ------------------------------ */

AROS_UFP3(struct Library *, LibInit,
    AROS_UFPA(struct ClassBase *, cb, D0),
    AROS_UFPA(BPTR, seglist, A0),
    AROS_UFPA(struct ExecBase *, sysbase, A6));

AROS_LD1(APTR, LibOpen,
    AROS_LDA(ULONG, version, D0),
    struct ClassBase *, cb, 1, LIB);
AROS_LD0(BPTR, LibClose,
    struct ClassBase *, cb, 2, LIB);
AROS_LD0(BPTR, LibExpunge,
    struct ClassBase *, cb, 3, LIB);
AROS_LD0(LONG, LibReserved,
    struct ClassBase *, cb, 4, LIB);
AROS_LD0(Class *, ObtainEngine,
    struct ClassBase *, cb, 5, LIB);
        
static Class *InitClass(struct ClassBase *cb);
static BOOL OpenLibraries(struct ClassBase *cb);
static void CloseLibraries(struct ClassBase *cb);

#ifdef USERCLASSBASEINIT
extern BOOL UserClassBaseInit(struct ClassBase *cb);
extern void UserClassBaseExpunge(struct ClassBase *cb);
#endif
#ifdef USERCLASSBASEOPEN
extern BOOL UserClassBaseOpen(struct ClassBase *cb);
extern void UserClassBaseClose(struct ClassBase *cb);
#endif

/* --------------------------------- data --------------------------------- */

/* libraries shouldn't started */
LONG entry(void)
{
   return 0;
}

#ifdef BETA
extern void KPrintF(const char *fmt,...);
#ifndef LIBID_STRING
#error "You have to provide the LIBID_STRING macro!"
#endif
#define LIBID_BETA(x)      VERS "b" # x " (" BETA_DATE ") expires on " BETA_EXPIRE "\n"
#else
#define LIBID_STRING    VSTRING
#endif

static const ALIGNED UBYTE LibName[] = NAME;
static const ALIGNED UBYTE LibId[]   = LIBID_STRING;

static const APTR LibFuncTable[] =
{
   AROS_SLIB_ENTRY(LibOpen, LIB),
   AROS_SLIB_ENTRY(LibClose, LIB),
   AROS_SLIB_ENTRY(LibExpunge, LIB),
   AROS_SLIB_ENTRY(LibReserved, LIB),
   AROS_SLIB_ENTRY(ObtainEngine, LIB),
   USERCLASSBASEFUNCTABLE

   (APTR) -1
};

static const ALIGNED ULONG LibInitTable[4] =
{
   (ULONG) sizeof(struct ClassBase),
   (ULONG) LibFuncTable,
   (ULONG) NULL,
   (ULONG) LibInit
};

static const ALIGNED struct Resident romtag =
{
   RTC_MATCHWORD,
   (APTR) &romtag,
   (APTR) (&romtag + 1),
   (RTF_AUTOINIT),
   VERSION,
   NT_LIBRARY,
   0,
   (char *) LibName,
   (char *) LibId,
   (ULONG *) LibInitTable
};

#ifdef USE_VERSTAG
#ifndef LIBVERSTAG
#error "You specified USE_VERSTAG, but never defined LIBVERSTAG"
#else
static const STRPTR verstag = LIBVERSTAG;
#endif
#endif

#ifdef __GNUC__
struct Library *__UtilityBase;
#undef SysBase
struct ExecBase *SysBase;
#define SysBase ((struct ExecBase *)cb->cb_SysBase)
#endif

/* ---------------------------- implementation ---------------------------- */

extern ULONG class_dispatcher(Class *cl, Object *obj, Msg msg);

AROS_UFH3S(IPTR, dispatcher,
	   AROS_UFHA(Class *, cl, A0),
	   AROS_UFHA(Object *, obj, A2),
	   AROS_UFHA(Msg, msg, A1))
{
    AROS_USERFUNC_INIT
    
    return class_dispatcher(cl, obj, msg);
    
    AROS_USERFUNC_EXIT
}

static Class *InitClass(struct ClassBase *cb)
{
   Class *cl;

   if((cl = MakeClass(CLASSNAME,SUPERCLASSNAME,NULL,INSTANCESIZE,0)) != NULL)
   {
      cl->cl_Dispatcher.h_Entry = (HOOKFUNC)AROS_ASMSYMNAME(dispatcher);
      cl->cl_UserData = (ULONG) cb;

      DB(("dispatcher : %lx,size : %ld\n",cl->cl_Dispatcher.h_Entry,INSTANCESIZE));

      AddClass(cl);
   }

   DB(("class : %lx\n",cl));

   return(cl);
}

static BOOL OpenLibraries(struct ClassBase *cb)
{
   /* system libraries must be opened correctly, because a pre 3.0 check
    * was done in LibInit()
    */

   IntuitionBase = OpenLibrary("intuition.library",39);
   UtilityBase   = OpenLibrary("utility.library",  39);

#ifdef __GNUC__
   /* support for libnix */
   __UtilityBase = UtilityBase;
#endif

#ifdef USE_DOSLIB
   DOSBase       = OpenLibrary("dos.library",      39);
#endif
#ifdef USE_GFXLIB
   GfxBase       = OpenLibrary("graphics.library", 39);
#endif

#ifdef USE_LAYERSLIB
   LayersBase    = OpenLibrary("layers.library",   39);
#endif

#ifdef USE_DATATYPESLIB
   if((DataTypesBase = OpenLibrary("datatypes.library",39)) == NULL)
      return FALSE;
#endif

#ifdef USE_IFFPARSELIB
   if((IFFParseBase  = OpenLibrary("iffparse.library", 39)) == NULL)
      return FALSE;
#endif

#ifdef USE_LOCALELIB
   if((LocaleBase  = OpenLibrary("locale.library", 38)) == NULL)
      return FALSE;
#endif

#ifdef USE_ASLLIB
   if((AslBase = OpenLibrary("asl.library",39)) == NULL)
      return FALSE;
#endif

#ifdef USE_ICONLIB
   if((IconBase = OpenLibrary("icon.library",39)) == NULL)
      return FALSE;
#endif

#ifdef USE_GADTOOLSLIB
   if((GadToolsBase = OpenLibrary("gadtools.library",39)) == NULL)
      return FALSE;
#endif

#ifdef USE_DISKFONTLIB
   if((DiskFontBase = OpenLibrary("diskfont.library",39)) == NULL)
      return FALSE;
#endif

#ifdef USE_REXXSYSLIB
   if((RexxSysBase = OpenLibrary("rexxsyslib.library", 36)) == NULL)
      return FALSE;
#endif

#ifdef USE_WBLIB
   if((WorkbenchBase = OpenLibrary("workbench.library", 39)) == NULL)
      return FALSE;
#endif
   return TRUE;
}
static void CloseLibraries(struct ClassBase *cb)
{
   CloseLibrary((struct Library *) UtilityBase);
   UtilityBase   = NULL;
   CloseLibrary((struct Library *) IntuitionBase);
   IntuitionBase = NULL;

#ifdef USE_DOSLIB
   CloseLibrary((struct Library *) DOSBase);
   DOSBase       = NULL;
#endif

#ifdef USE_GFXLIB
   CloseLibrary((struct Library *) GfxBase);
   GfxBase       = NULL;
#endif

#ifdef USE_DATATYPESLIB
   CloseLibrary((struct Library *) DataTypesBase);
   DataTypesBase = NULL;
#endif

#ifdef USE_IFFPARSELIB
   CloseLibrary((struct Library *) IFFParseBase);
   IFFParseBase  = NULL;
#endif

#ifdef USE_LOCALELIB
   CloseLibrary((struct Library *) LocaleBase);
   LocaleBase = NULL;
#endif

#ifdef USE_LAYERSLIB
   CloseLibrary((struct Library *) LayersBase);
   LayersBase = NULL;
#endif

#ifdef USE_ASLLIB
   CloseLibrary((struct Library *) AslBase);
   AslBase = NULL;
#endif

#ifdef USE_ICONLIB
   CloseLibrary((struct Library *) IconBase);
   IconBase = NULL;
#endif

#ifdef USE_GADTOOLSLIB
   CloseLibrary((struct Library *) GadToolsBase);
   GadToolsBase = NULL;
#endif

#ifdef USE_DISKFONTLIB
   CloseLibrary((struct Library *) DiskFontBase);
   DiskFontBase = NULL;
#endif

#ifdef USE_REXXSYSLIB
   CloseLibrary((struct Library *) RexxSysBase);
   RexxSysBase = NULL;
#endif

#ifdef USE_WBLIB
   CloseLibrary((struct Library *) WorkbenchBase);
   WorkbenchBase = NULL;
#endif

   /* indicate we have closed our libraries */
   cb->cb_Lib.cl_Pad = FALSE;
}

AROS_LH0(Class *, ObtainEngine,
    struct ClassBase *, cb, 5, LIB)
{
   AROS_LIBFUNC_INIT
   
   /* here we need no semaphore operations, because this function
    * can only be called between a OpenLibrary()/CloseLibrary()
    * call.
    */

   return cb->cb_Lib.cl_Class;
   
   AROS_LIBFUNC_EXIT
}

AROS_UFH3(struct Library *, LibInit,
    AROS_UFHA(struct ClassBase *, cb, D0),
    AROS_UFHA(BPTR, seglist, A0),
    AROS_UFHA(struct ExecBase *, sysbase, A6));
{
   AROS_USERFUNC_INIT
   
   if(sysbase->LibNode.lib_Version >= 39)
   {
#undef SysBase
      SysBase = sysbase;
#define SysBase ((struct ExecBase *)cb->cb_SysBase)

      cb->cb_SegList = seglist;
      cb->cb_SysBase = (struct Library *)sysbase;
      cb->cb_Lib.cl_Lib.lib_Revision = REVISION;
      cb->cb_Lib.cl_Pad = FALSE;

      InitSemaphore(&cb->cb_Lock);

#ifdef BETA
      {
	 if((DOSBase = OpenLibrary("dos.library",39)) != NULL)
	 {
	    struct DateTime dt = {NULL};
	    struct DateStamp ds;
	    STRPTR ptr;
	    UBYTE date[20];

	    strcpy(date, BETA_EXPIRE);
	    while((ptr = strchr(date,'.')) != NULL)
	       *ptr = '-';

	    dt.dat_Format  = FORMAT_CDN;
	    dt.dat_StrDate = date;

	    /* convert date string */
	    if(StrToDate(&dt) == 0)
	    {
	       D(KPrintF("Can't convert EXPIRE DATE: " BETA_EXPIRE "!\n"));
	    } else
	    {
	       /* get actual date */
	       DateStamp(&ds);

	       /* compare expire date with actual date */
	       if(CompareDates(&ds, &dt.dat_Stamp) >= 0)
	       {
		  CloseLibrary(DOSBase);
		  return (struct Library *) cb;
	       }

	       if((IntuitionBase = OpenLibrary("intuition.library",39)) != NULL)
	       {
		  struct EasyStruct es = {
		  sizeof(struct EasyStruct),
		  0,
		  "ClassLib Library",
		  LIBID_STRING
		  "\nPlease get a newer beta or release version!\n"
		  ,
		  "Ok"
		  };

		  EasyRequest(NULL, &es, NULL, NULL);

		  CloseLibrary(IntuitionBase);
	       }
	    }
	    CloseLibrary(DOSBase);
	 }
      }
#else
      return (struct Library *) cb;
#endif
   }

   FreeMem((APTR)((IPTR)(cb) - (IPTR)(cb->cb_Lib.cl_Lib.lib_NegSize)), cb->cb_Lib.cl_Lib.lib_NegSize + cb->cb_Lib.cl_Lib.lib_PosSize);

   return NULL;
   
   AROS_USERFUNC_EXIT
}


AROS_LH1(APTR, LibOpen,
    AROS_LHA(ULONG, version, D0),
    struct ClassBase *, cb, 1, LIB)
{
   AROS_LIBFUNC_INIT
   
   struct SignalSemaphore *lock= &cb->cb_Lock;
   APTR retval = cb;

   ObtainSemaphore(lock);

   /* clear delayed expunge flag */
   cb->cb_Lib.cl_Lib.lib_Flags &= ~LIBF_DELEXP;

   if(++cb->cb_Lib.cl_Lib.lib_OpenCnt == 1)
   {
      /* only open libraries directly after LibInit */
      if(cb->cb_Lib.cl_Pad == FALSE && !OpenLibraries(cb))
	 retval = (LONG) NULL;
      else
      {
#ifdef SUPERCLASSLIBRARY
      /* open superclass library */
      if((cb->cb_SuperClassBase = OpenLibrary(SUPERCLASSLIBRARY,SUPERCLASSLIBRARYVERSION)) != NULL)
      {
#endif
	 cb->cb_Lib.cl_Class = InitClass(cb);

#ifdef USERCLASSBASEINIT
	 /* call user classbase init function */
	 if(cb->cb_Lib.cl_Class != NULL)
	 {
	    if(cb->cb_Lib.cl_Pad == FALSE)
	    {
	       if(!UserClassBaseInit(cb))
	       {
                  /* let cleanup user classbase area */
                  UserClassBaseExpunge(cb);

		  CloseLibraries(cb);
		  FreeClass(cb->cb_Lib.cl_Class);
		  cb->cb_Lib.cl_Class = NULL;
	       }
	    }
	 }
#endif
#ifdef USERCLASSBASEOPEN
	 /* call user classbase open function */
	 if(cb->cb_Lib.cl_Class != NULL)
	 {
	    if(!UserClassBaseOpen(cb))
	    {
#ifdef USERCLASSBASEINIT
               /* let cleanup user classbase area */
               UserClassBaseExpunge(cb);
#endif
               UserClassBaseClose(cb);

	       CloseLibraries(cb);
	       FreeClass(cb->cb_Lib.cl_Class);
	       cb->cb_Lib.cl_Class = NULL;
	    }
	 }
#endif

#ifdef SUPERCLASSLIBRARY
	 if(cb->cb_Lib.cl_Class == NULL)
	 {
	    CloseLibrary(SuperClassBase);
	    SuperClassBase = NULL;
	 }
      } else
      {
	 CloseLibraries(cb);
	 retval = (LONG) NULL;
      }
#endif
      }
   }

   if(retval == (LONG) NULL)
      cb->cb_Lib.cl_Lib.lib_OpenCnt--;
   else
   {
      /* first LibOpen after LibInit finished */
      if(cb->cb_Lib.cl_Pad == FALSE)
	 cb->cb_Lib.cl_Pad = TRUE;
   }

   ReleaseSemaphore(lock);

   DB(("return = 0x%lx\n",retval));

   return retval;
   
   AROS_LIBFUNC_EXIT
}

static BPTR ExpungeLib(struct ClassBase *cb)
{
   BPTR seg = NULL;

   DB(("cnt = %ld, class = 0x%lx\n",cb->cb_Lib.cl_Lib.lib_OpenCnt,cb->cb_Lib.cl_Class));

   if(cb->cb_Lib.cl_Lib.lib_OpenCnt == 0)
   {
      seg = cb->cb_SegList;

#ifdef USERCLASSBASEINIT
       UserClassBaseExpunge(cb);
#endif

      Remove((struct Node *) cb);

      FreeMem((APTR)((ULONG)(cb) - (ULONG)(cb->cb_Lib.cl_Lib.lib_NegSize)), cb->cb_Lib.cl_Lib.lib_NegSize + cb->cb_Lib.cl_Lib.lib_PosSize);
   } else
   {
      cb->cb_Lib.cl_Lib.lib_Flags |= LIBF_DELEXP;
   }

   DB(("return = 0x%lx\n",seg));

   return seg;
}


AROS_LH0(BPTR, LibClose,
    struct ClassBase *, cb, 2, LIB)
{
   AROS_LIBFUNC_INIT
   
   BPTR retval = NULL;

   DB(("cnt = %ld, class = 0x%lx\n",cb->cb_Lib.cl_Lib.lib_OpenCnt,cb->cb_Lib.cl_Class));

   if(cb->cb_Lib.cl_Lib.lib_OpenCnt > 0)
   {
      if(--cb->cb_Lib.cl_Lib.lib_OpenCnt == 0)
      {
	 ObtainSemaphore(&cb->cb_Lock);

	 if(cb->cb_Lib.cl_Class != NULL && FreeClass(cb->cb_Lib.cl_Class))
	    cb->cb_Lib.cl_Class = NULL;

#ifdef SUPERCLASSLIBRARY
	 if(cb->cb_Lib.cl_Class == NULL)
	 {
	    CloseLibrary(SuperClassBase);
	    SuperClassBase = NULL;
	 }
#endif

#ifdef USERCLASSBASEOPEN
	 if(cb->cb_Lib.cl_Class == NULL)
	 {
	    UserClassBaseClose(cb);
	 }
#endif

	 if(cb->cb_Lib.cl_Class == NULL)
	 {
	    CloseLibraries(cb);
	 }

	 ReleaseSemaphore(&cb->cb_Lock);
      }
   }

   DB(("cnt = %ld, class = 0x%lx\n",cb->cb_Lib.cl_Lib.lib_OpenCnt,cb->cb_Lib.cl_Class));

   if(cb->cb_Lib.cl_Lib.lib_Flags & LIBF_DELEXP)
      retval = ExpungeLib(cb);

   DB(("return = 0x%lx\n",retval));

   return retval;
   
   AROS_LIBFUNC_EXIT
}


AROS_LH0(BPTR, LibExpunge,
    struct ClassBase *, cb, 3, LIB)
{
   AROS_LIBFUNC_INIT
   
   return ExpungeLib(cb);
   
   AROS_LIBFUNC_EXIT
}

AROS_LH0(LONG, LibReserved,
    struct ClassBase *, cb, 4, LIB)
{
   AROS_LIBFUNC_INIT
   
   return 0;
   
   AROS_LIBFUNC_EXIT
}
