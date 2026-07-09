#ifndef _AK_ETHERNET_H_
#define _AK_ETHERNET_H_

#include <linux/types.h>
#include "ak_gmac_if/ak_gmac_if.h"

/*
DMA Descriptor Structure
The structure is common for both receive and transmit descriptors
The descriptor is of 4 words, but our structrue contains 6 words where
last two words are to hold the virtual address of the network buffer pointers
for driver's use
From the GMAC core release 3.50a onwards, the Enhanced Descriptor structure got
changed. The descriptor (both transmit and receive) are of 8 words each rather
the 4 words of normal descriptor structure. Whenever IEEE 1588 Timestamping is
enabled TX/RX DESC6 provides the lower 32 bits of Timestamp value and TX/RX
DESC7 provides the upper 32 bits of Timestamp value In addition to this whenever
extended status bit is set (RX DESC0 bit 0), RX DESC4 contains the extended
status information
*/

#define MODULO_INTERRUPT                                                       \
    1 // if it is set to 1, interrupt is available for all the descriptors or
      // else interrupt is available only for
// The Mac Base address offset is 0x0000
#define MACBASE 0x0000
// Dma base address starts with an offset 0x1000
#define DMABASE 0x1000

#ifdef AVB_SUPPORT
#define DMABASE_CH0 DMABASE // DMA base address for Channel 0
#define DMABASE_CH1 0x1100 // DMA base address for Channel 1
#define DMABASE_CH2 0x1200 // DMA base address for Channel 2
#define ETHERNET_HEADER_AVB                                                    \
    18 // 6 byte Dest addr, 6 byte Src addr, 2 byte length/type
#endif

#define IRQF_DISABLED  0x00000020
#define NET_IF_TIMEOUT (10 * HZ)
#define CHECK_TIME     (HZ)
#define SA_SHIRQ       IRQF_SHARED
#define SA_INTERRUPT   IRQF_DISABLED
#define CHECKSUM_HW    CHECKSUM_PARTIAL
#define TRANSMIT_DESC_SIZE                                                     \
    128 // 32 Tx Descriptors needed in the Descriptor pool/queue
#define RECEIVE_DESC_SIZE                                                      \
    128 // 32 Rx Descriptors needed in the Descriptor pool/queue
#define ETHERNET_HEADER                                                        \
    14 // 6 byte Dest addr, 6 byte Src addr, 2 byte length/type
#define ETHERNET_CRC   4 // Ethernet CRC
#define ETHERNET_EXTRA 2 // Only God knows about this?????
#define ETHERNET_PACKET_COPY                                                   \
    250 // Maximum length when received data is copied on to a new skb
#define ETHERNET_PACKET_EXTRA                                                  \
    18 // Preallocated length for the rx packets is MTU + ETHERNET_PACKET_EXTRA
#define VLAN_TAG             4 // optional 802.1q VLAN Tag
#define MIN_ETHERNET_PAYLOAD 46 // Minimum Ethernet payload size
#define MAX_ETHERNET_PAYLOAD 1500 // Maximum Ethernet payload size
#define JUMBO_FRAME_PAYLOAD  9000 // Jumbo frame payload size
#define TX_BUF_SIZE                                                            \
    ETHERNET_HEADER + ETHERNET_CRC + MAX_ETHERNET_PAYLOAD + VLAN_TAG
#define IOCTL_READ_REGISTER  SIOCDEVPRIVATE + 1
#define IOCTL_WRITE_REGISTER SIOCDEVPRIVATE + 2
#define IOCTL_READ_IPSTRUCT  SIOCDEVPRIVATE + 3
#define IOCTL_READ_RXDESC    SIOCDEVPRIVATE + 4
#define IOCTL_READ_TXDESC    SIOCDEVPRIVATE + 5
#define IOCTL_POWER_DOWN     SIOCDEVPRIVATE + 6
#define IOCTL_AVB_TEST       SIOCDEVPRIVATE + 7
#define AVB_SET_CONFIG       0x00000001
#define AVB_CONFIG_HW        0x00000002
#define AVB_RUN_TEST         0x00000003
#define AVB_GET_RESULT       0x00000004
#define AVB_DEBUG_ENABLE     0x00000005
#define AVB_DEBUG_DISABLE    0x00000006
#define AVB_TX_FRAMES        0x00000007

// This is the IP's phy address. This is unique address for every MAC in the
// universe
#define DEFAULT_MAC_ADDRESS                                                    \
    {                                                                          \
        0x00, 0x55, 0x7B, 0xB5, 0x7D, 0xF7                                     \
    }

/*SynopGMAC can support up to 32 phys*/
enum GMACPhyBase {
    PHY0 = 0, // The device can support 32 phys, but we use first phy only
    PHY1 = 1,
    PHY31 = 31,
};

#define DEFAULT_PHY_BASE PHY1 // We use First Phy

enum DescMode {
    RINGMODE = 0x00000001,
    CHAINMODE = 0x00000002,
};

enum BufferMode {
    SINGLEBUF = 0x00000001,
    DUALBUF = 0x00000002,
};

/* synopGMAC device data */
// cdh:add, check ok, ignore type
typedef struct synopGMACDeviceStruct {
    u32 MacBase; /* base address of MAC registers           */
    u32 DmaBase; /* base address of DMA registers           */
    u32 PhyBase; /* PHY device address on MII interface     */
    u32 Version; /* Gmac Revision version                   */

    dma_addr_t
        TxDescDma; /* Dma-able address of first tx descriptor either in ring or
                      chain mode, this is used by the GMAC device*/
    dma_addr_t
        RxDescDma; /* Dma-albe address of first rx descriptor either in ring or
                      chain mode, this is used by the GMAC device*/
    DmaDesc* TxDesc; /* start address of TX descriptors ring or chain, this is
                        used by the driver  */
    DmaDesc* RxDesc; /* start address of RX descriptors ring or chain, this is
                        used by the driver  */

    u32 BusyTxDesc; /* Number of Tx Descriptors owned by DMA at any given time*/
    u32 BusyRxDesc; /* Number of Rx Descriptors owned by DMA at any given time*/

    u32 RxDescCount; /* number of rx descriptors in the tx descriptor queue/pool
                      */
    u32 TxDescCount; /* number of tx descriptors in the rx descriptor queue/pool
                      */

    u32 TxBusy; /* index of the tx descriptor owned by DMA, is obtained by
                   gmac_get_tx_qptr()                */
    u32 TxNext; /* index of the tx descriptor next available with driver, given
                   to DMA by gmac_set_tx_qptr() */
    u32 RxBusy; /* index of the rx descriptor owned by DMA, obtained by
                   gmac_get_rx_qptr()                   */
    s32 RxNext; /* index of the rx descriptor next available with driver, given
                   to DMA by gmac_set_rx_qptr() */

    DmaDesc* TxBusyDesc; /* Tx Descriptor address corresponding to the index
                            TxBusy */
    DmaDesc* TxNextDesc; /* Tx Descriptor address corresponding to the index
                            TxNext */
    DmaDesc* RxBusyDesc; /* Rx Descriptor address corresponding to the index
                            TxBusy */
    DmaDesc* RxNextDesc; /* Rx Descriptor address corresponding to the index
                            RxNext */

    u32 clkdiv_mdc; /* Clock divider value programmed in the hardware   */
    u32 LinkState; /* Link status as reported by the Marvel Phy        */
    u32 DuplexMode; /* Duplex mode of the Phy       */
    u32 Speed; /* Speed of the Phy             */
    u32 LoopBackMode; /* Loopback status of the Phy   */
    u32 phy_id1;
    u32 phy_id2;
    int gpio_index;
} gmac_device;

// cdh:add, check ok, ignore type
#ifdef AVB_SUPPORT
typedef struct AVBStruct {
    u8 ChSelMask; /* This gives which DMA channel is enabled and which is
                    disabled Bit0 for Ch0 Bit1 for Ch1 Bit2 for Ch2
                  */
    u8 DurationOfExp; /* Duration for which experiment should be conducted in
                         minutes - Default 2 Minutes */

    u8 AvControlCh; /* channel on which AV control channel must be received (Not
                       used)*/
    u8 PTPCh; /* Channel on which PTP packets must be received (Not Used)*/
    u8 PrioTagForAV; /* Used when more than One channel enabled in Rx path (Not
                       Used) for only CH1 Enabled: Frames sith Priority > Value
                       programmed, frames sent to CH1 Frames with priority <
                       Value programmed are sent to CH0 For both CH1 and CH2
                       Enabled: Frames sith Priority > Value programmed, frames
                       sent to CH2 Frames with priority < Value programmed are
                       sent to CH1
                     */

    u16 AvType; /* AV Ethernet Type to be programmed for Core to identify AV
                   type */

    u8 Ch1PrioWts;
    u8 Ch1Bw;
    u32 Ch1_frame_size;
    u8 Ch1_EnableSlotCheck; /* Enable checking of slot numbers programmed in the
                               Tx Desc*/
    u8 Ch1_AdvSlotInterval; /* When Set Data fetched for current slot and for
                              next 2 slots in advance When reset data fetched
                              for current slot and in advance for next slot*/

    u8 Ch1CrSh; /* When set Enables the credit based traffic shaping. Now works
                   with Strict priority style*/
    u8 Ch1SlotCount; /* Over which transmiteed bits per slot needs to be
                        computed (Only for Credit based shaping) */
    u32 Ch1AvgBitsPerSlot; /* Average bits per slot reported by core once in
                              Ch1SlotCount * 125 micro seconds */
    u32 Ch1AvgBitsPerSlotAccL; /* No of Avg Bits per slot on Channel1*/
    u32 Ch1AvgBitsPerSlotAccH; /* No of Avg Bits per slot on Channel1*/
    u32 Ch1AvgBitsNoOfInterrupts; /* Total Number of interrupts over which
                                     AvbBits are accumulated*/

    u8 Ch1CreditControl; /* Will be zero (Not used) */

    u8 Ch1_tx_rx_prio_policy; // Should Ch1 use Strict or RR policy
    u8 Ch1_use_tx_high_prio; // Should Ch1 Tx have High priority over Rx
    u8 Ch1_tx_rx_prio_ratio; // For Round Robin what is the ratio between tx-rx
                             // or rx-tx

    u8 Ch1_tx_desc_slot_no_start;
    u8 Ch1_tx_desc_slot_no_skip;

    u32 Ch1SendSlope;
    u32 Ch1IdleSlope;
    u32 Ch1HiCredit;
    u32 Ch1LoCredit;

    u32 Ch1FramecountTx; /* No of Frames Transmitted on Channel 1 */
    u32 Ch1FramecountRx; /* No of Frames Received on Channel 1 */

    u8 Ch2PrioWts;
    u8 Ch2Bw;
    u32 Ch2_frame_size;
    u8 Ch2_EnableSlotCheck; /* Enable checking of slot numbers programmed in the
                               Tx Desc*/
    u8 Ch2_AdvSlotInterval; /* When Set Data fetched for current slot and for
                              next 2 slots in advance When reset data fetched
                              for current slot and in advance for next slot*/
    u8 Ch2CrSh; /* When set Enables the credit based traffic shaping. Now works
                   with Strict priority style*/
    u8 Ch2SlotCount; /* Over which transmiteed bits per slot needs to be
                        computed (Only for Credit based shaping) */
    u32 Ch2AvgBitsPerSlot; /* Average bits per slot reported by core once in
                              Ch2SlotCount * 125 micro seconds */
    u32 Ch2AvgBitsPerSlotAccL; /* No of Avg Bits per slot on Channel2*/
    u32 Ch2AvgBitsPerSlotAccH; /* No of Avg Bits per slot on Channel2*/
    u32 Ch2AvgBitsNoOfInterrupts; /* Total Number of interrupts over which
                                     AvbBits are accumulated*/
    u8 Ch2CreditControl; /* Will be zero at present*/
    u8 Ch2_tx_rx_prio_policy; // Should Ch1 use Strict or RR policy
    u8 Ch2_use_tx_high_prio; // Should Ch1 Tx have High priority over Rx
    u8 Ch2_tx_rx_prio_ratio; // For Round Robin what is the ratio between tx-rx
                             // or rx-tx
    u8 Ch2_tx_desc_slot_no_start;
    u8 Ch2_tx_desc_slot_no_skip;
    u32 Ch2SendSlope;
    u32 Ch2IdleSlope;
    u32 Ch2HiCredit;
    u32 Ch2LoCredit;
    u32 Ch2FramecountTx; /* No of Frames Transmitted on Channel 2 */
    u32 Ch2FramecountRx; /* No of Frames Received on Channel 2 */
    u8 Ch0PrioWts;
    u8 Ch0_tx_rx_prio_policy; // Should Ch1 use Strict or RR policy
    u8 Ch0_use_tx_high_prio; // Should Ch1 Tx have High priority over Rx
    u8 Ch0_tx_rx_prio_ratio; // For Round Robin what is the ratio between tx-rx
                             // or rx-tx
    u32 Ch0_frame_size;
    u32 Ch0FramecountTx; /* No of Frames Transmitted on Channel 0 */
    u32 Ch0FramecountRx; /* No of Frames Received on Channel 0 */
} gmac_avb;
#endif

/* Structure/enum declaration ------------------------------- */
typedef struct mac_info {
    void __iomem* io_addr; /* Register I/O base address */
    u16 irq; /* IRQ */

    u16 tx_pkt_cnt;
    u16 queue_pkt_len;
    u16 queue_start_addr;
    u16 queue_ip_summed;
    u16 dbug_cnt;
    u8 io_mode; /* 0:word, 2:byte */
    u8 phy_addr;
    u8 imr_all;

    unsigned int flags;
    unsigned int in_suspend : 1;

    void (*inblk)(void __iomem* port, void* data, int length);
    void (*outblk)(void __iomem* port, void* data, int length);
    void (*dumpblk)(void __iomem* port, int length);

    struct device* dev; /* parent device */

    struct resource* addr_res; /* resources found */
    struct resource* addr_req; /* resources requested */
    struct resource* irq_res;

    struct mutex addr_lock; /* phy and eeprom access lock */

    struct delayed_work phy_poll;
    struct net_device* ndev;

    spinlock_t lock;

    u32 msg_enable;

    int rx_csum;
    int can_csum;
    int ip_summed;
    int phy_id;
    struct clk* clk;

    struct work_struct link_chg_task;
} mac_info_t;

// cdh:add, check ok, ignore type
typedef struct synopGMACAdapterStruct {
    /*Device Dependent Data structur*/
    gmac_device* gmacdev_pt;

    /*Os/Platform Dependent Data Structures*/
    struct mac_info* db_pt;
    struct net_device* netdev_pt;
    struct net_device_stats net_dev_stats;
    u32 pcistate[16];
    struct clk* mac_clk; // cdh:add for standard clk
    struct clk* phy_clk; // cdh:add for standard clk
    struct device* dev;
} nt_adapter;

/* Below is "88E1011/88E1011S Integrated 10/100/1000 Gigabit Ethernet
 * Transceiver" Register and their layouts. This Phy has been used in the Dot
 * Aster GMAC Phy daughter. Since the Phy register map is standard, this map
 * hardly changes to a different Ppy
 */

enum MiiRegisters {
    PHY_CONTROL_REG = 0x0000, /*Control Register*/
    PHY_STATUS_REG = 0x0001, /*Status Register */
    PHY_ID_HI_REG = 0x0002, /*PHY Identifier High Register*/
    PHY_ID_LOW_REG = 0x0003, /*PHY Identifier High Register*/
    PHY_AN_ADV_REG = 0x0004, /*Auto-Negotiation Advertisement Register*/
    PHY_LNK_PART_ABl_REG = 0x0005, /*Link Partner Ability Register (Base Page)*/
    PHY_AN_EXP_REG = 0x0006, /*Auto-Negotiation Expansion Register*/
    PHY_AN_NXT_PAGE_TX_REG = 0x0007, /*Next Page Transmit Register*/
    PHY_LNK_PART_NXT_PAGE_REG = 0x0008, /*Link Partner Next Page Register*/
    PHY_1000BT_CTRL_REG = 0x0009, /*1000BASE-T Control Register*/
    PHY_1000BT_STATUS_REG = 0x000a, /*1000BASE-T Status Register*/
    PHY_SPECIFIC_CTRL_REG = 0x0010, /*Phy specific control register*/
    PHY_SPECIFIC_STATUS_REG = 0x0011, /*Phy specific status register*/
    PHY_INTERRUPT_ENABLE_REG = 0x0012, /*Phy interrupt enable register*/
    PHY_INTERRUPT_STATUS_REG = 0x0013, /*Phy interrupt status register*/
    PHY_EXT_PHY_SPC_CTRL = 0x0014, /*Extended Phy specific control*/
    PHY_RX_ERR_COUNTER = 0x0015, /*Receive Error Counter*/
    PHY_EXT_ADDR_CBL_DIAG
    = 0x0016, /*Extended address for cable diagnostic register*/
    PHY_LED_CONTROL = 0x0018, /*LED Control*/
    PHY_MAN_LED_OVERIDE = 0x0019, /*Manual LED override register*/
    PHY_EXT_PHY_SPC_CTRL2 = 0x001a, /*Extended Phy specific control 2*/
    PHY_EXT_PHY_SPC_STATUS = 0x001b, /*Extended Phy specific status*/
    PHY_CBL_DIAG_REG = 0x001c, /*Cable diagnostic registers*/
    RTL8201_PAGE_SELECT = 31,
    RTL8201_P7_R16 = 16,
    RTL8201G_P0xD05 = 0xd05, /* Page 0xd05 */
    RTL8201G_P0xD05_R16 = 0x10, /* RMII Mode Setting Register 1(RMSR1) */
    RTL8201G_P0xD08 = 0xd08, /* Page 0xd08 */
    RTL8201G_P0xD08_R17 = 0x11, /* RMII Mode Setting Register 2(RMSR2) */
};

/* This is Control register layout. Control register is of 16 bit wide.
 */
enum Mii_GEN_CTRL {
    /*  Description                bits        R/W  default value  */
    Mii_reset = 0x8000,
    Mii_Speed_10 = 0x0000, /* 10   Mbps                    6:13   RW */
    Mii_Speed_100 = 0x2000, /* 100  Mbps                    6:13   RW */
    Mii_Speed_1000 = 0x0040, /* 1000 Mbit/s                  6:13   RW */

    Mii_Duplex = 0x0100, /* Full Duplex mode             8      RW */
    Mii_AN_restart = 0x0200, /* Autonegotiation restart      9      RW */

    Mii_Manual_Master_Config = 0x0800, /* Manual Master Config    11     RW */

    Mii_AN_En = 0x1000, /* Autonegotiation enable       12     RW */

    Mii_Loopback = 0x4000, /* Enable Loop back             14     RW */
    Mii_NoLoopback = 0x0000, /* Enable Loop back             14     RW */
};

/* This is Status register layout. Status register is of 16 bit wide.
 */
enum Mii_GEN_STATUS {
    Mii_AutoNegCmplt = 0x0020, /* Autonegotiation completed      5    RW */
    Mii_RemoteFault = 0x0010, /* Remote fault                   4    RW */
    Mii_AN_Ability = 0x0008, /* Autonegotiation ability        3    RW */
    Mii_Link = 0x0004, /* Link status                    2    RW */
};

enum Mii_Phy_Status {
    Mii_phy_status_speed_10 = 0x0000,
    Mii_phy_status_speed_100 = 0x4000,
    Mii_phy_status_speed_1000 = 0x8000,

    Mii_phy_status_full_duplex = 0x0100, // cdh:bit 8, old,13
    Mii_phy_status_half_duplex = 0x0000,

    Mii_phy_status_link_up = 0x0400,
};

enum Mii_Link_Status {
    LINKDOWN = 0,
    LINKUP = 1,
};

enum Mii_Duplex_Mode {
    HALFDUPLEX = 1,
    FULLDUPLEX = 2,
};
enum Mii_Link_Speed {
    SPEED10 = 1,
    SPEED100 = 2,
    SPEED1000 = 3,
};

enum Mii_Loop_Back {
    NOLOOPBACK = 0,
    LOOPBACK = 1,
};

#endif
