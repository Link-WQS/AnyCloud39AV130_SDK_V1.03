#ifndef _AK_GMAC_IF_H_
#define _AK_GMAC_IF_H_

#ifndef __KERNEL__
#include <stdint.h>
#include <stdbool.h>
#endif

/* Error Codes */
#define ESYNOPGMACNOERR  0
#define ESYNOPGMACNOMEM  1
#define ESYNOPGMACPHYERR 2
#define ESYNOPGMACBUSY   3

// cdh:add, check ok, ignore type
#ifdef ENH_DESC_8W
typedef struct DmaDescStruct {
    uint32_t status; /* Status */
    uint32_t length; /* Buffer 1  and Buffer 2 length */
    uint32_t buffer1; /* Network Buffer 1 pointer (Dma-able) */
    uint32_t buffer2; /* Network Buffer 2 pointer or next descriptor pointer
                    (Dma-able)in chain structure */
    /* This data below is used only by driver */
    uint32_t extstatus; /* Extended status of a Rx Descriptor */
    uint32_t reserved1; /* Reserved word */
    uint32_t timestamplow; /* Lower 32 bits of the 64 bit timestamp value */
    uint32_t timestamphigh; /* Higher 32 bits of the 64 bit timestamp value */
    uint32_t data1; /* This holds virtual address of buffer1, not used by DMA */
    uint32_t data2; /* This holds virtual address of buffer2, not used by DMA */
} DmaDesc;
#else
typedef struct DmaDescStruct {
    uint32_t status; /* Status */
    uint32_t length; /* Buffer 1  and Buffer 2 length */
    uint32_t buffer1; /* Network Buffer 1 pointer (Dma-able) */
    uint32_t buffer2; /* Network Buffer 2 pointer or next descriptor pointer
                    (Dma-able)in chain structure   */
    /* This data below is used only by driver */
    uint32_t data1; /* This holds virtual address of buffer1, not used by DMA */
    uint32_t data2; /* This holds virtual address of buffer2, not used by DMA */
} DmaDesc;
#endif

/**********************************************************
 * GMAC registers Map
 * For Pci based system address is BARx + GmacRegisterBase
 * For any other system translation is done accordingly
 **********************************************************/
enum GmacRegisters {
    GmacConfig = 0x0000, /* Mac config Register                       */
    GmacFrameFilter = 0x0004, /* Mac frame filtering controls              */
    GmacHashHigh = 0x0008, /* Multi-cast hash table high                */
    GmacHashLow = 0x000C, /* Multi-cast hash table low                 */
    GmacGmiiAddr = 0x0010, /* GMII address Register(ext. Phy)           */
    GmacGmiiData = 0x0014, /* GMII data Register(ext. Phy)              */
    GmacFlowControl = 0x0018, /* Flow control Register                     */
    GmacVlan = 0x001C, /* VLAN tag Register (IEEE 802.1Q)           */

    GmacVersion = 0x0020, /* GMAC Core Version Register                */
    GmacWakeupAddr = 0x0028, /* GMAC wake-up frame filter adrress reg     */
    GmacPmtCtrlStatus = 0x002C, /* PMT control and status register           */

#ifdef LPI_SUPPORT
    GmacLPICtrlSts
    = 0x0030, /* LPI (low power idle) Control and Status Register */
    GmacLPITimerCtrl = 0x0034, /* LPI timer control register               */
#endif

    GmacInterruptStatus = 0x0038, /* Mac Interrupt ststus register        */
    GmacInterruptMask = 0x003C, /* Mac Interrupt Mask register          */

    GmacAddr0High = 0x0040, /* Mac address0 high Register                */
    GmacAddr0Low = 0x0044, /* Mac address0 low Register                 */
    GmacAddr1High = 0x0048, /* Mac address1 high Register                */
    GmacAddr1Low = 0x004C, /* Mac address1 low Register                 */
    GmacAddr2High = 0x0050, /* Mac address2 high Register                */
    GmacAddr2Low = 0x0054, /* Mac address2 low Register                 */
    GmacAddr3High = 0x0058, /* Mac address3 high Register                */
    GmacAddr3Low = 0x005C, /* Mac address3 low Register                 */
    GmacAddr4High = 0x0060, /* Mac address4 high Register                */
    GmacAddr4Low = 0x0064, /* Mac address4 low Register                 */
    GmacAddr5High = 0x0068, /* Mac address5 high Register                */
    GmacAddr5Low = 0x006C, /* Mac address5 low Register                 */
    GmacAddr6High = 0x0070, /* Mac address6 high Register                */
    GmacAddr6Low = 0x0074, /* Mac address6 low Register                 */
    GmacAddr7High = 0x0078, /* Mac address7 high Register                */
    GmacAddr7Low = 0x007C, /* Mac address7 low Register                 */
    GmacAddr8High = 0x0080, /* Mac address8 high Register                */
    GmacAddr8Low = 0x0084, /* Mac address8 low Register                 */
    GmacAddr9High = 0x0088, /* Mac address9 high Register                */
    GmacAddr9Low = 0x008C, /* Mac address9 low Register                 */
    GmacAddr10High = 0x0090, /* Mac address10 high Register               */
    GmacAddr10Low = 0x0094, /* Mac address10 low Register                */
    GmacAddr11High = 0x0098, /* Mac address11 high Register               */
    GmacAddr11Low = 0x009C, /* Mac address11 low Register                */
    GmacAddr12High = 0x00A0, /* Mac address12 high Register               */
    GmacAddr12Low = 0x00A4, /* Mac address12 low Register                */
    GmacAddr13High = 0x00A8, /* Mac address13 high Register               */
    GmacAddr13Low = 0x00AC, /* Mac address13 low Register                */
    GmacAddr14High = 0x00B0, /* Mac address14 high Register               */
    GmacAddr14Low = 0x00B4, /* Mac address14 low Register                */
    GmacAddr15High = 0x00B8, /* Mac address15 high Register               */
    GmacAddr15Low = 0x00BC, /* Mac address15 low Register                */

    /*Time Stamp Register Map*/
    GmacTSControl
    = 0x0700, /* Controls the Timestamp update logic                         :
                 only when IEEE 1588 time stamping is enabled in corekit */

    GmacTSSubSecIncr = 0x0704, /* 8 bit value by which sub second register is
                                  incremented     : only when IEEE 1588 time
                                  stamping without external timestamp input */

    GmacTSHigh = 0x0708, /* 32 bit seconds(MS) : only when IEEE 1588 time
                            stamping without external timestamp input */
    GmacTSLow = 0x070C, /* 32 bit nano seconds(MS) : only when IEEE 1588 time
                           stamping without external timestamp input */

    GmacTSHighUpdate
    = 0x0710, /* 32 bit seconds(MS) to be written/added/subtracted           :
                 only when IEEE 1588 time stamping without external timestamp
                 input */
    GmacTSLowUpdate
    = 0x0714, /* 32 bit nano seconds(MS) to be writeen/added/subtracted      :
                 only when IEEE 1588 time stamping without external timestamp
                 input */

    GmacTSAddend = 0x0718, /* Used by Software to readjust the clock frequency
                              linearly   : only when IEEE 1588 time stamping
                              without external timestamp input */

    GmacTSTargetTimeHigh
    = 0x071C, /* 32 bit seconds(MS) to be compared with system time          :
                 only when IEEE 1588 time stamping without external timestamp
                 input */
    GmacTSTargetTimeLow
    = 0x0720, /* 32 bit nano seconds(MS) to be compared with system time     :
                 only when IEEE 1588 time stamping without external timestamp
                 input */

    GmacTSHighWord = 0x0724, /* Time Stamp Higher Word Register (Version 2
                                only); only lower 16 bits are valid */
    // GmacTSHighWordUpdate    = 0x072C,  /* Time Stamp Higher Word Update
    // Register (Version 2 only); only lower 16 bits are valid */

    GmacTSStatus = 0x0728, /* Time Stamp Status Register */
#ifdef AVB_SUPPORT
    GmacAvMacCtrl = 0x0738, /* AV mac control Register  */
#endif

};

/**********************************************************
 * GMAC Network interface registers
 * This explains the Register's Layout

 * FES is Read only by default and is enabled only when Tx
 * Config Parameter is enabled for RGMII/SGMII interface
 * during CoreKit Config.

 * DM is Read only with value 1'b1 in Full duplex only Config
 **********************************************************/

/* GmacConfig              = 0x0000,    Mac config Register  Layout */
enum GmacConfigReg {
    /* Bit description                      Bits         R/W   Reset value  */
    GmacWatchdog = 0x00800000,
    GmacWatchdogDisable
    = 0x00800000, /* (WD)Disable watchdog timer on Rx      23           RW */
    GmacWatchdogEnable = 0x00000000, /* Enable watchdog timer 0       */

    GmacJabber = 0x00400000,
    GmacJabberDisable
    = 0x00400000, /* (JD)Disable jabber timer on Tx        22           RW */
    GmacJabberEnable = 0x00000000, /* Enable jabber timer 0       */

    GmacFrameBurst = 0x00200000,
    GmacFrameBurstEnable
    = 0x00200000, /* (BE)Enable frame bursting during Tx   21           RW */
    GmacFrameBurstDisable = 0x00000000, /* Disable frame bursting 0       */

    GmacJumboFrame = 0x00100000,
    GmacJumboFrameEnable
    = 0x00100000, /* (JE)Enable jumbo frame for Tx         20           RW */
    GmacJumboFrameDisable = 0x00000000, /* Disable jumbo frame 0       */

    GmacInterFrameGap7
    = 0x000E0000, /* (IFG) Config7 - 40 bit times          19:17        RW */
    GmacInterFrameGap6 = 0x000C0000, /* (IFG) Config6 - 48 bit times */
    GmacInterFrameGap5 = 0x000A0000, /* (IFG) Config5 - 56 bit times */
    GmacInterFrameGap4 = 0x00080000, /* (IFG) Config4 - 64 bit times */
    GmacInterFrameGap3 = 0x00040000, /* (IFG) Config3 - 72 bit times */
    GmacInterFrameGap2 = 0x00020000, /* (IFG) Config2 - 80 bit times */
    GmacInterFrameGap1 = 0x00010000, /* (IFG) Config1 - 88 bit times */
    GmacInterFrameGap0 = 0x00000000, /* (IFG) Config0 - 96 bit times 000 */

    GmacDisableCrs = 0x00010000,
    GmacMiiGmii = 0x00008000,
    GmacSelectMii
    = 0x00008000, /* (PS)Port Select-MII mode              15           RW */
    GmacSelectGmii = 0x00000000, /* GMII mode 0       */

    GmacFESpeed100
    = 0x00004000, /*(FES)Fast Ethernet speed 100Mbps       14           RW */
    GmacFESpeed10 = 0x00000000, /* 10Mbps 0       */

    GmacRxOwn = 0x00002000,
    GmacDisableRxOwn
    = 0x00002000, /* (DO)Disable receive own packets       13           RW */
    GmacEnableRxOwn = 0x00000000, /* Enable receive own packets 0       */

    GmacLoopback = 0x00001000,
    GmacLoopbackOn
    = 0x00001000, /* (LM)Loopback mode for GMII/MII        12           RW */
    GmacLoopbackOff = 0x00000000, /* Normal mode 0       */

    GmacDuplex = 0x00000800,
    GmacFullDuplex
    = 0x00000800, /* (DM)Full duplex mode                  11           RW */
    GmacHalfDuplex = 0x00000000, /* Half duplex mode 0       */

    GmacRxIpcOffload
    = 0x00000400, /*IPC checksum offload           10           RW        0 */

    GmacRetry = 0x00000200,
    GmacRetryDisable
    = 0x00000200, /* (DR)Disable Retry                      9           RW */
    GmacRetryEnable = 0x00000000, /* Enable retransmission as per BL 0 */

    GmacLinkUp = 0x00000100, /* (LUD)Link UP                           8 RW */
    GmacLinkDown = 0x00000100, /* Link Down 0       */

    GmacPadCrcStrip = 0x00000080,
    GmacPadCrcStripEnable
    = 0x00000080, /* (ACS) Automatic Pad/Crc strip enable   7           RW */
    GmacPadCrcStripDisable
    = 0x00000000, /* Automatic Pad/Crc stripping disable 0       */

    GmacBackoffLimit = 0x00000060,
    GmacBackoffLimit3
    = 0x00000060, /* (BL)Back-off limit in HD mode          6:5         RW */
    GmacBackoffLimit2 = 0x00000040,
    GmacBackoffLimit1 = 0x00000020,
    GmacBackoffLimit0 = 0x00000000, /* 00 */

    GmacDeferralCheck = 0x00000010,
    GmacDeferralCheckEnable
    = 0x00000010, /* (DC)Deferral check enable in HD mode   4           RW */
    GmacDeferralCheckDisable = 0x00000000, /* Deferral check disable 0 */

    GmacTx = 0x00000008,
    GmacTxEnable
    = 0x00000008, /* (TE)Transmitter enable                 3           RW */
    GmacTxDisable = 0x00000000, /* Transmitter disable 0       */

    GmacRx = 0x00000004,
    GmacRxEnable
    = 0x00000004, /* (RE)Receiver enable                    2           RW */
    GmacRxDisable = 0x00000000, /* Receiver disable 0       */
};

/* GmacFrameFilter    = 0x0004,     Mac frame filtering controls Register
 * Layout*/
enum GmacFrameFilterReg {
    GmacFilter = 0x80000000,
    GmacFilterOff
    = 0x80000000, /* (RA)Receive all incoming packets       31         RW */
    GmacFilterOn = 0x00000000, /* Receive filtered packets only 0       */

    GmacHashPerfectFilter = 0x00000400, /*Hash or Perfect Filter enable 10
                                           RW         0       */

    GmacSrcAddrFilter = 0x00000200,
    GmacSrcAddrFilterEnable
    = 0x00000200, /* (SAF)Source Address Filter enable       9         RW */
    GmacSrcAddrFilterDisable = 0x00000000, /* 0 */

    GmacSrcInvaAddrFilter = 0x00000100,
    GmacSrcInvAddrFilterEn
    = 0x00000100, /* (SAIF)Inv Src Addr Filter enable        8         RW */
    GmacSrcInvAddrFilterDis = 0x00000000, /* 0 */

    GmacPassControl = 0x000000C0,
    GmacPassControl3
    = 0x000000C0, /* (PCS)Forwards ctrl frms that pass AF    7:6       RW */
    GmacPassControl2 = 0x00000080, /* Forwards all control frames */
    GmacPassControl1 = 0x00000040, /* Does not pass control frames */
    GmacPassControl0 = 0x00000000, /* Does not pass control frames 00      */

    GmacBroadcast = 0x00000020,
    GmacBroadcastDisable
    = 0x00000020, /* (DBF)Disable Rx of broadcast frames     5         RW */
    GmacBroadcastEnable = 0x00000000, /* Enable broadcast frames 0       */

    GmacMulticastFilter = 0x00000010,
    GmacMulticastFilterOff
    = 0x00000010, /* (PM) Pass all multicast packets         4         RW */
    GmacMulticastFilterOn
    = 0x00000000, /* Pass filtered multicast packets 0       */

    GmacDestAddrFilter = 0x00000008,
    GmacDestAddrFilterInv
    = 0x00000008, /* (DAIF)Inverse filtering for DA          3         RW */
    GmacDestAddrFilterNor = 0x00000000, /* Normal filtering for DA 0       */

    GmacMcastHashFilter = 0x00000004,
    GmacMcastHashFilterOn
    = 0x00000004, /* (HMC)perfom multicast hash filtering    2         RW */
    GmacMcastHashFilterOff = 0x00000000, /* perfect filtering only 0       */

    GmacUcastHashFilter = 0x00000002,
    GmacUcastHashFilterOn
    = 0x00000002, /* (HUC)Unicast Hash filtering only        1         RW */
    GmacUcastHashFilterOff = 0x00000000, /* perfect filtering only 0       */

    GmacPromiscuousMode = 0x00000001,
    GmacPromiscuousModeOn = 0x00000001, /* Receive all frames 0         RW */
    GmacPromiscuousModeOff = 0x00000000, /* Receive filtered packets only 0 */
};

/*GmacGmiiAddr             = 0x0010,    GMII address Register(ext. Phy) Layout
 */
enum GmacGmiiAddrReg {
    GmiiDevMask = 0x0000F800, /* (PA)GMII device address                 15:11
                                 RW         0x00    */
    GmiiDevShift = 11,

    GmiiRegMask = 0x000007C0, /* (GR)GMII register in selected Phy       10:6
                                 RW         0x00    */
    GmiiRegShift = 6,

    GmiiCsrClkMask = 0x0000003C, /* cdh:CSR Clock bit Mask           4:2    ,
                                    must :0x0000001C           */
    GmiiCsrClk5 = 0x00000014, /* (CR)CSR Clock Range     250-300 MHz      4:2
                                 RW         000     */
    GmiiCsrClk4 = 0x00000010, /*                         150-250 MHz */
    GmiiCsrClk3 = 0x0000000C, /*                         35-60 MHz */
    GmiiCsrClk2 = 0x00000008, /*                         20-35 MHz */
    GmiiCsrClk1 = 0x00000004, /*                         100-150 MHz */
    GmiiCsrClk0 = 0x00000000, /*                         60-100 MHz */

    GmiiWrite
    = 0x00000002, /* (GW)Write to register                      1      RW */
    GmiiRead = 0x00000000, /* Read from register 0      */

    GmiiBusy = 0x00000001, /* (GB)GMII interface is busy 0 RW 0 */
};

/* GmacGmiiData            = 0x0014,    GMII data Register(ext. Phy) Layout */
enum GmacGmiiDataReg {
    GmiiDataMask = 0x0000FFFF, /* (GD)GMII Data  15:0 RW 0x0000  */
};

/*GmacFlowControl    = 0x0018,    Flow control Register   Layout */
enum GmacFlowControlReg {
    GmacPauseTimeMask = 0xFFFF0000, /* (PT) PAUSE TIME field in the control
                                       frame  31:16   RW       0x0000  */
    GmacPauseTimeShift = 16,

    GmacPauseLowThresh = 0x00000030,
    GmacPauseLowThresh3
    = 0x00000030, /* (PLT)thresh for pause tmr 256 slot time      5:4    RW */
    GmacPauseLowThresh2
    = 0x00000020, /*                           144 slot time */
    GmacPauseLowThresh1
    = 0x00000010, /*                            28 slot time */
    GmacPauseLowThresh0
    = 0x00000000, /*                             4 slot time 000    */

    GmacUnicastPauseFrame = 0x00000008,
    GmacUnicastPauseFrameOn
    = 0x00000008, /* (UP)Detect pause frame with unicast addr.     3    RW */
    GmacUnicastPauseFrameOff
    = 0x00000000, /* Detect only pause frame with multicast addr. 0     */

    GmacRxFlowControl = 0x00000004,
    GmacRxFlowControlEnable
    = 0x00000004, /* (RFE)Enable Rx flow control                   2    RW */
    GmacRxFlowControlDisable = 0x00000000, /* Disable Rx flow control 0     */

    GmacTxFlowControl = 0x00000002,
    GmacTxFlowControlEnable
    = 0x00000002, /* (TFE)Enable Tx flow control                   1    RW */
    GmacTxFlowControlDisable = 0x00000000, /* Disable flow control 0     */

    GmacFlowControlBackPressure = 0x00000001,
    GmacSendPauseFrame = 0x00000001, /* (FCB/PBA)send pause frm/Apply back
                                        pressure   0    RW          0     */
};

/*  GmacInterruptStatus   = 0x0038,     Mac Interrupt ststus register */
enum GmacInterruptStatusBitDefinition {
    GmacTSIntSts = 0x00000200, /* set if int generated due to TS (Read Time
                                  Stamp Status Register to know details)*/
    GmacMmcRxChksumOffload = 0x00000080, /* set if int generated in MMC RX
                                            CHECKSUM OFFLOAD int register */
    GmacMmcTxIntSts
    = 0x00000040, /* set if int generated in MMC TX Int register             */
    GmacMmcRxIntSts
    = 0x00000020, /* set if int generated in MMC RX Int register             */
    GmacMmcIntSts = 0x00000010, /* set if any of the above bit [7:5] is set */
    GmacPmtIntSts
    = 0x00000008, /* set whenver magic pkt/wake-on-lan frame is received */
    GmacPcsAnComplete = 0x00000004, /* set when AN is complete in
                                       TBI/RTBI/SGMIII phy interface        */
    GmacPcsLnkStsChange = 0x00000002, /* set if any lnk status change in
                                         TBI/RTBI/SGMII interface        */
    GmacRgmiiIntSts
    = 0x00000001, /* set if any change in lnk status of RGMII interface */

};

/*  GmacInterruptMask       = 0x003C,     Mac Interrupt Mask register */
enum GmacInterruptMaskBitDefinition {
    GmacTSIntMask
    = 0x00000200, /* when set disables the time stamp interrupt generation */
    GmacPmtIntMask
    = 0x00000008, /* when set Disables the assertion of PMT interrupt */
    GmacPcsAnIntMask = 0x00000004, /* When set disables the assertion of PCS AN
                                      complete interrupt         */
    GmacPcsLnkStsIntMask = 0x00000002, /* when set disables the assertion of PCS
                                          lnk status change interrupt   */
    GmacRgmiiIntMask
    = 0x00000001, /* when set disables the assertion of RGMII int             */
};

/**********************************************************
 * GMAC DMA registers
 * For Pci based system address is BARx + GmaDmaBase
 * For any other system translation is done accordingly
 **********************************************************/

enum DmaRegisters {
    DmaBusMode = 0x0000, /* CSR0 - Bus Mode Register                          */
    DmaTxPollDemand = 0x0004, /* CSR1 - Transmit Poll Demand Register */
    DmaRxPollDemand = 0x0008, /* CSR2 - Receive Poll Demand Register */
    DmaRxBaseAddr = 0x000C, /* CSR3 - Receive Descriptor list base address */
    DmaTxBaseAddr = 0x0010, /* CSR4 - Transmit Descriptor list base address */
    DmaStatus = 0x0014, /* CSR5 - Dma status Register                        */
    DmaControl = 0x0018, /* CSR6 - Dma Operation Mode Register                */
    DmaInterrupt = 0x001C, /* CSR7 - Interrupt enable */
    DmaMissedFr = 0x0020, /* CSR8 - Missed Frame & Buffer overflow Counter */
    DmaTxCurrDesc = 0x0048, /*      - Current host Tx Desc Register */
    DmaRxCurrDesc = 0x004C, /*      - Current host Rx Desc Register */
    DmaTxCurrAddr = 0x0050, /* CSR20 - Current host transmit buffer address */
    DmaRxCurrAddr = 0x0054, /* CSR21 - Current host receive buffer address */

#ifdef AVB_SUPPORT
    HwFeature = 0x0058, /* Hardware Feature Register                         */

    DmaSlotFnCtrlSts = 0x0030, /* Slot function control and status register */

    DmaChannelCtrl
    = 0x0060, /* Channel Control register only for Channel1 and Channel2 */
    DmaChannelAvSts
    = 0x0064, /* Channel Status register only for Channel1 and  Channel2 */
    IdleSlopeCredit = 0x0068, /* Idle slope credit register */
    SendSlopeCredit = 0x006C, /* Send slope credit register */
    HighCredit = 0x0070, /* High Credit register */
    LoCredit = 0x0074, /* Lo Credit Register */
#endif

};

/**********************************************************
 * DMA Engine registers Layout
 **********************************************************/

/*DmaBusMode               = 0x0000,    CSR0 - Bus Mode */
enum DmaBusModeReg {
/* Bit description                                Bits     R/W   Reset value */
#ifdef AVB_SUPPORT
    DmaChannelPrioWt = 0x30000000, /* Channel priority weight mask 29:28    RW
                                      0       */
    DmaChannelPrio1 = 0x00000000, /* Channel priority weight 1 29:28    RW
                                     0       */
    DmaChannelPrio2 = 0x10000000, /* Channel priority weight 2 29:28    RW
                                     0       */
    DmaChannelPrio3 = 0x20000000, /* Channel priority weight 3 29:28    RW
                                     0       */
    DmaChannelPrio4 = 0x30000000, /* Channel priority weight 4 29:28    RW
                                     0       */

    DmaTxRxPrio = 0x08000000, /* When set indicates Tx Dma has more priority
                                 27      RW        0       */

    DmaPriorityRatio11 = 0x00000000, /* (PR)TX:RX DMA priority ratio 1:1 15:14
                                        RW        00      */
    DmaPriorityRatio21 = 0x00004000, /* (PR)TX:RX DMA priority ratio 2:1 */
    DmaPriorityRatio31 = 0x00008000, /* (PR)TX:RX DMA priority ratio 3:1 */
    DmaPriorityRatio41 = 0x0000C000, /* (PR)TX:RX DMA priority ratio 4:1 */

    DmaArbitration = 0x00000002, /* Dma Arbitration decides whether strict prio
                                    or RR  1      RW       0       */
    DmaArbitrationStrict = 0x00000002, /* Dma Arbitration decides whether strict
                                          prio or RR  1      RW       0       */
    DmaArbitrationRR = 0x00000000, /* Dma Arbitration decides whether strict
                                      prio or RR  0      RW       0       */
#endif

    DmaFixedBurstEnable = 0x00010000, /* (FB)Fixed Burst SINGLE, INCR4, INCR8 or
                                         INCR16   16     RW                */
    DmaFixedBurstDisable = 0x00000000, /*             SINGLE, INCR 0       */

    DmaTxPriorityRatio11 = 0x00000000, /* (PR)TX:RX DMA priority ratio 1:1
                                          15:14   RW        00      */
    DmaTxPriorityRatio21 = 0x00004000, /* (PR)TX:RX DMA priority ratio 2:1 */
    DmaTxPriorityRatio31 = 0x00008000, /* (PR)TX:RX DMA priority ratio 3:1 */
    DmaTxPriorityRatio41 = 0x0000C000, /* (PR)TX:RX DMA priority ratio 4:1 */

    DmaBurstLengthx8 = 0x01000000, /* When set mutiplies the PBL by 8 24 RW
                                      0      */

    DmaBurstLength256 = 0x01002000, /*(DmaBurstLengthx8 | DmaBurstLength32) =
                                       256      [24]:13:8                 */
    DmaBurstLength128 = 0x01001000, /*(DmaBurstLengthx8 | DmaBurstLength16) =
                                       128      [24]:13:8                 */
    DmaBurstLength64 = 0x01000800, /*(DmaBurstLengthx8 | DmaBurstLength8) = 64
                                      [24]:13:8                 */
    DmaBurstLength32 = 0x00002000, /* (PBL) programmable Dma burst length = 32
                                      13:8    RW                */
    DmaBurstLength16 = 0x00001000, /* Dma burst length = 16 */
    DmaBurstLength8 = 0x00000800, /* Dma burst length = 8 */
    DmaBurstLength4 = 0x00000400, /* Dma burst length = 4 */
    DmaBurstLength2 = 0x00000200, /* Dma burst length = 2 */
    DmaBurstLength1 = 0x00000100, /* Dma burst length = 1 */
    DmaBurstLength0 = 0x00000000, /* Dma burst length = 0 0x00   */

    DmaDescriptor8Words
    = 0x00000080, /* Enh Descriptor works  1=> 8 word descriptor      7 0 */
    DmaDescriptor4Words
    = 0x00000000, /* Enh Descriptor works  0=> 4 word descriptor      7 0 */

    DmaDescriptorSkip16 = 0x00000040, /* (DSL)Descriptor skip length (no.of
                                         dwords)       6:2     RW */
    DmaDescriptorSkip8 = 0x00000020, /* between two unchained descriptors */
    DmaDescriptorSkip4 = 0x00000010,
    DmaDescriptorSkip2 = 0x00000008,
    DmaDescriptorSkip1 = 0x00000004,
    DmaDescriptorSkip0 = 0x00000000, /* 0x00 */

    DmaArbitRr = 0x00000000, /* (DA) DMA RR arbitration 1     RW
                                0     */
    DmaArbitPr = 0x00000002, /* Rx has priority over Tx */

    DmaResetOn = 0x00000001, /* (SWR)Software Reset DMA engine 0     RW */
    DmaResetOff = 0x00000000, /* 0 */
};

/*DmaStatus         = 0x0014,    CSR5 - Dma status Register */
enum DmaStatusReg {
/*Bit 28 27 and 26 indicate whether the interrupt due to PMT GMACMMC or GMAC
 * LINE Remaining bits are DMA interrupts*/

#ifdef AVB_SUPPORT
    DmaSlotCounterIntr = 0x40000000, /* For Ch1 and Ch2 AVB slot interrupt
                                        status          31     RW       0 */
#endif
#ifdef LPI_SUPPORT
    GmacLPIIntr = 0x40000000, /* GMC LPI interrupt 31     RO       0       */
#endif

    GmacPmtIntr = 0x10000000, /* (GPI)Gmac subsystem interrupt 28     RO 0
                               */
    GmacMmcIntr = 0x08000000, /* (GMI)Gmac MMC subsystem interrupt 27     RO
                                 0       */
    GmacLineIntfIntr = 0x04000000, /* Line interface interrupt 26     RO 0
                                    */

    DmaErrorBit2 = 0x02000000, /* (EB)Error bits 0-data buffer, 1-desc. access
                                  25     RO       0       */
    DmaErrorBit1 = 0x01000000, /* (EB)Error bits 0-write trnsf, 1-read transfr
                                  24     RO       0       */
    DmaErrorBit0 = 0x00800000, /* (EB)Error bits 0-Rx DMA, 1-Tx DMA 23     RO
                                  0       */

    DmaTxState = 0x00700000, /* (TS)Transmit process state 22:20  RO */
    DmaTxStopped
    = 0x00000000, /* Stopped - Reset or Stop Tx Command issued 000      */
    DmaTxFetching = 0x00100000, /* Running - fetching the Tx descriptor */
    DmaTxWaiting = 0x00200000, /* Running - waiting for status */
    DmaTxReading = 0x00300000, /* Running - reading the data from host memory */
    DmaTxSuspended = 0x00600000, /* Suspended - Tx Descriptor unavailabe */
    DmaTxClosing = 0x00700000, /* Running - closing Rx descriptor */

    DmaRxState = 0x000E0000, /* (RS)Receive process state 19:17  RO */
    DmaRxStopped
    = 0x00000000, /* Stopped - Reset or Stop Rx Command issued 000      */
    DmaRxFetching = 0x00020000, /* Running - fetching the Rx descriptor */
    DmaRxWaiting = 0x00060000, /* Running - waiting for packet */
    DmaRxSuspended = 0x00080000, /* Suspended - Rx Descriptor unavailable */
    DmaRxClosing = 0x000A0000, /* Running - closing descriptor */
    DmaRxQueuing
    = 0x000E0000, /* Running - queuing the recieve frame into host memory */

    DmaIntNormal = 0x00010000, /* (NIS)Normal interrupt summary 16     RW 0
                                */
    DmaIntAbnormal = 0x00008000, /* (AIS)Abnormal interrupt summary 15     RW
                                    0       */

    DmaIntEarlyRx
    = 0x00004000, /* Early receive interrupt (Normal)       RW        0       */
    DmaIntBusError
    = 0x00002000, /* Fatal bus error (Abnormal)             RW        0       */
    DmaIntEarlyTx
    = 0x00000400, /* Early transmit interrupt (Abnormal)    RW        0       */
    DmaIntRxWdogTO
    = 0x00000200, /* Receive Watchdog Timeout (Abnormal)    RW        0       */
    DmaIntRxStopped
    = 0x00000100, /* Receive process stopped (Abnormal)     RW        0       */
    DmaIntRxNoBuffer
    = 0x00000080, /* Receive buffer unavailable (Abnormal)  RW        0       */
    DmaIntRxCompleted
    = 0x00000040, /* Completion of frame reception (Normal) RW        0       */
    DmaIntTxUnderflow
    = 0x00000020, /* Transmit underflow (Abnormal)          RW        0       */
    DmaIntRcvOverflow
    = 0x00000010, /* Receive Buffer overflow interrupt      RW        0       */
    DmaIntTxJabberTO
    = 0x00000008, /* Transmit Jabber Timeout (Abnormal)     RW        0       */
    DmaIntTxNoBuffer
    = 0x00000004, /* Transmit buffer unavailable (Normal)   RW        0       */
    DmaIntTxStopped
    = 0x00000002, /* Transmit process stopped (Abnormal)    RW        0       */
    DmaIntTxCompleted
    = 0x00000001, /* Transmit completed (Normal)            RW        0       */
};

/*DmaControl        = 0x0018,     CSR6 - Dma Operation Mode Register */
enum DmaControlReg {
    DmaDisableDropTcpCs = 0x04000000, /* (DT) Dis. drop. of tcp/ip CS error
                                         frames        26      RW        0 */
    DmaDisableFlush = 0x01000000,

    DmaStoreAndForward = 0x00200000, /* (SF)Store and forward 21      RW 0
                                      */
    DmaFlushTxFifo = 0x00100000, /* (FTF)Tx FIFO controller is reset to default
                                    20      RW        0       */

    DmaTxThreshCtrl = 0x0001C000, /* (TTC)Controls thre Threh of MTL tx Fifo
                                     16:14   RW                */
    DmaTxThreshCtrl16 = 0x0001C000, /* (TTC)Controls thre Threh of MTL tx Fifo
                                       16       16:14   RW                */
    DmaTxThreshCtrl24 = 0x00018000, /* (TTC)Controls thre Threh of MTL tx Fifo
                                       24       16:14   RW                */
    DmaTxThreshCtrl32 = 0x00014000, /* (TTC)Controls thre Threh of MTL tx Fifo
                                       32       16:14   RW                */
    DmaTxThreshCtrl40 = 0x00010000, /* (TTC)Controls thre Threh of MTL tx Fifo
                                       40       16:14   RW                */
    DmaTxThreshCtrl256 = 0x0000c000, /* (TTC)Controls thre Threh of MTL tx Fifo
                                        256      16:14   RW                */
    DmaTxThreshCtrl192 = 0x00008000, /* (TTC)Controls thre Threh of MTL tx Fifo
                                        192      16:14   RW                */
    DmaTxThreshCtrl128 = 0x00004000, /* (TTC)Controls thre Threh of MTL tx Fifo
                                        128      16:14   RW                */
    DmaTxThreshCtrl64 = 0x00000000, /* (TTC)Controls thre Threh of MTL tx Fifo
                                       64       16:14   RW        000     */

    DmaTxStart = 0x00002000, /* (ST)Start/Stop transmission 13      RW
                                0       */

    DmaRxFlowCtrlDeact = 0x00401800, /* (RFD)Rx flow control deact. threhold
                                        [22]:12:11   RW                 */
    DmaRxFlowCtrlDeact1K
    = 0x00000000, /* (RFD)Rx flow control deact. threhold (1kbytes)   [22]:12:11
                     RW        00       */
    DmaRxFlowCtrlDeact2K = 0x00000800, /* (RFD)Rx flow control deact. threhold
                                          (2kbytes)   [22]:12:11   RW */
    DmaRxFlowCtrlDeact3K = 0x00001000, /* (RFD)Rx flow control deact. threhold
                                          (3kbytes)   [22]:12:11   RW */
    DmaRxFlowCtrlDeact4K = 0x00001800, /* (RFD)Rx flow control deact. threhold
                                          (4kbytes)   [22]:12:11   RW */
    DmaRxFlowCtrlDeact5K = 0x00400000, /* (RFD)Rx flow control deact. threhold
                                          (4kbytes)   [22]:12:11   RW */
    DmaRxFlowCtrlDeact6K = 0x00400800, /* (RFD)Rx flow control deact. threhold
                                          (4kbytes)   [22]:12:11   RW */
    DmaRxFlowCtrlDeact7K = 0x00401000, /* (RFD)Rx flow control deact. threhold
                                          (4kbytes)   [22]:12:11   RW */

    DmaRxFlowCtrlAct = 0x00800600, /* (RFA)Rx flow control Act. threhold
                                      [23]:10:09   RW                 */
    DmaRxFlowCtrlAct1K
    = 0x00000000, /* (RFA)Rx flow control Act. threhold (1kbytes)    [23]:10:09
                     RW        00       */
    DmaRxFlowCtrlAct2K = 0x00000200, /* (RFA)Rx flow control Act. threhold
                                        (2kbytes)    [23]:10:09   RW */
    DmaRxFlowCtrlAct3K = 0x00000400, /* (RFA)Rx flow control Act. threhold
                                        (3kbytes)    [23]:10:09   RW */
    DmaRxFlowCtrlAct4K = 0x00000300, /* (RFA)Rx flow control Act. threhold
                                        (4kbytes)    [23]:10:09   RW */
    DmaRxFlowCtrlAct5K = 0x00800000, /* (RFA)Rx flow control Act. threhold
                                        (5kbytes)    [23]:10:09   RW */
    DmaRxFlowCtrlAct6K = 0x00800200, /* (RFA)Rx flow control Act. threhold
                                        (6kbytes)    [23]:10:09   RW */
    DmaRxFlowCtrlAct7K = 0x00800400, /* (RFA)Rx flow control Act. threhold
                                        (7kbytes)    [23]:10:09   RW */

    DmaRxThreshCtrl = 0x00000018, /* (RTC)Controls thre Threh of MTL rx Fifo
                                     4:3   RW                */
    DmaRxThreshCtrl64 = 0x00000000, /* (RTC)Controls thre Threh of MTL tx Fifo
                                       64       4:3   RW                */
    DmaRxThreshCtrl32 = 0x00000008, /* (RTC)Controls thre Threh of MTL tx Fifo
                                       32       4:3   RW                */
    DmaRxThreshCtrl96 = 0x00000010, /* (RTC)Controls thre Threh of MTL tx Fifo
                                       96       4:3   RW                */
    DmaRxThreshCtrl128 = 0x00000018, /* (RTC)Controls thre Threh of MTL tx Fifo
                                        128      4:3   RW                */

    DmaEnHwFlowCtrl = 0x00000100, /* (EFC)Enable HW flow control 8       RW
                                   */
    DmaDisHwFlowCtrl = 0x00000000, /* Disable HW flow control 0        */

    DmaFwdErrorFrames = 0x00000080, /* (FEF)Forward error frames 7       RW
                                       0       */
    DmaFwdUnderSzFrames = 0x00000040, /* (FUF)Forward undersize frames 6 RW
                                         0       */
    DmaTxSecondFrame = 0x00000004, /* (OSF)Operate on second frame 4       RW
                                      0       */
    DmaRxStart = 0x00000002, /* (SR)Start/Stop reception 1       RW        0
                              */
};

/*DmaInterrupt      = 0x001C,    CSR7 - Interrupt enable Register Layout     */
enum DmaInterruptReg {
    DmaIeNormal
    = DmaIntNormal, /* Normal interrupt enable                 RW        0 */
    DmaIeAbnormal = DmaIntAbnormal, /* Abnormal interrupt enable RW        0
                                     */

    DmaIeEarlyRx
    = DmaIntEarlyRx, /* Early receive interrupt enable          RW        0 */
    DmaIeBusError = DmaIntBusError, /* Fatal bus error enable RW        0 */
    DmaIeEarlyTx
    = DmaIntEarlyTx, /* Early transmit interrupt enable         RW        0 */
    DmaIeRxWdogTO = DmaIntRxWdogTO, /* Receive Watchdog Timeout enable RW 0
                                     */
    DmaIeRxStopped = DmaIntRxStopped, /* Receive process stopped enable RW
                                         0       */
    DmaIeRxNoBuffer = DmaIntRxNoBuffer, /* Receive buffer unavailable enable
                                           RW        0       */
    DmaIeRxCompleted = DmaIntRxCompleted, /* Completion of frame reception
                                             enable    RW        0       */
    DmaIeTxUnderflow = DmaIntTxUnderflow, /* Transmit underflow enable RW 0
                                           */

    DmaIeRxOverflow = DmaIntRcvOverflow, /* Receive Buffer overflow interrupt
                                            RW        0       */
    DmaIeTxJabberTO = DmaIntTxJabberTO, /* Transmit Jabber Timeout enable RW
                                           0       */
    DmaIeTxNoBuffer = DmaIntTxNoBuffer, /* Transmit buffer unavailable enable
                                           RW        0       */
    DmaIeTxStopped = DmaIntTxStopped, /* Transmit process stopped enable RW
                                         0       */
    DmaIeTxCompleted = DmaIntTxCompleted, /* Transmit completed enable RW 0
                                           */
};

#ifdef AVB_SUPPORT
/*DmaSlotFnCtrlSts  = 0x0030,     Slot function control and status register */
enum DmaSlotFnCtrlStsReg {
    SlotNum = 0x000F0000, /* Current Slot Number 19:16     R0         0 */
    AdvSlotInt = 0x00000002, /* Advance the slot interval for data fetch       1
                                RW         0      */
    EnaSlot = 0x00000001, /* Enable checking of Slot number                 0
                             RW         0      */
};

/*  DmaChannelCtrl    = 0x0060,     Channel Control register only for Channel1
 * and Channel2 */
enum DmaChannelCtrlReg {
    ChannelSlotIntEn = 0x00020000, /* Channel Slot Interrupt Enable 16 RW
                                      0      */
    ChannelSlotCount = 0x00000070, /* Channel Slot Count 6:4 RW
                                      0      */
    ChannelCreditCtrl = 0x00000002, /* Channel Credit Control 1          RW
                                       0      */
    ChannelCreditShDis = 0x00000001, /* Channel Credit based shaping disable
                                        0          RW         0      */
};

/*  DmaChannelSts     = 0x0064,     Channel Status register only for Channel1
 * and  Channel2 */
enum DmaChannelStsReg {
    ChannelAvBitsPerSlot = 0x0000FFFF, /* Channel Average Bits per slot 16:0
                                          RO         0      */
};

/*  IdleSlopeCredit   = 0x0068,     Idle slope credit register */
enum IdleSlopeCreditReg {
    ChannelIdleSlCr = 0x00003FFF, /*Channel Idle Slope Credit 13:0       RW
                                     0     */
};

/*SendSlopeCredit   = 0x006C,     Send slope credit register */
enum SendSlopeCreditReg {
    ChannelSendSlCr = 0x00003FFF, /*Channel Send Slope Credit 13:0       RW
                                     0     */
};

/*  HighCredit        = 0x0070,     High Credit register */
enum HighCreditReg {
    ChannelHiCr = 0x1FFFFFFF, /*Channel Hi Credit 28:0       RW         0 */
};

/*  LoCredit          = 0x0074,     Lo Credit Register */
enum LoCreditReg {
    ChannelLoCr = 0x1FFFFFFF, /* Channel Lo Credit 28:0      RW         0 */
};
/*DmaChannelAvSts   */
enum DmaChannelAvStsReg {
    ChannelAvgBitsPerSlotMsk = 0x0001FFFF,
};
#endif

/**********************************************************
 * DMA Engine descriptors
 **********************************************************/
#ifdef ENH_DESC
/*
**********Enhanced Descritpor structure to support 8K buffer per buffer
****************************

DmaRxBaseAddr     = 0x000C,   CSR3 - Receive Descriptor list base address
DmaRxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format
in Little endian with a 32 bit Data bus is as shown below

Similarly
DmaTxBaseAddr     = 0x0010,  CSR4 - Transmit Descriptor list base address
DmaTxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format
in Little endian with a 32 bit Data bus is as shown below
          -------------------------------------------------------------------
    RDES0     |OWN (31)| Status |
          -------------------------------------------------------------------
    RDES1     | Ctrl | Res | Byte Count Buffer 2 | Ctrl | Res | Byte Count
Buffer 1    |
          -------------------------------------------------------------------
    RDES2     |  Buffer 1 Address |
          -------------------------------------------------------------------
    RDES3     |  Buffer 2 Address / Next Descriptor Address |
          -------------------------------------------------------------------

          -------------------------------------------------------------------
    TDES0     |OWN (31)| Ctrl | Res | Ctrl | Res | Status |
          -------------------------------------------------------------------
    TDES1     | Res | Byte Count Buffer 2 | Res |         Byte Count Buffer 1 |
          -------------------------------------------------------------------
    TDES2     |  Buffer 1 Address |
          -------------------------------------------------------------------
    TDES3     |  Buffer 2 Address / Next Descriptor Address |
          -------------------------------------------------------------------

*/

enum DmaDescriptorStatus /* status word of DMA descriptor */
{

    DescOwnByDma = 0x80000000, /* (OWN)Descriptor is owned by DMA engine 31
                                  RW                  */

    DescDAFilterFail
    = 0x40000000, /* (AFM)Rx - DA Filter Fail for the rx frame           30 */

    DescFrameLengthMask = 0x3FFF0000, /* (FL)Receive descriptor frame length
                                         29:16                       */
    DescFrameLengthShift = 16,

    DescError
    = 0x00008000, /* (ES)Error summary bit  - OR of the follo. bits:     15 */
    /*  DE || OE || IPC || LC || RWT || RE || CE */
    DescRxTruncated
    = 0x00004000, /* (DE)Rx - no more descriptors for receive frame      14 */
    DescSAFilterFail
    = 0x00002000, /* (SAF)Rx - SA Filter Fail for the received frame     13 */
    DescRxLengthError
    = 0x00001000, /* (LE)Rx - frm size not matching with len field     12 */
    DescRxDamaged
    = 0x00000800, /* (OE)Rx - frm was damaged due to buffer overflow     11 */
    DescRxVLANTag
    = 0x00000400, /* (VLAN)Rx - received frame is a VLAN frame           10 */
    DescRxFirst = 0x00000200, /* (FS)Rx - first descriptor of the frame 9 */
    DescRxLast = 0x00000100, /* (LS)Rx - last descriptor of the frame 8 */
    DescRxLongFrame
    = 0x00000080, /* (Giant Frame)Rx - frame is longer than 1518/1522    7 */
    DescRxCollision
    = 0x00000040, /* (LC)Rx - late collision occurred during reception   6 */
    DescRxFrameEther
    = 0x00000020, /* (FT)Rx - Frame type - Ethernet, otherwise 802.3     5 */
    DescRxWatchdog
    = 0x00000010, /* (RWT)Rx - watchdog timer expired during reception   4 */
    DescRxMiiError
    = 0x00000008, /* (RE)Rx - error reported by MII interface            3 */
    DescRxDribbling
    = 0x00000004, /* (DE)Rx - frame contains non int multiple of 8 bits  2 */
    DescRxCrc = 0x00000002, /* (CE)Rx - CRC error 1 */
    // DescRxMacMatch        = 0x00000001,   /* (RX MAC Address) Rx mac address
    // reg(1 to 15)match   0                          */

    DescRxEXTsts = 0x00000001, /* Extended Status Available (RDES4) 0 */

    DescTxIntEnable = 0x40000000, /* (IC)Tx - interrupt on completion 30 */
    DescTxLast = 0x20000000, /* (LS)Tx - Last segment of the frame 29 */
    DescTxFirst = 0x10000000, /* (FS)Tx - First segment of the frame 28 */
    DescTxDisableCrc
    = 0x08000000, /* (DC)Tx - Add CRC disabled (first segment only)      27 */
    DescTxDisablePadd
    = 0x04000000, /* (DP)disable padding, added by - reyaz               26 */

    DescTxCisMask
    = 0x00c00000, /* Tx checksum offloading control mask             23:22 */
    DescTxCisBypass = 0x00000000, /* Checksum bypass */
    DescTxCisIpv4HdrCs = 0x00400000, /* IPv4 header checksum */
    DescTxCisTcpOnlyCs = 0x00800000, /* TCP/UDP/ICMP checksum. Pseudo header
                                        checksum is assumed to be present */
    DescTxCisTcpPseudoCs = 0x00c00000, /* TCP/UDP/ICMP checksum fully in
                                          hardware including pseudo header */

    TxDescEndOfRing = 0x00200000, /* (TER)End of descriptors ring 21 */
    TxDescChain
    = 0x00100000, /* (TCH)Second buffer address is chain address         20 */

    DescRxChkBit0 = 0x00000001, /*()  Rx - Rx Payload Checksum Error 0 */
    DescRxChkBit7
    = 0x00000080, /* (IPC CS ERROR)Rx - Ipv4 header checksum error       7 */
    DescRxChkBit5 = 0x00000020, /* (FT)Rx - Frame type - Ethernet, otherwise
                                   802.3         5                          */

    DescRxTSavail = 0x00000080, /* Time stamp available 7 */
    DescRxFrameType = 0x00000020, /* (FT)Rx - Frame type - Ethernet, otherwise
                                     802.3       5                          */

    DescTxIpv4ChkError = 0x00010000, /* (IHE) Tx Ip header error 16 */
    DescTxTimeout = 0x00004000, /* (JT)Tx - Transmit jabber timeout 14 */
    DescTxFrameFlushed
    = 0x00002000, /* (FF)Tx - DMA/MTL flushed the frame due to SW flush  13 */
    DescTxPayChkError = 0x00001000, /* (PCE) Tx Payload checksum Error 12 */
    DescTxLostCarrier
    = 0x00000800, /* (LC)Tx - carrier lost during tramsmission           11 */
    DescTxNoCarrier
    = 0x00000400, /* (NC)Tx - no carrier signal from the tranceiver      10 */
    DescTxLateCollision
    = 0x00000200, /* (LC)Tx - transmission aborted due to collision      9 */
    DescTxExcCollisions
    = 0x00000100, /* (EC)Tx - transmission aborted after 16 collisions   8 */
    DescTxVLANFrame = 0x00000080, /* (VF)Tx - VLAN-type frame 7 */

    DescTxCollMask = 0x00000078, /* (CC)Tx - Collision count 6:3 */
    DescTxCollShift = 3,

    DescTxExcDeferral = 0x00000004, /* (ED)Tx - excessive deferral 2 */
    DescTxUnderflow
    = 0x00000002, /* (UF)Tx - late data arrival from the memory          1 */
    DescTxDeferred = 0x00000001, /* (DB)Tx - frame transmision deferred 0 */

    /*
    This explains the RDES1/TDES1 bits layout
        --------------------------------------------------------------------
        RDES1/TDES1  | Control Bits | Byte Count Buffer 2 | Byte Count Buffer 1
    |
         --------------------------------------------------------------------
    */
    // DmaDescriptorLength     length word of DMA descriptor

    RxDisIntCompl
    = 0x80000000, /* (Disable Rx int on completion)             31 */
    RxDescEndOfRing = 0x00008000, /* (TER)End of descriptors ring 15 */
    RxDescChain
    = 0x00004000, /* (TCH)Second buffer address is chain address 14 */

    DescSize2Mask
    = 0x1FFF0000, /* (TBS2) Buffer 2 size                  28:16 */
    DescSize2Shift = 16,
    DescSize1Mask = 0x00001FFF, /* (TBS1) Buffer 1 size                  12:0 */
    DescSize1Shift = 0,

/*
This explains the RDES4 Extended Status bits layout
           --------------------------------------------------------------------
  RDES4   |                             Extended Status                        |
           --------------------------------------------------------------------
*/

#ifdef AVB_SUPPORT
    DescRxVlanPrioVal = 0x001C0000, /* Gives the VLAN Priority Value 20:18 */
    DescRxVlanPrioShVal
    = 18, /* Gives the shift value to get priority value in LS bits */

    DescRxAvTagPktRx
    = 0x00020000, /* Indicates AV tagged Packet is received              17 */
    DescRxAvPktRx = 0x00010000, /* Indicates AV Packet received 16 */
#endif

    DescRxPtpAvail = 0x00004000, /* PTP snapshot available 14 */
    DescRxPtpVer
    = 0x00002000, /* When set indicates IEEE1584 Version 2 (else Ver1)   13 */
    DescRxPtpFrameType
    = 0x00001000, /* PTP frame type Indicates PTP sent over ethernet     12 */
    DescRxPtpMessageType = 0x00000F00, /* Message Type 11:8 */
    DescRxPtpNo = 0x00000000, /* 0000 => No PTP message received */
    DescRxPtpSync = 0x00000100, /* 0001 => Sync (all clock types) received */
    DescRxPtpFollowUp
    = 0x00000200, /* 0010 => Follow_Up (all clock types) received */
    DescRxPtpDelayReq
    = 0x00000300, /* 0011 => Delay_Req (all clock types) received */
    DescRxPtpDelayResp
    = 0x00000400, /* 0100 => Delay_Resp (all clock types) received */
    DescRxPtpPdelayReq = 0x00000500, /* 0101 => Pdelay_Req (in P to P tras clk)
                                        or Announce in Ord and Bound clk     */
    DescRxPtpPdelayResp
    = 0x00000600, /* 0110 => Pdealy_Resp(in P to P trans clk) or Management in
                     Ord and Bound clk   */
    DescRxPtpPdelayRespFP
    = 0x00000700, /* 0111 => Pdealy_Resp_Follow_Up (in P to P trans clk) or
                     Signaling in Ord and Bound clk   */
    DescRxPtpIPV6 = 0x00000080, /* Received Packet is  in IPV6 Packet 7 */
    DescRxPtpIPV4 = 0x00000040, /* Received Packet is  in IPV4 Packet 6 */

    DescRxChkSumBypass = 0x00000020, /* When set indicates checksum offload
                                      engine          5 is bypassed */
    DescRxIpPayloadError
    = 0x00000010, /* When set indicates 16bit IP payload CS is in error  4 */
    DescRxIpHeaderError
    = 0x00000008, /* When set indicates 16bit IPV4 header CS is in       3
                   error or IP datagram version is not consistent
                   with Ethernet type value */
    DescRxIpPayloadType
    = 0x00000007, /* Indicate the type of payload encapsulated          2:0
                   in IPdatagram processed by COE (Rx) */
    DescRxIpPayloadUnknown
    = 0x00000000, /* Unknown or didnot process IP payload */
    DescRxIpPayloadUDP = 0x00000001, /* UDP */
    DescRxIpPayloadTCP = 0x00000002, /* TCP */
    DescRxIpPayloadICMP = 0x00000003, /* ICMP */

};

#else
/*

********** Default Descritpor structure  ****************************
DmaRxBaseAddr     = 0x000C,   CSR3 - Receive Descriptor list base address
DmaRxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format
in Little endian with a 32 bit Data bus is as shown below

Similarly
DmaTxBaseAddr     = 0x0010,  CSR4 - Transmit Descriptor list base address
DmaTxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format
in Little endian with a 32 bit Data bus is as shown below
          --------------------------------------------------------------------
    RDES0/TDES0  |OWN (31)| Status |
          --------------------------------------------------------------------
    RDES1/TDES1  | Control Bits | Byte Count Buffer 2 | Byte Count Buffer 1 |
          --------------------------------------------------------------------
    RDES2/TDES2  |  Buffer 1 Address |
          --------------------------------------------------------------------
    RDES3/TDES3  |  Buffer 2 Address / Next Descriptor Address |
          --------------------------------------------------------------------
*/
enum DmaDescriptorStatus /* status word of DMA descriptor */
{
    DescOwnByDma = 0x80000000, /* (OWN)Descriptor is owned by DMA engine 31
                                  RW */

    DescDAFilterFail
    = 0x40000000, /* (AFM)Rx - DA Filter Fail for the rx frame         30 */

    DescFrameLengthMask
    = 0x3FFF0000, /* (FL)Receive descriptor frame length               29:16 */
    DescFrameLengthShift = 16,

    DescError
    = 0x00008000, /* (ES)Error summary bit  - OR of the follo. bits:   15 */
    /*  DE || OE || IPC || LC || RWT || RE || CE */
    DescRxTruncated
    = 0x00004000, /* (DE)Rx - no more descriptors for receive frame    14 */
    DescSAFilterFail
    = 0x00002000, /* (SAF)Rx - SA Filter Fail for the received frame   13 */
    DescRxLengthError
    = 0x00001000, /* (LE)Rx - frm size not matching with len field       12 */
    DescRxDamaged
    = 0x00000800, /* (OE)Rx - frm was damaged due to buffer overflow   11 */
    DescRxVLANTag
    = 0x00000400, /* (VLAN)Rx - received frame is a VLAN frame         10 */
    DescRxFirst = 0x00000200, /* (FS)Rx - first descriptor of the frame 9 */
    DescRxLast = 0x00000100, /* (LS)Rx - last descriptor of the frame 8 */
    DescRxLongFrame
    = 0x00000080, /* (Giant Frame)Rx - frame is longer than 1518/1522   7 */
    DescRxCollision
    = 0x00000040, /* (LC)Rx - late collision occurred during reception  6 */
    DescRxFrameEther
    = 0x00000020, /* (FT)Rx - Frame type - Ethernet, otherwise 802.3    5 */
    DescRxWatchdog
    = 0x00000010, /* (RWT)Rx - watchdog timer expired during reception  4 */
    DescRxMiiError
    = 0x00000008, /* (RE)Rx - error reported by MII interface           3 */
    DescRxDribbling
    = 0x00000004, /* (DE)Rx - frame contains non int multiple of 8 bits 2 */
    DescRxCrc = 0x00000002, /* (CE)Rx - CRC error 1 */
    DescRxMacMatch
    = 0x00000001, /* (RX MAC Address) Rx mac address reg(1 to 15)match  0 */

    // Rx Descriptor Checksum Offload engine (type 2) encoding
    // DescRxPayChkError     = 0x00000001,   /* ()  Rx - Rx Payload Checksum
    // Error                 0 */ DescRxIpv4ChkError    = 0x00000080,   /* (IPC
    // CS ERROR)Rx - Ipv4 header checksum error      7 */

    DescRxChkBit0 = 0x00000001, /* ()  Rx - Rx Payload Checksum Error 0 */
    DescRxChkBit7
    = 0x00000080, /* (IPC CS ERROR)Rx - Ipv4 header checksum error      7 */
    DescRxChkBit5
    = 0x00000020, /* (FT)Rx - Frame type - Ethernet, otherwise 802.3    5 */

    DescTxIpv4ChkError = 0x00010000, /* (IHE) Tx Ip header error 16 */
    DescTxTimeout = 0x00004000, /* (JT)Tx - Transmit jabber timeout 14 */
    DescTxFrameFlushed
    = 0x00002000, /* (FF)Tx - DMA/MTL flushed the frame due to SW flush 13 */
    DescTxPayChkError = 0x00001000, /* (PCE) Tx Payload checksum Error 12 */
    DescTxLostCarrier
    = 0x00000800, /* (LC)Tx - carrier lost during tramsmission          11 */
    DescTxNoCarrier
    = 0x00000400, /* (NC)Tx - no carrier signal from the tranceiver     10 */
    DescTxLateCollision
    = 0x00000200, /* (LC)Tx - transmission aborted due to collision      9 */
    DescTxExcCollisions
    = 0x00000100, /* (EC)Tx - transmission aborted after 16 collisions   8 */
    DescTxVLANFrame = 0x00000080, /* (VF)Tx - VLAN-type frame 7 */

    DescTxCollMask = 0x00000078, /* (CC)Tx - Collision count 6:3 */
    DescTxCollShift = 3,

    DescTxExcDeferral = 0x00000004, /* (ED)Tx - excessive deferral 2 */
    DescTxUnderflow
    = 0x00000002, /* (UF)Tx - late data arrival from the memory           1 */
    DescTxDeferred = 0x00000001, /* (DB)Tx - frame transmision deferred 0 */

    /*
    This explains the RDES1/TDES1 bits layout
        --------------------------------------------------------------------
        RDES1/TDES1  | Control Bits | Byte Count Buffer 2 | Byte Count Buffer 1
    |
        --------------------------------------------------------------------
    */
    // DmaDescriptorLength     length word of DMA descriptor

    DescTxIntEnable = 0x80000000, /* (IC)Tx - interrupt on completion 31 */
    DescTxLast = 0x40000000, /* (LS)Tx - Last segment of the frame 30      */
    DescTxFirst = 0x20000000, /* (FS)Tx - First segment of the frame 29 */
    DescTxDisableCrc
    = 0x04000000, /* (DC)Tx - Add CRC disabled (first segment only)     26 */

    RxDisIntCompl = 0x80000000, /* (Disable Rx int on completion) 31      */
    RxDescEndOfRing = 0x02000000, /* (TER)End of descriptors ring */
    RxDescChain
    = 0x01000000, /* (TCH)Second buffer address is chain address        24 */

    DescTxDisablePadd = 0x00800000, /* (DP)disable padding, added by - reyaz
                                       23      */

    TxDescEndOfRing = 0x02000000, /* (TER)End of descriptors ring */
    TxDescChain
    = 0x01000000, /* (TCH)Second buffer address is chain address        24 */

    DescSize2Mask = 0x003FF800, /* (TBS2) Buffer 2 size 21:11   */
    DescSize2Shift = 11,
    DescSize1Mask = 0x000007FF, /* (TBS1) Buffer 1 size 10:0    */
    DescSize1Shift = 0,

    DescTxCisMask
    = 0x18000000, /* Tx checksum offloading control mask          28:27   */
    DescTxCisBypass = 0x00000000, /* Checksum bypass */
    DescTxCisIpv4HdrCs = 0x08000000, /* IPv4 header checksum */
    DescTxCisTcpOnlyCs = 0x10000000, /* TCP/UDP/ICMP checksum. Pseudo header
                                        checksum is assumed to be present */
    DescTxCisTcpPseudoCs = 0x18000000, /* TCP/UDP/ICMP checksum fully in
                                          hardware including pseudo header */
};
#endif

// Rx Descriptor COE type2 encoding
enum RxDescCOEEncode {
    RxLenLT600 = 0, /* Bit(5:7:0)=>0 IEEE 802.3 type frame Length field is
                       Lessthan 0x0600 */
    RxIpHdrPayLoadChkBypass = 1, /* Bit(5:7:0)=>1 Payload & Ip header checksum
                                    bypassed (unsuppported payload) */
    RxIpHdrPayLoadRes = 2, /* Bit(5:7:0)=>2 Reserved */
    RxChkBypass
    = 3, /* Bit(5:7:0)=>3 Neither IPv4 nor IPV6. So checksum bypassed */
    RxNoChkError = 4, /* Bit(5:7:0)=>4 No IPv4/IPv6 Checksum error detected */
    RxPayLoadChkError = 5, /* Bit(5:7:0)=>5 Payload checksum error detected for
                              Ipv4/Ipv6 frames */
    RxIpHdrChkError
    = 6, /* Bit(5:7:0)=>6 Ip header checksum error detected for Ipv4 frames */
    RxIpHdrPayLoadChkError = 7, /* Bit(5:7:0)=>7 Payload & Ip header checksum
                                   error detected for Ipv4/Ipv6 frames */
};

/**********************************************************
 * DMA engine interrupt handling functions
 **********************************************************/

enum synopGMACDmaIntEnum /* Intrerrupt types */
{
    synopGMACDmaRxNormal = 0x01, /* normal receiver interrupt */
    synopGMACDmaRxAbnormal = 0x02, /* abnormal receiver interrupt */
    synopGMACDmaRxStopped = 0x04, /* receiver stopped */
    synopGMACDmaTxNormal = 0x08, /* normal transmitter interrupt */
    synopGMACDmaTxAbnormal = 0x10, /* abnormal transmitter interrupt */
    synopGMACDmaTxStopped = 0x20, /* transmitter stopped */
    synopGMACDmaError = 0x80, /* Dma engine error */

#ifdef AVB_SUPPORT
    synopGMADmaSlotCounter
    = 0x40, /* Dma SlotCounter interrupt mask for Channel1 and Channel2*/
#endif
};

/**********************************************************
 * Initial register values
 **********************************************************/
enum InitialRegisters {
    /* Full-duplex mode with perfect filter on */
    GmacConfigInitFdx1000 = GmacWatchdogEnable | GmacJabberEnable
        | GmacFrameBurstEnable | GmacJumboFrameDisable | GmacSelectGmii
        | GmacEnableRxOwn | GmacLoopbackOff | GmacFullDuplex | GmacRetryEnable
        | GmacPadCrcStripDisable | GmacBackoffLimit0 | GmacDeferralCheckDisable
        | GmacTxEnable | GmacRxEnable,

    /* Full-duplex mode with perfect filter on */
    GmacConfigInitFdx110 = GmacWatchdogEnable | GmacJabberEnable
        | GmacFrameBurstEnable | GmacJumboFrameDisable | GmacSelectMii
        | GmacEnableRxOwn | GmacLoopbackOff | GmacFullDuplex | GmacRetryEnable
        | GmacPadCrcStripDisable | GmacBackoffLimit0 | GmacDeferralCheckDisable
        | GmacTxEnable | GmacRxEnable,

    /* Full-duplex mode */
    // CHANGED: Pass control config, dest addr filter normal, added source
    // address filter, multicast & unicast Hash filter.
    /*                        = GmacFilterOff         | GmacPassControlOff |
       GmacBroadcastEnable */
    GmacFrameFilterInitFdx = GmacFilterOn | GmacPassControl0
        | GmacBroadcastEnable | GmacSrcAddrFilterDisable | GmacMulticastFilterOn
        | GmacDestAddrFilterNor | GmacMcastHashFilterOff
        | GmacPromiscuousModeOff | GmacUcastHashFilterOff,

    /* Full-duplex mode */
    GmacFlowControlInitFdx = GmacUnicastPauseFrameOff | GmacRxFlowControlEnable
        | GmacTxFlowControlEnable,

    /* Full-duplex mode */
    GmacGmiiAddrInitFdx = GmiiCsrClk2,

    /* Half-duplex mode with perfect filter on */
    // CHANGED: Removed Endian configuration, added single bit config for
    // PAD/CRC strip,
    /*| GmacSelectMii      | GmacLittleEndian         | GmacDisableRxOwn      |
       GmacLoopbackOff*/
    GmacConfigInitHdx1000 = GmacWatchdogEnable | GmacJabberEnable
        | GmacFrameBurstEnable | GmacJumboFrameDisable | GmacSelectGmii
        | GmacDisableRxOwn | GmacLoopbackOff | GmacHalfDuplex | GmacRetryEnable
        | GmacPadCrcStripDisable | GmacBackoffLimit0 | GmacDeferralCheckDisable
        | GmacTxEnable | GmacRxEnable,

    /* Half-duplex mode with perfect filter on */
    GmacConfigInitHdx110 = GmacWatchdogEnable | GmacJabberEnable
        | GmacFrameBurstEnable | GmacJumboFrameDisable | GmacSelectMii
        | GmacDisableRxOwn | GmacLoopbackOff | GmacHalfDuplex | GmacRetryEnable
        | GmacPadCrcStripDisable | GmacBackoffLimit0 | GmacDeferralCheckDisable
        | GmacTxEnable | GmacRxEnable,

    /* Half-duplex mode */
    GmacFrameFilterInitHdx = GmacFilterOn | GmacPassControl0
        | GmacBroadcastEnable | GmacSrcAddrFilterDisable | GmacMulticastFilterOn
        | GmacDestAddrFilterNor | GmacMcastHashFilterOff
        | GmacUcastHashFilterOff | GmacPromiscuousModeOff,

    /* Half-duplex mode */
    GmacFlowControlInitHdx = GmacUnicastPauseFrameOff | GmacRxFlowControlDisable
        | GmacTxFlowControlDisable,

    /* Half-duplex mode */
    GmacGmiiAddrInitHdx = GmiiCsrClk2,

    /**********************************************
     *DMA configurations
     **********************************************/

    DmaBusModeInit
    = DmaFixedBurstEnable | DmaBurstLength8 | DmaDescriptorSkip2 | DmaResetOff,
    //   DmaBusModeInit         = DmaFixedBurstEnable |   DmaBurstLength8   |
    //   DmaDescriptorSkip4       | DmaResetOff,

    /* 1000 Mb/s mode */
    DmaControlInit1000 = DmaStoreAndForward, //       | DmaTxSecondFrame ,

    /* 100 Mb/s mode */
    DmaControlInit100 = DmaStoreAndForward,

    /* 10 Mb/s mode */
    DmaControlInit10 = DmaStoreAndForward,

    /* Interrupt groups */
    DmaIntErrorMask = DmaIntBusError, /* Error */
    DmaIntRxAbnMask = DmaIntRxNoBuffer, /* receiver abnormal interrupt */
    DmaIntRxNormMask = DmaIntRxCompleted, /* receiver normal interrupt   */
    DmaIntRxStoppedMask = DmaIntRxStopped, /* receiver stopped */
    DmaIntTxAbnMask = DmaIntTxUnderflow, /* transmitter abnormal interrupt */
    DmaIntTxNormMask = DmaIntTxCompleted, /* transmitter normal interrupt */
    DmaIntTxStoppedMask = DmaIntTxStopped, /* transmitter stopped */

    DmaIntEnable = DmaIeNormal | DmaIeAbnormal | DmaIntErrorMask
        | DmaIntRxAbnMask | DmaIntRxNormMask | DmaIntRxStoppedMask
        | DmaIntTxAbnMask | DmaIntTxNormMask | DmaIntTxStoppedMask
        | DmaIeTxNoBuffer,
    DmaIntDisable = 0,
};

/**********************************************************
 * Mac Management Counters (MMC)
 **********************************************************/

enum MMC_ENABLE {
    GmacMmcCntrl = 0x0100, /* mmc control for operating mode of MMC */
    GmacMmcIntrRx = 0x0104, /* maintains interrupts generated by rx counters */
    GmacMmcIntrTx = 0x0108, /* maintains interrupts generated by tx counters */
    GmacMmcIntrMaskRx
    = 0x010C, /* mask for interrupts generated from rx counters */
    GmacMmcIntrMaskTx
    = 0x0110, /* mask for interrupts generated from tx counters */
};
enum MMC_TX {
    GmacMmcTxOctetCountGb = 0x0114, /*Bytes Tx excl. of preamble and retried
                                       bytes     (Good or Bad)            */
    GmacMmcTxFrameCountGb
    = 0x0118, /*Frames Tx excl. of retried frames            (Good or Bad) */
    GmacMmcTxBcFramesG
    = 0x011C, /*Broadcast Frames Tx                  (Good)               */
    GmacMmcTxMcFramesG
    = 0x0120, /*Multicast Frames Tx                  (Good)               */

    GmacMmcTx64OctetsGb = 0x0124, /*Tx with len 64 bytes excl. of pre and
                                     retried    (Good or Bad)            */
    GmacMmcTx65To127OctetsGb = 0x0128, /*Tx with len >64 bytes <=127 excl. of
                                          pre and retried    (Good or Bad) */
    GmacMmcTx128To255OctetsGb = 0x012C, /*Tx with len >128 bytes <=255 excl. of
                                           pre and retried   (Good or Bad) */
    GmacMmcTx256To511OctetsGb = 0x0130, /*Tx with len >256 bytes <=511 excl. of
                                           pre and retried   (Good or Bad) */
    GmacMmcTx512To1023OctetsGb
    = 0x0134, /*Tx with len >512 bytes <=1023 excl. of pre and retried  (Good or
                 Bad)         */
    GmacMmcTx1024ToMaxOctetsGb
    = 0x0138, /*Tx with len >1024 bytes <=MaxSize excl. of pre and retried (Good
                 or Bad)      */

    GmacMmcTxUcFramesGb
    = 0x013C, /*Unicast Frames Tx                      (Good or Bad)          */
    GmacMmcTxMcFramesGb
    = 0x0140, /*Multicast Frames Tx                  (Good and Bad)           */
    GmacMmcTxBcFramesGb
    = 0x0144, /*Broadcast Frames Tx                  (Good and Bad)           */
    GmacMmcTxUnderFlowError
    = 0x0148, /*Frames aborted due to Underflow error                         */
    GmacMmcTxSingleColG = 0x014C, /*Successfully Tx Frames after singel
                                     collision in Half duplex mode         */
    GmacMmcTxMultiColG = 0x0150, /*Successfully Tx Frames after more than singel
                                    collision in Half duplex mode       */
    GmacMmcTxDeferred
    = 0x0154, /*Successfully Tx Frames after a deferral in Half duplex mode */
    GmacMmcTxLateCol = 0x0158, /*Frames aborted due to late collision error */
    GmacMmcTxExessCol
    = 0x015C, /*Frames aborted due to excessive (16) collision errors */
    GmacMmcTxCarrierError = 0x0160, /*Frames aborted due to carrier sense error
                                       (No carrier or Loss of carrier)     */
    GmacMmcTxOctetCountG
    = 0x0164, /*Bytes Tx excl. of preamble and retried bytes     (Good) */
    GmacMmcTxFrameCountG
    = 0x0168, /*Frames Tx                            (Good)               */
    GmacMmcTxExessDef
    = 0x016C, /*Frames aborted due to excessive deferral errors (deferred for
                 more than 2 max-sized frame times)*/

    GmacMmcTxPauseFrames = 0x0170, /*Number of good pause frames Tx. */
    GmacMmcTxVlanFramesG
    = 0x0174, /*Number of good Vlan frames Tx excl. retried frames */
};
enum MMC_RX {
    GmacMmcRxFrameCountGb
    = 0x0180, /*Frames Rx                            (Good or Bad)            */
    GmacMmcRxOctetCountGb = 0x0184, /*Bytes Rx excl. of preamble and retried
                                       bytes     (Good or Bad)            */
    GmacMmcRxOctetCountG
    = 0x0188, /*Bytes Rx excl. of preamble and retried bytes     (Good) */
    GmacMmcRxBcFramesG
    = 0x018C, /*Broadcast Frames Rx                  (Good)               */
    GmacMmcRxMcFramesG
    = 0x0190, /*Multicast Frames Rx                  (Good)               */

    GmacMmcRxCrcError = 0x0194, /*Number of frames received with CRC error */
    GmacMmcRxAlignError = 0x0198, /*Number of frames received with alignment
                                     (dribble) error. Only in 10/100mode      */
    GmacMmcRxRuntError = 0x019C, /*Number of frames received with runt (<64
                                    bytes and CRC error) error           */
    GmacMmcRxJabberError = 0x01A0, /*Number of frames rx with jabber (>1518/1522
                                      or >9018/9022 and CRC)            */
    GmacMmcRxUnderSizeG
    = 0x01A4, /*Number of frames received with <64 bytes without any error */
    GmacMmcRxOverSizeG = 0x01A8, /*Number of frames received with >1518/1522
                                    bytes without any error         */

    GmacMmcRx64OctetsGb = 0x01AC, /*Rx with len 64 bytes excl. of pre and
                                     retried    (Good or Bad)            */
    GmacMmcRx65To127OctetsGb = 0x01B0, /*Rx with len >64 bytes <=127 excl. of
                                          pre and retried    (Good or Bad) */
    GmacMmcRx128To255OctetsGb = 0x01B4, /*Rx with len >128 bytes <=255 excl. of
                                           pre and retried   (Good or Bad) */
    GmacMmcRx256To511OctetsGb = 0x01B8, /*Rx with len >256 bytes <=511 excl. of
                                           pre and retried   (Good or Bad) */
    GmacMmcRx512To1023OctetsGb
    = 0x01BC, /*Rx with len >512 bytes <=1023 excl. of pre and retried  (Good or
                 Bad)         */
    GmacMmcRx1024ToMaxOctetsGb
    = 0x01C0, /*Rx with len >1024 bytes <=MaxSize excl. of pre and retried (Good
                 or Bad)      */

    GmacMmcRxUcFramesG
    = 0x01C4, /*Unicast Frames Rx                      (Good)             */
    GmacMmcRxLengthError = 0x01C8, /*Number of frames received with Length type
                                      field != frame size            */
    GmacMmcRxOutOfRangeType = 0x01CC, /*Number of frames received with length
                                         field != valid frame size           */

    GmacMmcRxPauseFrames = 0x01D0, /*Number of good pause frames Rx. */
    GmacMmcRxFifoOverFlow
    = 0x01D4, /*Number of missed rx frames due to FIFO overflow */
    GmacMmcRxVlanFramesGb = 0x01D8, /*Number of good Vlan frames Rx */

    GmacMmcRxWatchdobError = 0x01DC, /*Number of frames rx with error due to
                                        watchdog timeout error              */
};

enum MMC_IP_RELATED {
    GmacMmcRxIpcIntrMask = 0x0200, /*Maintains the mask for interrupt generated
                                      from rx IPC statistic counters         */
    GmacMmcRxIpcIntr = 0x0208, /*Maintains the interrupt that rx IPC statistic
                                  counters generate           */

    GmacMmcRxIpV4FramesG = 0x0210, /*Good IPV4 datagrams received */
    GmacMmcRxIpV4HdrErrFrames
    = 0x0214, /*Number of IPV4 datagrams received with header errors */
    GmacMmcRxIpV4NoPayFrames = 0x0218, /*Number of IPV4 datagrams received which
                                          didnot have TCP/UDP/ICMP payload */
    GmacMmcRxIpV4FragFrames
    = 0x021C, /*Number of IPV4 datagrams received with fragmentation */
    GmacMmcRxIpV4UdpChkDsblFrames
    = 0x0220, /*Number of IPV4 datagrams received that had a UDP payload
                 checksum disabled        */

    GmacMmcRxIpV6FramesG = 0x0224, /*Good IPV6 datagrams received */
    GmacMmcRxIpV6HdrErrFrames
    = 0x0228, /*Number of IPV6 datagrams received with header errors */
    GmacMmcRxIpV6NoPayFrames = 0x022C, /*Number of IPV6 datagrams received which
                                          didnot have TCP/UDP/ICMP payload */

    GmacMmcRxUdpFramesG
    = 0x0230, /*Number of good IP datagrams with good UDP payload */
    GmacMmcRxUdpErrorFrames = 0x0234, /*Number of good IP datagrams with UDP
                                         payload having checksum error */

    GmacMmcRxTcpFramesG
    = 0x0238, /*Number of good IP datagrams with good TDP payload */
    GmacMmcRxTcpErrorFrames = 0x023C, /*Number of good IP datagrams with TCP
                                         payload having checksum error */

    GmacMmcRxIcmpFramesG
    = 0x0240, /*Number of good IP datagrams with good Icmp payload */
    GmacMmcRxIcmpErrorFrames = 0x0244, /*Number of good IP datagrams with Icmp
                                          payload having checksum error */

    GmacMmcRxIpV4OctetsG = 0x0250, /*Good IPV4 datagrams received excl. Ethernet
                                      hdr,FCS,Pad,Ip Pad bytes          */
    GmacMmcRxIpV4HdrErrorOctets
    = 0x0254, /*Number of bytes in IPV4 datagram with header errors */
    GmacMmcRxIpV4NoPayOctets = 0x0258, /*Number of bytes in IPV4 datagram with
                                          no TCP/UDP/ICMP payload             */
    GmacMmcRxIpV4FragOctets
    = 0x025C, /*Number of bytes received in fragmented IPV4 datagrams */
    GmacMmcRxIpV4UdpChkDsblOctets
    = 0x0260, /*Number of bytes received in UDP segment that had UDP checksum
                 disabled        */

    GmacMmcRxIpV6OctetsG = 0x0264, /*Good IPV6 datagrams received excl. Ethernet
                                      hdr,FCS,Pad,Ip Pad bytes          */
    GmacMmcRxIpV6HdrErrorOctets
    = 0x0268, /*Number of bytes in IPV6 datagram with header errors */
    GmacMmcRxIpV6NoPayOctets = 0x026C, /*Number of bytes in IPV6 datagram with
                                          no TCP/UDP/ICMP payload             */

    GmacMmcRxUdpOctetsG
    = 0x0270, /*Number of bytes in IP datagrams with good UDP payload */
    GmacMmcRxUdpErrorOctets = 0x0274, /*Number of bytes in IP datagrams with UDP
                                         payload having checksum error        */

    GmacMmcRxTcpOctetsG
    = 0x0278, /*Number of bytes in IP datagrams with good TDP payload */
    GmacMmcRxTcpErrorOctets = 0x027C, /*Number of bytes in IP datagrams with TCP
                                         payload having checksum error        */

    GmacMmcRxIcmpOctetsG
    = 0x0280, /*Number of bytes in IP datagrams with good Icmp payload */
    GmacMmcRxIcmpErrorOctets
    = 0x0284, /*Number of bytes in IP datagrams with Icmp payload having
                 checksum error       */
};

enum MMC_CNTRL_REG_BIT_DESCRIPTIONS {
    GmacMmcCounterFreeze
    = 0x00000008, /* when set MMC counters freeze to current value */
    GmacMmcCounterResetOnRead
    = 0x00000004, /* when set MMC counters will be reset to 0 after read */
    GmacMmcCounterStopRollover
    = 0x00000002, /* when set counters will not rollover after max value */
    GmacMmcCounterReset
    = 0x00000001, /* when set all counters wil be reset (automatically cleared
                     after 1 clk)   */

};

enum MMC_RX_INTR_MASK_AND_STATUS_BIT_DESCRIPTIONS {
    GmacMmcRxWDInt
    = 0x00800000, /* set when rxwatchdog error reaches half of max value */
    GmacMmcRxVlanInt = 0x00400000, /* set when GmacMmcRxVlanFramesGb counter
                                      reaches half of max value     */
    GmacMmcRxFifoOverFlowInt
    = 0x00200000, /* set when GmacMmcRxFifoOverFlow counter reaches half of max
                     value     */
    GmacMmcRxPauseFrameInt
    = 0x00100000, /* set when GmacMmcRxPauseFrames counter reaches half of max
                     value      */
    GmacMmcRxOutOfRangeInt
    = 0x00080000, /* set when GmacMmcRxOutOfRangeType counter reaches half of
                     max value       */
    GmacMmcRxLengthErrorInt
    = 0x00040000, /* set when GmacMmcRxLengthError counter reaches half of max
                     value      */
    GmacMmcRxUcFramesInt = 0x00020000, /* set when GmacMmcRxUcFramesG counter
                                          reaches half of max value        */
    GmacMmcRx1024OctInt = 0x00010000, /* set when GmacMmcRx1024ToMaxOctetsGb
                                         counter reaches half of max value    */
    GmacMmcRx512OctInt = 0x00008000, /* set when GmacMmcRx512To1023OctetsGb
                                        counter reaches half of max value    */
    GmacMmcRx256OctInt = 0x00004000, /* set when GmacMmcRx256To511OctetsGb
                                        counter reaches half of max value     */
    GmacMmcRx128OctInt = 0x00002000, /* set when GmacMmcRx128To255OctetsGb
                                        counter reaches half of max value     */
    GmacMmcRx65OctInt = 0x00001000, /* set when GmacMmcRx65To127OctetsG counter
                                       reaches half of max value       */
    GmacMmcRx64OctInt = 0x00000800, /* set when GmacMmcRx64OctetsGb counter
                                       reaches half of max value       */
    GmacMmcRxOverSizeInt = 0x00000400, /* set when GmacMmcRxOverSizeG counter
                                          reaches half of max value        */
    GmacMmcRxUnderSizeInt = 0x00000200, /* set when GmacMmcRxUnderSizeG counter
                                           reaches half of max value       */
    GmacMmcRxJabberErrorInt
    = 0x00000100, /* set when GmacMmcRxJabberError counter reaches half of max
                     value      */
    GmacMmcRxRuntErrorInt = 0x00000080, /* set when GmacMmcRxRuntError counter
                                           reaches half of max value        */
    GmacMmcRxAlignErrorInt = 0x00000040, /* set when GmacMmcRxAlignError counter
                                            reaches half of max value       */
    GmacMmcRxCrcErrorInt = 0x00000020, /* set when GmacMmcRxCrcError counter
                                          reaches half of max value         */
    GmacMmcRxMcFramesInt = 0x00000010, /* set when GmacMmcRxMcFramesG counter
                                          reaches half of max value        */
    GmacMmcRxBcFramesInt = 0x00000008, /* set when GmacMmcRxBcFramesG counter
                                          reaches half of max value        */
    GmacMmcRxOctetGInt = 0x00000004, /* set when GmacMmcRxOctetCountG counter
                                        reaches half of max value      */
    GmacMmcRxOctetGbInt = 0x00000002, /* set when GmacMmcRxOctetCountGb counter
                                         reaches half of max value     */
    GmacMmcRxFrameInt = 0x00000001, /* set when GmacMmcRxFrameCountGb counter
                                       reaches half of max value     */
};

enum MMC_TX_INTR_MASK_AND_STATUS_BIT_DESCRIPTIONS {

    GmacMmcTxVlanInt = 0x01000000, /* set when GmacMmcTxVlanFramesG counter
                                      reaches half of max value      */
    GmacMmcTxPauseFrameInt
    = 0x00800000, /* set when GmacMmcTxPauseFrames counter reaches half of max
                     value      */
    GmacMmcTxExessDefInt = 0x00400000, /* set when GmacMmcTxExessDef counter
                                          reaches half of max value         */
    GmacMmcTxFrameInt = 0x00200000, /* set when GmacMmcTxFrameCount counter
                                       reaches half of max value       */
    GmacMmcTxOctetInt = 0x00100000, /* set when GmacMmcTxOctetCountG counter
                                       reaches half of max value      */
    GmacMmcTxCarrierErrorInt
    = 0x00080000, /* set when GmacMmcTxCarrierError counter reaches half of max
                     value     */
    GmacMmcTxExessColInt = 0x00040000, /* set when GmacMmcTxExessCol counter
                                          reaches half of max value         */
    GmacMmcTxLateColInt = 0x00020000, /* set when GmacMmcTxLateCol counter
                                         reaches half of max value          */
    GmacMmcTxDeferredInt = 0x00010000, /* set when GmacMmcTxDeferred counter
                                          reaches half of max value         */
    GmacMmcTxMultiColInt = 0x00008000, /* set when GmacMmcTxMultiColG counter
                                          reaches half of max value        */
    GmacMmcTxSingleCol = 0x00004000, /* set when GmacMmcTxSingleColG counter
                                        reaches half of max value       */
    GmacMmcTxUnderFlowErrorInt
    = 0x00002000, /* set when GmacMmcTxUnderFlowError counter reaches half of
                     max value       */
    GmacMmcTxBcFramesGbInt = 0x00001000, /* set when GmacMmcTxBcFramesGb counter
                                            reaches half of max value       */
    GmacMmcTxMcFramesGbInt = 0x00000800, /* set when GmacMmcTxMcFramesGb counter
                                            reaches half of max value       */
    GmacMmcTxUcFramesInt = 0x00000400, /* set when GmacMmcTxUcFramesGb counter
                                          reaches half of max value       */
    GmacMmcTx1024OctInt = 0x00000200, /* set when GmacMmcTx1024ToMaxOctetsGb
                                         counter reaches half of max value    */
    GmacMmcTx512OctInt = 0x00000100, /* set when GmacMmcTx512To1023OctetsGb
                                        counter reaches half of max value    */
    GmacMmcTx256OctInt = 0x00000080, /* set when GmacMmcTx256To511OctetsGb
                                        counter reaches half of max value     */
    GmacMmcTx128OctInt = 0x00000040, /* set when GmacMmcTx128To255OctetsGb
                                        counter reaches half of max value     */
    GmacMmcTx65OctInt = 0x00000020, /* set when GmacMmcTx65To127OctetsGb counter
                                       reaches half of max value      */
    GmacMmcTx64OctInt = 0x00000010, /* set when GmacMmcTx64OctetsGb counter
                                       reaches half of max value       */
    GmacMmcTxMcFramesInt = 0x00000008, /* set when GmacMmcTxMcFramesG counter
                                          reaches half of max value        */
    GmacMmcTxBcFramesInt = 0x00000004, /* set when GmacMmcTxBcFramesG counter
                                          reaches half of max value        */
    GmacMmcTxFrameGbInt = 0x00000002, /* set when GmacMmcTxFrameCountGb counter
                                         reaches half of max value     */
    GmacMmcTxOctetGbInt = 0x00000001, /* set when GmacMmcTxOctetCountGb counter
                                         reaches half of max value     */

};

/**********************************************************
 * Power Management (PMT) Block
 **********************************************************/

/**
 * PMT supports the reception of network (remote) wake-up frames and Magic
 * packet frames. It generates interrupts for wake-up frames and Magic packets
 * received by GMAC. PMT sits in Rx path and is enabled with remote wake-up
 * frame enable and Magic packet enable. These enable are in PMT control and
 * Status register and are programmed by apllication.
 *
 * When power down mode is enabled in PMT, all rx frames are dropped by the
 * core. Core comes out of power down mode only when either Magic packe tor a
 * Remote wake-up frame is received and the corresponding detection is enabled.
 *
 * Driver need not be modified to support this feature. Only Api to put the
 * device in to power down mode is sufficient
 */

#define WAKEUP_REG_LENGTH                                                      \
    8 /*This is the reg length for wake up register configuration*/

enum GmacPmtCtrlStatusBitDefinition {
    GmacPmtFrmFilterPtrReset
    = 0x80000000, /* when set remote wake-up frame filter register pointer to
                     3'b000 */
    GmacPmtGlobalUnicast = 0x00000200, /* When set enables any unicast packet to
                                          be a wake-up frame       */
    GmacPmtWakeupFrameReceived = 0x00000040, /* Wake up frame received */
    GmacPmtMagicPktReceived = 0x00000020, /* Magic Packet received */
    GmacPmtWakeupFrameEnable = 0x00000004, /* Wake-up frame enable */
    GmacPmtMagicPktEnable = 0x00000002, /* Magic packet enable */
    GmacPmtPowerDown = 0x00000001, /* Power Down                              */
};

/**********************************************************
 * IEEE 1588-2008 Precision Time Protocol (PTP) Support
 **********************************************************/
enum PTPMessageType {
    SYNC = 0x0,
    Delay_Req = 0x1,
    Pdelay_Req = 0x2,
    Pdelay_Resp = 0x3,
    Follow_up = 0x8,
    Delay_Resp = 0x9,
    Pdelay_Resp_Follow_Up = 0xA,
    Announce = 0xB,
    Signaling = 0xC,
    Management = 0xD,
};

typedef struct TimeStampStruct {
    uint32_t TSversion; /* PTP Version 1 or PTP version2 */
    uint32_t TSmessagetype; /* Message type associated with this time stamp */

    uint16_t TShighest16; /* Highest 16 bit time stamp value, Valid onley when
                        ADV_TIME_HIGH_WORD configured in corekit       */
    uint32_t TSupper32; /* Most significant 32 bit time stamp value */
    uint32_t TSlower32; /* Least Significat 32 bit time stamp value */

} TimeStamp;

/**
 * IEEE 1588-2008 is the optional module to support Ethernet frame time
 * stamping. Sixty four (+16) bit time stamps are given in each frames transmit
 * and receive status. The driver assumes the following
 *  1. "IEEE 1588 Time Stamping" "TIME_STAMPING"is ENABLED in corekit
 *  2. "IEEE 1588 External Time Stamp Input Enable" "EXT_TIME_STAMPING" is
 * DISABLED in corekit
 *  3. "IEEE 1588 Advanced Time Stamp support" "ADV_TIME_STAMPING" is ENABLED in
 * corekit
 *  4. "IEEE 1588 Higher Word Register Enable" "ADV_TIME_HIGH_WORD" is ENABLED
 * in corekit
 */

/* GmacTSControl  = 0x0700,   Controls the Timestamp update logic  : only when
 * IEEE 1588 time stamping is enabled in corekit         */
enum GmacTSControlReg {
    GmacTSENMACADDR = 0x00040000, /* Enable Mac Addr for PTP filtering     18
                                     RW         0     */

    GmacTSCLKTYPE = 0x00030000, /* Select the type of clock node         17:16
                                   RW         00    */
    /*
        TSCLKTYPE        TSMSTRENA      TSEVNTENA         Messages for wihich TS
       snapshot is taken 00/01                X             0              SYNC,
       FOLLOW_UP, DELAY_REQ, DELAY_RESP 00/01                1             0
       DELAY_REQ 00/01                0             1              SYNC 10 NA 0
       SYNC, FOLLOW_UP, DELAY_REQ, DELAY_RESP 10                  NA 1 SYNC,
       FOLLOW_UP 11                  NA            0              SYNC,
       FOLLOW_UP, DELAY_REQ, DELAY_RESP, PDELAY_REQ, PDELAY_RESP 11 NA 1 SYNC,
       PDELAY_REQ, PDELAY_RESP
    */
    GmacTSOrdClk = 0x00000000, /* 00=> Ordinary clock*/
    GmacTSBouClk = 0x00010000, /* 01=> Boundary clock*/
    GmacTSEtoEClk = 0x00020000, /* 10=> End-to-End transparent clock*/
    GmacTSPtoPClk = 0x00030000, /* 11=> P-to-P transparent clock*/

    GmacTSMSTRENA = 0x00008000, /* Ena TS Snapshot for Master Messages   15 RW
                                   0     */
    GmacTSEVNTENA = 0x00004000, /* Ena TS Snapshot for Event Messages    14 RW
                                   0     */
    GmacTSIPV4ENA = 0x00002000, /* Ena TS snapshot for IPv4              13 RW
                                   1     */
    GmacTSIPV6ENA = 0x00001000, /* Ena TS snapshot for IPv6              12 RW
                                   0     */
    GmacTSIPENA = 0x00000800, /* Ena TS snapshot for PTP over E'net    11 RW
                                 0     */
    GmacTSVER2ENA = 0x00000400, /* Ena PTP snooping for version 2        10 RW
                                   0     */

    GmacTSCTRLSSR = 0x00000200, /* Digital or Binary Rollover           9 RW
                                   0     */

    GmacTSENALL = 0x00000100, /* Enable TS fro all frames (Ver2 only) 8 RW
                                 0     */

    GmacTSADDREG = 0x00000020, /* Addend Register Update           5 RW_SC
                                  0     */
    GmacTSUPDT = 0x00000008, /* Time Stamp Update            3             RW_SC
                                0     */
    GmacTSINT = 0x00000004, /* Time Atamp Initialize            2 RW_SC      0
                             */

    GmacTSTRIG = 0x00000010, /* Time stamp interrupt Trigger Enable  4 RW_SC
                                0     */

    GmacTSCFUPDT = 0x00000002, /* Time Stamp Fine/Coarse           1 RW 0
                                */
    GmacTSCUPDTCoarse = 0x00000000, /* 0=> Time Stamp update method is coarse */
    GmacTSCUPDTFine = 0x00000002, /* 1=> Time Stamp update method is fine */

    GmacTSENA = 0x00000001, /* Time Stamp Enable                    0 RW 0
                             */
};

/*  GmacTSSubSecIncr          = 0x0704,   8 bit value by which sub second
 * register is incremented     : only when IEEE 1588 time stamping without
 * external timestamp input */
enum GmacTSSubSecIncrReg {
    GmacSSINCMsk = 0x000000FF, /* Only Lower 8 bits are valid bits     7:0 RW
                                  00    */
};

/*  GmacTSLow         = 0x070C,   Indicates whether the timestamp low count is
 * positive or negative; for Adv timestamp it is always zero */
enum GmacTSSign {
    GmacTSSign = 0x80000000, /* PSNT                                  31 RW
                                0    */
    GmacTSPositive = 0x00000000,
    GmacTSNegative = 0x80000000,
};

/*GmacTargetTimeLow       = 0x0718,   32 bit nano seconds(MS) to be compared
 * with system time     : only when IEEE 1588 time stamping without external
 * timestamp input */
enum GmacTSLowReg {
    GmacTSDecThr = 0x3B9AC9FF, /*when TSCTRLSSR is set the max value for
                                  GmacTargetTimeLowReg and GmacTimeStampLow
                                  register is 0x3B9AC9FF at 1ns precision */
};

/* GmacTSHighWord          = 0x0724,   Time Stamp Higher Word Register (Version
 * 2 only); only lower 16 bits are valid */
enum GmacTSHighWordReg {
    GmacTSHighWordMask = 0x0000FFFF, /* Time Stamp Higher work register has only
                                        lower 16 bits valid           */
};
/*GmacTSStatus            = 0x0728,   Time Stamp Status Register */
enum GmacTSStatusReg {
    GmacTSTargTimeReached = 0x00000002, /* Time Stamp Target Time Reached 1
                                           RO          0    */
    GmacTSSecondsOverflow = 0x00000001, /* Time Stamp Seconds Overflow 0 RO
                                           0    */
};

/* GmacAvMacCtrl            = 0x0738,   AV mac control Register */
#ifdef AVB_SUPPORT
enum GmacAvMacCtrlReg {
    GmacAvCtrlCh = 0x03000000, /* Channel on which AV control packets to be
                                  received                   25:24  RW   0    */
    GmacPtpCh = 0x00180000, /* Channel on which PTP packets to be received
                               20:19  RW   0    */
    GmacAvPrio
    = 0x00070000, /* Priority tag value for AV Packets 18:16  RW   4    */
    GmacAvTypeMask = 0x0000FFFF, /* Ethernet Type value to be used for comparing
                                    and detecting AV packet 15:0   RW   0    */
};
#endif

typedef void (*ak_gmac_sys_pr_info_func_t)(const char *fmt, ...);
typedef void (*ak_gmac_sys_pr_err_func_t)(const char *fmt, ...);
typedef void (*ak_gmac_sys_udelay_func_t)(unsigned int usecs);
typedef int (*ak_gmac_get_mdio_clock_func_t)(void);

typedef struct ak_gmac_if {
    uintptr_t MacBase; /* base address of MAC registers           */
    uintptr_t DmaBase; /* base address of DMA registers           */
    uint32_t  PhyBase; /* PHY device address on MII interface     */
    ak_gmac_get_mdio_clock_func_t get_mdio_clk;
    ak_gmac_sys_pr_info_func_t    sys_pr_info;
    ak_gmac_sys_pr_err_func_t     sys_pr_err;
    ak_gmac_sys_udelay_func_t     sys_udelay;
} ak_gmac_if_t;

void ak_gmac_if_init(ak_gmac_if_t *gmac_if);

#define AK_GMAC_IF_VERSION(X, Y, Z)    ((uint32_t)((X)<<16 | (Y)<<8 | (Z)<<0))

/*
 * get GMAC library version
 */
uint32_t ak_gmac_if_version(void);

/*
 * @BRIEF        The Low level function to read register contents from Hardware.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *regbase:pointer to the base of register map
 * @PARAM[in]    regoffset:offset from the base
 * @RETURN       u32
 * @RETVAL       returns the register contents
 * @NOTES
 */
uint32_t ak_gmac_read_reg(uintptr_t regbase, uint32_t regoffset);

/*
 * @BRIEF        The Low level function to write to a register in Hardware.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *regbase:pointer to the base of register map
 * @PARAM[in]    regoffset:offset from the base
 * @PARAM[in]    regdata:data to be written
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_write_reg(uintptr_t regbase, uint32_t regoffset, uint32_t regdata);

/*
 * @BRIEF        Function to read the Phy register. The access to phy register.
 *               is a slow process as the data is moved accross MDI/MDO
 * interface
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *RegBase:pointer to Register Base (It is the mac base in our
 * case)
 * @PARAM[in]    PhyBase:PhyBase register is the index of one of supported 32
 * PHY devices
 * @PARAM[in]    RegOffset:Register offset is the index of one of the 32 phy
 * register.
 * @PARAM[out]   *data:u16 data read from the respective phy register (only
 * valid iff return value is 0).
 * @RETURN       int
 * @RETVAL       return Returns 0 on success else return the error status.
 * @NOTES
 */
int ak_gmac_read_phy_reg(uint32_t RegOffset, uint16_t* data);

/*
 * @BRIEF        Function to write to the Phy register. The access to phy
 * register. is a slow process as the data is moved accross MDI/MDO interface
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *RegBase:pointer to Register Base (It is the mac base in our
 * case)
 * @PARAM[in]    PhyBase:PhyBase register is the index of one of supported 32
 * PHY devices
 * @PARAM[in]    RegOffset:Register offset is the index of one of the 32 phy
 * register.
 * @PARAM[in]    data:data to be written to the respective phy register.
 * @RETURN       int
 * @RETVAL       return Returns 0 on success else return the error status.
 * @NOTES
 */
int ak_gmac_write_phy_reg(uint32_t RegOffset, uint16_t data);

/*
 * @BRIEF        Function to reset mmc. Mac Management Counters (MMC)
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_mmc_counters_reset(void);

/*
 * @BRIEF        Configures the MMC to stop rollover
 * Programs MMC interface so that counters will not rollover after reaching
 * maximum value.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_mmc_counters_disable_rollover(void);

/*
 * @BRIEF        Read the MMC Rx interrupt status
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       uint32_t
 * @RETVAL       returns the Rx interrupt status.
 * @NOTES
 */
uint32_t ak_gmac_read_mmc_rx_int_status(void);

/*
 * @BRIEF        Read the MMC Tx interrupt status
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       uint32_t
 * @RETVAL       returns the Tx interrupt status.
 * @NOTES
 */
uint32_t ak_gmac_read_mmc_tx_int_status(void);

/*
 * @BRIEF        Function to reset the GMAC core.
 * This reests the DMA and GMAC core. After reset all the registers holds their
 * respective reset value
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       int
 * @RETVAL       return 0 on success else return the error status.
 * @NOTES
 */
int ak_gmac_reset(void);

/*
 * @BRIEF        Sets the Mac address in to GMAC register.
 *               This function sets the MAC address to the MAC register in
 * question.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device to populate mac dma and phy
 * addresses.
 * @PARAM[in]    MacHigh:Register offset for Mac address high
 * @PARAM[in]    MacLow:Register offset for Mac address low
 * @PARAM[in]    *MacAddr:buffer containing mac address to be programmed.
 * @RETURN       int
 * @RETVAL       return 0 upon success. Error code upon failure.
 * @NOTES
 */
int ak_gmac_set_mac_addr(uint32_t MacHigh, uint32_t MacLow, uint8_t* MacAddr);

/*
 * @BRIEF        Get the Mac address in to the address specified.
 *               The mac register contents are read and written to buffer
 * passed.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device to populate mac dma and phy
 * addresses.
 * @PARAM[in]    MacHigh:Register offset for Mac address high
 * @PARAM[in]    MacLow:Register offset for Mac address low
 * @PARAM[out]   *MacAddr:buffer containing the device mac address.
 * @RETURN       int
 * @RETVAL       return 0 upon success. Error code upon failure.
 * @NOTES
 */
int ak_gmac_get_mac_addr(uint32_t MacHigh, uint32_t MacLow, uint8_t* MacAddr);

/*
 * @BRIEF        Function to read the GMAC IP Version and populates the same in
 * device data structure.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       int
 * @RETVAL       return Always return 0.
 * @NOTES
 */
uint32_t ak_gmac_get_version(void);

/*
 * @BRIEF        Enable all the interrupts.
 *               Enables the DMA interrupt as specified by the bit mask.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    interrupts:bit mask of interrupts to be enabled.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_enable_interrupt(uint32_t interrupts);

/*
 * @BRIEF        Disable all the interrupts.
 *               Disable the DMA interrupt as specified by the bit mask.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES        This function disabled all the interrupts, if you want to
 * disable a particular interrupt then use gmac_disable_interrupt().
 */
void ak_gmac_disable_interrupt_all(void);

/*
 * @BRIEF        Checks whether the packet received is a magic packet?.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       bool
 * @RETVAL       returns True if magic packet received else returns false.
 * @NOTES
 */
bool ak_gmac_is_magic_packet_received(void);

/*
 * @BRIEF        Checks whether the packet received is a wakeup frame?.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       bool
 * @RETVAL       returns true if wakeup frame received else returns false.
 * @NOTES
 */
bool ak_gmac_is_wakeup_frame_received(void);

/*
 * @BRIEF        Enables the assertion of PMT interrupt.
 *               This enables the assertion of PMT interrupt due to Magic Pkt or
 * Wakeup frame reception.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_pmt_int_enable(void);

/*
 * @BRIEF        Disables the assertion of PMT interrupt.
 *               This disables the assertion of PMT interrupt due to Magic Pkt
 * or Wakeup frame reception.
 * @AUTHOR       cao_donghua
 * @DATE date    2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_pmt_int_disable(void);

/*
 * @BRIEF        Enable the reception of frames on GMII/MII.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_rx_enable(void);

/*
 * @BRIEF        Disable the reception of frames on GMII/MII.
 *               GMAC receive state machine is disabled after completion of
 * reception of current frame.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_rx_disable(void);

/*
 * @BRIEF        Enable the DMA Reception.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_enable_dma_rx(void);

/*
 * @BRIEF        Enable the DMA Transmission.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_enable_dma_tx(void);

/*
 * @BRIEF        Enable the transmission of frames on GMII/MII.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_tx_enable(void);

/*
 * @BRIEF        Disable the transmission of frames on GMII/MII.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_tx_disable(void);

/*
 * @BRIEF        Returns the all unmasked interrupt status after reading the
 * DmaStatus register.
 * @AUTHOR       cao_donghua
 * @DATE     2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       uint32_t
 * @RETVAL       return 0 upon success. Error code upon failure.
 * @NOTES
 */
uint32_t ak_gmac_get_interrupt_type(void);

/*
 * @BRIEF        Enables the ip checksum offloading in receive path.
 *               When set GMAC calculates 16 bit 1's complement of all received
 * ethernet frame payload. It also checks IPv4 Header checksum is correct. GMAC
 * core appends the 16 bit checksum calculated for payload of IP datagram and
 * appends it to Ethernet frame transferred to the application.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_enable_rx_chksum_offload(void);

/*
 * @BRIEF        Instruct the DMA to drop the packets fails tcp ip checksum.
 *               This is to instruct the receive DMA engine to drop the recevied
 * packet if they fails the tcp/ip checksum in hardware. Valid only when full
 * checksum offloading is enabled(type-2).
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_rx_tcpip_chksum_drop_enable(void);

/*
 * @BRIEF        Checks if any Ipv4 header checksum error in the frame just
 * transmitted. This serves as indication that error occureed in the IPv4 header
 * checksum insertion. The sent out frame doesnot carry any ipv4 header checksum
 * inserted by the hardware.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    status:uint32_t status field of the corresponding descriptor.
 * @RETURN       bool
 * @RETVAL       returns true if error in ipv4 header checksum, else returns
 * false.
 * @NOTES
 */
bool ak_gmac_is_tx_ipv4header_checksum_error(uint32_t status);

/*
 * @BRIEF        Checks if any payload checksum error in the frame just
 * transmitted. This serves as indication that error occureed in the payload
 * checksum insertion. The sent out frame doesnot carry any payload checksum
 * inserted by the hardware.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    status:uint32_t status field of the corresponding descriptor.
 * @RETURN       bool
 * @RETVAL       returns true if error in ipv4 header checksum, else returns
 * false.
 * @NOTES
 */
bool ak_gmac_is_tx_payload_checksum_error(uint32_t status);

/*
 * @BRIEF        Decodes the Rx Descriptor status to various checksum error
 * conditions.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    status:uint32_t status field of the corresponding descriptor.
 * @RETURN       uint32_t
 * @RETVAL       returns decoded enum (uint32_t) indicating the status.
 * @NOTES
 */
uint32_t ak_gmac_is_rx_checksum_error(uint32_t status);

/*
 * @BRIEF        Disable the DMA for Transmission.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_disable_dma_tx(void);

/*
 * @BRIEF        Disable the DMA for Reception.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_disable_dma_rx(void);

/*
 * @BRIEF        Checks whether this rx descriptor is in chain mode.
 *               This returns true if it is this descriptor is in chain mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @RETURN       bool
 * @RETVAL       returns true if chain mode is set, false if not.
 * @NOTES
 */
bool ak_gmac_is_rx_desc_chained(DmaDesc* desc);

/*
 * @BRIEF        Checks whether this tx descriptor is in chain mode.
 *               This returns true if it is this descriptor is in chain mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @RETURN       bool
 * @RETVAL       return returns true if chain mode is set, false if not.
 * @NOTES
 */
bool ak_gmac_is_tx_desc_chained(DmaDesc* desc);

/*
 * @BRIEF        Take ownership of this Descriptor.
 *               The function is same for both the ring mode and the chain mode
 * DMA structures.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_take_desc_ownership(DmaDesc* desc);

/*
 * @BRIEF        Initialize the rx descriptors for ring or chain mode operation.
 *               - Status field is initialized to 0.
 *               - EndOfRing set for the last descriptor.
 *               - buffer1 and buffer2 set to 0 for ring mode of operation.
 * (note)
 *               - data1 and data2 set to 0. (note)
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @PARAM[in]    *last_ring_desc:whether end of ring
 * @RETURN       void
 * @RETVAL       none
 * @NOTES        Initialization of the buffer1, buffer2, data1,data2 and status
 * are not done here. This only initializes whether one wants to use this
 * descriptor in chain mode or ring mode. For chain mode of operation the
 * buffer2 and data2 are programmed before calling this function.
 */
void ak_gmac_rx_desc_init_ring(DmaDesc* desc, bool last_ring_desc);

/*
 * @BRIEF        Initialize the tx descriptors for ring or chain mode operation.
 *               - Status field is initialized to 0.
 *               - EndOfRing set for the last descriptor.
 *               - buffer1 and buffer2 set to 0 for ring mode of operation.
 * (note)
 *               - data1 and data2 set to 0. (note)
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @PARAM[in]    *last_ring_desc:whether end of ring
 * @RETURN       void
 * @RETVAL       none
 * @NOTES        Initialization of the buffer1, buffer2, data1,data2 and status
 * are not done here. This only initializes whether one wants to use this
 * descriptor in chain mode or ring mode. For chain mode of operation the
 * buffer2 and data2 are programmed before calling this function.
 */
void ak_gmac_tx_desc_init_ring(DmaDesc* desc, bool last_ring_desc);

/*
 * @BRIEF        Function to program DMA bus mode register.
 *               The Bus Mode register is programmed with the value given. The
 * bits to be set are bit wise or'ed and sent as the second argument to this
 * function.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @PARAM[in]    init_value:the data to be programmed.
 * @RETURN       s32
 * @RETVAL       return all 0
 * @NOTES
 */
int ak_gmac_set_dma_bus_mode(uint32_t init_value);

/*
 * @BRIEF        Function to program DMA Control register.
 *               The Dma Control register is programmed with the value given.
 * The bits to be set are bit wise or'ed and sent as the second argument to this
 * function.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @PARAM[in]    init_value:the data to be programmed.
 * @RETURN       s32
 * @RETVAL       return all 0
 * @NOTES
 */
int ak_gmac_set_dma_control(uint32_t init_value);

/*
 * @BRIEF        Programs the DmaRxBaseAddress with the Rx descriptor base
 * address. Rx Descriptor's base address is available in the gmacdev structure.
 * This function progrms the Dma Rx Base address with the starting address of
 * the descriptor ring or chain.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_set_rx_desc_base(uint32_t rx_base_addr);

/*
 * @BRIEF        Programs the DmaTxBaseAddress with the Tx descriptor base
 * address Tx Descriptor's base address is available in the gmacdev structure.
 * This function progrms the Dma Tx Base address with the starting address of
 * the descriptor ring or chain.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_set_tx_desc_base(uint32_t tx_base_addr);

/*
 * @BRIEF        Enable the watchdog timer on the receiver, Gmac configuration
 * functions. When enabled, Gmac enables Watchdog timer, and GMAC allows no more
 * than 2048 bytes of data (10,240 if Jumbo frame enabled).
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_wd_enable(void);

/*
 * @BRIEF        Enables the Jabber frame support.
 *               When enabled, GMAC disabled the jabber timer, and can transfer
 * 16,384 byte frames.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_jab_enable(void);

/*
 * @BRIEF        Enables Frame bursting (Only in Half Duplex Mode).
 *               When enabled, GMAC allows frame bursting in GMII Half Duplex
 * mode. Reserved in 10/100 and Full-Duplex configurations.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_frame_burst_enable(void);

/*
 * @BRIEF        Disable Jumbo frame support.
 *               When Disabled GMAC does not supports jumbo frames.
 *               Giant frame error is reported in receive frame status.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_jumbo_frame_disable(void);

/*
 * @BRIEF        Enables Receive Own bit (Only in Half Duplex Mode).
 *               When enaled GMAC receives all the packets given by phy while
 * transmitting.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_rx_own_enable(void);

/*
 * @BRIEF        Sets the GMAC in Normal mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_loopback_off(void);

/*
 * @BRIEF        Sets the GMAC core in Full-Duplex mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_set_full_duplex(void);

/*
 * @BRIEF        Sets the GMAC core in Half-Duplex mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_set_half_duplex(void);

/*
 * @BRIEF        GMAC tries retransmission (Only in Half Duplex mode).
 *               If collision occurs on the GMII/MII, GMAC attempt retries based
 * on the back off limit configured.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES        This function is tightly coupled with
 * gmac_back_off_limit(gmacdev_pt *, uint32_t).
 */
void ak_gmac_retry_enable(void);

/*
 * @BRIEF        GMAC doesnot strips the Pad/FCS field of incoming frames.
 *               GMAC will pass all the incoming frames to Host unmodified.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_pad_crc_strip_disable(void);

/*
 * @BRIEF        GMAC programmed with the back off limit value.
 *               GMAC will pass all the incoming frames to Host unmodified.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @PARAM[in]    value: data of limint to back off
 * @RETURN       void
 * @RETVAL       none
 * @NOTES        This function is tightly coupled with
 * gmac_retry_enable(gmac_device * gmacdev)
 */
void ak_gmac_back_off_limit(uint32_t value);

/*
 * @BRIEF        Disables the Deferral check in GMAC (Only in Half Duplex mode).
 *               GMAC defers until the CRS signal goes inactive.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_deferral_check_disable(void);

/*
 * @BRIEF        Selects the GMII port.
 *               When called GMII (1000Mbps) port is selected (programmable only
 * in 10/100/1000 Mbps configuration).
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_select_gmii(void);

/*
 * @BRIEF        Selects the MII port.
 *               When called MII (10/100Mbps) port is selected (programmable
 * only in 10/100/1000 Mbps configuration).
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_select_mii(void);

/*
 * @BRIEF        Enables reception of all the frames to application.
 *               Receive frame filter configuration functions
 *               GMAC passes all the frames received to application irrespective
 * of whether they
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_frame_filter_enable(void);

/*
 * @BRIEF        Enables forwarding of control frames.
 *               When set forwards all the control frames (incl. unicast and
 * multicast PAUSE frames)
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @PARAM[in]    passcontrol:
 * @RETURN       void
 * @RETVAL       none
 * @NOTES        Depends on RFE of FlowControlRegister[2]
 */
void ak_gmac_set_pass_control(uint32_t passcontrol);

/*
 * @BRIEF        Enables Broadcast frames.
 *               When enabled Address filtering module passes all incoming
 * broadcast frames.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @PARAM[in]    passcontrol:
 * @RETURN       void
 * @RETVAL       none
 * @NOTES        Depends on RFE of FlowControlRegister[2]
 */
void ak_gmac_broadcast_enable(void);

/*
 * @BRIEF        disable Broadcast frames.
 *               When enabled Address filtering module passes all incoming
 * broadcast frames.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @PARAM[in]    passcontrol:
 * @RETURN       void
 * @RETVAL       none
 * @NOTES        Depends on RFE of FlowControlRegister[2]
 */
void ak_gmac_broadcast_disable(void);

/*
 * @BRIEF        Disables Source address filtering.
 *               When disabled GMAC forwards the received frames with updated
 * SAMatch bit in RxStatus.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_src_addr_filter_disable(void);

/*
 * @BRIEF        Enables Multicast frames.
 *               When enabled all multicast frames are passed.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_multicast_enable(void);

/*
 * @BRIEF        Disable Multicast frames.
 *               When disabled multicast frame filtering depends on HMC bit.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_multicast_disable(void);

/*
 * @BRIEF        Enables the normal Destination address filtering.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_dst_addr_filter_normal(void);

/*
 * @BRIEF        Enables multicast hash filtering.
 *               When enabled GMAC performs teh destination address filtering
 * according to the hash table.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_multicast_hash_filter_enable(void);

/*
 * @BRIEF        Disables multicast hash filtering.
 *               When disabled GMAC performs perfect destination address
 * filtering for multicast frames, it compares DA field with the value
 * programmed in DA register.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_multicast_hash_filter_disable(void);

/*
 * @BRIEF        Clears promiscous mode.
 *               When called the GMAC falls back to normal operation from
 * promiscous mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_promisc_disable(void);
/*
 * @BRIEF        enable promiscous mode.
 *               When called the GMAC falls back to normal operation from
 * promiscous mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_promisc_enable(void);

/*
 * @BRIEF        Disables multicast hash filtering.
 *               When disabled GMAC performs perfect destination address
 * filtering for unicast frames, it compares DA field with the value programmed
 * in DA register.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_unicast_hash_filter_disable(void);

/*
 * @BRIEF        Disables detection of pause frames with stations unicast
 * address. When disabled GMAC only detects with the unique multicast address
 * (802.3x).
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_unicast_pause_frame_detect_disable(void);

/*
 * @BRIEF        Rx flow control enable.
 *               When Enabled GMAC will decode the rx pause frame and disable
 * the tx for a specified time.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_rx_flow_control_enable(void);

/*
 * @BRIEF        Rx flow control disable.
 *               When disabled GMAC will not decode pause frame.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_rx_flow_control_disable(void);

/*
 * @BRIEF        Tx flow control enable.
 *               When Enabled
 *               - In full duplex GMAC enables flow control operation to
 * transmit pause frames.
 *               - In Half duplex GMAC enables the back pressure operation
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_tx_flow_control_enable(void);

/*
 * @BRIEF        Tx flow control disable.
 *               When Enabled
 *               - In full duplex GMAC will not transmit any pause frames.
 *               - In Half duplex GMAC disables the back pressure feature.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_tx_flow_control_disable(void);

/*
 * @BRIEF        Checks whether the descriptor is owned by DMA.
 *               If descriptor is owned by DMA then the OWN bit is set to 1.
 * This API is same for both ring and chain mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @RETURN       bool
 * @RETVAL       returns true if Dma owns descriptor and false if not.
 * @NOTES
 */
bool ak_gmac_is_desc_owned_by_dma(DmaDesc* desc);

/*
 * @BRIEF        Checks whether the descriptor is empty.
 *               If the buffer1 and buffer2 lengths are zero in ring mode
 * descriptor is empty. In chain mode buffer2 length is 0 but buffer2 itself
 * contains the next descriptor address.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @RETURN       bool
 * @RETVAL       returns true if descriptor is empty, false if not empty.
 * @NOTES
 */
bool ak_gmac_is_desc_empty(DmaDesc* desc);

/*
 * @BRIEF        Initialize the rx descriptors for chain mode of operation.
 *               - Status field is initialized to 0.
 *               - EndOfRing set for the last descriptor.
 *               - data1 and data2 set to 0.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_rx_desc_init_chain(DmaDesc* desc);

/*
 * @BRIEF        Checks whether the rx descriptor is valid.
 *               if rx descripor is not in error and complete frame is available
 * in the same descriptor
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    status:rx descriptor status.
 * @RETURN       bool
 * @RETVAL       returns true if no error and first and last desc bits are set,
 * otherwise it returns false.
 * @NOTES
 */
bool ak_gmac_is_rx_desc_valid(uint32_t status);

/*
 * @BRIEF        returns the byte length of received frame including CRC.
 *               This returns the no of bytes received in the received ethernet
 * frame including CRC(FCS).
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    status:rx descriptor status.
 * @RETURN       bool
 * @RETVAL       returns the length of received frame lengths in bytes.
 * @NOTES
 */
uint32_t ak_gmac_get_rx_desc_frame_length(uint32_t status);

/*
 * @BRIEF        Check for damaged frame due to collision.
 *               Retruns true if rx frame was damaged due to late collision in
 * half duplex mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    status:rx descriptor status.
 * @RETURN       bool
 * @RETVAL       returns true if error else returns false.
 * @NOTES
 */
bool ak_gmac_is_rx_frame_collision(uint32_t status);

/*
 * @BRIEF        Check for receive CRC error.
 *               Retruns true if rx frame CRC error occured.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    status:rx descriptor status.
 * @RETURN       bool
 * @RETVAL       returns true if error else returns false.
 * @NOTES
 */
bool ak_gmac_is_rx_crc(uint32_t status);

/*
 * @BRIEF        Indicates rx frame has non integer multiple of bytes. (odd
 * nibbles). Retruns true if dribbling error in rx frame.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    status:rx descriptor status.
 * @RETURN       bool
 * @RETVAL       returns true if error else returns false.
 * @NOTES
 */
bool ak_gmac_is_frame_dribbling_errors(uint32_t status);

/*
 * @BRIEF        Indicates error in rx frame length.
 *               Retruns true if received frame length doesnot match with the
 * length field
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    status:rx descriptor status.
 * @RETURN       bool
 * @RETVAL       returns true if error else returns false.
 * @NOTES
 */
bool ak_gmac_is_rx_frame_length_errors(uint32_t status);

/*
 * @BRIEF        Initialize the rx descriptors for chain mode of operation.
 *               - Status field is initialized to 0.
 *               - EndOfRing set for the last descriptor.
 *               - buffer1 and buffer2 set to 0.
 *               - data1 and data2 set to 0.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_tx_desc_init_chain(DmaDesc* desc);

/*
 * @BRIEF        Resumes the DMA Transmission.
 *               the DmaTxPollDemand is written. (the data writeen could be
 * anything). This forces the DMA to resume transmission.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_resume_dma_tx(void);

/*
 * @BRIEF        Resumes the DMA Reception.
 *               the DmaRxPollDemand is written. (the data writeen could be
 * anything). This forces the DMA to resume reception.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_resume_dma_rx(void);

/*
 * @BRIEF        Checks whether the descriptor is valid
 *               if no errors such as CRC/Receive Error/Watchdog Timeout/Late
 * collision/Giant Frame/Overflow/Descriptor error the descritpor is said to be
 * a valid descriptor.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    status:tx status.
 * @RETURN       bool
 * @RETVAL       return True if desc valid. false if error.
 * @NOTES
 */
bool ak_gmac_is_desc_valid(uint32_t status);

/**
 * @BRIEF        Checks whether the tx is aborted due to collisions.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    status:tx status.
 * @RETURN       bool
 * @RETVAL       returns true if collisions, else returns false.
 * @NOTES
 */
bool ak_gmac_is_tx_aborted(uint32_t status);

/**
 * @BRIEF        Checks whether the tx carrier error.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    status:tx status.
 * @RETURN       bool
 * @RETVAL       returns true if carrier error occured, else returns falser.
 * @NOTES
 */
bool ak_gmac_is_tx_carrier_error(uint32_t status);

/**
 * @BRIEF        Gives the transmission collision count.
 *               returns the transmission collision count indicating number of
 * collisions occured before the frame was transmitted. Make sure to check
 * excessive collision didnot happen to ensure the count is valid.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    status:tx status.
 * @RETURN       uint32_t
 * @RETVAL       returns the count value of collision.
 * @NOTES
 */
uint32_t ak_gmac_get_tx_collision_count(uint32_t status);

/**
 * @BRIEF        This enables the pause frame generation after programming the
 * appropriate registers. presently activation is set at 3k and deactivation set
 * at 4k. These may have to tweaked if found any issues
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 */
void ak_gmac_pause_control(void);

/**
 * @BRIEF        The check summ offload engine is enabled to do only IPV4 header
 * checksum. IPV4 header Checksum is computed in the Hardware.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    *desc:Pointer to tx descriptor for which  ointer to
 * gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_tx_checksum_offload_ipv4hdr(DmaDesc* desc);

/**
 * @BRIEF        The check summ offload engine is enabled to do TCPIP checsum
 * assuming Pseudo header is available. Hardware computes the tcp ip checksum
 * assuming pseudo header checksum is computed in software. Ipv4 header checksum
 * is also inserted.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    *desc:Pointer to tx descriptor for which  ointer to
 * gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_tx_checksum_offload_tcponly(DmaDesc* desc);

/**
 * @BRIEF        Clears all the pending interrupts.
 *               If the Dma status register is read then all the interrupts gets
 * cleared
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_clear_interrupt(void);

/**
 * @BRIEF        Disable the MMC Tx interrupt.
 *               The MMC tx interrupts are masked out as per the mask specified.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    mask:tx interrupt bit mask for which interrupts needs to be
 * disabled.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_disable_mmc_tx_interrupt(uint32_t mask);

/**
 * @BRIEF        Disable the MMC Rx interrupt.
 *               The MMC rx interrupts are masked out as per the mask specified.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    mask:rx interrupt bit mask for which interrupts needs to be
 * disabled.
 * @RETURN       void
 * @RETVAL       return returns void.
 * @NOTES
 */
void ak_gmac_disable_mmc_rx_interrupt(uint32_t mask);

/**
 * @BRIEF        Disable the MMC ipc rx checksum offload interrupt.
 *               The MMC ipc rx checksum offload interrupts are masked out as
 * per the mask specified.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    mask:rx interrupt bit mask for which interrupts needs to be
 * disabled.
 * @RETURN       void
 * @RETVAL       return returns void.
 * @NOTES
 */
void ak_gmac_disable_mmc_ipc_rx_interrupt(uint32_t mask);

/**
 * @BRIEF        Function to set the MDC clock for mdio transactiona
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to device structure.
 * @PARAM[in]    clk_div_val:clk divider value.
 * @RETURN       int
 * @RETVAL       return Reuturns 0 on success else return the error value.
 * @NOTES
 */
int ak_gmac_set_mdc_clk_div(uint32_t clk_div_val);

/**
 * @BRIEF        Returns the current MDC divider value programmed in the ip.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to device structure.
 * @RETURN       unsigned int
 * @RETVAL       return Returns the MDC divider value read.
 * @NOTES
 */
uint32_t ak_gmac_get_mdc_clk_div(void);

/**
 * @BRIEF        Enables GMAC to look for Magic packet.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_magic_packet_enable(void);

/**
 * @BRIEF        Populates the remote wakeup frame registers.
 *               Consecutive 8 writes to GmacWakeupAddr writes the wakeup frame
 * filter registers. Before commensing a new write, frame filter pointer is
 * reset to 0x0000. A small delay is introduced to allow frame filter pointer
 * reset operation.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    *filter_contents:pointer to frame filter contents array.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_write_wakeup_frame_register(uint32_t* filter_contents);

/**
 * @BRIEF        Enables GMAC to look for wake up frame.
 *               Wake up frame is defined by the user.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_wakeup_frame_enable(void);

/**
 * @BRIEF        Disables the powerd down setting of GMAC.
 *               If the driver wants to bring up the GMAC from powerdown mode,
 * even though the magic packet or the wake up frames received from the network,
 * this function should be called.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_power_down_disable(void);

/**
 * @BRIEF        Enables the power down mode of GMAC.
 *               This function puts the Gmac in power down mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
void ak_gmac_power_down_enable(void);

#endif
