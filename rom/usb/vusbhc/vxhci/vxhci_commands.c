/*
    Copyright � 2014, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/

#include <aros/macros.h>

#include <proto/exec.h>
#include <proto/arossupport.h>

#include <devices/usb_hub.h>
#include <devices/usbhardware.h>

#define DEBUG 1
#include <aros/debug.h>

#include LC_LIBDEFS_FILE

/* we cannot use AROS_WORD2LE in struct initializer */
#if AROS_BIG_ENDIAN
#define WORD2LE(w) (UWORD)(((w) >> 8) & 0x00FF) | (((w) << 8) & 0xFF00)
#else
#define WORD2LE(w) (w)
#endif

const struct UsbSSHubDesc  RHSSHubDesc = { 12,                                           // 0 Number of bytes in this descriptor, including this byte. (12 bytes)
                                           UDT_SSHUB,                                    // 1 Descriptor Type, value: 2AH for SuperSpeed hub descriptor
                                           0,                                            // 2 Number of downstream facing ports that this hub supports. The maximum number of ports a hub can support is 15
                                           WORD2LE(UHCF_INDIVID_POWER|UHCF_INDIVID_OVP), // 3 wHubCharacteristics
                                           0,                                            // 5 bPwrOn2PwrGood
                                           10,                                           // 6 bHubContrCurrent
                                           0,                                            // 7 bHubHdrDecLat
                                           0,                                            // 8 wHubDelay
                                           0                                             // 10 DeviceRemovable
                                         };

CONST_STRPTR root_hub_strings[] = { "The AROS Development Team.", "Virtual Root Hub Unit %d", "Standard Config", "Hub interface" };

WORD cmdQueryDevice(struct IOUsbHWReq *ioreq) {
    struct TagItem *taglist = (struct TagItem *) ioreq->iouh_Data;
    struct TagItem *tag;
    ULONG count = 0;

    while((tag = LibNextTagItem(&taglist)) != NULL) {
        switch (tag->ti_Tag) {
            case UHA_Manufacturer:
                *((STRPTR *) tag->ti_Data) = "The AROS Development Team";
                count++;
                break;
            case UHA_Version:
                *((ULONG *) tag->ti_Data) = VERSION_NUMBER;
                count++;
                break;
            case UHA_Revision:
                *((ULONG *) tag->ti_Data) = REVISION_NUMBER;
                count++;
                break;
            case UHA_Copyright:
                *((STRPTR *) tag->ti_Data) ="�2014 The AROS Development Team";
                count++;
                break;
            case UHA_ProductName:
                *((STRPTR *) tag->ti_Data) = "VXHCI";
                count++;
                break;
            case UHA_Capabilities:
                *((ULONG *) tag->ti_Data) = (UHCF_USB20|UHCF_USB30);
                count++;
                break;
            default:
                break;
        }
    }

    ioreq->iouh_Actual = count;
    return RC_OK;
}

WORD cmdUsbReset(struct IOUsbHWReq *ioreq) {
    bug("[VXHCI] cmdUsbReset: Entering function\n");

    struct VXHCIUnit *unit = (struct VXHCIUnit *) ioreq->iouh_Req.io_Unit;

    /* We should do a proper reset sequence with a real driver */
    unit->unit_state = UHSF_OPERATIONAL;
    bug("[VXHCI] cmdUsbReset: Done\n\n");
    return RC_OK;
}

WORD cmdControlXFer(struct IOUsbHWReq *ioreq) {
    bug("[VXHCI] cmdControlXFer: Entering function\n");

    struct VXHCIUnit *unit = (struct VXHCIUnit *) ioreq->iouh_Req.io_Unit;

    bug("[VXHCI] cmdControlXFer: ioreq->iouh_DevAddr %lx\n", ioreq->iouh_DevAddr);

    /*
        Check the status of the controller
        We might encounter these states:
        UHSB_OPERATIONAL USB can be used for transfers
        UHSB_RESUMING    USB is currently resuming
        UHSB_SUSPENDED   USB is in suspended state
        UHSB_RESET       USB is just inside a reset phase
    */
    if(unit->unit_state == UHSF_OPERATIONAL) {
        bug("[VXHCI] cmdControlXFer: Unit state: UHSF_OPERATIONAL\n");
    } else {
        bug("[VXHCI] cmdControlXFer: Unit state: UHSF_SUSPENDED\n");
        return UHIOERR_USBOFFLINE;
    }

    /* Assuming when iouh_DevAddr is 0 it addresses hub... no no, all wrong... */
//    if(ioreq->iouh_DevAddr == 0) {
        return(cmdControlXFerRootHub(ioreq));
//    }

    return RC_DONTREPLY;
}

WORD cmdControlXFerRootHub(struct IOUsbHWReq *ioreq) {
    bug("[VXHCI] cmdControlXFerRootHub: Entering function\n");

    //UWORD rt = ioreq->iouh_SetupData.bmRequestType;
    //UWORD req = ioreq->iouh_SetupData.bRequest;
    //UWORD idx = AROS_WORD2LE(ioreq->iouh_SetupData.wIndex);
    //UWORD val = AROS_WORD2LE(ioreq->iouh_SetupData.wValue);
    //UWORD len = AROS_WORD2LE(ioreq->iouh_SetupData.wLength);

    UWORD bmRequestType      = (ioreq->iouh_SetupData.bmRequestType) & (URTF_STANDARD | URTF_CLASS | URTF_VENDOR);
    UWORD bmRequestDirection = (ioreq->iouh_SetupData.bmRequestType) & (URTF_IN | URTF_OUT);
    UWORD bmRequestRecipient = (ioreq->iouh_SetupData.bmRequestType) & (URTF_DEVICE | URTF_INTERFACE | URTF_ENDPOINT | URTF_OTHER);

    UWORD bRequest           = (ioreq->iouh_SetupData.bRequest);
    UWORD wValue             = AROS_WORD2LE(ioreq->iouh_SetupData.wValue);
    UWORD wLength            = AROS_WORD2LE(ioreq->iouh_SetupData.wLength);

    struct VXHCIUnit *unit = (struct VXHCIUnit *) ioreq->iouh_Req.io_Unit;

    bug("[VXHCI] cmdControlXFerRootHub: Endpoint number is %ld \n", ioreq->iouh_Endpoint);

    bug("[VXHCI] cmdControlXFerRootHub: bmRequestDirection ");
    switch (bmRequestDirection) {
        case URTF_IN:
            bug("URTF_IN\n");
            break;
        case URTF_OUT:
            bug("URTF_OUT\n");
            break;
    }

    bug("[VXHCI] cmdControlXFerRootHub: bmRequestType ");
    switch(bmRequestType) {
        case URTF_STANDARD:
            bug("URTF_STANDARD\n");
            break;
        case URTF_CLASS:
            bug("URTF_CLASS\n");
            break;
        case URTF_VENDOR:
            bug("URTF_VENDOR\n");
            break;
    }

    bug("[VXHCI] cmdControlXFerRootHub: bmRequestRecipient ");
    switch (bmRequestRecipient) {
        case URTF_DEVICE:
            bug("URTF_DEVICE\n");
            break;
        case URTF_INTERFACE:
            bug("URTF_INTERFACE\n");
            break;
        case URTF_ENDPOINT:
            bug("URTF_ENDPOINT\n");
            break;
        case URTF_OTHER:
            bug("URTF_OTHER\n");
            break;
    }

    bug("[VXHCI] cmdControlXFerRootHub: bRequest ");
    switch(bRequest) {
        case USR_GET_STATUS:
            bug("USR_GET_STATUS\n");
            break;
        case USR_CLEAR_FEATURE:
            bug("USR_CLEAR_FEATURE\n");
            break;
        case USR_SET_FEATURE:
            bug("USR_SET_FEATURE\n");
            break;
        case USR_SET_ADDRESS:
            bug("USR_SET_ADDRESS\n");
            break;
        case USR_GET_DESCRIPTOR:
            bug("USR_GET_DESCRIPTOR\n");
            break;
        case USR_SET_DESCRIPTOR:
            bug("USR_SET_DESCRIPTOR\n");
            break;
        case USR_GET_CONFIGURATION:
            bug("USR_GET_CONFIGURATION\n");
            break;
        case USR_SET_CONFIGURATION:
            bug("USR_SET_CONFIGURATION\n");
            break;
        case USR_GET_INTERFACE:
            bug("USR_GET_INTERFACE\n");
            break;
        case USR_SET_INTERFACE:
            bug("USR_SET_INTERFACE\n");
            break;
        case USR_SYNCH_FRAME:
            bug("USR_SYNCH_FRAME\n");
            break;
    }

    bug("[VXHCI] cmdControlXFerRootHub: wValue %x\n", wValue);
    bug("[VXHCI] cmdControlXFerRootHub: wLength %d\n", wLength);

    /* Endpoint 0 is used for control transfers only and can not be assigned to any other function. */
    if(ioreq->iouh_Endpoint != 0) {
        return UHIOERR_BADPARAMS;
    }

    /* Check the request */
    if(bmRequestDirection) {
        bug("[VXHCI] cmdControlXFerRootHub: Request direction is device to host\n");

        switch(bmRequestType) {
            case URTF_STANDARD:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_STANDARD\n");

                switch(bmRequestRecipient) {
                    case URTF_DEVICE:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_DEVICE\n");

                        switch(bRequest) {
                            case USR_GET_DESCRIPTOR:
                                bug("[VXHCI] cmdControlXFerRootHub: USR_GET_DESCRIPTOR\n");

                                switch( (wValue>>8) ) {
                                    case UDT_DEVICE:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_DEVICE\n");
                                        bug("[VXHCI] cmdControlXFerRootHub: GetDeviceDescriptor (%ld)\n", wLength);

                                        struct UsbStdDevDesc *usdd = (struct UsbStdDevDesc *) ioreq->iouh_Data;

                                        usdd->bLength            = sizeof(struct UsbStdDevDesc);
                                        usdd->bDescriptorType    = UDT_DEVICE;
                                        //usdd->bcdUSB             = WORD2LE(0x0110);
                                        usdd->bDeviceClass       = HUB_CLASSCODE;
                                        usdd->bDeviceSubClass    = 0;
                                        usdd->bDeviceProtocol    = 0;
                                        usdd->bMaxPacketSize0    = 8;
                                        usdd->idVendor           = WORD2LE(0x0000);
                                        usdd->idProduct          = WORD2LE(0x0000);
                                        usdd->bcdDevice          = WORD2LE(0x0100);
                                        usdd->iManufacturer      = 0; //1 strings not yeat implemented
                                        usdd->iProduct           = 0; //2 strings not yeat implemented
                                        usdd->iSerialNumber      = 0;
                                        usdd->bNumConfigurations = 1;

                                        if(unit->unit_type == 2) {
                                            bug("[VXHCI] cmdControlXFerRootHub: USB2.0 unit\n");
                                            usdd->bcdUSB = AROS_WORD2LE(0x0200); // signal a highspeed root hub
                                        } else {
                                            bug("[VXHCI] cmdControlXFerRootHub: USB3.0 unit\n");
                                            usdd->bcdUSB = AROS_WORD2LE(0x0300); // signal a superspeed root hub
                                        }

                                        ioreq->iouh_Actual = wLength;
                                        bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                        return(0);
                                        break;

                                    case UDT_CONFIGURATION:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_CONFIGURATION\n");

                                        typedef struct RHConfig{
                                            struct UsbStdCfgDesc rhcfgdesc;
                                            struct UsbStdIfDesc  rhifdesc;
                                            struct UsbStdEPDesc  rhepdesc;
                                        } RHConfig;

                                        struct RHConfig *rhconfig = (struct RHConfig *) ioreq->iouh_Data;

                                        rhconfig->rhcfgdesc.bLength             = sizeof(struct UsbStdCfgDesc);
                                        rhconfig->rhcfgdesc.bDescriptorType     = UDT_CONFIGURATION;
                                        rhconfig->rhcfgdesc.wTotalLength        = AROS_WORD2LE(sizeof(struct RHConfig));
                                        rhconfig->rhcfgdesc.bNumInterfaces      = 1;
                                        rhconfig->rhcfgdesc.bConfigurationValue = 1;
                                        rhconfig->rhcfgdesc.iConfiguration      = 0; // 3 strings not yeat implemented
                                        rhconfig->rhcfgdesc.bmAttributes        = (USCAF_ONE|USCAF_SELF_POWERED);
                                        rhconfig->rhcfgdesc.bMaxPower           = 0;

                                        rhconfig->rhifdesc.bLength              = sizeof(struct UsbStdIfDesc);
                                        rhconfig->rhifdesc.bDescriptorType      = UDT_INTERFACE;
                                        rhconfig->rhifdesc.bInterfaceNumber     = 0;
                                        rhconfig->rhifdesc.bAlternateSetting    = 0;
                                        rhconfig->rhifdesc.bNumEndpoints        = 1;
                                        rhconfig->rhifdesc.bInterfaceClass      = HUB_CLASSCODE;
                                        rhconfig->rhifdesc.bInterfaceSubClass   = 0;
                                        rhconfig->rhifdesc.bInterfaceProtocol   = 0;
                                        rhconfig->rhifdesc.iInterface           = 0; //4 strings not yeat implemented

                                        rhconfig->rhepdesc.bLength              = sizeof(struct UsbStdEPDesc);
                                        rhconfig->rhepdesc.bDescriptorType      = UDT_ENDPOINT;
                                        rhconfig->rhepdesc.bEndpointAddress     = (URTF_IN|1);
                                        rhconfig->rhepdesc.bmAttributes         = USEAF_INTERRUPT;
                                        rhconfig->rhepdesc.wMaxPacketSize       = WORD2LE(8);
                                        rhconfig->rhepdesc.bInterval            = 12;

                                        bug("sizeof(struct RHConfig) = %ld (should be 25)\n", sizeof(struct RHConfig));
                                        ioreq->iouh_Actual = sizeof(struct RHConfig);
                                        bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                        return(0);

                                        break;

                                    case UDT_STRING:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_STRING\n");
                                        break;

                                    case UDT_INTERFACE:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_INTERFACE\n");
                                        break;

                                    case UDT_ENDPOINT:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_ENDPOINT\n");
                                        break;

                                    case UDT_DEVICE_QUALIFIER:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_DEVICE_QUALIFIER\n");
                                        break;

                                    case UDT_OTHERSPEED_QUALIFIER:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_OTHERSPEED_QUALIFIER\n");
                                        break;

                                    case UDT_INTERFACE_POWER:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_INTERFACE_POWER\n");
                                        break;

                                    case UDT_OTG:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_OTG\n");
                                        break;

                                    case UDT_DEBUG:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_DEBUG\n");
                                        break;

                                    case UDT_INTERFACE_ASSOCIATION:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_INTERFACE_ASSOCIATION\n");
                                        break;

                                    case UDT_SECURITY:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_SECURITY\n");
                                        break;

                                    case UDT_ENCRYPTION_TYPE:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_ENCRYPTION_TYPE\n");
                                        break;

                                    case UDT_BOS:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_BOS\n");
                                        break;

                                    case UDT_DEVICE_CAPABILITY:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_DEVICE_CAPABILITY\n");
                                        break;

                                    case UDT_WIRELESS_EP_COMP:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_WIRELESS_EP_COMP\n");
                                        break;

                                    default:
                                        bug("[VXHCI] cmdControlXFerRootHub: switch( (wValue>>8) ) %ld\n", (wValue>>8));
                                        break;

                                } /* switch( (wValue>>8) ) */
                                break; /* case USR_GET_DESCRIPTOR */

                            case USR_GET_CONFIGURATION:
                                bug("[VXHCI] cmdControlXFerRootHub: USR_GET_CONFIGURATION\n");

                                ((UBYTE *) ioreq->iouh_Data)[0] = 1;
                                ioreq->iouh_Actual = wLength;
                                bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                return(0);
                                break;

                        } /* switch(bRequest) */
                        break; /* case URTF_DEVICE: */

                    case URTF_INTERFACE:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_INTERFACE\n");
                        break;

                    case URTF_ENDPOINT:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_ENDPOINT\n");
                        break;

                    case URTF_OTHER:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_OTHER\n");
                        break;

                    default:
                        bug("[VXHCI] cmdControlXFerRootHub: %ld\n", bRequest);
                        break;

                } /* switch(bmRequestRecipient) */
                break;

            case URTF_CLASS:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_CLASS\n");
                break;

            case URTF_VENDOR:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_VENDOR\n");
                break;
        } /* switch(bmRequestType) */

    } else { /* if(bmRequestDirection) */
        bug("[VXHCI] cmdControlXFerRootHub: Request direction is host to device\n");

        switch(bmRequestType) {
            case URTF_STANDARD:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_STANDARD\n");

                switch(bmRequestRecipient) {
                    case URTF_DEVICE:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_DEVICE\n");

                        switch(bRequest) {
                            case USR_SET_ADDRESS:
                                bug("[VXHCI] cmdControlXFerRootHub: USR_SET_ADDRESS\n");
                                unit->unit_roothubaddr = wValue;
                                ioreq->iouh_Actual = wLength;
                                bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                return(0);
                                break;

                            case USR_SET_CONFIGURATION:
                                /* We do not have alternative configuration */
                                bug("[VXHCI] cmdControlXFerRootHub: USR_SET_CONFIGURATION\n");
                                ioreq->iouh_Actual = wLength;
                                bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                return(0);
                                break;

                        } /* switch(bRequest) */
                        break;

                    case URTF_INTERFACE:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_INTERFACE\n");
                        break;

                    case URTF_ENDPOINT:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_ENDPOINT\n");
                        break;

                    case URTF_OTHER:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_OTHER\n");
                        break;

                } /* switch(bmRequestRecipient) */
                break;

            case URTF_CLASS:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_CLASS\n");
                break;

            case URTF_VENDOR:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_VENDOR\n");
                break;

        } /* switch(bmRequestType) */

    } /* if(bmRequestDirection) */

    return UHIOERR_BADPARAMS;
}


