/*
    (C) 1997-2000 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: English
*/

#define CATCOMP_ARRAY
#include "more_strings.h"

#include <libraries/locale.h>
#include <proto/locale.h>
#include <proto/exec.h>
#include <proto/alib.h>

#define DEBUG 0
#include <aros/debug.h>

/*********************************************************************************************/

struct LocaleBase 	*LocaleBase;
struct Catalog 		*catalog;

/*********************************************************************************************/

void InitLocale(STRPTR catname, ULONG version)
{
    LocaleBase = (struct LocaleBase *)OpenLibrary("locale.library", 39);
    if (LocaleBase)
    {
	catalog = OpenCatalog(NULL, catname, OC_Version, version,
    					     TAG_DONE);
    }
}

/*********************************************************************************************/

void CleanupLocale(void)
{
    if (catalog) CloseCatalog(catalog);
    if (LocaleBase) CloseLibrary((struct Library *)LocaleBase);
}

/*********************************************************************************************/

STRPTR MSG(ULONG id)
{
    STRPTR retval;
    
    if (catalog)
    {
        retval = GetCatalogStr(catalog, id, CatCompArray[id].cca_Str);
    } else {
        retval = CatCompArray[id].cca_Str;
    }
    
    return retval;
}

/*********************************************************************************************/
