/*
    (C) 2000 AROS - The Amiga Research OS
    $Id$

    Desc: Graphics function OpenMonitor()
    Lang: english
*/
#include <graphics/monitor.h>

/*****************************************************************************

    NAME */
#include <proto/graphics.h>

        AROS_LH2(struct MonitorSpec *, OpenMonitor,

/*  SYNOPSIS */
        AROS_LHA(STRPTR, monitor_name, A1),
        AROS_LHA(ULONG, display_id, D0),

/*  LOCATION */
        struct GfxBase *, GfxBase, 119, Graphics)

/*  FUNCTION

    INPUTS
        monitor_name - pointer to a null terminated string
        display_id   - optional 32 bit monitor/mode identifier

    RESULT
        mspc - pointer to an open MonitorSpec structure
               NULL if MonitorSpec could not be opened

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        CloseMonitor() graphics/monitor.h

    INTERNALS

    HISTORY


******************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct GfxBase *,GfxBase)

#warning TODO: Write graphics/OpenMonitor()
    aros_print_not_implemented ("OpenMonitor");

    return NULL;

    AROS_LIBFUNC_EXIT
} /* OpenMonitor */
