/*
    Copyright � 2013, The AROS Development Team. All rights reserved.
    $Id$
*/

#ifndef USB2OTG_H
#define USB2OTG_H

/** Maximum number of Periodic FIFOs */ 

#define PERIFIFO_MAX                                    15                      // Max no. of Periodic FIFO's
#define TXFIFO_MAX                                      15                      // Max no. of Tx FIFO's
#define EPSCHANS_MAX                                    16                      // Max no. of Endpoints/Host Chan's

#define USB2OTG_BASE                                    (BCM_PHYSBASE + 0x980000)

#define USB2OTG_OTGCTRL                                 (USB2OTG_BASE + 0x000)
#define USB2OTG_OTGINTR                                 (USB2OTG_BASE + 0x004)
#define USB2OTG_AHB                                     (USB2OTG_BASE + 0x008)
#define USB2OTG_USB                                     (USB2OTG_BASE + 0x00c)
#define USB2OTG_RESET                                   (USB2OTG_BASE + 0x010)
#define USB2OTG_INTR                                    (USB2OTG_BASE + 0x014)
#define USB2OTG_INTRMASK                                (USB2OTG_BASE + 0x018)
#define USB2OTG_RCVPEEK                                 (USB2OTG_BASE + 0x01c)
#define USB2OTG_RCVPOP                                  (USB2OTG_BASE + 0x020)
#define USB2OTG_RCVSIZE                                 (USB2OTG_BASE + 0x024)
#define USB2OTG_NONPERIFIFOSIZE                         (USB2OTG_BASE + 0x028)
#define USB2OTG_NONPERIFIFOSTATUS                       (USB2OTG_BASE + 0x02c)
#define USB2OTG_I2CCTRL                                 (USB2OTG_BASE + 0x030)
#define USB2OTG_PHYVENDCTRL                             (USB2OTG_BASE + 0x034)
#define USB2OTG_GPIO                                    (USB2OTG_BASE + 0x038)
#define USB2OTG_USERID                                  (USB2OTG_BASE + 0x03c)
#define USB2OTG_VENDORID                                (USB2OTG_BASE + 0x040)
#define USB2OTG_HARDWARE                                (USB2OTG_BASE + 0x044)
#define USB2OTG_LPMCONFIG                               (USB2OTG_BASE + 0x048)
#define USB2OTG_MDIOCTRL                                (USB2OTG_BASE + 0x080)
#define USB2OTG_MDIOGEN                                 (USB2OTG_BASE + 0x084)
#define USB2OTG_MDIOREAD                                USB2OTG_MDIOGEN
#define USB2OTG_MDIOWRITE                               USB2OTG_MDIOGEN
#define USB2OTG_MISCCTRL                                (USB2OTG_BASE + 0x088)
#define USB2OTG_PERIFIFOSIZE                            (USB2OTG_BASE + 0x100)
#define USB2OTG_PERIFIFOBASE                            (USB2OTG_BASE + 0x104)

#define USB2OTG_HOSTCFG                                 (USB2OTG_BASE + 0x400)
#define USB2OTG_HOSTFRAMEINTERV                         (USB2OTG_BASE + 0x404)
#define USB2OTG_HOSTFRAMENO                             (USB2OTG_BASE + 0x408)
#define USB2OTG_HOSTFIFOSTATUS                          (USB2OTG_BASE + 0x410)
#define USB2OTG_HOSTINTR                                (USB2OTG_BASE + 0x414)
#define USB2OTG_HOSTINTRMASK                            (USB2OTG_BASE + 0x418)
#define USB2OTG_HOSTFRAMELST                            (USB2OTG_BASE + 0x41c)
#define USB2OTG_HOSTPORT                                (USB2OTG_BASE + 0x440)
#define USB2OTG_HOSTCHANCHARBASE                        (USB2OTG_BASE + 0x500)
#define USB2OTG_HOSTCHANSPLITCTRL                       (USB2OTG_BASE + 0x504)
#define USB2OTG_HOSTCHANINTR                            (USB2OTG_BASE + 0x508)
#define USB2OTG_HOSTCHANINTRMASK                        (USB2OTG_BASE + 0x50c)
#define USB2OTG_HOSTCHANTRANSSIZE                       (USB2OTG_BASE + 0x510)
#define USB2OTG_HOSTCHANDMAADDR                         (USB2OTG_BASE + 0x514)
#define USB2OTG_HOSTCHANDMABUFF                         (USB2OTG_BASE + 0x51c)

#define USB2OTG_POWER                                   (USB2OTG_BASE + 0xe00)

/* Bits for USB2OTG_OTGCTRL */
#DEFINE USB2OTG_OTGCTRL_SESREQSCS                       (1 << 0)
#DEFINE USB2OTG_OTGCTRL_SESREQ                          (1 << 1)
#DEFINE USB2OTG_OTGCTRL_VBVALIDOVEN                     (1 << 2)
#DEFINE USB2OTG_OTGCTRL_VBVALIDOVVAL                    (1 << 3)
#DEFINE USB2OTG_OTGCTRL_AVALIDOVEN                      (1 << 4)
#DEFINE USB2OTG_OTGCTRL_AVALIDOVVAL                     (1 << 5)
#DEFINE USB2OTG_OTGCTRL_BVALIDOVEN                      (1 << 6)
#DEFINE USB2OTG_OTGCTRL_BVALIDOVVAL                     (1 << 7)
#DEFINE USB2OTG_OTGCTRL_HSTNEGSCS                       (1 << 8)
#DEFINE USB2OTG_OTGCTRL_HNPREQ                          (1 << 9)
#DEFINE USB2OTG_OTGCTRL_HOSTSETHNPENABLE                (1 << 10)
#DEFINE USB2OTG_OTGCTRL_DEVHNPEN                        (1 << 11)
#DEFINE USB2OTG_OTGCTRL_CONIDSTS                        (1 << 16)
#DEFINE USB2OTG_OTGCTRL_DBNCTIME                        (1 << 17)
#DEFINE USB2OTG_OTGCTRL_ASESSIONVALID                   (1 << 18)
#DEFINE USB2OTG_OTGCTRL_BSESSIONVALID                   (1 << 19)
#DEFINE USB2OTG_OTGCTRL_OTGVERSION                      (1 << 20)
#DEFINE USB2OTG_OTGCTRL_MULTVALIDBC                     (1 << 22)
#DEFINE USB2OTG_OTGCTRL_CHIRPEN                         (1 << 27)

/* Bits in USB2OTG_OTGINTR */
#DEFINE USB2OTG_OTGINTR_SESSENDDETECTED                 (1 << 2)
#DEFINE USB2OTG_OTGINTR_SESSRQSTSUCCESSSTATUSCHANGE     (1 << 8)
#DEFINE USB2OTG_OTGINTR_HOSTNEGSUCCESSSTATUSCHANGE      (1 << 9)
#DEFINE USB2OTG_OTGINTR_HOSTNEGDETECTED                 (1 << 17)
#DEFINE USB2OTG_OTGINTR_ADEVICETIMEOUTCHANGE            (1 << 18)
#DEFINE USB2OTG_OTGINTR_DEBOUNCEDONE                    (1 << 19)

/* Bits in USB2OTG_AHB */
#DEFINE USB2OTG_AHB_INTENABLE                           (1 << 0)
#DEFINE USB2OTG_AHB_AXIBURSTLENGTH                      (1 << 1)
#DEFINE USB2OTG_AHB_WAITFORAXIWRITES                    (1 << 4)
#DEFINE USB2OTG_AHB_DMAENABLE                           (1 << 5)
#DEFINE USB2OTG_AHB_TRANSFEREMPTYLEVEL                  (1 << 7)
#DEFINE USB2OTG_AHB_PERIODICTRANSFEREMPTYLEVEL          (1 << 8)
#DEFINE USB2OTG_AHB_REMMEMSUPP                          (1 << 21)
#DEFINE USB2OTG_AHB_NOTIALLDMAWRIT                      (1 << 22)
#DEFINE USB2OTG_AHB_DMAREMAINDERMODE                    (1 << 23)

/* Bits in USB2OTG_USB */
#DEFINE USB2OTG_USB_TOUTCAL                             (1 << 0)
#DEFINE USB2OTG_USB_PHYINTERFACE                        (1 << 3)
#DEFINE USB2OTG_USB_MODESELECT                          (1 << 4)
#DEFINE USB2OTG_USB_FSINTF                              (1 << 5)
#DEFINE USB2OTG_USB_PHYSEL                              (1 << 6)
#DEFINE USB2OTG_USB_DDRSEL                              (1 << 7)
#DEFINE USB2OTG_USB_SRPCAPABLE                          (1 << 8)
#DEFINE USB2OTG_USB_HNPCAPABLE                          (1 << 9)
#DEFINE USB2OTG_USB_USBTRDTIM                           (1 << 10)
#DEFINE USB2OTG_USB_PHY_LPM_CLK_SEL                     (1 << 15)
#DEFINE USB2OTG_USB_OTGUTMIFSSEL                        (1 << 16)
#DEFINE USB2OTG_USB_ULPIFSLS                            (1 << 17)
#DEFINE USB2OTG_USB_ULPI_AUTO_RES                       (1 << 18)
#DEFINE USB2OTG_USB_ULPI_CLK_SUS_M                      (1 << 19)
#DEFINE USB2OTG_USB_ULPIDRIVEEXTERNALVBUS               (1 << 20)
#DEFINE USB2OTG_USB_ULPI_INT_VBUS_INDICATOR             (1 << 21)
#DEFINE USB2OTG_USB_TSDLINEPULSEENABLE                  (1 << 22)
#DEFINE USB2OTG_USB_INDICATOR_COMPLEMENT                (1 << 23)
#DEFINE USB2OTG_USB_INDICATOR_PASS_THROUGH              (1 << 24)
#DEFINE USB2OTG_USB_ULPI_INT_PROT_DIS                   (1 << 25)
#DEFINE USB2OTG_USB_IC_USB_CAPABLE                      (1 << 26)
#DEFINE USB2OTG_USB_IC_TRAFFIC_PULL_REMOVE              (1 << 27)
#DEFINE USB2OTG_USB_TX_END_DELAY                        (1 << 28)
#DEFINE USB2OTG_USB_FORCE_HOST_MODE                     (1 << 29)
#DEFINE USB2OTG_USB_FORCE_DEV_MODE                      (1 << 30)

/* Bits in USB2OTG_I2CCTRL */
#DEFINE USB2OTG_I2CCTRL_READWRITEDATA                   (1 << 0)
#DEFINE USB2OTG_I2CCTRL_REGISTERADDRESS                 (1 << 8)
#DEFINE USB2OTG_I2CCTRL_ADDRESS                         (1 << 16)
#DEFINE USB2OTG_I2CCTRL_I2CENABLE                       (1 << 23)
#DEFINE USB2OTG_I2CCTRL_ACKNOWLEDGE                     (1 << 24)
#DEFINE USB2OTG_I2CCTRL_I2CSUSPENDCONTROL               (1 << 25)
#DEFINE USB2OTG_I2CCTRL_I2CDEVICEADDRESS                (1 << 26)
#DEFINE USB2OTG_I2CCTRL_READWRITE                       (1 << 30)
#DEFINE USB2OTG_I2CCTRL_BSYDNE                          (1 << 31)

/* Bits in USB2OTG_LPMCONFIG */
#DEFINE USB2OTG_LPMCONFIG_LOWPOWERMODECAPABLE           (1 << 0)
#DEFINE USB2OTG_LPMCONFIG_APPLICATIONRESPONSE           (1 << 1)
#DEFINE USB2OTG_LPMCONFIG_HOSTINITIATEDRESUMEDURATION   (1 << 2)
#DEFINE USB2OTG_LPMCONFIG_REMOTEWAKEUPENABLED           (1 << 6)
#DEFINE USB2OTG_LPMCONFIG_UTMISLEEPENABLED              (1 << 7)
#DEFINE USB2OTG_LPMCONFIG_HOSTINITRESDURATIONTHRESHOLD  (1 << 8)
#DEFINE USB2OTG_LPMCONFIG_LOWPOWERMODERESPONSE          (1 << 13)
#DEFINE USB2OTG_LPMCONFIG_PORTSLEEPSTATUS               (1 << 15)
#DEFINE USB2OTG_LPMCONFIG_SLEEPSTATERESUMEOK            (1 << 16)
#DEFINE USB2OTG_LPMCONFIG_LOWPOWERMODECHANNELINDEX      (1 << 17)
#DEFINE USB2OTG_LPMCONFIG_RETRYCOUNT                    (1 << 21)
#DEFINE USB2OTG_LPMCONFIG_SENDLOWPOWERMODE              (1 << 24)
#DEFINE USB2OTG_LPMCONFIG_RETRYCOUNTSTATUS              (1 << 25)
#DEFINE USB2OTG_LPMCONFIG_HSICCONNECT                   (1 << 30)
#DEFINE USB2OTG_LPMCONFIG_INVERSESELECTHSIC             (1 << 31)

/* Bits in USB2OTG_MDIOSTRL */
#DEFINE USB2OTG_MDIOCTRL_READ                           (1 << 0)
#DEFINE USB2OTG_MDIOCTRL_CLOCKRATIO                     (1 << 16)
#DEFINE USB2OTG_MDIOCTRL_FREERUN                        (1 << 20)
#DEFINE USB2OTG_MDIOCTRL_BITHASHENABLE                  (1 << 21)
#DEFINE USB2OTG_MDIOCTRL_MDCWRITE                       (1 << 22)
#DEFINE USB2OTG_MDIOCTRL_MDOWRITE                       (1 << 23)
#DEFINE USB2OTG_MDIOCTRL_BUSY                           (1 << 31)
        
/* Bits in USB2OTG_MISCCTRL */
#DEFINE USB2OTG_MISCCTRL_SESSIONEND                     (1 << 0)
#DEFINE USB2OTG_MISCCTRL_VBUSVALID                      (1 << 1)
#DEFINE USB2OTG_MISCCTRL_BSESSIONVALID                  (1 << 2)
#DEFINE USB2OTG_MISCCTRL_ASESSIONVALID                  (1 << 3)
#DEFINE USB2OTG_MISCCTRL_DISCHARGEVBUS                  (1 << 4)
#DEFINE USB2OTG_MISCCTRL_CHARGEVBUS                     (1 << 5)
#DEFINE USB2OTG_MISCCTRL_DRIVEVBUS                      (1 << 6)
#DEFINE USB2OTG_MISCCTRL_DISABLEDRIVING                 (1 << 7)
#DEFINE USB2OTG_MISCCTRL_VBUSIRQENABLED                 (1 << 8)
#DEFINE USB2OTG_MISCCTRL_VBUSIRQ                        (1 << 9)
#DEFINE USB2OTG_MISCCTRL_AXIPRIORITYLEVEL               (1 << 16)

/* Core Interrupt bits */
#define USB2OTG_INTRCORE_CURRENTMODE                    (1 << 0)
#define USB2OTG_INTRCORE_MODEMISMATCH                   (1 << 1)
#DEFINE USB2OTG_INTRCORE_OTG                            (1 << 2)
#DEFINE USB2OTG_INTRCORE_DMASTARTOFFRAME                (1 << 3)
#DEFINE USB2OTG_INTRCORE_RECEIVESTATUSLEVEL             (1 << 4)
#DEFINE USB2OTG_INTRCORE_NPTRANSMITFIFOEMPTY            (1 << 5)
#DEFINE USB2OTG_INTRCORE_GINNAKEFF                      (1 << 6)
#DEFINE USB2OTG_INTRCORE_GOUTNAKEFF                     (1 << 7)
#DEFINE USB2OTG_INTRCORE_ULPICK                         (1 << 8)
#DEFINE USB2OTG_INTRCORE_I2C                            (1 << 9)
#DEFINE USB2OTG_INTRCORE_EARLYSUSPEND                   (1 << 10)
#DEFINE USB2OTG_INTRCORE_USBSUSPEND                     (1 << 11)
#DEFINE USB2OTG_INTRCORE_USBRESET                       (1 << 12)
#DEFINE USB2OTG_INTRCORE_ENUMERATIONDONE                (1 << 13)
#DEFINE USB2OTG_INTRCORE_ISOCHRONOUSOUTDROP             (1 << 14)
#DEFINE USB2OTG_INTRCORE_EOPFRAME                       (1 << 15)
#DEFINE USB2OTG_INTRCORE_RESTOREDONE                    (1 << 16)
#DEFINE USB2OTG_INTRCORE_ENDPOINTMISMATCH               (1 << 17)
#DEFINE USB2OTG_INTRCORE_INENDPOINT                     (1 << 18)
#DEFINE USB2OTG_INTRCORE_OUTENDPOINT                    (1 << 19)
#DEFINE USB2OTG_INTRCORE_INCOMPLETEISOCHRONOUSIN        (1 << 20)
#DEFINE USB2OTG_INTRCORE_INCOMPLETEISOCHRONOUSOUT       (1 << 21)
#DEFINE USB2OTG_INTRCORE_FETSETUP                       (1 << 22)
#DEFINE USB2OTG_INTRCORE_RESETDETECT                    (1 << 23)
#DEFINE USB2OTG_INTRCORE_PORT                           (1 << 24)
#DEFINE USB2OTG_INTRCORE_HOSTCHANNEL                    (1 << 25)
#DEFINE USB2OTG_INTRCORE_HPTRANSMITFIFOEMPTY            (1 << 26)
#DEFINE USB2OTG_INTRCORE_LPMTRANSRCVD                   (1 << 27)
#DEFINE USB2OTG_INTRCORE_CONNECTIONIDSTATUSCHANGE       (1 << 28)
#DEFINE USB2OTG_INTRCORE_DISCONNECT                     (1 << 29)
#DEFINE USB2OTG_INTRCORE_SESSIONREQUEST                 (1 << 30)
#DEFINE USB2OTG_INTRCORE_WAKEUP                         (1 << 31)

/* Channel Interrupt bits */
#DEFINE USB2OTG_INTRCHAN_TRANSFERCOMPLETE               (1 << 0)
#DEFINE USB2OTG_INTRCHAN_HALT                           (1 << 1)
#DEFINE USB2OTG_INTRCHAN_AHBERROR                       (1 << 2)
#DEFINE USB2OTG_INTRCHAN_STALL                          (1 << 3)
#DEFINE USB2OTG_INTRCHAN_NEGATIVEACKNOWLEDGE            (1 << 4)
#DEFINE USB2OTG_INTRCHAN_ACKNOWLEDGE                    (1 << 5)
#DEFINE USB2OTG_INTRCHAN_NOTREADY                       (1 << 6)
#DEFINE USB2OTG_INTRCHAN_TRANSACTIONERROR               (1 << 7)
#DEFINE USB2OTG_INTRCHAN_BABBLEERROR                    (1 << 8)
#DEFINE USB2OTG_INTRCHAN_FRAMEOVERRUN                   (1 << 9)
#DEFINE USB2OTG_INTRCHAN_DATATOGGLEERROR                (1 << 10)
#DEFINE USB2OTG_INTRCHAN_BUFFERNOTAVAILABLE             (1 << 11)
#DEFINE USB2OTG_INTRCHAN_EXCESSIVETRANSMISSION          (1 << 12)
#DEFINE USB2OTG_INTRCHAN_FRAMELISTROLLOVER              (1 << 13)

/* Bits in the power register */
#DEFINE USB2OTG_POWER_STOPPCLOCK                        (1 << 0)
#DEFINE USB2OTG_POWER_GATEHCLOCK                        (1 << 1)
#DEFINE USB2OTG_POWER_POWERCLAMP                        (1 << 2)
#DEFINE USB2OTG_POWER_POWERDOWNMODULES                  (1 << 3)
#DEFINE USB2OTG_POWER_PHYSUSPENDED                      (1 << 4)
#DEFINE USB2OTG_POWER_ENABLESLEEPCLOCKGATING            (1 << 5)
#DEFINE USB2OTG_POWER_PHYSLEEPING                       (1 << 6)
#DEFINE USB2OTG_POWER_DEEPSLEEP                         (1 << 7)

#endif	/* USB2OTG_H */
