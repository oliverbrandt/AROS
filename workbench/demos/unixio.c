#define AROS_ALMOST_COMPATIBLE
#include <hidd/unixio.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/oop.h>
#include <oop/oop.h>

struct Library * OOPBase;
struct DosLibrary * DOSBase;

int main (int argc, char ** argv)
{
    APTR hidd;
    LONG ret;
    int fd;
    IPTR vpa[1];
    struct TagItem tags[ ]= {{TAG_DONE, 0UL}};
    struct uioMsg uio_msg;

    OOPBase = OpenLibrary ("oop.library", 0);

    if (!OOPBase)
    {
	vpa[0] = (IPTR)"oop.library";
	VPrintf("Can't open library \"%s\"\n", vpa);
	return 10;
    }

    DOSBase = (struct DosLibrary *) OpenLibrary (DOSNAME, 0);

    if (!DOSBase)
    {
	CloseLibrary (OOPBase);

	vpa[0] = (IPTR)DOSNAME;
	VPrintf("Can't open library \"%s\"\n", vpa);
	return 10;
    }


    hidd = NewObject (NULL, CLID_UnixIO_Hidd, tags);

    if (!hidd)
    {
	CloseLibrary (OOPBase);
	CloseLibrary ((struct Library *)DOSBase);

	vpa[0] = (IPTR)CLID_UnixIO_Hidd;
	VPrintf("Need \"%s\" class\n", vpa);
	return 10;
    }

    fd = 0;
    uio_msg.um_MethodID = GetMethodID(IID_UnixIO, HIDDMO_UnixIO_Wait);
    uio_msg.um_Filedesc = fd;
    uio_msg.um_Mode	= HIDDV_UnixIO_Read;
    ret = DoMethod(hidd, (Msg)&uio_msg);

    vpa[0] = ret;
    VPrintf ("return code = %ld\n", vpa);

    DisposeObject (hidd);

    CloseLibrary ((struct Library *)DOSBase);
    CloseLibrary (OOPBase);

    return 0;
}

