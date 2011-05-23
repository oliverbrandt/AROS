#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DB_LEVEL 1

//#define DEBUG 1

//#define MEMDEBUG

//#define USB3 1

#include <proto/debug.h>

// DEBUG 0 should equal undefined DEBUG
#ifdef DEBUG
#if DEBUG == 0
#undef DEBUG
#endif
#endif

#ifdef DEBUG
#define XPRINTF(l, x) do { if ((l) >= DB_LEVEL) \
     { KPrintF("%s:%s/%lu: ", __FILE__, __FUNCTION__, __LINE__); KPrintF x;} } while (0)
#if DEBUG > 1
#define KPRINTF(l, x) do { if ((l) >= DB_LEVEL) \
     { KPrintF("%s:%s/%lu: ", __FILE__, __FUNCTION__, __LINE__); KPrintF x;} } while (0)
#else
#define KPRINTF(l, x)
#endif
#define DB(x) x
   void dumpmem(void *mem, unsigned long int len);
#else /* !DEBUG */

#define KPRINTF(l, x)
#define XPRINTF(l, x) 
#define DB(x)

#endif /* DEBUG */

#endif /* __DEBUG_H__ */
