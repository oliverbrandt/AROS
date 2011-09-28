/*
    Copyright � 1995-2011, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Display an alert, iOS-hosted version
    Lang: english
*/

#include <aros/atomic.h>
#include <aros/libcall.h>
#include <proto/exec.h>

#include <inttypes.h>
#include <signal.h>

#include "hostinterface.h"
#include "kernel_base.h"
#include "kernel_debug.h"
#include "kernel_intern.h"

/* This comes from kernel_startup.c */
extern struct HostInterface *HostIFace;

static void (*displayAlert)(const char *text) = NULL;

void krnDisplayAlert(const char *text, struct KernelBase *KernelBase)
{
    if (!displayAlert)
    {
	/*
	 * Early alert. call hook is not initialized yet.
	 * Fail back to debug output.
	 */
    	bug(text);
    	return;
    }

    /* Display the alert via our UIKit helper */
    displayAlert(text);
    AROS_HOST_BARRIER
}

static int Alert_Init(void)
{
    APTR libHandle;
    char *err;

    /*
     * We use local variable for the handle because we never expunge
     * so we will never close it
     */
    libHandle = HostIFace->hostlib_Open("Libs/Host/alert.dylib", &err);
    if (!libHandle)
    {
    	bug("Failed to load alert.dylib: %s\n", err);
	return FALSE;
    }

    displayAlert = HostIFace->hostlib_GetPointer(libHandle, "DisplayAlert", &err);
    if (displayAlert)
	return TRUE;

    HostIFace->hostlib_Close(libHandle, NULL);

    bug("Failed to load DisplayAlert helper: %s\n", err);
    return FALSE;
}

ADD2INIT(Alert_Init, 0);
