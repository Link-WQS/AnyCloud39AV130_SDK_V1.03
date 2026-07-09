/*
 * Anyka MAC Fast Ethernet driver for Linux.
 * Features
 * Copyright (C) 2020 ANYKA
 * AUTHOR cao_donghua
 * 20-02-26 09:08:08
 */
#include <asm-generic/gpio.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/clk.h>
#include <linux/crc32.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <mach/anyka_types.h>
#include <mach/map.h>

#include <linux/ethtool.h>
#include <linux/of_net.h>
#include <linux/phy.h>

/* for proc interface*/
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <mach/ak_l2.h>

#include "ak_ethernet.h"

#define DRV_NAME                   "AK_MAC"
#define MACNAME                    "ak_mac"
#define DRV_VERSION                "1.0.10"
#define MAC_FILE_NAME              "MAC"
#define CTOI(c)                    (isdigit(c) ? (c - '0') : (c - 'A' + 10))
#define MAC_ADDR_APSTRING_LEN      8
#define OTHER_END_PARTNER_AN_ABITY (0x1 << 0)
#define SPEED_100M_FULL            (0x8)
#define SPEED_100M_HALF            (0x4)
#define SPEED_10M_FULL             (0x2)
#define NAN_SPEED_10M_HALF         ((0x0 << 7) | (0x1 << 5))
#define NAN_SPEED_100M_HALF        ((0x1 << 7) | (0x0 << 5))
#define MAC_ADDR_LEN               6
#define MAC_ADDR_STRING_LEN        (MAC_ADDR_LEN * 3 - 1)
#define MODULE_RESET_CON1          (AK_VA_SYSCTRL + 0x20)
#define PHY_WAIT_IDLE_TIMER_CNT    (200)
#define BROADCAST_ADDR_CNT         (6)
#define WAIT_PHY_ANG_CNT           (100000)

#define MOUNTED_PHY_SMALL_BOARD   (1)
#define UNMOUNTED_PHY_SMALL_BOARD (0)

#define AK_CHIP_VERSION_REG              (AK_VA_SYSCTRL + 0x0)

/*
 * wether use hardware checksum or not of mac controller
 */
#define IPC_OFFLOAD       1

//#define SMALL_TXDONE_INT 1

#ifdef SMALL_TXDONE_INT
#define TXDONE_INT_INTERVAL 32
#endif

static int disable_broadcast_count = 100;
module_param(disable_broadcast_count, int, 0644);
MODULE_PARM_DESC(disable_broadcast_count,
        "disable broadcast frames when larger then");

static int disable_multicast_count = 100;
module_param(disable_multicast_count, int, 0644);
MODULE_PARM_DESC(disable_multicast_count,
        "disable multicast frames when larger then");

static int broadcast_enable_per_ms = 500;
module_param(broadcast_enable_per_ms, int, 0644);
MODULE_PARM_DESC(broadcast_enable_per_ms, "enable broadcast frames per ms");

static bool eee_is_enable = 0;
module_param(eee_is_enable, bool, 0644);
MODULE_PARM_DESC(eee_is_enable, "enable eee(LPI) mode");

static int eth0_cable_plug_timer = 0;
module_param(eth0_cable_plug_timer, int, 0644);
MODULE_PARM_DESC(eth0_cable_plug_timer,
        "enable timer for eth0 cable plug detect");

static int eth1_cable_plug_timer = 0;
module_param(eth1_cable_plug_timer, int, 0644);
MODULE_PARM_DESC(eth1_cable_plug_timer,
        "enable timer for eth1 cable plug detect");

static int min_packet_mtu = MAX_ETHERNET_PAYLOAD;
module_param(min_packet_mtu, int, 0644);
MODULE_PARM_DESC(min_packet_mtu,
        "min packet mtu in bytes, default 1500");

typedef int (*timer_handler)(void *data);
extern int ak_timer_start(
        timer_handler handler, void *data, void *priv, int hz);
extern void *ak_timer_probe(int which_timer);
extern int ak_timer_remove(void *priv);

void *aktimer;
void *aktimer1;

#define MAC_CNT 2

static int broadcast_count[MAC_CNT] = {0};
static int broadcast_arp_count[MAC_CNT] = {0};
static int broadcast_ip_count[MAC_CNT] = {0};
static int host_count[MAC_CNT] = {0};
static int multicast_count[MAC_CNT] = {0};
static int otherhost_count[MAC_CNT] = {0};

static u32 GMAC0_Power_down;
static u32 GMAC1_Power_down;

static struct timer_list eth0_timer_cable_unplug;
static struct timer_list eth1_timer_cable_unplug;

static struct timer_list eth0_broadcast_reset_timer;
static struct timer_list eth1_broadcast_reset_timer;

static struct proc_dir_entry* eth0_proc;
static struct proc_dir_entry* eth1_proc;

static int num_heaps = 1;

static bool is_eee_mode = 0;

struct eth_info {
    char mac_name[12]; /* mac driver name */
    char mac_addr[18]; /* mac address */
    u32 phy_id[2]; /* phy id0 and id1 */
    u32 phy_onboard; /* phy small board mounted status */
    u32 link_state; /* net wire plug in&out status */
    u32 duplex_mode; /* mac work duplex or simplex mode */
    u32 speed; /* mac speed mode */
};

static struct eth_info eth0_proc_info;
static struct eth_info eth1_proc_info;

static u32 ak_gmac_wakeup_filter_config3[] = {
    0x00000000, /* For Filter0 CRC is not computed may be it is 0x0000 */
    0x000000FF, /* For Filter1 CRC is computed on 0,1,2,3,4,5,6,7 bytes from
                   offset */
    0x00000000, /* For Filter2 CRC is not computed may be it is 0x0000 */
    0x00000000, /* For Filter3 CRC is not computed may be it is 0x0000 */
    0x00000100, /* Filter 0,2,3 are disabled, Filter 1 is enabled and filtering
                   applies to only unicast packets */
    0x00003200, /* Filter 0,2,3 (no significance), filter 1 offset is 50 bytes
                   from start of Destination MAC address */
    0x7eED0000, /* No significance of CRC for Filter0, Filter1 CRC is 0x7EED, */
    0x00000000 /* No significance of CRC for Filter2 and Filter3 */
};

/*
 * brief: app level get mac controller reg value from this struct
 * u32 unit: accress reg type by this varaible
 * u32 addr: reg address offset
 * u32 data: reg value
 */
struct ifr_data_struct {
    u32 unit;
    u32 addr;
    u32 data;
};

/*
 * mac controller private data
 */
struct ak_gmac100 {
    struct resource* res;
    void __iomem* base;
    int irq;
    spinlock_t tx_lock;
    struct net_device* netdev;
    struct device* dev;

    /* Device Dependent Data structur */
    gmac_device gmacdev;

    /* phy device infor */
    struct mii_bus* mii_bus;
    int phy_irq[PHY_MAX_ADDR];
    struct phy_device* phydev;
    int old_speed;
    int old_link;
    int old_duplex;

    /* Serialise access to device */
    spinlock_t lock;
    struct clk* mac_clk;
    struct clk* phy_clk;
    int mac_id;
    int inner_phy_mode;

    struct pinctrl* pinctrl;
    struct pinctrl_state* pinctrl_pins_inner_sz18201;
    struct work_struct linkup_work;
};

/*
 * @BRIEF        extrn function, used for check mac addres validate
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *dev:input net device struct pointer
 * @RETURN       int
 * @RETVAL       0:success, -49:failed
 * @NOTES
 */
extern int eth_validate_addr(struct net_device* dev);

static void ak_set_eee(struct net_device* netdev);
static void ak_exit_eee(struct net_device* netdev);

static int ak_mac_get_packet_mtu(struct net_device *netdev)
{
    return (netdev->mtu < min_packet_mtu)? min_packet_mtu: netdev->mtu;
}

/*
 * @BRIEF        save mac address for proc mac address inerface
 * @AUTHOR       cao_donghua
 * @DATE         2020-04-15
 * @PARAM[in]    dev_addr[MAC_ADDR_LEN]:input net device struct pointer
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void save_ether_addr(
    struct net_device* netdev, unsigned char dev_addr[MAC_ADDR_LEN])
{
    int i;
    char* mac;
    struct ak_gmac100* priv = netdev_priv(netdev);

    if (priv->mac_id == 0) {
        mac = (char*)eth0_proc_info.mac_addr;
        memset(mac, '\0', sizeof(eth0_proc_info.mac_addr));
    }

    if (priv->mac_id == 1) {
        mac = (char*)eth1_proc_info.mac_addr;
        memset(mac, '\0', sizeof(eth1_proc_info.mac_addr));
    }

    for (i = 0; i < MAC_ADDR_LEN; i++) {
        sprintf(mac + strlen(mac), "%.2X", dev_addr[i]);
        if (i < (MAC_ADDR_LEN - 1)) {
            strcat(mac, ":");
        }
    }
}

/*
 * @BRIEF        set mac clock of rmii interface
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *pdev:platform device
 * @PARAM[in]    *mac_clk:dts mac clock inode
 * @PARAM[in]    enable:set clock enable or disable
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static int ak_gmac_rmii_interface_50mclk(
    struct platform_device* pdev, struct ak_gmac100* priv, unsigned char enable)
{
    int i = 0;
    int ret = 0;
    u32 regval;
    void __iomem* reg = AK_VA_SYSCTRL;
    struct device_node* np = pdev->dev.of_node;

    pdev->id = of_alias_get_id(np, "ethernet");
    if (enable == AK_TRUE) {
#if defined(CONFIG_MACH_AK3918AV100) || defined(CONFIG_MACH_AK3918EV300L) \
        || defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130)
        pr_info("%s\n", __func__);
        /* first  mac_speed_cfg=1(100m) */
        regval = __raw_readl(reg + 0x14);
        regval |= (0x1 << 23);
        __raw_writel(regval, reg + 0x14);

        /* enable opclk  */
        if (clk_prepare_enable(priv->phy_clk)) {
            pr_err("%s prepare and enable %p  fail\n", __func__, priv->phy_clk);
            return -ENODEV;
        }
        /* set opclk rate */
        clk_set_rate(priv->phy_clk, 50000000);
        pr_info("%s get rate:%lu end\n", __func__, clk_get_rate(priv->phy_clk));
        /* enable mac ctronller clk  */
        ret = clk_prepare_enable(priv->mac_clk);
        if (ret) {
            pr_err("clk_prepare_enable failed!\n");
            return -ENODEV;
        }
#elif defined(CONFIG_MACH_AK37E)
        if (pdev->id == 0) {
            /* mac0 to choose rmii interface */
            regval = __raw_readl(reg + 0x14);
            regval &= ~(0x1 << 22);
            __raw_writel(regval, reg + 0x14);

            /* mac0_speed_cfg */
            regval = __raw_readl(reg + 0x14);
            regval |= (0x1 << 23);
            __raw_writel(regval, reg + 0x14);

            /* bit[28],mac feedback clk en,lan tiantian add */
            regval = __raw_readl(reg + 0x14);
            regval |= (0x1 << 28);
            __raw_writel(regval, reg + 0x14);

            /* set 50M clock */
            regval = __raw_readl(reg + 0x18);
            regval &= ~(0x3f << 0);
            regval |= (0x9 << 0) | (0x1 << 8);
            __raw_writel(regval, reg + 0x18);

            regval = __raw_readl(reg + 0x18);
            regval |= (0x1 << 9);
            __raw_writel(regval, reg + 0x18);
        }

        if (pdev->id == 1) {
            /* mac1 to choose rmii interface */
            regval = __raw_readl(reg + 0x14);
            regval &= ~(0x1 << 20);
            __raw_writel(regval, reg + 0x14);

            /* mac1_speed_cfg */
            regval = __raw_readl(reg + 0x14);
            regval |= (0x1 << 21);
            __raw_writel(regval, reg + 0x14);

            /* bit[28],mac feedback clk en,lan tiantian add */
            regval = __raw_readl(reg + 0x14);
            regval |= (0x1 << 28);
            __raw_writel(regval, reg + 0x14);

            /* set 50M clock */
            regval = __raw_readl(reg + 0x18);
            regval &= ~(0x3f << 20);
            regval |= (0x9 << 20) | (0x1 << 28);
            __raw_writel(regval, reg + 0x18);

            regval = __raw_readl(reg + 0x18);
            regval |= (0x1 << 29);
            __raw_writel(regval, reg + 0x18);
        }
        /* enable mac ctronller clk  */
        ret = clk_prepare_enable(priv->mac_clk);
        if (ret) {
            pr_err("clk_prepare_enable failed!\n");
            return -ENODEV;
        }

#else
        /* rmii, select top rmii interface and 100mbps speed, and make 50M clock
         * to phy */
        regval = __raw_readl(reg + 0x14);
        regval &= ~(0x1 << 22);
        __raw_writel(regval, reg + 0x14);

        /* first  mac_speed_cfg=1(100m) */
        regval = __raw_readl(reg + 0x14);
        regval |= (0x1 << 23);
        __raw_writel(regval, reg + 0x14);

        /* bit[21],enable generate 50m */
        regval = __raw_readl(reg + 0x14);
        regval |= (0x1 << 21);
        __raw_writel(regval, reg + 0x14);

        /* bit[28],mac feedback clk en,lan tiantian add */
        regval = __raw_readl(reg + 0x14);
        regval |= (0x1 << 28);
        __raw_writel(regval, reg + 0x14);

        /* set bit[21],enable generate 50m, bit[18], select 25m clock of mac
         * from pll div */
        regval = __raw_readl(reg + 0x14);
        regval |= ((0x1 << 16) | (0x1 << 18));
        __raw_writel(regval, reg + 0x14);

        /* bit[20]:select mac 50M clock from 50M crystal or pll div,here do
         * what's mean? ,pg said at least repeat twice */
        for (i = 0; i < 6; i++) {
            regval = __raw_readl(reg + 0x14);
            regval |= (0x1 << 20);
            __raw_writel(regval, reg + 0x14);

            regval = __raw_readl(reg + 0x14);
            regval &= ~(0x1 << 20);
            __raw_writel(regval, reg + 0x14);
        }

        /* enable mac ctronller clk  */
        ret = clk_prepare_enable(priv->mac_clk);
        if (ret) {
            pr_err("clk_prepare_enable failed!\n");
            return -ENODEV;
        }
#endif
    } else {
#if defined(CONFIG_MACH_AK3918AV100) || defined(CONFIG_MACH_AK3918EV300L) \
        || defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130)

        /* release mac_speed_cfg */
        regval = __raw_readl(reg + 0x14);
        regval &= ~(0x1 << 23);
        __raw_writel(regval, reg + 0x14);

        /* disable opclk */
        clk_disable_unprepare(priv->phy_clk);

        /* disable mac ctronller clk  */
        clk_disable_unprepare(priv->mac_clk);

#elif defined(CONFIG_MACH_AK37E)
        if (pdev->id == 0) {
            /* release mac0 to choose rmii interface */
            regval = __raw_readl(reg + 0x14);
            regval &= ~(0x1 << 22);
            __raw_writel(regval, reg + 0x14);

            /* release mac0_speed_cfg */
            regval = __raw_readl(reg + 0x14);
            regval &= ~(0x1 << 23);
            __raw_writel(regval, reg + 0x14);

            // printk(KERN_ERR"%s, line:%d, mac_id:%d, remove\n",
            // __func__,__LINE__,pdev->id);
        }

        if (pdev->id == 1) {
            /* release mac1 to choose rmii interface */
            regval = __raw_readl(reg + 0x14);
            regval &= ~(0x1 << 20);
            __raw_writel(regval, reg + 0x14);

            /* release mac1_speed_cfg */
            regval = __raw_readl(reg + 0x14);
            regval &= ~(0x1 << 21);
            __raw_writel(regval, reg + 0x14);
            // printk(KERN_ERR"%s, line:%d, mac_id:%d, remove\n",
            // __func__,__LINE__,pdev->id);
        }

        /* disable mac ctronller clk  */
        clk_disable_unprepare(priv->mac_clk);

#else
        /* release Rmii, select top rmii interface and 100mbps speed, and make
         * 50M clock to phy release first  mac_speed_cfg=1(100m) release
         * bit[21],enable generate 50m release bit[16],enable 1/24 divider
         * generate 50m,
         */
        regval = __raw_readl(reg + 0x14);
        // regval &= ~((0x1 << 16) | (0x1 << 21) | (0x1 << 22) | (0x1 << 23));
        regval &= ~((0x1 << 21) | (0x1 << 22) | (0x1 << 23));
        __raw_writel(regval, reg + 0x14);

        /* release bit[28],mac feedback clk en,lan tiantian add
         * release  bit[18], select 25m clock of mac from pll div
         */
        regval = __raw_readl(reg + 0x14);
        regval |= ((0x1 << 18) | (0x1 << 28));
        __raw_writel(regval, reg + 0x14);

        /* disable mac ctronller clk  */
        clk_disable_unprepare(priv->mac_clk);
#endif
    } /* enable == AK_FALSE */

    return ret;
}
/*end of ak_gmac_rmii_interface_50mclk*/

/*
 * @BRIEF        set mac sharepin of rmii interface
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *pdev:platform device
 * @PARAM[in]    enable:set mac sharepin enable or disable
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_gmac_rmii_interface_sharepin(
    struct platform_device* pdev, unsigned char enable)
{
    struct device* dev = &pdev->dev;
    struct pinctrl* pinctrl;
    struct pinctrl_state* pinctrl_state;
    int ret;
    struct net_device* netdev = platform_get_drvdata(pdev);
    struct ak_gmac100* priv = netdev_priv(netdev);
    u16 data_id1;
    u16 data_id2;

    if (enable == AK_TRUE) {
        pr_info("Configed MAC RMII interface!\n");
        if (__raw_readl(AK_CHIP_VERSION_REG) == 0x53533501) {
            pinctrl = devm_pinctrl_get(dev);
            if (IS_ERR(pinctrl)) {
                dev_err(dev, "get pinctrl err!\n");
                return;
            }
            if (priv->inner_phy_mode == 1) {
                ret = ak_gmac_read_phy_reg(PHY_ID_HI_REG, &data_id1);
                if (ret) {
                    printk(
                        "phy read phy id1 waiting Error,data=0x%x\n", data_id1);
                    return;
                }
                ret = ak_gmac_read_phy_reg(PHY_ID_LOW_REG, &data_id2);
                if (ret) {
                    printk(
                        "phy read phy id2 waiting Error,data=0x%x\n", data_id2);
                    return;
                }
                if ((data_id1 == 0x0) && (data_id2 == 0x118)) { // 内置SZ18201
                    pinctrl_state
                        = pinctrl_lookup_state(pinctrl, "inner_sz18201_c_chip");
                    if (IS_ERR(pinctrl_state)) {
                        dev_err(dev, "pinctrl lookup state err!\n");
                        return;
                    }
                } else {
                    pinctrl_state
                        = pinctrl_lookup_state(pinctrl, "inner_jl1101_c_chip");
                    if (IS_ERR(pinctrl_state)) {
                        dev_err(dev, "pinctrl lookup state err!\n");
                        return;
                    }
                }
            } else {
                pinctrl_state = pinctrl_lookup_state(pinctrl, "c_chip");
                if (IS_ERR(pinctrl_state)) {
                    dev_err(dev, "pinctrl lookup state err!\n");
                    return;
                }
            }
            ret = pinctrl_select_state(pinctrl, pinctrl_state);
            if (ret) {
                dev_err(dev, "pinctrl select state err!\n");
                return;
            }
        } else {
            /* disable gpio47 pull down and set rmii sharepin */
            pinctrl_pm_select_default_state(dev);
        }
    }
    if (enable == AK_FALSE) {
        pr_info("Configed MAC RMII interface to sleep mode!\n");
        /* disable gpio47 pull down and set rmii sharepin */
        pinctrl_pm_select_sleep_state(dev);
    }
}

static void ak_gmac_phy_software_reset(gmac_device* gmacdev)
{
    s32 status;
    u16 data;
    u16 value = 0;
    u32 tcnt = 0;

    ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data);
    data |= (0x1 << 15);
    ak_gmac_write_phy_reg(PHY_CONTROL_REG, data);
    mdelay(25);
    do {
        status = ak_gmac_read_phy_reg(PHY_CONTROL_REG, &value);
        if (status) {
            continue;
        }

        if ((value & Mii_reset) == 0) {
            printk("phy software reset OK\n");
            break;
        }
        mdelay(5);
        tcnt++;
    } while (tcnt < 200);

    if (tcnt == 200) {
        printk("phy sofware reset timeout!\n");
    }
    return;
}

static void ak_gmac_phy_pwd(gmac_device* gmacdev)
{
    u16 data;
    ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data);
    /*
     * PHY_CONTROL_REG
     * [bit] 11
     * Power Down
     * RW
     * This bit turns down the power of the PHY chip, including the
     * internal crystal oscillator circuit.
     * The MDC, MDIO is still alive for accessing the MAC.
     * 1: Power down 0: Normal operation
     *
     */
    data |= (0x1 << 11);
    ak_gmac_write_phy_reg(PHY_CONTROL_REG, data);

    return;
}

/*
 * @BRIEF        set phy reset of rmii interface
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *pdev:platform device
 * @PARAM[in]    enable:set mac sharepin enable or disable
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_gmac_rmii_phy_reset(
    struct platform_device* pdev, unsigned char enable)
{
    struct device_node* np = pdev->dev.of_node;
    struct net_device* netdev = platform_get_drvdata(pdev);
    struct ak_gmac100* priv = netdev_priv(netdev);
    int gpio_index;

    if (enable == AK_TRUE) {
#ifdef CONFIG_MACH_AK3918AV100_FPGA
        gpio_index = 8;
#else
        /* get phy reset gpio and reset phy */
        gpio_index = of_get_named_gpio(np, "reset-gpios", 0);
#endif
        priv->gmacdev.gpio_index = gpio_index;
        if (gpio_index < 0) {
            printk(
                "%s, line:%d, Err, Not find phy reset-gpios,gpio_index:%d \n",
                __func__, __LINE__, gpio_index);
            ak_gmac_phy_software_reset((gmac_device*)&priv->gmacdev);
        } else {
#ifdef CONFIG_MACH_AK3918AV100_FPGA
            // gpio_direction_output(gpio_index, 1);
            void __iomem* reg = AK_VA_SUBCTRL + 0x70000;
            /* set gpio direction to output */
            unsigned long data = __raw_readl(reg);
            data |= (1 << gpio_index);
            __raw_writel(data, reg);

            /* set gpio output value to 1 */
            data = __raw_readl(reg + 0x14);
            data |= (1 << gpio_index);
            __raw_writel(data, (reg + 0x14));

            mdelay(1);
            // gpio_set_value(gpio_index, 0);

            /* set gpio output value to 0 */
            data = __raw_readl(reg + 0x14);
            data &= ~(1 << gpio_index);
            __raw_writel(data, (reg + 0x14));

            mdelay(20);
            // gpio_set_value(gpio_index, 1);

            /* set gpio output value to 1 */
            data = __raw_readl(reg + 0x14);
            data |= (1 << gpio_index);
            __raw_writel(data, (reg + 0x14));

            mdelay(200);
#else
            gpio_direction_output(gpio_index, 1);
            mdelay(1);
            gpio_set_value(gpio_index, 0);
            mdelay(20);
            gpio_set_value(gpio_index, 1);
            mdelay(200);
#endif
        }
    }
}

/*
 * @BRIEF        set phy reset and mac clock of rmii interface
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *pdev:platform device
 * @PARAM[in]    enable:set mac sharepin and phy reset enable or disable
 * @RETURN       int
 * @RETVAL       0:success, -6:failed
 * @NOTES
 */
static int ak_gmac_clk_sharepin_init(
    struct platform_device* pdev, unsigned char enable)
{
    //unsigned long flags;
    struct net_device* netdev = platform_get_drvdata(pdev);
    struct ak_gmac100* priv = netdev_priv(netdev);

#if defined(CONFIG_MACH_AK3918AV100) || defined(CONFIG_MACH_AK3918EV300L) \
    || defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130)
    if (!priv->mac_clk)
        priv->mac_clk = clk_get(&pdev->dev, "mac_gclk");
    if (!priv->phy_clk)
        priv->phy_clk = clk_get(&pdev->dev, "mac0_opclk");
#elif defined(CONFIG_MACH_AK37E) || defined(CONFIG_MACH_AK37D)                 \
    || defined(CONFIG_MACH_AK39EV330)
    if (!priv->mac_clk)
        priv->mac_clk = clk_get(&pdev->dev, NULL);
#endif

    if (priv->mac_clk == NULL) {
        printk(KERN_ERR "clk_get failed!\n");
        return -ENXIO;
    }

    //local_irq_save(flags);
    /* set rmii interface and 50m phy clock sharepin  */
    ak_gmac_rmii_interface_sharepin(pdev, enable);

    /* open 50M phy clock */
    ak_gmac_rmii_interface_50mclk(pdev, priv, enable);

    /* reset phy */
    ak_gmac_rmii_phy_reset(pdev, enable);
    //local_irq_restore(flags);

    return 0;
}

/*
 * @BRIEF        platform delay function
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    delay:delay time
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_plat_delay(unsigned int usecs)
{
    udelay(usecs);
}

/*
 * @BRIEF        This is a wrapper function for consistent dma-able Memory
 * allocation routine
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *pmnetdev:platform device
 * @PARAM[in]    size:alloc memory size
 * @PARAM[in]    mac_dma_handle:get dma memory physic address
 * @RETURN       void *
 * @RETVAL       alloc virtual address pointer of dma memory
 * @NOTES
 */
static void* ak_plat_alloc_consistent_mem(
    struct net_device* pmnetdev, u32 size, dma_addr_t* mac_dma_handle)
{
    void* pt = dma_alloc_coherent(NULL, size, mac_dma_handle, GFP_KERNEL);
    return pt;
}

/*
 * @BRIEF        This is a wrapper function for freeing consistent dma-able
 * Memory.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *pmnetdev:platform device
 * @PARAM[in]    size:free memory size
 * @PARAM[in]    *addr:free dma virtual address
 * @PARAM[in]    mac_dma_handle:free dma physic address
 * @RETURN       void *
 * @RETVAL       none
 * @NOTES
 */
static void ak_plat_free_consistent_mem(struct net_device* pmnetdev, u32 size,
    void* addr, dma_addr_t mac_dma_handle)
{
    dma_free_coherent(NULL, size, addr, mac_dma_handle);
    return;
}

static int ak_gmac_get_mdio_clock(void)
{
#if defined(CONFIG_MACH_AK3918AV100_FPGA)
    return 0;
#elif defined(CONFIG_MACH_AK3918AV100) || defined(CONFIG_MACH_AK3918EV300L) \
    || defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130)
    return GmiiCsrClk4;
#elif defined(CONFIG_MACH_AK37D) || defined(CONFIG_MACH_AK39EV330)             \
    || defined(CONFIG_MACH_AK37E)
    return GmiiCsrClk1;
#else
    return 0;
#endif
}

static void ak_gmac_pr_info(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintk_emit(0, LOGLEVEL_INFO, NULL, 0, fmt, args);
    va_end(args);
}

static void ak_gmac_pr_err(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintk_emit(0, LOGLEVEL_ERR, NULL, 0, fmt, args);
    va_end(args);
}

static ak_gmac_if_t ak_gmac_if = {
    .get_mdio_clk = ak_gmac_get_mdio_clock,
    .sys_pr_info = ak_gmac_pr_info,
    .sys_pr_err = ak_gmac_pr_err,
    .sys_udelay = ak_plat_delay,
};

/*
 * @BRIEF        Attaches the synopGMAC device structure to the hardware.
 *               Device structure is populated with MAC/DMA and PHY base
 * addresses.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *pdev:pointer to platform device
 * @PARAM[in]    *gmacdev:pointer to gmac_device to populate mac dma and phy
 * addresses.
 * @PARAM[in]    macBase:GMAC IP mac base address.
 * @PARAM[in]    dmaBase:GMAC IP dma base address.
 * @PARAM[in]    phyBase:GMAC IP phy base address.
 * @RETURN       int
 * @RETVAL       return 0 upon success. Error code upon failure.
 * @NOTES        This is important function. No kernel api provided by Synopsys
 */
static int ak_gmac_attach(struct platform_device* pdev, gmac_device* gmacdev,
    u32 macBase, u32 dmaBase, u32 phyBase)
{
    /* make sure the device data strucure is cleared before we proceed further*/
    memset((void*)gmacdev, 0, sizeof(gmac_device));

    /* populate the mac and dma base addresses */
    gmacdev->MacBase = macBase;
    gmacdev->DmaBase = dmaBase;
    gmacdev->PhyBase = phyBase;

    ak_gmac_if.MacBase = macBase;
    ak_gmac_if.DmaBase = dmaBase;
    ak_gmac_if.PhyBase = phyBase;
    return 0;
}

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
static int ak_gmac_read_version(gmac_device* gmacdev)
{
    u32 data = 0;

    /* available the mac ip version */
    data = ak_gmac_get_version();
    gmacdev->Version = data;

    // printk(KERN_ERR"gmac version data read from reg:0x%08x,value:0x%08x\n",
    // (gmacdev->MacBase + GmacVersion), data);

    return 0;
}

/*
 * @BRIEF        power up mac hardware.
 * @AUTHOR       cao_donghua
 * @DATE     2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_powerup_mac(struct net_device* netdev, gmac_device* gmacdev)
{
    struct ak_gmac100* priv = netdev_priv(netdev);

    if (priv->mac_id == 0) {
        GMAC0_Power_down = 0;
    } else if (priv->mac_id == 1) {
        GMAC1_Power_down = 0;
    }

    if (ak_gmac_is_magic_packet_received())
        printk(KERN_ERR "GMAC wokeup due to Magic Pkt Received\n");

    if (ak_gmac_is_wakeup_frame_received())
        printk(KERN_ERR "GMAC wokeup due to Wakeup Frame Received\n");

    /* Disable the assertion of PMT interrupt */
    ak_gmac_pmt_int_disable();

    /* Enable the mac and Dma rx and tx paths */
    ak_gmac_rx_enable();
    ak_gmac_enable_dma_rx();

    ak_gmac_tx_enable();
    ak_gmac_enable_dma_tx();

    return;
}

/*
 * @BRIEF        Take ownership of all the rx Descriptors.
 *               This function is called when there is fatal error in DMA
 * transmission. When called it takes the ownership of all the rx descriptor in
 * rx descriptor pool/queue from DMA. The function is same for both the ring
 * mode and the chain mode DMA structures.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_gmac_take_desc_ownership_rx(gmac_device* gmacdev)
{
    s32 i;
    DmaDesc* desc;
    desc = gmacdev->RxDesc;

    for (i = 0; i < gmacdev->RxDescCount; i++) {
        if (ak_gmac_is_rx_desc_chained(desc)) {
            ak_gmac_take_desc_ownership(desc);
            desc = (DmaDesc*)desc->data2;
        } else {
            ak_gmac_take_desc_ownership(desc + i);
        }
    }
}

/*
 * @BRIEF        Take ownership of all the tx Descriptors.
 *               This function is called when there is fatal error in DMA
 * transmission. When called it takes the ownership of all the tx descriptor in
 * tx descriptor pool/queue from DMA. The function is same for both the ring
 * mode and the chain mode DMA structures.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_gmac_take_desc_ownership_tx(gmac_device* gmacdev)
{
    s32 i;
    DmaDesc* desc;
    desc = gmacdev->TxDesc;

    for (i = 0; i < gmacdev->TxDescCount; i++) {
        if (ak_gmac_is_tx_desc_chained(desc)) {
            ak_gmac_take_desc_ownership(desc);
            desc = (DmaDesc*)desc->data2;
        } else {
            ak_gmac_take_desc_ownership(desc + i);
        }
    }
}

/*
 * @BRIEF        Initialize the tx and rx descriptors queue.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @RETURN       s32
 * @RETVAL       return all 0
 * @NOTES
 */
static s32 ak_gmac_init_tx_rx_desc_queue(gmac_device* gmacdev)
{
    s32 i;

    for (i = 0; i < gmacdev->TxDescCount; i++) {
        ak_gmac_tx_desc_init_ring(
            gmacdev->TxDesc + i, i == gmacdev->TxDescCount - 1);
    }

    for (i = 0; i < gmacdev->RxDescCount; i++) {
        ak_gmac_rx_desc_init_ring(
            gmacdev->RxDesc + i, i == gmacdev->RxDescCount - 1);
    }

    gmacdev->TxNext = 0;
    gmacdev->TxBusy = 0;
    gmacdev->RxNext = 0;
    gmacdev->RxBusy = 0;

    return -ESYNOPGMACNOERR;
}

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
static void ak_gmac_init_rx_desc_base(gmac_device* gmacdev)
{
    //u32 rx_desc_addr;

    if (gmacdev->RxDescDma) {
        ak_gmac_set_rx_desc_base((u32)gmacdev->RxDescDma);
        //rx_desc_addr = ak_gmac_read_reg(gmacdev->DmaBase, DmaRxBaseAddr);
    }
    //return;
}

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
static void ak_gmac_init_tx_desc_base(gmac_device* gmacdev)
{
    if (gmacdev->TxDescDma)
        ak_gmac_set_tx_desc_base((u32)gmacdev->TxDescDma);
}

/*
 * @BRIEF        Example mac initialization sequence.
 *               This function calls the initialization routines to initialize
 * the GMAC register. One can change the functions invoked here to have
 * different configuration as per the requirement
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static int ak_gmac_mac_init(gmac_device* gmacdev)
{
    u32 PHYreg;
    struct ak_gmac100* priv = container_of(gmacdev, struct ak_gmac100, gmacdev);
    struct net_device* netdev = priv->netdev;

    if (gmacdev->DuplexMode == FULLDUPLEX) {
        ak_gmac_wd_enable();
        ak_gmac_jab_enable();
        ak_gmac_frame_burst_enable();
        ak_gmac_jumbo_frame_disable();
        ak_gmac_rx_own_enable();
        ak_gmac_loopback_off();
        ak_gmac_set_full_duplex();
        ak_gmac_retry_enable();
        ak_gmac_pad_crc_strip_disable();
        ak_gmac_back_off_limit(GmacBackoffLimit0);
        ak_gmac_deferral_check_disable();
        ak_gmac_tx_enable(); /* CDH:TX Enable */
        ak_gmac_rx_enable(); /* CDH:RX Enable */

        if (gmacdev->Speed == SPEED1000)
            ak_gmac_select_gmii();
        else
            ak_gmac_select_mii(); /* cdh:we use mii*/

        /* Frame Filter Configuration */
        ak_gmac_frame_filter_enable();
        ak_gmac_set_pass_control(GmacPassControl0);
        ak_gmac_broadcast_enable();
        ak_gmac_src_addr_filter_disable();
        ak_gmac_multicast_enable();
        ak_gmac_dst_addr_filter_normal();
        ak_gmac_multicast_hash_filter_enable();
        if ((netdev->flags & IFF_PROMISC) && (netdev->flags & IFF_UP)) {
            ak_gmac_promisc_enable();
        } else {
            ak_gmac_promisc_disable();
        }
        ak_gmac_unicast_hash_filter_disable();

        /* Flow Control Configuration */
        ak_gmac_unicast_pause_frame_detect_disable();
        ak_gmac_rx_flow_control_enable();
        ak_gmac_tx_flow_control_enable();
    } else {
        ak_gmac_wd_enable();
        ak_gmac_jab_enable();
        ak_gmac_frame_burst_enable();
        ak_gmac_jumbo_frame_disable();
        ak_gmac_rx_own_enable();
        ak_gmac_loopback_off();
        ak_gmac_set_half_duplex();
        ak_gmac_retry_enable();
        ak_gmac_pad_crc_strip_disable();
        ak_gmac_back_off_limit(GmacBackoffLimit0);
        ak_gmac_deferral_check_disable();
        ak_gmac_tx_enable();
        ak_gmac_rx_enable();

        if (gmacdev->Speed == SPEED1000)
            ak_gmac_select_gmii();
        else
            ak_gmac_select_mii();

        /* Frame Filter Configuration */
        ak_gmac_frame_filter_enable();
        ak_gmac_set_pass_control(GmacPassControl0);
        ak_gmac_broadcast_enable();
        ak_gmac_src_addr_filter_disable();
        ak_gmac_multicast_disable();
        ak_gmac_dst_addr_filter_normal();
        ak_gmac_multicast_hash_filter_disable();
        if ((netdev->flags & IFF_PROMISC) && (netdev->flags & IFF_UP)) {
            ak_gmac_promisc_enable();
        } else {
            ak_gmac_promisc_disable();
        }
        ak_gmac_unicast_hash_filter_disable();

        /* Flow Control Configuration */
        ak_gmac_unicast_pause_frame_detect_disable();
        ak_gmac_rx_flow_control_disable();
        ak_gmac_tx_flow_control_disable();

        /* To set PHY register to enable CRS on Transmit */
        ak_gmac_write_reg(
            gmacdev->MacBase, GmacGmiiAddr, GmiiBusy | 0x00000408);
        PHYreg = ak_gmac_read_reg(gmacdev->MacBase, GmacGmiiData);
        ak_gmac_write_reg(
            gmacdev->MacBase, GmacGmiiData, PHYreg | 0x00000800);
        ak_gmac_write_reg(
            gmacdev->MacBase, GmacGmiiAddr, GmiiBusy | 0x0000040a);
    }

    return 0;
}

/*
 * @BRIEF        Checks whether this rx descriptor is last rx descriptor.
 *               This returns true if it is last descriptor either in ring mode
 * or in chain mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to devic structure
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @RETURN       bool
 * @RETVAL       returns true if it is last descriptor, false if not.
 * @NOTES        This function should not be called before initializing the
 * descriptor using synopGMAC_desc_init().
 */
static bool ak_gmac_is_last_rx_desc(gmac_device* gmacdev, DmaDesc* desc)
{
    return (((desc->length & RxDescEndOfRing) == RxDescEndOfRing)
        || ((u32)gmacdev->RxDesc == desc->data2));
}

/*
 * @BRIEF        Get back the descriptor from DMA after data has been received.
 *               When the DMA indicates that the data is received (interrupt is
 * generated), this function should be called to get the descriptor and hence
 * the data buffers received. With successful return from this function caller
 * gets the descriptor fields for processing. check the parameters to understand
 * the fields returned.`
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[out]   *Status:pointer to hold the status of DMA.
 * @PARAM[out]   *Buffer1:Dma-able buffer1 pointer.
 * @PARAM[out]   *Length1:pointer to hold length of buffer1 (Max is 2048).
 * @PARAM[out]   *Data1:virtual pointer for buffer1.
 * @PARAM[out]   *Buffer2:Dma-able buffer2 pointer.
 * @PARAM[out]   *Length2:pointer to hold length of buffer2 (Max is 2048).
 * @PARAM[out]   *Data2:virtual pointer for buffer2.
 * @RETURN       s32
 * @RETVAL       if success, pointer to rx next descriptor else -1
 * @NOTES
 */
static s32 ak_gmac_get_rx_qptr(gmac_device* gmacdev, u32* Status, u32* Buffer1,
    u32* Length1, u32* Data1, u32* Buffer2, u32* Length2, u32* Data2)
{
    u32 rxnext = gmacdev->RxBusy;
    DmaDesc* rxdesc = gmacdev->RxBusyDesc;

    if (ak_gmac_is_desc_owned_by_dma(rxdesc))
        return -1;
    if (ak_gmac_is_desc_empty(rxdesc))
        return -1;

    if (Status != 0)
        *Status = rxdesc->status; // send the status of this descriptor

    if (Length1 != 0)
        *Length1 = (rxdesc->length & DescSize1Mask) >> DescSize1Shift;
    if (Buffer1 != 0)
        *Buffer1 = rxdesc->buffer1;
    if (Data1 != 0)
        *Data1 = rxdesc->data1;

    if (Length2 != 0)
        *Length2 = (rxdesc->length & DescSize2Mask) >> DescSize2Shift;
    if (Buffer2 != 0)
        *Buffer2 = rxdesc->buffer2;
    if (Data1 != 0)
        *Data2 = rxdesc->data2;

    gmacdev->RxBusy = ak_gmac_is_last_rx_desc(gmacdev, rxdesc) ? 0 : rxnext + 1;

    if (ak_gmac_is_rx_desc_chained(rxdesc)) {
        gmacdev->RxBusyDesc = (DmaDesc*)rxdesc->data2;
        ak_gmac_rx_desc_init_chain(rxdesc);
    } else {
        gmacdev->RxBusyDesc = ak_gmac_is_last_rx_desc(gmacdev, rxdesc)
            ? gmacdev->RxDesc
            : (rxdesc + 1);
        ak_gmac_rx_desc_init_ring(
            rxdesc, ak_gmac_is_last_rx_desc(gmacdev, rxdesc));
    }

    (gmacdev->BusyRxDesc)--; // This returns one descriptor to processor. So
                             // busy count will be decremented by one

    return (rxnext);
}

/*
 * @BRIEF        Prepares the descriptor to receive packets.
 *               The descriptor is allocated with the valid buffer addresses
 * (sk_buff address) and the length fields and handed over to DMA by setting the
 * ownership. After successful return from this function the descriptor is added
 * to the receive descriptor pool/queue. This api is same for both ring mode and
 * chain mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    Buffer1:Dma-able buffer1 pointer.
 * @PARAM[in]    Length1:length of buffer1 (Max is 2048).
 * @PARAM[in]    Data1:u32 data indicating whether the descriptor is in ring
 * mode or chain mode.
 * @PARAM[in]    Buffer2:Dma-able buffer2 pointer.
 * @PARAM[in]    Length2:length of buffer2 (Max is 2048).
 * @PARAM[in]    Data2:u32 data indicating whether the descriptor is in ring
 * mode or chain mode.
 * @RETURN       s32
 * @RETVAL       returns present rx descriptor index on success. Negative value
 * if error.
 * @NOTES
 */
static s32 ak_gmac_set_rx_qptr(gmac_device* gmacdev, u32 Buffer1, u32 Length1,
    u32 Data1, u32 Buffer2, u32 Length2, u32 Data2)
{
    s32 rxnext = gmacdev->RxNext;
    DmaDesc* rxdesc = gmacdev->RxNextDesc;

    if (!ak_gmac_is_desc_empty(rxdesc))
        return -1;

    if (ak_gmac_is_rx_desc_chained(rxdesc)) {
        rxdesc->length |= ((Length1 << DescSize1Shift) & DescSize1Mask);
        rxdesc->buffer1 = Buffer1; // cdh:rx data address
        rxdesc->data1 = Data1;

        if ((rxnext % MODULO_INTERRUPT) != 0)
            rxdesc->length |= RxDisIntCompl;

        rxdesc->status = DescOwnByDma;

        gmacdev->RxNext
            = ak_gmac_is_last_rx_desc(gmacdev, rxdesc) ? 0 : rxnext + 1;
        gmacdev->RxNextDesc = (DmaDesc*)rxdesc->data2;
    } else {
        rxdesc->length |= (((Length1 << DescSize1Shift) & DescSize1Mask)
            | ((Length2 << DescSize2Shift) & DescSize2Mask));

        rxdesc->buffer1 = Buffer1;
        rxdesc->data1 = Data1;

        rxdesc->buffer2 = Buffer2;
        rxdesc->data2 = Data2;

        if ((rxnext % MODULO_INTERRUPT) != 0)
            rxdesc->length |= RxDisIntCompl;

        rxdesc->status = DescOwnByDma;

        gmacdev->RxNext
            = ak_gmac_is_last_rx_desc(gmacdev, rxdesc) ? 0 : rxnext + 1;
        gmacdev->RxNextDesc = ak_gmac_is_last_rx_desc(gmacdev, rxdesc)
            ? gmacdev->RxDesc
            : (rxdesc + 1);
    }

    (gmacdev->BusyRxDesc)++; // One descriptor will be given to Hardware. So
                             // busy count incremented by one
    return rxnext;
}

/*
 * @BRIEF        Checks whether this tx descriptor is last tx descriptor.
 *               This returns true if it is last descriptor either in ring mode
 * or in chain mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to devic structure.
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @RETURN       bool
 * @RETVAL       returns true if it is last descriptor, false if not.
 * @NOTES        This function should not be called before initializing the
 * descriptor using synopGMAC_desc_init().
 */
static bool ak_gmac_is_last_tx_desc(gmac_device* gmacdev, DmaDesc* desc)
{
    return (((desc->length & TxDescEndOfRing) == TxDescEndOfRing)
        || ((u32)gmacdev->TxDesc == desc->data2));
}

/*
 * @BRIEF        Get the index and address of Tx desc.
 *               This api is same for both ring mode and chain mode.
 *               This function tracks the tx descriptor the DMA just closed
 * after the transmission of data from this descriptor is over. This returns the
 * descriptor fields to the caller.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[out]   *Status:
 * @PARAM[out]   *Buffer1:
 * @PARAM[out]   *Length1:
 * @PARAM[out]   *Data1:
 * @PARAM[out]   *Buffer2:
 * @PARAM[out]   *Length2:
 * @PARAM[out]   *Data2:
 * @RETURN       s32
 * @RETVAL       returns present tx descriptor index on success. Negative value
 * if error.
 * @NOTES
 */
static s32 ak_gmac_get_tx_qptr(gmac_device* gmacdev, u32* Status, u32* Buffer1,
    u32* Length1, u32* Data1, u32* Buffer2, u32* Length2, u32* Data2)
{
    u32 txover = gmacdev->TxBusy;
    DmaDesc* txdesc = gmacdev->TxBusyDesc;

    if (ak_gmac_is_desc_owned_by_dma(txdesc))
        return -1;
    if (ak_gmac_is_desc_empty(txdesc))
        return -1;

    (gmacdev->BusyTxDesc)--;

    if (Status != 0)
        *Status = txdesc->status;

    if (Buffer1 != 0)
        *Buffer1 = txdesc->buffer1;
    if (Length1 != 0)
        *Length1 = (txdesc->length & DescSize1Mask) >> DescSize1Shift;
    if (Data1 != 0)
        *Data1 = txdesc->data1;

    if (Buffer2 != 0)
        *Buffer2 = txdesc->buffer2;
    if (Length2 != 0)
        *Length2 = (txdesc->length & DescSize2Mask) >> DescSize2Shift;
    if (Data1 != 0)
        *Data2 = txdesc->data2;

    gmacdev->TxBusy = ak_gmac_is_last_tx_desc(gmacdev, txdesc) ? 0 : txover + 1;

    if (ak_gmac_is_tx_desc_chained(txdesc)) {
        gmacdev->TxBusyDesc = (DmaDesc*)txdesc->data2;
        ak_gmac_tx_desc_init_chain(txdesc);
    } else {
        gmacdev->TxBusyDesc = ak_gmac_is_last_tx_desc(gmacdev, txdesc)
            ? gmacdev->TxDesc
            : (txdesc + 1);
        ak_gmac_tx_desc_init_ring(
            txdesc, ak_gmac_is_last_tx_desc(gmacdev, txdesc));
    }

    return txover;
}

/*
 * @BRIEF        Function to Receive a packet from the interface.
 *               After Receiving a packet, DMA transfers the received packet to
 * the system memory and generates corresponding interrupt (if it is enabled).
 * This function prepares the sk_buff for received packet after removing the
 * ethernet CRC, and hands it over to linux networking stack.
 *               - Updataes the networking interface statistics
 *               - Keeps track of the rx descriptors
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device structure.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES        This function runs in interrupt context.
 */
static void ak_gmac_handle_received_data(struct net_device* netdev)
{
    s32 desc_index;
    u32 data1;
    u32 data2;
    u32 len;
    u32 status;
    u32 dma_addr1;
    u32 dma_addr2;
    struct sk_buff* skb; // This is the pointer to hold the received data
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev;

    gmacdev = (gmac_device*)&priv->gmacdev;

    /*Handle the Receive Descriptors*/
    do {
        desc_index = ak_gmac_get_rx_qptr(gmacdev, &status, &dma_addr1, NULL,
            &data1, &dma_addr2, NULL, &data2);
        if (desc_index >= 0 && data1 != 0) {
            dma_unmap_single(NULL, dma_addr1, 0, DMA_FROM_DEVICE);
            skb = (struct sk_buff*)data1;
            if (ak_gmac_is_rx_desc_valid(status)) {
                len = ak_gmac_get_rx_desc_frame_length(status) - 4;
                skb_put(skb, len);

#ifdef IPC_OFFLOAD
                skb->ip_summed = CHECKSUM_NONE;
                if (ak_gmac_is_rx_checksum_error(status)
                    == RxNoChkError) { // IPv4/IPv6 Type frame, no checksum
                                       // error detected
                    skb->ip_summed
                        = CHECKSUM_UNNECESSARY; // Let Kernel bypass computing
                                                // the Checksum
                }

                if (ak_gmac_is_rx_checksum_error(status)
                    == RxIpHdrChkError) { // IPv4/IPv6 Type frame with an IP
                                          // header checksum error detected
                    skb->ip_summed
                        = CHECKSUM_UNNECESSARY; // Let Kernel bypass the TCP/UDP
                                                // checksum computation, this
                                                // packet will be discard by MAC
                                                // or ip_rcv()
                }

                if (ak_gmac_is_rx_checksum_error(status)
                    == RxLenLT600) { // IEEE 802.3 Type frame (Length field
                                     // value is less than 0x0600)
                    skb->ip_summed
                        = CHECKSUM_NONE; // IEEE 802.3 frame, let Kernel compute
                                         // the Checksum
                }

                if (ak_gmac_is_rx_checksum_error(status)
                    == RxIpHdrPayLoadChkBypass) { // IPv4/IPv6 Type frame with
                                                  // no IP header checksum error
                                                  // and the payload check
                                                  // bypassed, due to an
                                                  // unsupported payload
                    skb->ip_summed
                        = CHECKSUM_NONE; // Let Kernel compute the Checksum
                }

                if (ak_gmac_is_rx_checksum_error(status)
                    == RxChkBypass) { // A Type frame that is neither IPv4 or
                                      // IPv6 (the Checksum Offload engine
                                      // bypasses checksum completely)
                    skb->ip_summed
                        = CHECKSUM_NONE; // Let Kernel compute the Checksum
                }

                if (ak_gmac_is_rx_checksum_error(status)
                    == RxPayLoadChkError) { // IPv4/IPv6 Type frame with a
                                            // payload checksum error detected
                    skb->ip_summed
                        = CHECKSUM_NONE; // Let Kernel compute the Checksum
                }

                if (ak_gmac_is_rx_checksum_error(status)
                    == RxIpHdrPayLoadChkError) { // IPv4/IPv6 Type frame with
                                                 // both IP header and payload
                                                 // checksum errors detected
                    skb->ip_summed
                        = CHECKSUM_NONE; // Let Kernel compute the Checksum
                }
#endif

                skb->dev = netdev;
                skb->protocol = eth_type_trans(skb, netdev);

                if (broadcast_enable_per_ms > 0) {
                    if (skb->pkt_type == PACKET_BROADCAST) {
                        broadcast_count[priv->mac_id]++;
                        if (skb->protocol == ETH_P_ARP) {
                            broadcast_arp_count[priv->mac_id]++;
                        } else if (skb->protocol == ETH_P_IP) {
                            broadcast_ip_count[priv->mac_id]++;
                        }
                    } else if (skb->pkt_type == PACKET_MULTICAST) {
                        multicast_count[priv->mac_id]++;
                    } else if (skb->pkt_type == PACKET_OTHERHOST) {
                        otherhost_count[priv->mac_id]++;
                    } else {
                        host_count[priv->mac_id]++;
                    }

                    if ((skb->pkt_type == PACKET_BROADCAST)
                            && (broadcast_count[priv->mac_id] >
                                    disable_broadcast_count)) {
                        //dev_kfree_skb_irq(skb);
                        //netdev->stats.rx_dropped++;
                        pr_debug("eth%d broadcast_count %d > %d disable\n",
                                priv->mac_id,
                                broadcast_count[priv->mac_id],
                                disable_broadcast_count);
                        ak_gmac_broadcast_disable();
                    } else if ((skb->pkt_type == PACKET_MULTICAST)
                            && (multicast_count[priv->mac_id] >
                                    disable_multicast_count)) {
                        //dev_kfree_skb_irq(skb);
                        //netdev->stats.rx_dropped++;
                        pr_debug("eth%d multicast_count %d > %d disable\n",
                                priv->mac_id,
                                multicast_count[priv->mac_id],
                                disable_multicast_count);
                        ak_gmac_multicast_disable();
                    }
                }

                netif_rx(skb); // printk("netif_rx end!\n");

                netdev->last_rx = jiffies;
                netdev->stats.rx_packets++;
                netdev->stats.rx_bytes += len;
            } else {
                /*Now the present skb should be set free*/
                dev_kfree_skb_irq(skb);
                netdev->stats.rx_errors++;
                netdev->stats.collisions
                    += ak_gmac_is_rx_frame_collision(status);
                netdev->stats.rx_crc_errors += ak_gmac_is_rx_crc(status);
                netdev->stats.rx_frame_errors
                    += ak_gmac_is_frame_dribbling_errors(status);
                netdev->stats.rx_length_errors
                    += ak_gmac_is_rx_frame_length_errors(status);
            }

            skb = netdev_alloc_skb(netdev,
                    ak_mac_get_packet_mtu(netdev) + ETHERNET_PACKET_EXTRA);
            if (skb == NULL) {
                printk(KERN_ERR "skb memory allocation failed \n");
                netdev->stats.rx_dropped++;
                continue;
            }

            dma_addr1 = dma_map_single(
                NULL, skb->data, skb_tailroom(skb), DMA_FROM_DEVICE);
            desc_index = ak_gmac_set_rx_qptr(
                gmacdev, dma_addr1, skb_tailroom(skb), (u32)skb, 0, 0, 0);
            if (desc_index < 0) {
                dev_kfree_skb_irq(skb);
            }
        }
    } while (desc_index >= 0);
}
/*end of ak_gmac_handle_received_data*/

/*
 * @BRIEF        Function to handle housekeeping after a packet is transmitted
 * over the wire. After the transmission of a packet DMA generates corresponding
 * interrupt (if it is enabled). It takes care of returning the sk_buff to the
 * linux kernel, updating the networking statistics and tracking the
 * descriptors.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net device struct
 * @RETURN       void
 * @RETVAL       none
 * @NOTES        This function runs in interrupt context
 */
void ak_gmac_handle_transmit_over(struct net_device* netdev)
{
    s32 desc_index;
    u32 data1, data2;
    u32 status;
    u32 length1, length2;
    u32 dma_addr1, dma_addr2;
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev = (gmac_device*)&priv->gmacdev;

    /*Handle the transmit Descriptors*/
    do {

        desc_index = ak_gmac_get_tx_qptr(gmacdev, &status, &dma_addr1, &length1,
            &data1, &dma_addr2, &length2, &data2);
        if (desc_index >= 0 && data1 != 0) {
#ifdef IPC_OFFLOAD
            if (ak_gmac_is_tx_ipv4header_checksum_error(status)) {
                printk(
                    KERN_ERR "Harware Failed to Insert IPV4 Header Checksum\n");
            }

            if (ak_gmac_is_tx_payload_checksum_error(status)) {
                printk(KERN_ERR "Harware Failed to Insert Payload Checksum\n");
            }
#endif

            dma_unmap_single(NULL, dma_addr1, length1, DMA_TO_DEVICE);
            dev_kfree_skb_irq((struct sk_buff*)data1);
            if (ak_gmac_is_desc_valid(status)) {
                netdev->stats.tx_bytes += length1;
                netdev->stats.tx_packets++;
            } else {
                netdev->stats.tx_errors++;
                netdev->stats.tx_aborted_errors
                    += ak_gmac_is_tx_aborted(status);
                netdev->stats.tx_carrier_errors
                    += ak_gmac_is_tx_carrier_error(status);
            }
        }
        netdev->stats.collisions += ak_gmac_get_tx_collision_count(status);
    } while (desc_index >= 0);
    netif_wake_queue(netdev);
}

/*
 * @BRIEF        Interrupt service routing bottom half.
 *               This is the function registered as ISR for device interrupts.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    intr_num:interrupt number.
 * @PARAM[in]    *dev_id:void pointer to device unique structure (Required for
 * shared interrupts in Linux).
 * @RETURN       irqreturn_t
 * @RETVAL       Returns IRQ_NONE if not device interrupts IRQ_HANDLED for
 * device interrupts.
 * @NOTES        This function runs in interrupt context
 */
irqreturn_t ak_gmac_intr_handler(s32 intr_num, void* dev_id)
{
    u32 interrupt;
    u32 dma_status_reg;
    s32 status;
    u32 dma_addr;
    gmac_device* gmacdev;
    struct ak_gmac100* priv;
    struct net_device* netdev;

    netdev = (struct net_device*)dev_id;
    if (netdev == NULL) {
        printk(KERN_ERR "Unknown Device\n");
        return -1;
    }

    priv = (struct ak_gmac100*)netdev_priv(netdev);
    gmacdev = (gmac_device*)&priv->gmacdev;

    /* Read the Dma interrupt status to know whether the interrupt got generated
     * by our device or not */
    dma_status_reg = ak_gmac_read_reg(gmacdev->DmaBase, DmaStatus);
    if (dma_status_reg == 0)
        return IRQ_NONE;

    ak_gmac_disable_interrupt_all();

    /* power managent module(PMT) */
    if (dma_status_reg & GmacPmtIntr) {
        ak_powerup_mac(netdev, gmacdev);
    }

    /* check mmc interrupt */
    if (dma_status_reg & GmacMmcIntr) {
        printk(KERN_ERR "%s:: Interrupt due to MMC module\n", __FUNCTION__);
        printk(KERN_ERR "%s:: ak_gmac_rx_int_status = %08x\n", __FUNCTION__,
            ak_gmac_read_mmc_rx_int_status());
        printk(KERN_ERR "%s:: ak_gmac_tx_int_status = %08x\n", __FUNCTION__,
            ak_gmac_read_mmc_tx_int_status());
    }

    /* check pcs(link change and an complete) */
    if (dma_status_reg & GmacLineIntfIntr) {
        printk(
            KERN_ERR "%s:: Interrupt due to GMAC LINE module\n", __FUNCTION__);
    }

    /*Now lets handle the DMA interrupts*/
    interrupt = ak_gmac_get_interrupt_type();
    if (interrupt & synopGMACDmaError) {
        ak_gmac_disable_dma_tx();
        ak_gmac_disable_dma_rx();

        ak_gmac_take_desc_ownership_tx(gmacdev);
        ak_gmac_take_desc_ownership_rx(gmacdev);

        ak_gmac_init_tx_rx_desc_queue(gmacdev);

        ak_gmac_reset(); // reset the DMA engine and the GMAC ip

        ak_gmac_set_mac_addr(
            GmacAddr0High, GmacAddr0Low, netdev->dev_addr);
        ak_gmac_set_dma_bus_mode(
            DmaFixedBurstEnable | DmaBurstLength8 | DmaDescriptorSkip2);
        ak_gmac_set_dma_control(DmaStoreAndForward);
        ak_gmac_init_rx_desc_base(gmacdev);
        ak_gmac_init_tx_desc_base(gmacdev);
        ak_gmac_mac_init(gmacdev);
        // ak_gmac_promisc_enable(gmacdev);
        ak_gmac_enable_dma_rx();
        ak_gmac_enable_dma_tx();
    }

    if (interrupt & synopGMACDmaRxNormal) {
#ifdef SMALL_TXDONE_INT
        ak_gmac_handle_transmit_over(
            netdev); // Do whatever you want after the transmission is over
#endif
        ak_gmac_handle_received_data(netdev);
    }

    if (interrupt & synopGMACDmaRxAbnormal) {
        if (((GMAC0_Power_down == 0) && (priv->mac_id == 0))
            || ((GMAC1_Power_down == 0) && (priv->mac_id == 1))) {
            netdev->stats.rx_over_errors++;
            /*Now Descriptors have been created in synop_handle_received_data().
             * Just issue a poll demand to resume DMA operation*/
            ak_gmac_resume_dma_rx(); // To handle GBPS with 12
                                            // descriptors
        }
    }

    if (interrupt & synopGMACDmaRxStopped) {
        if (((GMAC0_Power_down == 0) && (priv->mac_id == 0))
            || ((GMAC1_Power_down == 0) && (priv->mac_id == 1))) {
            netdev->stats.rx_over_errors++;

            do {
                struct sk_buff* skb = alloc_skb(
                        ak_mac_get_packet_mtu(netdev)+ETHERNET_HEADER+ETHERNET_CRC,
                        GFP_ATOMIC);
                if (skb == NULL) {
                    printk(KERN_ERR "%s::ERROR in skb buffer allocation Better "
                                    "Luck Next time\n",
                        __FUNCTION__);
                    break;
                }

                dma_addr = dma_map_single(
                    NULL, skb->data, skb_tailroom(skb), DMA_FROM_DEVICE);
                status = ak_gmac_set_rx_qptr(
                    gmacdev, dma_addr, skb_tailroom(skb), (u32)skb, 0, 0, 0);
                printk(KERN_ERR "%s::Set Rx Descriptor no %08x for skb %08x \n",
                    __FUNCTION__, status, (u32)skb);
                if (status < 0)
                    dev_kfree_skb_irq(skb); // changed from dev_free_skb. If
                                            // problem check this again

            } while (status >= 0);
            ak_gmac_enable_dma_rx();
        }
    }

    if (interrupt & synopGMACDmaTxNormal) {
        ak_gmac_handle_transmit_over(
            netdev); // Do whatever you want after the transmission is over
    }

    if (interrupt & synopGMACDmaTxAbnormal) {
        if (((GMAC0_Power_down == 0) && (priv->mac_id == 0))
            || ((GMAC1_Power_down == 0) && (priv->mac_id == 1))) {
            ak_gmac_handle_transmit_over(netdev);
        }
    }

    if (interrupt & synopGMACDmaTxStopped) {
        if (((GMAC0_Power_down == 0) && (priv->mac_id == 0))
            || ((GMAC1_Power_down == 0) && (priv->mac_id == 1))) {
            ak_gmac_disable_dma_tx();
            ak_gmac_take_desc_ownership_tx(gmacdev);

            ak_gmac_enable_dma_tx();
            netif_wake_queue(netdev);
        }
    }

    /* Enable the interrrupt before returning from ISR*/
    ak_gmac_enable_interrupt(DmaIntEnable);
    return IRQ_HANDLED;
}
/*end of ak_gmac_intr_handler*/

/*
 * @BRIEF        This sets up the transmit Descriptor queue in ring or chain
 * mode. This function is tightly coupled to the platform and operating system
 *               Device is interested only after the descriptors are setup.
 * Therefore this function is not included in the device driver API. This
 * function should be treated as an example code to design the descriptor
 * structures for ring mode or chain mode. This function depends on the pcidev
 * structure for allocation consistent dma-able memory in case of linux. This
 * limitation is due to the fact that linux uses pci structure to allocate a
 * dmable memory
 *               - Allocates the memory for the descriptors.
 *               - Initialize the Busy and Next descriptors indices to
 * 0(Indicating first descriptor).
 *               - Initialize the Busy and Next descriptors to first descriptor
 * address.
 *               - Initialize the last descriptor with the endof ring in case of
 * ring mode.
 *               - Initialize the descriptors in chain mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:interrupt number.
 * @PARAM[in]    *pnetdev:pointer to pci_device structure.
 * @PARAM[in]    no_of_desc:number of descriptor expected in tx descriptor
 * queue.
 * @PARAM[in]    desc_mode:whether descriptors to be created in RING mode or
 * CHAIN mode.
 * @RETURN       int
 * @RETVAL       Rreturn 0 upon success. Error code upon failure.
 * @NOTES        his function fails if allocation fails for required number of
 * descriptors in Ring mode, but in chain mode function returns -ESYNOPGMACNOMEM
 * in the process of descriptor chain creation. once returned from this function
 *               user should for gmacdev->TxDescCount to see how many
 * descriptors are there in the chain. Should continue further only if the
 * number of descriptors in the chain meets the requirements
 */
static int ak_setup_tx_desc_queue(gmac_device* gmacdev,
    struct net_device* pnetdev, u32 no_of_desc, u32 desc_mode)
{
    s32 i;

    DmaDesc* first_desc = NULL;
    DmaDesc* second_desc = NULL;
    dma_addr_t dma_addr = 0;
    gmacdev->TxDescCount = 0;

    if (desc_mode == RINGMODE) {
        // printk("Total size of memory required for Tx Descriptors in Ring Mode
        // = 0x%08x\n", ((sizeof(DmaDesc) * no_of_desc)));
        first_desc = (DmaDesc*)ak_plat_alloc_consistent_mem(
            pnetdev, sizeof(DmaDesc) * no_of_desc, &dma_addr);
        if (first_desc == NULL) {
            printk(KERN_ERR "Error in Tx Descriptors memory allocation\n");
            return -ESYNOPGMACNOMEM;
        }
        gmacdev->TxDescCount = no_of_desc; // cdh:ring descriptor count
        gmacdev->TxDesc
            = first_desc; // cdh:ring descriptor base virtual address
        gmacdev->TxDescDma
            = dma_addr; // cdh:ring descriptor base physic address

        for (i = 0; i < gmacdev->TxDescCount; i++) {
            ak_gmac_tx_desc_init_ring(
                gmacdev->TxDesc + i, i == gmacdev->TxDescCount - 1);
        }
    } else {
        // Allocate the head descriptor
        first_desc = (DmaDesc*)ak_plat_alloc_consistent_mem(
            pnetdev, sizeof(DmaDesc), &dma_addr);
        if (first_desc == NULL) {
            printk(KERN_ERR
                "Error in Tx Descriptor Memory allocation in Ring mode\n");
            return -ESYNOPGMACNOMEM;
        }
        gmacdev->TxDesc
            = first_desc; // cdh:ring descriptor base virtual address
        gmacdev->TxDescDma
            = dma_addr; // cdh:ring descriptor base physic address
        first_desc->buffer2 = gmacdev->TxDescDma;
        first_desc->data2 = (u32)gmacdev->TxDesc;

        gmacdev->TxDescCount = 1;

        for (i = 0; i < (no_of_desc - 1); i++) {
            second_desc = ak_plat_alloc_consistent_mem(
                pnetdev, sizeof(DmaDesc), &dma_addr);
            if (second_desc == NULL) {
                return -ESYNOPGMACNOMEM;
            }
            first_desc->buffer2 = dma_addr;
            first_desc->data2 = (u32)second_desc;
            second_desc->buffer2 = gmacdev->TxDescDma;
            second_desc->data2 = (u32)gmacdev->TxDesc;

            ak_gmac_tx_desc_init_chain(first_desc);
            gmacdev->TxDescCount += 1;
            first_desc = second_desc;
        }

        ak_gmac_tx_desc_init_chain(first_desc);
    }

    gmacdev->TxNext = 0;
    gmacdev->TxBusy = 0;
    gmacdev->TxNextDesc = gmacdev->TxDesc;
    gmacdev->TxBusyDesc = gmacdev->TxDesc;
    gmacdev->BusyTxDesc = 0;

    return -ESYNOPGMACNOERR;
}

/**
 * @BRIEF        This sets up the receive Descriptor queue in ring or chain
 * mode. This function is tightly coupled to the platform and operating system
 *               Device is interested only after the descriptors are setup.
 * Therefore this function is not included in the device driver API. This
 * function should be treated as an example code to design the descriptor
 * structures in ring mode or chain mode. This function depends on the pcidev
 * structure for allocation of consistent dma-able memory in case of linux. This
 * limitation is due to the fact that linux uses pci structure to allocate a
 * dmable memory
 *               - Allocates the memory for the descriptors.
 *               - Initialize the Busy and Next descriptors indices to
 * 0(Indicating first descriptor).
 *               - Initialize the Busy and Next descriptors to first descriptor
 * address.
 *               - Initialize the last descriptor with the endof ring in case of
 * ring mode.
 *               - Initialize the descriptors in chain mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    *pnetdev:pointer to pci_device structure.
 * @PARAM[in]    no_of_desc:number of descriptor expected in rx descriptor
 * queue.
 * @PARAM[in]    desc_mode:whether descriptors to be created in RING mode or
 * CHAIN mode.
 * @RETURN       int
 * @RETVAL       return 0 upon success. Error code upon failure.
 * @NOTES        This function fails if allocation fails for required number of
 * descriptors in Ring mode, but in chain mode function returns -ESYNOPGMACNOMEM
 * in the process of descriptor chain creation. once returned from this function
 *               user should for gmacdev->RxDescCount to see how many
 * descriptors are there in the chain. Should continue further only if the
 * number of descriptors in the chain meets the requirements
 */
static int ak_setup_rx_desc_queue(gmac_device* gmacdev,
    struct net_device* pnetdev, u32 no_of_desc, u32 desc_mode)
{
    s32 i;

    DmaDesc* first_desc = NULL;
    DmaDesc* second_desc = NULL;
    dma_addr_t dma_addr;
    gmacdev->RxDescCount = 0;

    if (desc_mode == RINGMODE) {
        first_desc = ak_plat_alloc_consistent_mem(
            pnetdev, sizeof(DmaDesc) * no_of_desc, &dma_addr);
        if (first_desc == NULL) {
            return -ESYNOPGMACNOMEM;
        }
        gmacdev->RxDescCount = no_of_desc;
        gmacdev->RxDesc = first_desc; // cdh:the first desc virtual addr
        gmacdev->RxDescDma = dma_addr; // cdh:the first desc physic addr

        for (i = 0; i < gmacdev->RxDescCount; i++) {
            ak_gmac_rx_desc_init_ring(
                gmacdev->RxDesc + i, i == gmacdev->RxDescCount - 1);
        }
    } else {
        // Allocate the head descriptor
        first_desc
            = ak_plat_alloc_consistent_mem(pnetdev, sizeof(DmaDesc), &dma_addr);
        if (first_desc == NULL) {
            return -ESYNOPGMACNOMEM;
        }
        gmacdev->RxDesc = first_desc;
        gmacdev->RxDescDma = dma_addr;
        first_desc->buffer2 = gmacdev->RxDescDma;
        first_desc->data2 = (u32)gmacdev->RxDesc;
        gmacdev->RxDescCount = 1;

        for (i = 0; i < (no_of_desc - 1); i++) {
            second_desc = ak_plat_alloc_consistent_mem(
                pnetdev, sizeof(DmaDesc), &dma_addr);
            if (second_desc == NULL) {
                return -ESYNOPGMACNOMEM;
            }
            first_desc->buffer2 = dma_addr;
            first_desc->data2 = (u32)second_desc;
            second_desc->buffer2 = gmacdev->RxDescDma;
            second_desc->data2 = (u32)gmacdev->RxDesc;

            ak_gmac_rx_desc_init_chain(first_desc);
            gmacdev->RxDescCount += 1;
            first_desc = second_desc;
        }
        ak_gmac_rx_desc_init_chain(first_desc);
    }

    gmacdev->RxNext = 0;
    gmacdev->RxBusy = 0;
    gmacdev->RxNextDesc = gmacdev->RxDesc;
    gmacdev->RxBusyDesc = gmacdev->RxDesc;
    gmacdev->BusyRxDesc = 0;

    return -ESYNOPGMACNOERR;
}

/**
 * @BRIEF        Driver Api to get the descriptor field information.
 *               This returns the status, dma-able address of buffer1, the
 * length of buffer1, virtual address of buffer1 dma-able address of buffer2,
 * length of buffer2, virtural adddress of buffer2.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *desc:pointer to DmaDesc structure.
 * @PARAM[out]   *Status:pointer to status field fo descriptor.
 * @PARAM[out]   *Buffer1:dma-able address of buffer1.
 * @PARAM[out]   *Length1:length of buffer1.
 * @PARAM[out]   *Data1:virtual address of buffer1.
 * @PARAM[out]   *Buffer2:dma-able address of buffer2.
 * @PARAM[out]   *Length2:length of buffer2.
 * @PARAM[out]   *Data2:virtual address of buffer2.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_gmac_get_desc_data(DmaDesc* desc, u32* Status, u32* Buffer1,
    u32* Length1, u32* Data1, u32* Buffer2, u32* Length2, u32* Data2)
{

    if (Status != 0)
        *Status = desc->status;

    if (Buffer1 != 0)
        *Buffer1 = desc->buffer1;
    if (Length1 != 0)
        *Length1 = (desc->length & DescSize1Mask) >> DescSize1Shift;
    if (Data1 != 0)
        *Data1 = desc->data1;

    if (Buffer2 != 0)
        *Buffer2 = desc->buffer2;
    if (Length2 != 0)
        *Length2 = (desc->length & DescSize2Mask) >> DescSize2Shift;
    if (Data2 != 0)
        *Data2 = desc->data2;

    return;
}

/**
 * @BRIEF        This gives up the receive Descriptor queue in ring or chain
 * mode. This function is tightly coupled to the platform and operating system
 *               Once device's Dma is stopped the memory descriptor memory and
 * the buffer memory deallocation, is completely handled by the operating
 * system, this call is kept outside the device driver Api. This function should
 * be treated as an example code to de-allocate the descriptor structures in
 * ring mode or chain mode and network buffer deallocation. This function
 * depends on the pcidev structure for dma-able memory deallocation for both
 * descriptor memory and the network buffer memory under linux. The
 * responsibility of this function is to
 *               - Free the network buffer memory if any.
 *               - Fee the memory allocated for the descriptors.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    *pnetdev:pointer to pci_device structure.
 * @PARAM[in]    desc_mode:whether descriptors to be created in RING mode or
 * CHAIN mode.
 * @RETURN       void
 * @RETVAL       return 0 upon success. Error code upon failure.
 * @NOTES        note No referece should be made to descriptors once this
 * function is called. This function is invoked when the device is closed.
 */
static void ak_giveup_rx_desc_queue(
    gmac_device* gmacdev, struct net_device* pnetdev, u32 desc_mode)
{
    s32 i;
    DmaDesc* first_desc = NULL;
    dma_addr_t first_desc_dma_addr;
    u32 status;
    dma_addr_t dma_addr1;
    dma_addr_t dma_addr2;
    u32 length1;
    u32 length2;
    u32 data1;
    u32 data2;

    if (desc_mode == RINGMODE) {
        for (i = 0; i < gmacdev->RxDescCount; i++) {
            ak_gmac_get_desc_data(gmacdev->RxDesc + i, &status, &dma_addr1,
                &length1, &data1, &dma_addr2, &length2, &data2);
            if ((length1 != 0) && (data1 != 0)) {
                dma_unmap_single(NULL, dma_addr1, 0, DMA_FROM_DEVICE);
                dev_kfree_skb((struct sk_buff*)data1); // free buffer1
            }

            if ((length2 != 0) && (data2 != 0)) {
                dma_unmap_single(NULL, dma_addr2, 0, DMA_FROM_DEVICE);
                dev_kfree_skb((struct sk_buff*)data2); // free buffer2
            }
        }
        if (gmacdev->RxDesc)
            ak_plat_free_consistent_mem(pnetdev,
                (sizeof(DmaDesc) * gmacdev->RxDescCount), gmacdev->RxDesc,
                gmacdev->RxDescDma); // free descriptors memory
    } else {
        first_desc = gmacdev->RxDesc;
        first_desc_dma_addr = gmacdev->RxDescDma;

        for (i = 0; i < gmacdev->RxDescCount; i++) {
            ak_gmac_get_desc_data(first_desc, &status, &dma_addr1, &length1,
                &data1, &dma_addr2, &length2, &data2);
            if ((length1 != 0) && (data1 != 0)) {
                dma_unmap_single(NULL, dma_addr1, 0, DMA_FROM_DEVICE);
                dev_kfree_skb((struct sk_buff*)data1); // free buffer1
            }

            ak_plat_free_consistent_mem(pnetdev, (sizeof(DmaDesc)), first_desc,
                first_desc_dma_addr); // free descriptors

            first_desc = (DmaDesc*)data2;
            first_desc_dma_addr = dma_addr2;
        }
    }
    gmacdev->RxDesc = NULL;
    gmacdev->RxDescDma = 0;

    return;
}

/**
 * @BRIEF        This gives up the transmit Descriptor queue in ring or chain
 * mode. This function is tightly coupled to the platform and operating system
 *               Once device's Dma is stopped the memory descriptor memory and
 * the buffer memory deallocation, is completely handled by the operating
 * system, this call is kept outside the device driver Api. This function should
 * be treated as an example code to de-allocate the descriptor structures in
 * ring mode or chain mode and network buffer deallocation. This function
 * depends on the pcidev structure for dma-able memory deallocation for both
 * descriptor memory and the network buffer memory under linux. The
 * responsibility of this function is to
 *               - Free the network buffer memory if any.
 *               - Fee the memory allocated for the descriptors.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    *pnetdev:pointer to pci_device structure.
 * @PARAM[in]    desc_mode:whether descriptors to be created in RING mode or
 * CHAIN mode.
 * @RETURN       void
 * @RETVAL       return 0 upon success. Error code upon failure.
 * @NOTES        note No reference should be made to descriptors once this
 * function is called. This function is invoked when the device is closed.
 */
static void ak_giveup_tx_desc_queue(
    gmac_device* gmacdev, struct net_device* pnetdev, u32 desc_mode)
{
    s32 i;
    DmaDesc* first_desc = NULL;
    dma_addr_t first_desc_dma_addr;
    u32 status;
    dma_addr_t dma_addr1;
    dma_addr_t dma_addr2;
    u32 length1;
    u32 length2;
    u32 data1;
    u32 data2;

    if (desc_mode == RINGMODE) {
        for (i = 0; i < gmacdev->TxDescCount; i++) {
            ak_gmac_get_desc_data(gmacdev->TxDesc + i, &status, &dma_addr1,
                &length1, &data1, &dma_addr2, &length2, &data2);
            if ((length1 != 0) && (data1 != 0)) {
                dma_unmap_single(NULL, dma_addr1, 0, DMA_TO_DEVICE);
                dev_kfree_skb((struct sk_buff*)data1); // free buffer1
            }
            if ((length2 != 0) && (data2 != 0)) {
                dma_unmap_single(NULL, dma_addr2, 0, DMA_TO_DEVICE);
                dev_kfree_skb((struct sk_buff*)data2); // free buffer2
            }
        }
        if (gmacdev->TxDesc)
            ak_plat_free_consistent_mem(pnetdev,
                (sizeof(DmaDesc) * gmacdev->TxDescCount), gmacdev->TxDesc,
                gmacdev->TxDescDma); // free descriptors
    } else {
        first_desc = gmacdev->TxDesc;
        first_desc_dma_addr = gmacdev->TxDescDma;
        for (i = 0; i < gmacdev->TxDescCount; i++) {
            ak_gmac_get_desc_data(first_desc, &status, &dma_addr1, &length1,
                &data1, &dma_addr2, &length2, &data2);
            if ((length1 != 0) && (data1 != 0)) {
                dma_unmap_single(NULL, dma_addr1, 0, DMA_TO_DEVICE);
                dev_kfree_skb((struct sk_buff*)data2); // free buffer
            }
            ak_plat_free_consistent_mem(pnetdev, (sizeof(DmaDesc)), first_desc,
                first_desc_dma_addr); // free descriptors
            first_desc = (DmaDesc*)data2;
            first_desc_dma_addr = dma_addr2;
        }
    }
    gmacdev->TxDesc = NULL;
    gmacdev->TxDescDma = 0;

    return;
}

/**
 * @BRIEF        Populate the tx desc structure with the buffer address.
 *               Once the driver has a packet ready to be transmitted, this
 * function is called with the valid dma-able buffer addresses and their
 * lengths. This function populates the descriptor and make the DMA the owner
 * for the descriptor. This function also controls whetther Checksum offloading
 * to be done in hardware or not. This api is same for both ring mode and chain
 * mode.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @PARAM[in]    Buffer1:Dma-able buffer1 pointer.
 * @PARAM[in]    Length1:length of buffer1 (Max is 2048).
 * @PARAM[in]    Data1:virtual pointer for buffer1.
 * @PARAM[in]    Buffer2:Dma-able buffer2 pointer.
 * @PARAM[in]    Length2:length of buffer2 (Max is 2048).
 * @PARAM[in]    Data2:virtual pointer for buffer2.
 * @PARAM[in]    offload_needed:u32 indicating whether the checksum offloading
 * in HW/SW.
 * @RETURN       int
 * @RETVAL       return returns present tx descriptor index on success. Negative
 * value if error.
 */
static int ak_gmac_set_tx_qptr(gmac_device* gmacdev, u32 Buffer1, u32 Length1,
    u32 Data1, u32 Buffer2, u32 Length2, u32 Data2, u32 offload_needed,
    int done_int)
{
    u32 txnext = gmacdev->TxNext;
    DmaDesc* txdesc = gmacdev->TxNextDesc;

    if (!ak_gmac_is_desc_empty(txdesc)) {
        printk(KERN_ERR "gmac tx desc not empty!\n");
        return -1;
    }

    (gmacdev->BusyTxDesc)++; // busy tx descriptor is incremented by one as it
                             // will be handed over to DMA

    if (ak_gmac_is_tx_desc_chained(txdesc)) {
        printk(KERN_ERR "#");
        txdesc->length |= ((Length1 << DescSize1Shift) & DescSize1Mask);
        if (done_int) {
            txdesc->length |= (DescTxFirst | DescTxLast
                | DescTxIntEnable); // Its always assumed that complete data
                                    // will fit in to one descriptor
        } else {
            txdesc->length |= (DescTxFirst
                | DescTxLast); // Its always assumed that complete data will fit
                               // in to one descriptor
        }
        txdesc->buffer1 = Buffer1;
        txdesc->data1 = Data1;

        if (offload_needed) {
            /**
             *Make sure that the OS you are running supports the IP and TCP
             *checkusm offloaidng, before calling any of the functions given
             *below.
             */
            ak_gmac_tx_checksum_offload_tcponly(txdesc);
        }

        txdesc->status = DescOwnByDma;
        gmacdev->TxNext
            = ak_gmac_is_last_tx_desc(gmacdev, txdesc) ? 0 : txnext + 1;
        gmacdev->TxNextDesc = (DmaDesc*)txdesc->data2;
    } else {
        // printk(KERN_ERR"@1");
        txdesc->length |= (((Length1 << DescSize1Shift) & DescSize1Mask)
            | ((Length2 << DescSize2Shift) & DescSize2Mask));
        if (done_int) {
            txdesc->length |= (DescTxFirst | DescTxLast
                | DescTxIntEnable); // Its always assumed that complete data
                                    // will fit in to one descriptor
        } else {
            txdesc->length |= (DescTxFirst
                | DescTxLast); // Its always assumed that complete data will fit
                               // in to one descriptor
        }
        txdesc->buffer1 = Buffer1;
        txdesc->data1 = Data1;
        txdesc->buffer2 = Buffer2;
        txdesc->data2 = Data2;

        if (offload_needed) {
            // printk(KERN_ERR"@2");
            /**
             * make sure that the OS you are running supports the IP and TCP
             * checkusm offloaidng, before calling any of the functions given
             * below.
             */
            ak_gmac_tx_checksum_offload_tcponly(txdesc);
        }

        txdesc->status = DescOwnByDma;
        gmacdev->TxNext
            = ak_gmac_is_last_tx_desc(gmacdev, txdesc) ? 0 : txnext + 1;
        gmacdev->TxNextDesc = ak_gmac_is_last_tx_desc(gmacdev, txdesc)
            ? gmacdev->TxDesc
            : (txdesc + 1);
    }

    return txnext;
}

/*
 * @BRIEF        wait mac phy enter idle.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       s32
 * @RETVAL       if wait phy idle ok,return status, else return -6
 * @NOTES
 */
static s32 ak_gmac_phy_wait_idle(void)
{
    u32 i;
    s32 status;
    u16 data;
    unsigned long last_time;

    last_time = jiffies;
    /* wait phy idle, reset cycle < 0.5s  */
    for (i = 0; i < PHY_WAIT_IDLE_TIMER_CNT; i++) {
        status = ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data);
        if (status) {
            continue;
        }

        /* Mii_reset=1(soft_reset), or=0(normal), self-clearing */
        if ((data & Mii_reset) == 0) {
            break;
        }
        if (jiffies - last_time >= HZ / 100) {
            schedule();
            last_time = jiffies;
        }
    }

    if (i >= PHY_WAIT_IDLE_TIMER_CNT) {
        status = -ENXIO;
        printk("timeout in waiting phy to idle!\n");
    }

    return status;
}

/*
 *   修改日期：2023/11/25 16:29
 *   修改者：吴乾仕
 */
static int yt8510_rate_model = 0x01;
module_param(yt8510_rate_model, int, 0664);

/*
 * @BRIEF        init mac phy.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       s32
 * @RETVAL       if init phy ok,return 0, else return -6
 * @NOTES
 */
static s32 ak_gmac_phy_init(struct net_device* netdev, gmac_device* gmacdev)
{
    s32 status;
    u16 data_ctl;
    u16 data_id1;
    u16 data_id2;
    u32 regval;
    int i, ret;
    struct ak_gmac100* priv = netdev_priv(netdev);

    /* check reg0 bit 15, wait phy idle for software reset case */
    if (ak_gmac_phy_wait_idle()) {
        printk("%s,phy_wait_idle err!\n", __func__);
    }

#if defined(CONFIG_MACH_AK37D) || defined(CONFIG_MACH_AK39EV330)
    mdelay(200);
#endif

    status = ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data_ctl);
    if (status) {
        printk("phy read phy ctrl waiting Error\n");
        goto err_phy_reg;
    }

    status = ak_gmac_read_phy_reg(PHY_ID_HI_REG, &data_id1);
    if (status) {
        printk("phy read phy id1 waiting Error,data=0x%x\n", data_id1);
        goto err_phy_reg;
    } else {
        printk("phy read phy id1 waiting ok,data=0x%x\n", data_id1);
        if (priv->mac_id == 0) {
            eth0_proc_info.phy_id[0] = (u32)data_id1;
        } else if (priv->mac_id == 1) {
            eth1_proc_info.phy_id[0] = (u32)data_id1;
        }

        gmacdev->phy_id1 = data_id1;
    }

    status = ak_gmac_read_phy_reg(PHY_ID_LOW_REG, &data_id2);
    if (status) {
        printk("phy read phy id2 waiting Error,data=0x%x\n", data_id2);
        goto err_phy_reg;
    } else {
        printk("phy read phy id2 waiting ok,data=0x%x\n", data_id2);
        if (priv->mac_id == 0) {
            eth0_proc_info.phy_id[1] = (u32)data_id2;
        } else if (priv->mac_id == 1) {
            eth1_proc_info.phy_id[1] = (u32)data_id2;
        }

        gmacdev->phy_id2 = data_id2;
        if (data_id2 != 0xffff) {
            if (priv->mac_id == 0) {
                eth0_proc_info.phy_onboard = MOUNTED_PHY_SMALL_BOARD;
            } else if (priv->mac_id == 1) {
                eth1_proc_info.phy_onboard = MOUNTED_PHY_SMALL_BOARD;
            }
        } else {
            if (priv->mac_id == 0) {
                eth0_proc_info.phy_onboard = UNMOUNTED_PHY_SMALL_BOARD;
            } else if (priv->mac_id == 1) {
                eth1_proc_info.phy_onboard = UNMOUNTED_PHY_SMALL_BOARD;
            }
        }
    }

    if (priv->inner_phy_mode) {
        if ((data_id1 == 0x0) && (data_id2 == 0x118)
            && (!IS_ERR(priv->pinctrl_pins_inner_sz18201))) { // 内置SZ18201
            // pr_info("inner SZ18201\n");

            ret = pinctrl_select_state(
                priv->pinctrl, priv->pinctrl_pins_inner_sz18201);
            if (ret) {
                printk("pinctrl_select_state inner_sz18201 error!\n");
                return -ENXIO;
            }

            ak_gmac_write_phy_reg(0x001e, 0x20e2);
            ak_gmac_read_phy_reg(0x001f, &data_ctl);
            data_ctl &= ~(1 << 7);
            ak_gmac_write_phy_reg(0x001e, 0x20e2);
            ak_gmac_write_phy_reg(0x001f, data_ctl);

            ak_gmac_read_phy_reg(0x0000, &data_ctl);
            data_ctl |= (1 << 15);
            ak_gmac_write_phy_reg(0x0000, data_ctl);

            for (i = 0; i < 1000; i++) {
                ak_gmac_read_phy_reg(0x0000, &data_ctl);
                if (data_ctl & (1 << 15)) {
                    msleep(1);
                    continue;
                }
                break;
            }

            if (i >= 1000) {
                pr_info("wait for PHY software reset failed!\n");
                return -ENXIO;
            } else {
                ak_gmac_write_phy_reg(0x001e, 0x4000);
                ak_gmac_read_phy_reg(0x001f, &data_ctl);
                data_ctl |= 3;
                ak_gmac_write_phy_reg(0x001e, 0x4000);
                ak_gmac_write_phy_reg(0x001f, data_ctl);

                regval = __raw_readl(AK_VA_SYSCTRL + 0x194); // funcmux_reg7
                regval &= ~(0x7 << 3); // GPIO82 -> OPCLK
                __raw_writel(regval, AK_VA_SYSCTRL + 0x194);

                ak_gmac_write_phy_reg(0x001e, 0x00);
            }
        } else { // 内置JL1101 PHY
            // pr_info("inner JL1101\n");

            ak_gmac_write_phy_reg(31, 7); // select page 7
            ak_gmac_read_phy_reg(0xf0, &data_ctl); // read RMSR
            pr_info("PHY RMSR=0x%x\n", data_ctl);
            ak_gmac_write_phy_reg(0xf0, 0xdf5a); // RMII mode, RMII Clock Input
            ak_gmac_read_phy_reg(0xf0, &data_ctl); // read RMSR
            pr_info("PHY RMSR=0x%x\n", data_ctl);

            regval = __raw_readl(AK_VA_SYSCTRL + 0x194); // funcmux_reg7
            regval &= ~(0x7 << 3); // GPIO82 -> OPCLK
            __raw_writel(regval, AK_VA_SYSCTRL + 0x194);

            ak_gmac_write_phy_reg(31, 0); // select page 0
        }
    }

    if ((gmacdev->phy_id1 == 0xffff) && (gmacdev->phy_id2 == 0xffff)) {
        pr_err("cannot read phy id\n");
        goto err_phy_reg;
    }

    if (data_id1 == 0x22) {
        printk("cdh:id 0x22 set!\n");
        /* enable auto-negotiation process */
        ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data_ctl);
        data_ctl |= 0x1000;
        ak_gmac_write_phy_reg(PHY_CONTROL_REG, data_ctl);
    } else if (data_id1 == 0x0
        && data_id2 == 0x128) { /*SZ18201 initialization*/
        printk("cdh:id 0x128 set!\n");

        ak_gmac_write_phy_reg(0x0000, 0x9140);
        ak_gmac_read_phy_reg(0x0000, &data_ctl);
        printk("read reg 0x00 : 0x%x\n", data_ctl);

        ak_gmac_write_phy_reg(0x001e, 0x50);
        ak_gmac_read_phy_reg(0x001f, &data_ctl);
        printk("read reg 0x50 : 0x%x\n", data_ctl);
        data_ctl |= 0x0040;
        ak_gmac_write_phy_reg(0x001e, 0x50);
        ak_gmac_write_phy_reg(0x001f, data_ctl);
        printk("write reg 0x50 : 0x%x\n", data_ctl);
        ak_gmac_write_phy_reg(0x001e, 0x00);
    }
//#if defined(CONFIG_MACH_AK37D) || defined(CONFIG_MACH_AK39EV330)
    else if (data_id1 == 0x1c && data_id2 == 0xc829) // rtl8201g
    {
        printk("Realtek rtl8201g special init!\n");

        // set CRS/CRS_DV pin is RXDV signal
        data_ctl = RTL8201G_P0xD05; // select page 0xd05 (RTL8201G_P0xD05)
        ak_gmac_write_phy_reg(RTL8201_PAGE_SELECT, data_ctl);
        data_ctl = 0x00;
        ak_gmac_read_phy_reg(RTL8201G_P0xD05_R16, &data_ctl);
        data_ctl &= 0x10;
        // printk("set RTL8201G_P0xD05_R16: 0x%04x\n", data_ctl);
        ak_gmac_write_phy_reg(RTL8201G_P0xD05_R16, data_ctl);

        // set RMII Mode
        data_ctl = RTL8201G_P0xD08; // select page 0xD08 (RTL8201G_P0xD08)
        ak_gmac_write_phy_reg(RTL8201_PAGE_SELECT, data_ctl);
        data_ctl = 0x20; // Enable RMII
        // printk("set RTL8201G_P0xD08_R17: 0x%04x\n", data_ctl);
        ak_gmac_write_phy_reg(RTL8201G_P0xD08_R17, data_ctl);

        // set page0
        ak_gmac_write_phy_reg(RTL8201_PAGE_SELECT, 0x00);
    }
//#endif
    /*
     *   修改日期：2023/11/25 16:29
     *   修改者：吴乾仕
     */
    else if (data_id1 == 0x0 && data_id2 == 0x109)
    {
        /*
        *** 8510需要访问三种寄存器 MII，LDS MII, EXT 寄存器
        *** 访问 EXT寄存器
        ** W: mii_w(0x1e,reg); mii_w(0x1f,data);
        ** r: mii_w(0x1e,reg); mii_r(0x1f,&data);

        *** 访问 LDS寄存器(ext reg0x100[1] = 1)
        ** mii_r_ext(0x100,&data); mii_w_ext(0x100,data&0xFFFD);

        *** 访问 MII寄存器
        ** mii_r_ext(0x100,&data); mii_w_ext(0x100,data|0x2);
        */

        printk("Link: CHH ID:0x109 SET \n");
        /***** 設置100/10BT 自协商 *****/

        /*** 切换到mii寄存器下面 ***/
        ak_gmac_write_phy_reg(0x1e, 0x100);
        ak_gmac_read_phy_reg(0x1f, &data_ctl);
        data_ctl |= 0x02;
        ak_gmac_write_phy_reg(0x1e, 0x100);
        ak_gmac_write_phy_reg(0x1f, data_ctl);

        /*** reg0x00[12]=1,打开自协商 ***/
        ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data_ctl);
        data_ctl |= ((0x01 << 12) | (0x01 << 9));
        ak_gmac_write_phy_reg(PHY_CONTROL_REG, data_ctl);
        /*** reg0x04[8,5],打开自协商时的100/10BT的速度双工能力 ***/
        ak_gmac_read_phy_reg(PHY_AN_ADV_REG, &data_ctl);
        data_ctl |= ((0x01 << 8) | (0x01 << 5));
        ak_gmac_write_phy_reg(PHY_AN_ADV_REG, data_ctl);

        /*** 软复位 reg0[15] = 1  ***/
        ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data_ctl);
        data_ctl |= (0x01 << 15);
        ak_gmac_write_phy_reg(PHY_CONTROL_REG, data_ctl);

        /***** lds LRE100/LRE10 配置 *****/

        /*** 切换到lds mii寄存器下面 ***/
        ak_gmac_write_phy_reg(0x1e, 0x100);
        ak_gmac_read_phy_reg(0x1f, &data_ctl);
        data_ctl &= 0xFFFD;
        ak_gmac_write_phy_reg(0x1e, 0x100);
        ak_gmac_write_phy_reg(0x1f, data_ctl);

        /*** lds reg0x00[12]=1,打开自协商 ***/
        ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data_ctl);
        data_ctl |= ((0x01 << 12) | (0x01 << 9));
        ak_gmac_write_phy_reg(PHY_CONTROL_REG, data_ctl);

        /*** lds  reg0x04[5,1],打开自协商时的100/10LRE的速度双工能力 ***/
        ak_gmac_read_phy_reg(PHY_AN_ADV_REG, &data_ctl);
        data_ctl |= ((0x01 << 1) | (0x01 << 5));
        if (yt8510_rate_model == 0x01)
        {
            data_ctl &= (~(0x01 << 5));
            printk(KERN_ERR "Mandatory LAR - 10M !\n");
        }
        else if (yt8510_rate_model == 0x00)
        {
            printk(KERN_ERR "Full automatic negotiation !\n");
        }
        ak_gmac_write_phy_reg(PHY_AN_ADV_REG, data_ctl);

        /*** 切换到mii寄存器下面 ***/
        ak_gmac_write_phy_reg(0x1e, 0x100);
        ak_gmac_read_phy_reg(0x1f, &data_ctl);
        data_ctl |= 0x02;
        ak_gmac_write_phy_reg(0x1e, 0x100);
        ak_gmac_write_phy_reg(0x1f, data_ctl);

        /* 解决10 M idle issue Bit[7]清零*/
        ak_gmac_write_phy_reg(0x1e, 0x2023);
        ak_gmac_read_phy_reg(0x1f, &data_ctl);
        data_ctl &= 0x7F;
        ak_gmac_write_phy_reg(0x1e, 0x2023);
        ak_gmac_write_phy_reg(0x1f, data_ctl);

        /*** 软复位 lds reg0[15] = 1  ***/
        ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data_ctl);
        data_ctl |= (0x01 << 15);
        ak_gmac_write_phy_reg(PHY_CONTROL_REG, data_ctl);
    }
    else {
        printk("cdh:id other set!\n");
        /* bit12 enable auto-negotiation process, set full-duplex mode */
        ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data_ctl);
        data_ctl |= 0x1100;
        ak_gmac_write_phy_reg(PHY_CONTROL_REG, data_ctl);
        data_ctl = 0x01;
        ak_gmac_write_phy_reg(PHY_EXT_PHY_SPC_CTRL, data_ctl);

        /* read page 1 reg 23, set 13bit = 0 normal mode, 13bit = 1 loopback
         * mode */
        ak_gmac_read_phy_reg(23, &data_ctl);
        data_ctl &= 0xdfff;
        ak_gmac_write_phy_reg(23, data_ctl);
    }

    return 0;

err_phy_reg:

    return -ENXIO;
}
/*end of ak_gmac_phy_init*/

/*
 * @BRIEF        reset mac controller and phy
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device struct.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_gmac_mac_and_phy_reset(struct net_device* netdev)
{
    u32 regval;
    unsigned long flags;
    int gpio_index = 0;
    struct ak_gmac100* priv = netdev_priv(netdev);

    pr_info("Reset MAC%d controller and PHY!\n", priv->mac_id);
#if !defined(CONFIG_MACH_AK3918AV100) && !defined(CONFIG_MACH_KM01A) && !defined(CONFIG_MACH_AK3918AV130)
    /* reset mac controller */
    local_irq_save(flags);
    if (priv->mac_id == 0) {
        regval = __raw_readl(MODULE_RESET_CON1);
        regval |= (0x1 << 13);
        __raw_writel(regval, MODULE_RESET_CON1);
        // printk(KERN_ERR"%s, line:%d, mac_id:%d \n",
        // __func__,__LINE__,priv->mac_id);
    } else if (priv->mac_id == 1) {
        regval = __raw_readl(MODULE_RESET_CON1);
        regval |= (0x1 << 16);
        __raw_writel(regval, MODULE_RESET_CON1);
        // printk(KERN_ERR"%s, line:%d, mac_id:%d \n",
        // __func__,__LINE__,priv->mac_id);
    }
    local_irq_restore(flags);
    mdelay(5);
#endif
    /* release mac controller reset */
    local_irq_save(flags);
    if (priv->mac_id == 0) {
        regval = __raw_readl(MODULE_RESET_CON1);
        regval &= ~(0x1 << 13);
        __raw_writel(regval, MODULE_RESET_CON1);
        // printk(KERN_ERR"%s, line:%d, mac_id:%d \n",
        // __func__,__LINE__,priv->mac_id);
    } else if (priv->mac_id == 1) {
        regval = __raw_readl(MODULE_RESET_CON1);
        regval &= ~(0x1 << 16);
        __raw_writel(regval, MODULE_RESET_CON1);
        // printk(KERN_ERR"%s, line:%d, mac_id:%d \n",
        // __func__,__LINE__,priv->mac_id);
    }
    local_irq_restore(flags);

    /* reset phy chip,check IP101G set low10ms, after rise 2.5ms stable */
    gpio_index = priv->gmacdev.gpio_index;
    if (gpio_index < 0) {
        printk("%s, line:%d, Err, Not find phy reset-gpios,gpio_index:%d \n",
            __func__, __LINE__, gpio_index);
        ak_gmac_phy_software_reset((gmac_device*)&priv->gmacdev);
    } else {
        gpio_set_value(gpio_index, 1);
        mdelay(1);
        gpio_set_value(gpio_index, 0);
        msleep(20);
        gpio_set_value(gpio_index, 1);
        msleep(200);
    }
}

/*
 * @BRIEF        init mac controller broadcast addr
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device struct.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_init_broadcast_addr(struct net_device* ndev)
{
    signed int i;

    /* Now set the broadcast address */
    for (i = 0; i < BROADCAST_ADDR_CNT; i++) {
        ndev->broadcast[i] = 0xff;
    }

    return;
}

/*
 * @BRIEF        init one buffer for mac controller rx dma
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device struct.
 * @PARAM[in]    *gmacdev:pointer to gmac_device struct.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_initial_one_rx_dma_buffer(
    struct net_device* ndev, gmac_device* gmacdev)
{
    signed int status = 0;
    unsigned int dma_addr;
    struct sk_buff* skb;

    /**
     *initial one rx buffer dma infor, why loop 32 ci
     */
    do {
        skb = netdev_alloc_skb(
            ndev, ak_mac_get_packet_mtu(ndev)+ETHERNET_HEADER+ETHERNET_CRC);
        if (skb == NULL) {
            printk(KERN_ERR "ERROR in skb buffer allocation\n");
            break;
        }

        /* cdh:malloc buffer to descriptor dma buffer, dma_addr as physic
         * address */
        dma_addr = dma_map_single(
            NULL, skb->data, skb_tailroom(skb), DMA_FROM_DEVICE);
        status = ak_gmac_set_rx_qptr(
            gmacdev, dma_addr, skb_tailroom(skb), (u32)skb, 0, 0, 0);
        if (status < 0)
            dev_kfree_skb(skb);
    } while (status >= 0);
}

/*
 * @BRIEF        re-init the mac and the phy
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device struct.
 * @RETURN       int
 * @RETVAL       if init ok or failed, return status
 * @NOTES
 */
static int ak_mac_ctrl_reinit(struct net_device* netdev)
{
    signed int status = 0;
    u8 mac_addr0[6] = DEFAULT_MAC_ADDRESS;
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev;
    gmacdev = (gmac_device*)&priv->gmacdev;

    /* reset mac controller and phy  */
    ak_gmac_mac_and_phy_reset(netdev);

    /* Now platform dependent initialization.cdh:check ok*/
    ak_gmac_mmc_counters_reset();
    ak_gmac_mmc_counters_disable_rollover();

    /* the resets all of the GMAC internal registers and logic, */
    ak_gmac_reset();

    /* Program/flash in the station/IP's Mac address */
    ak_gmac_set_mac_addr(GmacAddr0High, GmacAddr0Low, mac_addr0);

    /* Lets read the version of ip, cdh:check ok */
    ak_gmac_read_version(gmacdev);

    /* Lets set ipaddress in to device structure, cdh:check ok*/
    ak_gmac_get_mac_addr(
        GmacAddr0High, GmacAddr0Low, netdev->dev_addr);

    /* Now set the broadcast address*/
    ak_init_broadcast_addr(netdev);

    /* set MDO_CLK for MDIO transmit, note GmacGmiiAddr bit5, and 802.3
     * limit 2.5MHz */
    ak_gmac_set_mdc_clk_div(ak_gmac_get_mdio_clock());

    /* get MDO_CLK div */
    gmacdev->clkdiv_mdc = ak_gmac_get_mdc_clk_div();

    /* initial phy and check phy link status,must add check */
    status = ak_gmac_phy_init(netdev, gmacdev);
    if (status) {
        printk(KERN_ERR "%s,gmac_phy_init err!\n", __func__);
    }

    /* Set up the 32 unit, tx descriptor queue/ring */
    ak_setup_tx_desc_queue(gmacdev, netdev, TRANSMIT_DESC_SIZE, RINGMODE);
    ak_gmac_init_tx_desc_base(gmacdev);

    /* Set up the 32 unit, rx descriptor queue/ring */
    ak_setup_rx_desc_queue(gmacdev, netdev, RECEIVE_DESC_SIZE, RINGMODE);
    ak_gmac_init_rx_desc_base(gmacdev);

    /* dma busrt=32words=128B, two dma_descriptor interval = 2Bytes  */
    ak_gmac_set_dma_bus_mode(DmaBurstLength32 | DmaDescriptorSkip2);

    /* set dma transmit method */
    ak_gmac_set_dma_control(
        DmaDisableFlush | DmaStoreAndForward | DmaTxSecondFrame
            | DmaRxThreshCtrl128);

    /*Initialize the mac interface
     * cdh:check ok, initial mac ctrl part
     */
    ak_gmac_mac_init(gmacdev);

    /**
     * inital dma and mac flow control
     */
    ak_gmac_pause_control(); // This enables the pause control in Full
                                    // duplex mode of operation

    /**
     *initial one rx buffer dma infor, why loop 32 ci
     */
    ak_initial_one_rx_dma_buffer(netdev, gmacdev);

    /**
     * Clear all the interrupts
     */
    ak_gmac_clear_interrupt();

    /**
     * Disable the interrupts generated by MMC and IPC counters.
     * If these are not disabled ISR should be modified accordingly to handle
     * these interrupts.
     */
    ak_gmac_disable_mmc_tx_interrupt(0xFFFFFFFF);
    ak_gmac_disable_mmc_rx_interrupt(0xFFFFFFFF);
    ak_gmac_disable_mmc_ipc_rx_interrupt(0xFFFFFFFF);

    /**
     * Enable Tx and Rx DMA
     */
    ak_gmac_enable_interrupt(DmaIntEnable);
    ak_gmac_enable_dma_rx();
    ak_gmac_enable_dma_tx();

    return status;
}

#ifdef CONFIG_PM
/* base on ak_mac_open */
static int ak_mac_ctrl_resume(struct net_device* netdev)
{

    signed int status = 0;
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev;
    u16 data;
    int gpio_index = 0;
    gmacdev = (gmac_device*)&priv->gmacdev;

    /* reset mac controller and phy  */
    ak_gmac_mac_and_phy_reset(netdev);

    /* Now platform dependent initialization.cdh:check ok*/
    ak_gmac_mmc_counters_reset();
    ak_gmac_mmc_counters_disable_rollover();

    /* the resets all of the GMAC internal registers and logic, */
    ak_gmac_reset();

    /* Program/flash in the station/IP's Mac address */
    ak_gmac_set_mac_addr(
        GmacAddr0High, GmacAddr0Low, netdev->dev_addr);

    /* Lets read the version of ip, cdh:check ok */
    // ak_gmac_read_version(gmacdev);
    // ak_gmac_read_version(gmacdev);

    /* Lets set ipaddress in to device structure, cdh:check ok*/
    // ak_gmac_get_mac_addr(gmacdev, GmacAddr0High, GmacAddr0Low,
    // netdev->dev_addr);

    /* Now set the broadcast address*/
    // ak_init_broadcast_addr(netdev);

    /* set MDO_CLK for MDIO transmit, note GmacGmiiAddr bit5, and 802.3
     * limit 2.5MHz */
    ak_gmac_set_mdc_clk_div(ak_gmac_get_mdio_clock());

    /* get MDO_CLK div */
    gmacdev->clkdiv_mdc = ak_gmac_get_mdc_clk_div();

    /* initial phy and check phy link status,must add check */
    status = ak_gmac_phy_init(netdev, gmacdev);
    if (status) {
        printk(KERN_ERR "%s,gmac_phy_init err!\n", __func__);
    }

    /* Set up the 32 unit, tx descriptor queue/ring */
    ak_setup_tx_desc_queue(gmacdev, netdev, TRANSMIT_DESC_SIZE, RINGMODE);
    ak_gmac_init_tx_desc_base(gmacdev);

    /* Set up the 32 unit, rx descriptor queue/ring */
    ak_setup_rx_desc_queue(gmacdev, netdev, RECEIVE_DESC_SIZE, RINGMODE);
    ak_gmac_init_rx_desc_base(gmacdev);

    /* dma busrt=32words=128B, two dma_descriptor interval = 2Bytes  */
    ak_gmac_set_dma_bus_mode(DmaBurstLength32 | DmaDescriptorSkip2);

    /* set dma transmit method */
    ak_gmac_set_dma_control(
        DmaDisableFlush | DmaStoreAndForward | DmaTxSecondFrame
            | DmaRxThreshCtrl128);

    /*Initialize the mac interface
     * cdh:check ok, initial mac ctrl part
     */
    ak_gmac_mac_init(gmacdev);

    /**
     * inital dma and mac flow control
     */
    ak_gmac_pause_control(); // This enables the pause control in Full
                                    // duplex mode of operation

    /**
     *initial one rx buffer dma infor, why loop 32 ci
     */
    ak_initial_one_rx_dma_buffer(netdev, gmacdev);

    /**
     * Clear all the interrupts
     */
    ak_gmac_clear_interrupt();

    /**
     * Disable the interrupts generated by MMC and IPC counters.
     * If these are not disabled ISR should be modified accordingly to handle
     * these interrupts.
     */
    ak_gmac_disable_mmc_tx_interrupt(0xFFFFFFFF);
    ak_gmac_disable_mmc_rx_interrupt(0xFFFFFFFF);
    ak_gmac_disable_mmc_ipc_rx_interrupt(0xFFFFFFFF);

    /**
     * Enable Tx and Rx DMA
     */
    ak_gmac_enable_interrupt(DmaIntEnable);
    ak_gmac_enable_dma_rx();
    ak_gmac_enable_dma_tx();

    /**
     *start platform dependent network interface
     */
    netif_start_queue(netdev);

    return status;
}

/* base on ak_mac_close */
static int ak_mac_ctrl_suspend(struct net_device* netdev)
{
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev = (gmac_device*)&priv->gmacdev;

    /* disable all interrupts */
    ak_gmac_disable_interrupt_all();

    /* stop net queue */
    netif_stop_queue(netdev);
    netif_carrier_off(netdev);

    /* disable the reception & transmission */
    ak_gmac_disable_dma_rx();
    ak_gmac_disable_dma_tx();

    ak_gmac_take_desc_ownership_rx(gmacdev);
    ak_gmac_take_desc_ownership_tx(gmacdev);

    /* free the Rx & Tx Descriptor contents */
    ak_giveup_rx_desc_queue(gmacdev, netdev, RINGMODE);
    ak_giveup_tx_desc_queue(gmacdev, netdev, RINGMODE);

    return 0;
}
#endif

/**
 * @BRIEF        Checks and initialze phy.
 *               This function checks whether the phy initialization is
 * complete.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       s32
 * @RETVAL       return 0 if success else returns the error number.
 * @NOTES
 */
static s32 ak_gmac_check_phy_init(
    struct net_device* netdev, gmac_device* gmacdev)
{
    u16 data;
    u16 data2;
    u16 tmp_speed = 0;
    s32 status = -ESYNOPGMACNOERR;
    s32 loop_count = WAIT_PHY_ANG_CNT;
    u32 regval;
    u16 data_id1;
    u16 data_id2;
    void __iomem* reg = AK_VA_SYSCTRL;
    struct ak_gmac100* priv = netdev_priv(netdev);
    unsigned long last_time;

    status = ak_gmac_read_phy_reg(PHY_ID_HI_REG, &data_id1);
    if (status) {
        printk("phy read phy id1 waiting Error,data=0x%x\n", data_id1);
    } else {
        // printk("phy read phy id1 waiting ok,data=0x%x\n", data_id1);
    }
    status = ak_gmac_read_phy_reg(PHY_ID_LOW_REG, &data_id2);
    if (status) {
        printk("phy read phy id2 waiting Error,data=0x%x\n", data_id2);
    } else {
        // printk("phy read phy id2 waiting ok,data=0x%x\n", data_id2);
    }
    if (data_id1 == 0x0 && data_id2 == 0x128) { // SZ18201
        status = ak_gmac_read_phy_reg(PHY_SPECIFIC_STATUS_REG, &data);
        if (status) {
            return status;
        }
        if ((data & Mii_phy_status_link_up) == 0) // cdh:Mii_phy_status_link_up
        {
            // printk("No Link\n");
            gmacdev->LinkState = LINKDOWN;
            ak_set_eee(netdev);
            return -ESYNOPGMACPHYERR;
        } else {
            gmacdev->LinkState = LINKUP;
            ak_exit_eee(netdev);
            // printk("Link UP\n");
            // msleep(500);
        }
    } 
    /*
    *   修改日期：2023/11/25 16:29
    *   修改者：吴乾仕
    */
	else if(data_id1 == 0x0 && data_id2 == 0x109)
	{
		/*** 切换到mii寄存器下面 ***/
		ak_gmac_write_phy_reg(0x1e, 0x100);
		ak_gmac_read_phy_reg(0x1f, &data);
		data |= 0x02;
		ak_gmac_write_phy_reg(0x1e, 0x100);
		ak_gmac_write_phy_reg(0x1f, data);

		status = ak_gmac_read_phy_reg(PHY_STATUS_REG, &data);
		if (status)
		{
			goto err_phy_reg;
		}

		if ((data & Mii_Link) == 0)
		{
			printk(KERN_ERR "No Link!!!\n");
			gmacdev->LinkState = LINKDOWN;
			status = -ESYNOPGMACPHYERR;
			goto err_phy_reg;
		}
		else
		{
			gmacdev->LinkState = LINKUP;
			printk(KERN_ERR "Link UP!!!\n");
		}
	}else {
        /* note may be for same special phy define register, not same 8201F
         * phy,cdh: PHY_SPECIFIC_STATUS_REG */
        status = ak_gmac_read_phy_reg(PHY_STATUS_REG, &data);
        if (status || (data == 0xFFFF) ) {
            goto err_phy_reg;
        }
        if ((data & Mii_Link) == 0) {
            // printk("No Link\n");
            gmacdev->LinkState = LINKDOWN;
            ak_set_eee(netdev);
            status = -ESYNOPGMACPHYERR;
            goto err_phy_reg;
        } else {
            gmacdev->LinkState = LINKUP;
            ak_exit_eee(netdev);
            // printk("Link UP\n");
        }
    }

    last_time = jiffies;
    if((gmacdev->phy_id1 != 0x00) && (gmacdev->phy_id2 != 0x109))
    {
        /*
        * Auto-Negotiation
        * The KSZ8041NL conforms to the auto-negotiation protocol, defined in
        * Clause 28 of the IEEE 802.3u specification. Autonegotiation is enabled by
        * either hardware pin strapping (pin 30) or software (register 0h bit 12).
        */
        while (loop_count-- > 0) {
            if(loop_count % 100)
                printk("Autonegotiation loop_count:%d id:%x:%x\n",loop_count,gmacdev->phy_id1,gmacdev->phy_id2);
            status = ak_gmac_read_phy_reg(PHY_STATUS_REG, &data);
            if (status)
                continue;

            if ((data & Mii_AutoNegCmplt) != 0) {
                printk(KERN_ERR "Autonegotiation Complete,loop_count:%d\n",
                    loop_count);
                break;
            }
            if (jiffies - last_time >= HZ / 100) {
                schedule();
                last_time = jiffies;
            }
        }
    }

    if (loop_count <= 0) {
        printk("Autonegotiation Failed\n");
        status = -ENXIO;
        goto err_phy_reg;
    }

    if (priv->mac_id == 0) {
        eth0_proc_info.link_state = gmacdev->LinkState;
    } else if (priv->mac_id == 1) {
        eth1_proc_info.link_state = gmacdev->LinkState;
    }
    /*
    *   修改日期：2023/11/25 16:29
    *   修改者：吴乾仕
    */
    if ((gmacdev->phy_id1 == 0x00) && (gmacdev->phy_id2 == 0x109))
    {
        /***** 读取0x11 获取状态 *****/
        /*** bit15-bit14: speed mode
        ** b11---系统保留
        ** b10---1000M
        ** b01---100M
        ** b00---10M
        ** bit13: deuplex: 1---full duplex 0--- half duplex
        ** bit10: link status 1-link up 0-link down
        ** bit9:1---link 100BT
        ** bit8:1---link 10BT
        ** bit3:1---link 100lre-1
        ** bit2:1---link 10lre
        ***/
        status = ak_gmac_read_phy_reg(0x11, &data);
        if (status)
        {
            goto err_phy_reg;
        }

        /***** 连接的是10BT/10LRE-1 *****/
        if ((data) & ((0x01 << 2) | (0x01 << 8)))
        {
            /*
             * set chip system ctrl reg base address
             * first  mac_speed_cfg=0 (10m)
             */
            if (priv->mac_id == 0)
            {
                regval = __raw_readl(reg + 0x14);
                regval &= ~(0x1 << 23);
                __raw_writel(regval, reg + 0x14);
            }

            if (priv->mac_id == 1)
            {
                regval = __raw_readl(reg + 0x14);
                regval &= ~(0x1 << 21);
                __raw_writel(regval, reg + 0x14);
            }
            gmacdev->Speed = SPEED10;
            printk(KERN_ERR "SRAN Link is with 10M Speed \n");
        }
        /***** 连接的是100BT/100LRE-1 *****/
        else if ((data) & ((0x01 << 3) | (0x01 << 9)))
        {
            if (priv->mac_id == 0)
            {
                regval = __raw_readl(reg + 0x14);
                regval |= (0x1 << 23);
                __raw_writel(regval, reg + 0x14);
            }

            if (priv->mac_id == 1)
            {
                regval = __raw_readl(reg + 0x14);
                regval |= (0x1 << 21);
                __raw_writel(regval, reg + 0x14);
            }
            gmacdev->Speed = SPEED100;
            printk(KERN_ERR "SRAN Link is with 100M Speed \n");
        }

        gmacdev->DuplexMode = HALFDUPLEX;
        /***** 全双工 *****/
        if ((data) & ((0x01 << 2) | (0x02 << 3) | (0x01 << 13)))
        {
            gmacdev->DuplexMode = FULLDUPLEX;
            printk(KERN_ERR "FULLDUPLEX mode\n");
        }
        else
        {
            printk(KERN_ERR "HALFDUPLEX mode\n");
        }
    }
    else
    {
        /* get other end partner AN ability from Reg6.0 */
        status = ak_gmac_read_phy_reg(PHY_AN_EXP_REG, &data);
        if (status) {
            goto err_phy_reg;
        }

        if ((data & OTHER_END_PARTNER_AN_ABITY) == OTHER_END_PARTNER_AN_ABITY) {
            /*
            * read phy Reg4[8:5]&Reg5[8:5] for speed
            * Local&Partner Support AN Ability
            */
            status = ak_gmac_read_phy_reg(PHY_AN_ADV_REG, &data);
            if (status) {
                goto err_phy_reg;
            }

            status = ak_gmac_read_phy_reg(PHY_LNK_PART_ABl_REG, &data2);
            if (status) {
                goto err_phy_reg;
            }

            /* get Reg4[8:5] for loacl AN speed */
            tmp_speed = ((data >> 5) & (data2 >> 5)) & 0xF;
            if (((tmp_speed & SPEED_100M_FULL) == SPEED_100M_FULL)
                || ((tmp_speed & SPEED_100M_HALF) == SPEED_100M_HALF)) {
                /*
                * first  mac_speed_cfg=1(100m)
                * set chip system ctrl reg base address
                */
                if (priv->mac_id == 0) {
                    regval = __raw_readl(reg + 0x14);
                    regval |= (0x1 << 23);
                    __raw_writel(regval, reg + 0x14);
                }

                if (priv->mac_id == 1) {
                    regval = __raw_readl(reg + 0x14);
                    regval |= (0x1 << 21);
                    __raw_writel(regval, reg + 0x14);
                }

                gmacdev->Speed = SPEED100;
                if (tmp_speed & SPEED_100M_FULL) {
                    gmacdev->DuplexMode = FULLDUPLEX;
                    printk("LPAN Link is with 100M Speed, FULLDUPLEX mode\n");
                } else {
                    gmacdev->DuplexMode = HALFDUPLEX;
                    printk("LPAN Link is with 100M Speed, HALFDUPLEX mode\n");
                }
            } else {
                /*
                * set chip system ctrl reg base address
                * first  mac_speed_cfg=0 (10m)
                */
                if (priv->mac_id == 0) {
                    regval = __raw_readl(reg + 0x14);
                    regval &= ~(0x1 << 23);
                    __raw_writel(regval, reg + 0x14);
                }

                if (priv->mac_id == 1) {
                    regval = __raw_readl(reg + 0x14);
                    regval &= ~(0x1 << 21);
                    __raw_writel(regval, reg + 0x14);
                }

                gmacdev->Speed = SPEED10;
                if (tmp_speed & SPEED_10M_FULL) {
                    gmacdev->DuplexMode = FULLDUPLEX;
                    printk("LPAN Link is with 10M Speed, FULLDUPLEX mode\n");
                } else {
                    gmacdev->DuplexMode = HALFDUPLEX;
                    printk("LPAN Link is with 10M Speed, HALFDUPLEX mode\n");
                }
            }

        } else {

            if (((gmacdev->phy_id1 == 0x1c) && (gmacdev->phy_id2 == 0xc816))
                || ((gmacdev->phy_id1 == 0x243) && (gmacdev->phy_id2 == 0xc54))) {
                /*
                * SPECIAL FOR SZ8201F and IP101 100M or 10M half duplex AN
                */
                status = ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data);
                if (status) {
                    goto err_phy_reg;
                }

                if (data & (0x1 << 13)) {
                    /*
                    * set chip system ctrl reg base address
                    * first mac_speed_cfg=1(100m)
                    */
                    if (priv->mac_id == 0) {
                        regval = __raw_readl(reg + 0x14);
                        regval |= (0x1 << 23);
                        __raw_writel(regval, reg + 0x14);
                    }

                    if (priv->mac_id == 1) {
                        regval = __raw_readl(reg + 0x14);
                        regval |= (0x1 << 21);
                        __raw_writel(regval, reg + 0x14);
                    }

                    gmacdev->Speed = SPEED100;
                    if (data & (0x1 << 8)) {
                        gmacdev->DuplexMode = FULLDUPLEX;
                        printk("SRAN Link is with 100M Speed, FULLDUPLEX mode\n");
                    } else {
                        gmacdev->DuplexMode = HALFDUPLEX;
                        printk("SRAN Link is with 100M Speed, HALFDUPLEX mode\n");
                    }
                } else {
                    /*
                    * set chip system ctrl reg base address
                    * first  mac_speed_cfg=0 (10m)
                    */
                    if (priv->mac_id == 0) {
                        regval = __raw_readl(reg + 0x14);
                        regval &= ~(0x1 << 23);
                        __raw_writel(regval, reg + 0x14);
                    }

                    if (priv->mac_id == 1) {
                        regval = __raw_readl(reg + 0x14);
                        regval &= ~(0x1 << 21);
                        __raw_writel(regval, reg + 0x14);
                    }

                    gmacdev->Speed = SPEED10;
                    if (data & (0x1 << 8)) {
                        gmacdev->DuplexMode = FULLDUPLEX;
                        printk("SRAN Link is with 10M Speed, FULLDUPLEX mode\n");
                    } else {
                        gmacdev->DuplexMode = HALFDUPLEX;
                        printk("SRAN Link is with 10M Speed, HALFDUPLEX mode\n");
                    }
                }
            } else {
                /*
                * phy Reg0.12 == 0 ? , AN disable
                * Only Local Support AN Ability
                */
                status = ak_gmac_read_phy_reg(PHY_CONTROL_REG, &data);
                if (status) {
                    goto err_phy_reg;
                }

                if ((data & Mii_AN_En) != Mii_AN_En) {
                    if (data & (0x1 << 13)) {
                        /*
                        * set chip system ctrl reg base address
                        * first mac_speed_cfg=1(100m)
                        */
                        if (priv->mac_id == 0) {
                            regval = __raw_readl(reg + 0x14);
                            regval |= (0x1 << 23);
                            __raw_writel(regval, reg + 0x14);
                        }

                        if (priv->mac_id == 1) {
                            regval = __raw_readl(reg + 0x14);
                            regval |= (0x1 << 21);
                            __raw_writel(regval, reg + 0x14);
                        }

                        gmacdev->Speed = SPEED100;
                        if (data & (0x1 << 8)) {
                            gmacdev->DuplexMode = FULLDUPLEX;
                            printk(
                                "NOAN Link is with 100M Speed, FULLDUPLEX mode\n");
                        } else {
                            gmacdev->DuplexMode = HALFDUPLEX;
                            printk(
                                "NOAN Link is with 100M Speed, HALFDUPLEX mode\n");
                        }
                    } else {
                        /*
                        * set chip system ctrl reg base address
                        * first  mac_speed_cfg=0 (10m)
                        */
                        if (priv->mac_id == 0) {
                            regval = __raw_readl(reg + 0x14);
                            regval &= ~(0x1 << 23);
                            __raw_writel(regval, reg + 0x14);
                        }

                        if (priv->mac_id == 1) {
                            regval = __raw_readl(reg + 0x14);
                            regval &= ~(0x1 << 21);
                            __raw_writel(regval, reg + 0x14);
                        }

                        gmacdev->Speed = SPEED10;
                        if (data & (0x1 << 8)) {
                            gmacdev->DuplexMode = FULLDUPLEX;
                            printk(
                                "NOAN Link is with 10M Speed, FULLDUPLEX mode\n");
                        } else {
                            gmacdev->DuplexMode = HALFDUPLEX;
                            printk(
                                "NOAN Link is with 10M Speed, HALFDUPLEX mode\n");
                        }
                    }
                } else {
                    /* read phy Reg5[8,6] for speed */
                    status = ak_gmac_read_phy_reg(PHY_LNK_PART_ABl_REG, &data);
                    if (status) {
                        goto err_phy_reg;
                    }

                    if ((data & NAN_SPEED_100M_HALF) == NAN_SPEED_100M_HALF) {
                        /*
                        * set chip system ctrl reg base address
                        * first  mac_speed_cfg=1(100m)
                        */
                        if (priv->mac_id == 0) {
                            regval = __raw_readl(reg + 0x14);
                            regval |= (0x1 << 23);
                            __raw_writel(regval, reg + 0x14);
                        }

                        if (priv->mac_id == 1) {
                            regval = __raw_readl(reg + 0x14);
                            regval |= (0x1 << 21);
                            __raw_writel(regval, reg + 0x14);
                        }

                        gmacdev->Speed = SPEED100;
                        gmacdev->DuplexMode = HALFDUPLEX;
                        printk("LAN Link is with 100M Speed, HALFDUPLEX mode\n");
                    } else {
                        /*
                        * set chip system ctrl reg base address
                        * first mac_speed_cfg=0 (10m)
                        */
                        if (priv->mac_id == 0) {
                            regval = __raw_readl(reg + 0x14);
                            regval &= ~(0x1 << 23);
                            __raw_writel(regval, reg + 0x14);
                        }

                        if (priv->mac_id == 1) {
                            regval = __raw_readl(reg + 0x14);
                            regval &= ~(0x1 << 21);
                            __raw_writel(regval, reg + 0x14);
                        }

                        gmacdev->Speed = SPEED10;
                        gmacdev->DuplexMode = HALFDUPLEX;
                        printk("LAN Link is with 10M Speed, HALFDUPLEX mode\n");
                    }
                }
            }
        }
    }

    if (priv->mac_id == 0) {
        eth0_proc_info.speed = gmacdev->Speed;
        eth0_proc_info.duplex_mode = gmacdev->DuplexMode;
    } else if (priv->mac_id == 1) {
        eth1_proc_info.speed = gmacdev->Speed;
        eth1_proc_info.duplex_mode = gmacdev->DuplexMode;
    }

    return -ESYNOPGMACNOERR;

err_phy_reg:

    return status;
}
/*end of ak_gmac_check_phy_init*/

static void linkup_work_proc(struct work_struct* work)
{
    struct ak_gmac100* priv
        = container_of(work, struct ak_gmac100, linkup_work);
    gmac_device* gmacdev = (gmac_device*)&priv->gmacdev;

    pr_debug("#LINK UP %d!#\n", priv->mac_id);
    if (!gmacdev->LinkState) {
        ak_gmac_check_phy_init(priv->netdev, gmacdev);
        ak_gmac_mac_init(gmacdev);
    }
    if (priv->mac_id == 0) {
        if (eth0_cable_plug_timer <= 0)
            add_timer(&eth0_timer_cable_unplug);
    } else {
        if (eth1_cable_plug_timer <= 0)
            add_timer(&eth1_timer_cable_unplug);
    }
    netif_carrier_on(priv->netdev); // cdh:add date 2015.8.29
}

static void ak_eth_broadcast_reset_proc(unsigned long data)
{
    struct ak_gmac100 *priv = (struct ak_gmac100 *)data;
    gmac_device *gmacdev = (gmac_device *)&priv->gmacdev;

    pr_debug("eth%d packet filter reset\n", priv->mac_id);
    pr_debug("broadcast %d arp %d ip %d\n",
            broadcast_count[priv->mac_id],
            broadcast_arp_count[priv->mac_id],
            broadcast_ip_count[priv->mac_id]);
    pr_debug("multicast %d host %d otherhost %d\n",
            host_count[priv->mac_id],
            multicast_count[priv->mac_id],
            otherhost_count[priv->mac_id]);

    broadcast_count[priv->mac_id] = 0;
    broadcast_arp_count[priv->mac_id] = 0;
    broadcast_ip_count[priv->mac_id] = 0;
    host_count[priv->mac_id] = 0;
    multicast_count[priv->mac_id] = 0;
    otherhost_count[priv->mac_id] = 0;

    ak_gmac_broadcast_enable();
    if (gmacdev->DuplexMode == FULLDUPLEX) {
        ak_gmac_multicast_enable();
    }
    if (priv->mac_id == 0) {
        mod_timer(&eth0_broadcast_reset_timer,
                msecs_to_jiffies(broadcast_enable_per_ms) + jiffies);
    } else {
        mod_timer(&eth1_broadcast_reset_timer,
                msecs_to_jiffies(broadcast_enable_per_ms) + jiffies);
    }
}

/*
 * @BRIEF        check netcable plug in or out of mac phy
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    notused:no use.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_eth0_timer_cable_unplug_proc(u32 notused)
{
    s32 status;
    u16 data;
    struct ak_gmac100* priv = (struct ak_gmac100*)eth0_timer_cable_unplug.data;
    gmac_device* gmacdev = (gmac_device*)&priv->gmacdev;

    init_timer(&eth0_timer_cable_unplug);
    eth0_timer_cable_unplug.expires = CHECK_TIME + jiffies;
    status = ak_gmac_read_phy_reg(PHY_STATUS_REG, &data);
    if (status) {
        printk("cdh:%s, read phy err,status:0x%x\n", __func__, status);
        return;
    }

    if ( ( (data & Mii_Link) == 0 ) || (data == 0xFFFF)) {
        pr_debug("#NO LINK!#\n");
        ak_set_eee(priv->netdev);
        gmacdev->LinkState = 0;
        gmacdev->DuplexMode = 0;
        gmacdev->Speed = 0;
        gmacdev->LoopBackMode = 0;
        add_timer(&eth0_timer_cable_unplug);
        netif_carrier_off(priv->netdev); // cdh:add date 2015.8.29
        eth0_proc_info.link_state = gmacdev->LinkState;
        eth0_proc_info.speed = gmacdev->Speed;
        eth0_proc_info.duplex_mode = gmacdev->DuplexMode;
    } else {
        schedule_work(&priv->linkup_work);
    }
}

static int ak_eth_cable_plug_handler(void *dat)
{
    struct ak_gmac100* priv = dat;
    s32 status;
    u16 data;
    gmac_device* gmacdev = (gmac_device*)&priv->gmacdev;

    status = ak_gmac_read_phy_reg(PHY_STATUS_REG, &data);
    if (status) {
        printk("cdh:%s, read phy err,status:0x%x\n", __func__, status);
        return 0;
    }

    if ( ( (data & Mii_Link) == 0 ) || (data == 0xFFFF)) {
        pr_debug("#NO LINK!#\n");
        ak_set_eee(priv->netdev);
        gmacdev->LinkState = 0;
        gmacdev->DuplexMode = 0;
        gmacdev->Speed = 0;
        gmacdev->LoopBackMode = 0;
        netif_carrier_off(priv->netdev); // cdh:add date 2015.8.29
        if (priv->mac_id == 0) {
            eth0_proc_info.link_state = gmacdev->LinkState;
            eth0_proc_info.speed = gmacdev->Speed;
            eth0_proc_info.duplex_mode = gmacdev->DuplexMode;
        } else {
            eth1_proc_info.link_state = gmacdev->LinkState;
            eth1_proc_info.speed = gmacdev->Speed;
            eth1_proc_info.duplex_mode = gmacdev->DuplexMode;
        }
    } else {
        schedule_work(&priv->linkup_work);
    }
    return 0;
}

static void ak_eth1_timer_cable_unplug_proc(u32 notused)
{
    s32 status;
    u16 data;
    struct ak_gmac100* priv = (struct ak_gmac100*)eth1_timer_cable_unplug.data;
    gmac_device* gmacdev = (gmac_device*)&priv->gmacdev;

    init_timer(&eth1_timer_cable_unplug);
    eth1_timer_cable_unplug.expires = CHECK_TIME + jiffies;
    status = ak_gmac_read_phy_reg(PHY_STATUS_REG, &data);
    if (status) {
        printk("cdh:%s, read phy err,status:0x%x\n", __func__, status);
        return;
    }

    if ((data & Mii_Link) == 0) {
        // printk(KERN_ERR"#NO LINK!#\n");
        ak_set_eee(priv->netdev);
        gmacdev->LinkState = 0;
        gmacdev->DuplexMode = 0;
        gmacdev->Speed = 0;
        gmacdev->LoopBackMode = 0;
        add_timer(&eth1_timer_cable_unplug);
        netif_carrier_off(priv->netdev); // cdh:add date 2015.8.29
        eth1_proc_info.link_state = gmacdev->LinkState;
        eth1_proc_info.speed = gmacdev->Speed;
        eth1_proc_info.duplex_mode = gmacdev->DuplexMode;
    } else {
        schedule_work(&priv->linkup_work);
    }
}

/*
 * @BRIEF        open mac controller
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device struct.
 * @RETURN       int
 * @RETVAL       if open success, return 0, else return -3
 * @NOTES
 */
static int ak_mac_open(struct net_device* netdev)
{
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev = (gmac_device*)&priv->gmacdev;
    int status;

    pr_debug("enter %s\n", __func__);
    /* reset mac controller and phy */
    ak_gmac_mac_and_phy_reset(netdev);

    /* now platform dependent initialization. */
    ak_gmac_mmc_counters_reset();
    ak_gmac_mmc_counters_disable_rollover();

    /* software reset , the resets all of the GMAC internal registers and logic,
     */
    if (ak_gmac_reset()) {
        return -EIO;
    }

    /* lets read the version of ip, */
    ak_gmac_read_version(gmacdev);

    /* program/flash in the station/IP's Mac address */
    ak_gmac_set_mac_addr(
        GmacAddr0High, GmacAddr0Low, netdev->dev_addr);

    /* lets set ipaddress in to device structure, */
    ak_gmac_get_mac_addr(
        GmacAddr0High, GmacAddr0Low, netdev->dev_addr);

    /* now set the broadcast address */
    ak_init_broadcast_addr(netdev);

    /* set mdo_clk for mdio transmit, note GmacGmiiAddr bit5, and 802.3
     * limit 2.5MHz */
    ak_gmac_set_mdc_clk_div(ak_gmac_get_mdio_clock());
    /* get mdo_clk div */
    gmacdev->clkdiv_mdc = ak_gmac_get_mdc_clk_div();

    /* initial phy and check phy link status */
    if (ak_gmac_phy_init(netdev, gmacdev)) {
        return -EIO;
    }

    ak_gmac_check_phy_init(netdev, gmacdev);

    /* request for an shared interrupt. Instead of using netdev->irq lets use
     * pcidev->irq */
    printk(KERN_ERR "%s owns a shared interrupt on netdev->irq %d\n",
        netdev->name, netdev->irq);

    status = request_irq(
        netdev->irq, ak_gmac_intr_handler, 0, netdev->name, netdev);
    if (status) {
        printk(KERN_ERR "Error in request_irq\n");
        goto error_in_irq;
    }

    status
        = ak_setup_tx_desc_queue(gmacdev, netdev, TRANSMIT_DESC_SIZE, RINGMODE);
    /* set up the 32 unit, tx descriptor queue/ring */
    if (0 == status)
        ak_gmac_init_tx_desc_base(gmacdev);
    else
        goto error_in_setup_tx_desc;

    /* set up the 32 unit, rx descriptor queue/ring */
    status
        = ak_setup_rx_desc_queue(gmacdev, netdev, RECEIVE_DESC_SIZE, RINGMODE);
    if (0 == status)
        ak_gmac_init_rx_desc_base(gmacdev);
    else
        goto error_in_setup_rx_desc;

    /* dma busrt=32words=128B, two dma_descriptor interval = 2Bytes  */
    ak_gmac_set_dma_bus_mode(DmaBurstLength32 | DmaDescriptorSkip2);

    /* set dma transmit method */
    ak_gmac_set_dma_control(
        DmaDisableFlush | DmaStoreAndForward | DmaTxSecondFrame
            | DmaRxThreshCtrl128);

    /* check ok, initial mac part ip */
    ak_gmac_mac_init(gmacdev);

    /**
     *cdh:inital dma and mac flow control
     */
    ak_gmac_pause_control();

#ifdef IPC_OFFLOAD
    /*IPC Checksum offloading is enabled for this driver.
      Should only be used if Full Ip checksumm offload engine is configured in
      the hardware*/
    ak_gmac_enable_rx_chksum_offload();
    ak_gmac_rx_tcpip_chksum_drop_enable();

    /*
     *The FEF bit in DMA control register is configured to 0 indicating DMA to
     *drop the errored frames. Inform the Linux Networking stack about the
     *hardware capability of checksum offloading
     */
    netdev->features = NETIF_F_ALL_CSUM;
#endif

    /**
     * initial one rx buffer dma infor, why loop 32 ci
     */
    ak_initial_one_rx_dma_buffer(netdev, gmacdev);

    /**
     *inital one timer to detect the cable plugging and unplugging
     * initial phy and check phy link status
     */
    if (priv->mac_id == 0) {
        printk(KERN_ERR "setting up the cable unplug timer0 for mac0\n");
        if (eth0_cable_plug_timer > 0) {
            aktimer = ak_timer_probe(eth0_cable_plug_timer);
            ak_timer_start(ak_eth_cable_plug_handler, priv, aktimer, 1);
        } else {
            init_timer(&eth0_timer_cable_unplug);
            eth0_timer_cable_unplug.function
                = (void*)ak_eth0_timer_cable_unplug_proc;
            eth0_timer_cable_unplug.data = (u32)priv;
            eth0_timer_cable_unplug.expires = CHECK_TIME + jiffies;
            add_timer(&eth0_timer_cable_unplug);
        }

        if (broadcast_enable_per_ms > 0) {
            init_timer(&eth0_broadcast_reset_timer);
            eth0_broadcast_reset_timer.function = ak_eth_broadcast_reset_proc;
            eth0_broadcast_reset_timer.data = (u32) priv;
            eth0_broadcast_reset_timer.expires =
                msecs_to_jiffies(broadcast_enable_per_ms) + jiffies;
            add_timer(&eth0_broadcast_reset_timer);
        }
    }

    if (priv->mac_id == 1) {
        printk(KERN_ERR "setting up the cable unplug timer1 for mac1\n");
        if (eth1_cable_plug_timer > 0) {
            aktimer1 = ak_timer_probe(eth1_cable_plug_timer);
            ak_timer_start(ak_eth_cable_plug_handler, priv, aktimer1, 1);
        } else {
            init_timer(&eth1_timer_cable_unplug);
            eth1_timer_cable_unplug.function
                = (void*)ak_eth1_timer_cable_unplug_proc;
            eth1_timer_cable_unplug.data = (u32)priv;
            eth1_timer_cable_unplug.expires = CHECK_TIME + jiffies;
            add_timer(&eth1_timer_cable_unplug);
        }

        if (broadcast_enable_per_ms > 0) {
            init_timer(&eth1_broadcast_reset_timer);
            eth1_broadcast_reset_timer.function = ak_eth_broadcast_reset_proc;
            eth1_broadcast_reset_timer.data = (u32) priv;
            eth1_broadcast_reset_timer.expires =
                msecs_to_jiffies(broadcast_enable_per_ms) + jiffies;
            add_timer(&eth1_broadcast_reset_timer);
        }
    }

    /**
     *clear all the interrupts
     */
    ak_gmac_clear_interrupt();

    /**
     * disable the interrupts generated by mmc and ipc counters.
     * if these are not disabled isr should be modified accordingly to handle
     * these interrupts.
     */
    ak_gmac_disable_mmc_tx_interrupt(0xFFFFFFFF);
    ak_gmac_disable_mmc_rx_interrupt(0xFFFFFFFF);
    ak_gmac_disable_mmc_ipc_rx_interrupt(0xFFFFFFFF);

    /**
     *enable tx and rx dma
     */
    ak_gmac_enable_interrupt(DmaIntEnable);
    ak_gmac_enable_dma_rx();
    ak_gmac_enable_dma_tx();

    /**
     *start platform dependent network interface
     */
    netif_start_queue(netdev);

    return 0;

error_in_setup_rx_desc:
    ak_giveup_tx_desc_queue(gmacdev, netdev, RINGMODE);
error_in_setup_tx_desc:
    free_irq(priv->irq, netdev);
error_in_irq:
    return -ESYNOPGMACBUSY;
}
/*end of ak_mac_open*/

/**
 * @BRIEF        Function used when the interface is closed.
 *               This function is registered to linux stop() function. This
 * function is called whenever ifconfig (in Linux) closes the device (for
 * example "ifconfig eth0 down"). This releases all the system resources
 * allocated during open call. system resources int needs Disable the device
 * interrupts Stop the receiver and get back all the rx descriptors from the DMA
 *               Stop the transmitter and get back all the tx descriptors from
 * the DMA Stop the Linux network queue interface Free the irq (ISR registered
 * is removed from the kernel) Release the TX and RX descripor memory
 *               De-initialize one second timer rgistered for cable plug/unplug
 * tracking
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device structure.
 * @RETURN       int
 * @RETVAL       Returns 0 on success and error status upon failure.
 * @NOTES
 */
static int ak_mac_close(struct net_device* netdev)
{
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev = (gmac_device*)&priv->gmacdev;
    u16 data;
    pr_debug("enter %s\n", __func__);

    /* disable all interrupts */
    ak_gmac_disable_interrupt_all();

    /* stop net queue */
    netif_stop_queue(netdev);
    netif_carrier_off(netdev);

    if (priv->mac_id == 0) {
        /* free the unplug check */
        if (eth0_cable_plug_timer <= 0) {
            del_timer(&eth0_timer_cable_unplug);
        } else {
            ak_timer_remove(aktimer);
        }
        if (broadcast_enable_per_ms > 0)
            del_timer(&eth0_broadcast_reset_timer);
    } else if (priv->mac_id == 1) {
        /* free the unplug check */
        if (eth1_cable_plug_timer <= 0) {
            del_timer(&eth1_timer_cable_unplug);
        } else {
            ak_timer_remove(aktimer1);
        }
        if (broadcast_enable_per_ms > 0)
            del_timer(&eth1_broadcast_reset_timer);
    }

    /* disable the reception & transmission */
    ak_gmac_disable_dma_rx();
    ak_gmac_disable_dma_tx();

    /* now free the irq */
    free_irq(priv->irq, netdev);

    ak_gmac_take_desc_ownership_rx(gmacdev);
    ak_gmac_take_desc_ownership_tx(gmacdev);

    /* free the Rx & Tx Descriptor contents */
    ak_giveup_rx_desc_queue(gmacdev, netdev, RINGMODE);
    ak_giveup_tx_desc_queue(gmacdev, netdev, RINGMODE);

    if ((gmacdev->phy_id1 != 0xffff) && (gmacdev->phy_id2 != 0xffff)) {
        pr_debug("reset phy and power down phy\n");
        /* reset  phy  */
        if (priv->gmacdev.gpio_index < 0) {
            printk(
                "%s, line:%d, Err, Not find phy reset-gpios,gpio_index:%d \n",
                __func__, __LINE__, priv->gmacdev.gpio_index);
            ak_gmac_phy_software_reset((gmac_device*)&priv->gmacdev);
        } else {
#if !defined(CONFIG_MACH_AK3918AV100) && !defined(CONFIG_MACH_KM01A) && !defined(CONFIG_MACH_AK3918AV130)
            gpio_set_value(priv->gmacdev.gpio_index, 1);
            mdelay(1);
            gpio_set_value(priv->gmacdev.gpio_index, 0);
            msleep(20);
            gpio_set_value(priv->gmacdev.gpio_index, 1);
            msleep(1);
#endif
        }
        /* power down phy*/
        ak_gmac_phy_pwd(gmacdev);
    }

    return -ESYNOPGMACNOERR;
}

#ifdef SMALL_TXDONE_INT
static int no_doneint_count = 0;
#endif

/**
 * @BRIEF        Function to transmit a given packet on the wire.
 *               Whenever Linux Kernel has a packet ready to be transmitted,
 * this function is called. The function prepares a packet and prepares the
 * descriptor and enables/resumes the transmission.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *skb:pointer to sk_buff structure.
 * @PARAM[in]    *netdev:pointer to net_device structure.
 * @RETURN       int
 * @RETVAL       return Returns 0 on success and Error code on failure.
 * @NOTES        note structure sk_buff is used to hold packet in Linux
 * networking stacks.
 */
static netdev_tx_t ak_mac_xmit_frames(
    struct sk_buff* skb, struct net_device* netdev)
{
    int done_int = 1;
    s32 status = 0;
    u32 offload_needed = 0;
    u32 dma_addr;
    unsigned long flags;
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev;

    gmacdev = (gmac_device*)&priv->gmacdev;

    if (skb == NULL) {
        printk(KERN_ERR "skb is null\n ");
        return NETDEV_TX_OK;
    }

    /* stop the network queue*/
    netif_stop_queue(netdev);

    /* for ipc offload used */
    if (skb->ip_summed == CHECKSUM_HW) {
        offload_needed = 0x00000001;
    }

    /* because dma_map_single hase owned dma_spin lock ,so we need protect,Now
     * we have skb ready and OS invoked this function. Lets make our DMA know
     * about this*/
    dma_addr = dma_map_single(NULL, skb->data, skb->len, DMA_TO_DEVICE);

    /* protect share resources here,and we used only one data buffer */
    spin_lock_irqsave(&priv->tx_lock, flags);
#ifdef SMALL_TXDONE_INT
    no_doneint_count++;
    if (no_doneint_count > TXDONE_INT_INTERVAL) {
        done_int = 1;
        no_doneint_count = 0;
    } else {
        done_int = 0;
    }
#endif
#if !defined(CONFIG_MACH_KM01A) && !defined(CONFIG_MACH_AK3918AV130)
    l2_before_dma_out();
#endif
    status = ak_gmac_set_tx_qptr(gmacdev, dma_addr, skb->len, (u32)skb, 0, 0, 0,
        offload_needed, done_int);
    if (status < 0) {
        printk(KERN_ERR "%s No More Free Tx Descriptors\n", __FUNCTION__);
        spin_unlock_irqrestore(&priv->tx_lock, flags);
        dma_unmap_single(NULL, dma_addr, 0, DMA_TO_DEVICE);
        return NETDEV_TX_BUSY;
    }

    /* now force the DMA to start transmission */
    ak_gmac_resume_dma_tx();
    netdev->trans_start = jiffies;
    spin_unlock_irqrestore(&priv->tx_lock, flags);

    /* now start the netdev queue */
    netif_wake_queue(netdev);

    return NETDEV_TX_OK;
}

/**
 * @BRIEF        Function to handle a Tx Hang.
 *               This is a software hook (Linux) to handle transmitter hang if
 * any. We get transmitter hang in the device interrupt status, and is handled
 *               in ISR. This function is here as a place holder.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device structure
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_mac_tx_timeout(struct net_device* netdev)
{
#if 0
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev;
    gmacdev = (gmac_device*)&priv->gmacdev;

    /* Disable all the interrupts */
    ak_gmac_disable_interrupt_all(gmacdev);

    /* Disable the reception*/
    ak_gmac_disable_dma_rx(gmacdev);
    ak_gmac_take_desc_ownership_rx(gmacdev);

    /* Disable the transmission*/
    ak_gmac_disable_dma_tx(gmacdev);
    ak_gmac_take_desc_ownership_tx(gmacdev);

    netif_stop_queue(netdev);

    /* Free the Rx Descriptor contents*/
    ak_giveup_rx_desc_queue(gmacdev, netdev, RINGMODE);
    ak_giveup_tx_desc_queue(gmacdev, netdev, RINGMODE);

    /* retry initial mac and phy, because request interrupt already save, so not
     * re-init */
    ak_mac_ctrl_reinit(netdev);

    /**
     * start platform dependent network interface
     */
    netif_start_queue(netdev);

    return;
#endif
}

/**
 * @BRIEF        Function to change the Maximum Transfer Unit.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device structure.
 * @PARAM[in]    newmtu:New value for maximum frame size.
 * @RETURN       int
 * @RETVAL       always return 0;
 * @NOTES        not allow anyone to change mtu size
 */
static int ak_mac_change_mtu(struct net_device* netdev, s32 newmtu)
{

    if (newmtu < 68 || newmtu > ETH_DATA_LEN) {
        printk(KERN_ERR "[%s][%d] not support mtu size %d\n", __func__,
            __LINE__, newmtu);
        return -EINVAL;
    }

    printk(KERN_ERR "[%s][%d] change mtu size from %d to %d\n", __func__,
        __LINE__, netdev->mtu, newmtu);
    netdev->mtu = newmtu;

    return 0;
}

/**
 * @BRIEF        Function to set ethernet address of the NIC.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device structure.
 * @PARAM[in]    *macaddr:pointer to an address structure.
 * @RETURN       int
 * @RETVAL       return Returns 0 on success Errorcode on failure.
 * @NOTES
 */
static int ak_set_mac_address(struct net_device* netdev, void* macaddr)
{
    struct sockaddr* addr = macaddr;
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev = (gmac_device*)&priv->gmacdev;
    unsigned long flags;

    if (!is_valid_ether_addr(addr->sa_data)) {
        printk(KERN_ERR "mac address is invalid\n");
        return -EADDRNOTAVAIL;
    }

    local_irq_save(flags);
    ak_gmac_set_mac_addr(GmacAddr0High, GmacAddr0Low, addr->sa_data);
    ak_gmac_get_mac_addr(
        GmacAddr0High, GmacAddr0Low, netdev->dev_addr);

    save_ether_addr(netdev, netdev->dev_addr);

    local_irq_restore(flags);
    return 0;
}

/**
 * @BRIEF        power down mac controller
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *gmacdev:pointer to gmac_device.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void ak_powerdown_mac(struct net_device* netdev, gmac_device* gmacdev)
{
    struct ak_gmac100* priv = netdev_priv(netdev);

    // Disable the Dma engines in tx path
    if (priv->mac_id == 0) {
        GMAC0_Power_down = 1;
    } else if (priv->mac_id == 1) {
        GMAC1_Power_down = 1;
    }

    ak_gmac_disable_dma_tx();
    ak_plat_delay(10000);

    // Disable the Mac for both tx and rx
    ak_gmac_tx_disable();
    ak_gmac_rx_disable();
    ak_plat_delay(10000); // Allow any pending buffer to be read by host

    // Disable the Dma in rx path
    ak_gmac_disable_dma_rx();

    // prepare the gmac for magic packet reception and wake up frame reception
    ak_gmac_magic_packet_enable();
    ak_gmac_write_wakeup_frame_register(ak_gmac_wakeup_filter_config3);
    ak_gmac_wakeup_frame_enable();

    // gate the application and transmit clock inputs to the code. This is not
    // done in this driver :).

    // enable the Mac for reception
    ak_gmac_rx_enable();

    // Enable the assertion of PMT interrupt
    ak_gmac_pmt_int_enable();
    // enter the power down mode
    ak_gmac_power_down_enable();

    return;
}

/**
 * @BRIEF        IOCTL interface.
 *               This function is mainly for debugging purpose.
 *               This provides hooks for Register read write, Retrieve
 * descriptor status and Retreiving Device structure information.
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device structure.
 * @PARAM[in]    *ifr:pointer to ifreq structure.
 * @PARAM[in]    cmd:ioctl command.
 * @RETURN       int
 * @RETVAL       return Returns 0 on success Error code on failure.
 * @NOTES
 */
static int ak_mac_do_ioctl(
    struct net_device* netdev, struct ifreq* ifr, s32 cmd)
{
    s32 retval = 0;
    u16 temp_data = 0;
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev = (gmac_device*)&priv->gmacdev;
    struct ifr_data_struct req;
    void __user* useraddr = ifr->ifr_data;
    unsigned long flags;

    if (netdev == NULL) {
        dev_err(priv->dev, "no net device!\n");
        return -EIO;
    }

    if (ifr == NULL) {
        dev_err(priv->dev, "no net ifr!\n");
        return -EIO;
    }

    if (copy_from_user(&req, useraddr, sizeof(struct ifr_data_struct)))
        return -EFAULT;

    switch (cmd) {
        case IOCTL_READ_REGISTER:
            local_irq_save(flags);
            if (req.unit == 0)
                req.data = ak_gmac_read_reg(gmacdev->MacBase, req.addr);
            else if (req.unit == 1)
                req.data = ak_gmac_read_reg(gmacdev->DmaBase, req.addr);
            else if (req.unit == 2) {
                retval = ak_gmac_read_phy_reg(req.addr, &temp_data);
                req.data = (u32)temp_data;
                if (retval != -ESYNOPGMACNOERR)
                    printk(KERN_ERR "ERROR in Phy read\n");
            }
            local_irq_restore(flags);
            if (copy_to_user(useraddr, &req, sizeof(struct ifr_data_struct)))
                return -EFAULT;
            break;

        case IOCTL_WRITE_REGISTER:
            local_irq_save(flags);
            if (req.unit == 0)
                ak_gmac_write_reg(gmacdev->MacBase, req.addr, req.data);
            else if (req.unit == 1)
                ak_gmac_write_reg(gmacdev->DmaBase, req.addr, req.data);
            else if (req.unit == 2) {
                retval = ak_gmac_write_phy_reg(req.addr, (u16)req.data);
                if (retval != -ESYNOPGMACNOERR)
                    printk(KERN_ERR "ERROR in Phy write\n");
            }
            local_irq_restore(flags);
            break;
        default:
            retval = -1;
    }

    return retval;
}

/**
 * @BRIEF        Function provides the network interface statistics.
 *               Function is registered to linux get_stats() function. This
 * function is called whenever ifconfig (in Linux) asks for networkig statistics
 *               (for example "ifconfig eth0").
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *netdev:pointer to net_device structure.
 * @RETURN       net_device_stats
 * @RETVAL       return Returns pointer to net_device_stats structure.
 * @NOTES
 */
struct net_device_stats* ak_mac_get_stats(struct net_device* netdev)
{
    return &netdev->stats;
}

static void ak_mac_ndo_set_rx_mode(struct net_device* netdev)
{
    struct ak_gmac100* priv = netdev_priv(netdev);
    gmac_device* gmacdev = (gmac_device*)&priv->gmacdev;

    if ((netdev->flags & IFF_PROMISC) && (netdev->flags & IFF_UP)) {
        /* Enable promiscuous mode */
        ak_gmac_promisc_enable();
    } else {
        /* Disable promiscuous mode */
        ak_gmac_promisc_disable();
    }
}

static const struct net_device_ops ak_mac_netdev_ops = {
    .ndo_open = ak_mac_open,
    .ndo_stop = ak_mac_close,
    .ndo_start_xmit = ak_mac_xmit_frames,
    .ndo_tx_timeout = ak_mac_tx_timeout,
    .ndo_change_mtu = ak_mac_change_mtu,
    .ndo_set_mac_address = ak_set_mac_address,
    .ndo_do_ioctl = ak_mac_do_ioctl,
    .ndo_get_stats = ak_mac_get_stats,
    .ndo_validate_addr = eth_validate_addr,
    .ndo_set_rx_mode = ak_mac_ndo_set_rx_mode,
};

/******************************************************************************
 * struct ethtool_ops functions
 *****************************************************************************/
#if 0
/**
* @BRIEF        get mac controller information
* @AUTHOR       cao_donghua
* @DATE         2020-02-26
* @PARAM[in]    *netdev:pointer to net_device.
* @PARAM[out]   *info:pointer to gmac_device.
* @RETURN       void
* @RETVAL       none        
* @NOTES
*/
static void ak_gmac100_get_drvinfo(struct net_device *netdev,
                  struct ethtool_drvinfo *info)
{
    strlcpy(info->driver, DRV_NAME, sizeof(info->driver));
    strlcpy(info->version, DRV_VERSION, sizeof(info->version));
    strlcpy(info->bus_info, dev_name(&netdev->dev), sizeof(info->bus_info));
}

/**
* @BRIEF        get mac controller settings
* @AUTHOR       cao_donghua
* @DATE         2020-02-26
* @PARAM[in]    *netdev:pointer to net_device.
* @PARAM[in]    *cmd:pointer to ethtool_cmd.
* @RETURN       int
* @RETVAL       mac settings        
* @NOTES
*/
static int ak_gmac100_get_settings(struct net_device *netdev,
                  struct ethtool_cmd *cmd)
{
    struct ak_gmac100 *priv = netdev_priv(netdev);

    return phy_ethtool_gset(priv->phydev, cmd);
}

/**
* @BRIEF        set mac controller settings
* @AUTHOR       cao_donghua
* @DATE         2020-02-26
* @PARAM[in]    *netdev:pointer to net_device.
* @PARAM[in]    *cmd:pointer to ethtool_cmd.
* @RETURN       int
* @RETVAL       mac settings        
* @NOTES
*/
static int ak_gmac100_set_settings(struct net_device *netdev,
                  struct ethtool_cmd *cmd)
{
    struct ak_gmac100 *priv = netdev_priv(netdev);

    return phy_ethtool_sset(priv->phydev, cmd);
}
#endif

static void ak_set_eee(struct net_device* netdev)
{
    struct ak_gmac100* priv;
    gmac_device* gmacdev;

    if (!eee_is_enable)
        return;

    if (is_eee_mode)
        return;

    if (!netdev)
        return;

    priv = netdev_priv(netdev);
    if (!priv)
        return;

    gmacdev = (gmac_device*)&priv->gmacdev;

    // JL11x1
    if ((eth0_proc_info.phy_id[0] == 0x937c)
        && ((eth0_proc_info.phy_id[1] >> 4) == 0x402)) {
        printk("enter set eee, 0x%x, 0x%x\n", eth0_proc_info.phy_id[0],
            eth0_proc_info.phy_id[1]);
        /* 1.设置MMD 7.60.1=0x1 */
        /* page_sel 0 */
        ak_gmac_write_phy_reg(0x1f, 0);

        /* set device 0x7 */
        ak_gmac_write_phy_reg(0xd, 0x7);

        /* set address 0x3c */
        ak_gmac_write_phy_reg(0xe, 0x3c);

        /* prepare to write register */
        ak_gmac_write_phy_reg(0xd, 0x4007);

        /* Advertise 100Base-TX EEE */
        ak_gmac_write_phy_reg(0xe, 0x2);
        ak_gmac_write_phy_reg(31, 0); // select page 0

        /* 2. software reset */
        ak_gmac_phy_software_reset(gmacdev);

        is_eee_mode = 1;
    }
}

static void ak_exit_eee(struct net_device* netdev)
{
    struct ak_gmac100* priv;
    gmac_device* gmacdev;

    if (!eee_is_enable)
        return;

    if (!is_eee_mode)
        return;

    if (!netdev)
        return;

    priv = netdev_priv(netdev);
    if (!priv)
        return;

    gmacdev = (gmac_device*)&priv->gmacdev;

    // JL11x1
    if ((eth0_proc_info.phy_id[0] == 0x937c)
        && ((eth0_proc_info.phy_id[1] >> 4) == 0x402)) {
        printk("enter exit eee, 0x%x, 0x%x\n", eth0_proc_info.phy_id[0],
            eth0_proc_info.phy_id[1]);
        /* 1.设置MMD 7.60.1=0x1 */
        /* page_sel 0 */
        ak_gmac_write_phy_reg(0x1f, 0);

        /* set device 0x7 */
        ak_gmac_write_phy_reg(0xd, 0x7);

        /* set address 0x3c */
        ak_gmac_write_phy_reg(0xe, 0x3c);

        /* prepare to write register */
        ak_gmac_write_phy_reg(0xd, 0x4007);

        /* Advertise 100Base-TX EEE */
        ak_gmac_write_phy_reg(0xe, 0x0);
        ak_gmac_write_phy_reg(31, 0); // select page 0

        /* 2. software reset */
        ak_gmac_phy_software_reset(gmacdev);
        is_eee_mode = 0;
    }
}

static const struct ethtool_ops ak_gmac100_ethtool_ops = {
#if 0
    .set_settings       = ak_gmac100_set_settings,
    .get_settings       = ak_gmac100_get_settings,
    .get_drvinfo        = ak_gmac100_get_drvinfo,
#endif
    .get_link = ethtool_op_get_link,
};

/*
 * /proc/eth
 */
static int eth0_proc_show(struct seq_file* m, void* v)
{
    int i;
    char* ion_type;

    for (i = 0; i < num_heaps; i++) {
        seq_printf(m,
            "name: %s\n"
            "mac_addr: %s\n"
            "phy_id0: 0x%x\n"
            "phy_id1: 0x%x\n"
            "phy_onboard: %s\n"
            "link_state: %s\n"
            "duplex_mode: %s\n"
            "speed: %s\n",
            eth0_proc_info.mac_name, eth0_proc_info.mac_addr,
            eth0_proc_info.phy_id[0], eth0_proc_info.phy_id[1],
            (eth0_proc_info.phy_onboard == 1) ? "mounted" : "unmount",
            (eth0_proc_info.link_state == 1) ? "link_up" : "link_down",
            (eth0_proc_info.duplex_mode == 2) ? "duplex" : "simplex",
            (eth0_proc_info.speed == 2) ? "100M" : "10M");
    }
    return 0;
}

static int eth1_proc_show(struct seq_file* m, void* v)
{
    int i;
    char* ion_type;

    for (i = 0; i < num_heaps; i++) {
        seq_printf(m,
            "name: %s\n"
            "mac_addr: %s\n"
            "phy_id0: 0x%x\n"
            "phy_id1: 0x%x\n"
            "phy_onboard: %s\n"
            "link_state: %s\n"
            "duplex_mode: %s\n"
            "speed: %s\n",
            eth1_proc_info.mac_name, eth1_proc_info.mac_addr,
            eth1_proc_info.phy_id[0], eth1_proc_info.phy_id[1],
            (eth1_proc_info.phy_onboard == 1) ? "mounted" : "unmount",
            (eth1_proc_info.link_state == 1) ? "link_up" : "link_down",
            (eth1_proc_info.duplex_mode == 2) ? "duplex" : "simplex",
            (eth1_proc_info.speed == 2) ? "100M" : "10M");
    }
    return 0;
}

static int eth0_info_open(struct inode* inode, struct file* filp)
{
    return single_open(filp, eth0_proc_show, NULL);
}

static int eth1_info_open(struct inode* inode, struct file* filp)
{
    return single_open(filp, eth1_proc_show, NULL);
}

static const struct file_operations proc_eth0_operations = {
    .open = eth0_info_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static const struct file_operations proc_eth1_operations = {
    .open = eth1_info_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

/**
 * @BRIEF        mac proble function
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *pdev:pointer to platform device.
 * @RETURN       int
 * @RETVAL       if probe ok,return 0, else return -n
 * @NOTES
 */
static int ak_mac_probe(struct platform_device* pdev)
{
    struct device* dev = &pdev->dev;
    struct device_node* node = dev->of_node;
    struct resource* res;
    struct net_device* netdev;
    struct ak_gmac100* priv;
    int irq;
    int err;
    const char* mac_addr;
    u32 macphy_addr = 1;
    u32 inner_phy_mode = 0;
    int ret;

    /* check pdev device */
    if (!pdev)
        return -ENODEV;

    pdev->id = of_alias_get_id(node, "ethernet");

    /* reset eth_proc_info struct */
    if (pdev->id == 0) {
        memset(&eth0_proc_info, 0x00, sizeof(eth0_proc_info));
        strcpy(eth0_proc_info.mac_name, MACNAME);
    } else if (pdev->id == 1) {
        memset(&eth1_proc_info, 0x00, sizeof(eth1_proc_info));
        strcpy(eth1_proc_info.mac_name, MACNAME);
    } else {
        printk(KERN_ERR "%s,line:%d, err no pdev->id:%d\n", __func__, __LINE__,
            pdev->id);
        return -ENODEV;
    }

    /* get ioresource */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res)
        return -ENXIO;

    /* get irq index */
    irq = platform_get_irq(pdev, 0);
    if (irq < 0)
        return irq;

    /* setup net_device */
    netdev = alloc_etherdev(sizeof(*priv));
    if (!netdev) {
        err = -ENOMEM;
        dev_err(dev, "could not allocate device.\n");
        goto err_alloc_etherdev;
    }

    SET_NETDEV_DEV(netdev, &pdev->dev);

    /* init netdevice protocol parameter */
    netdev->ethtool_ops = &ak_gmac100_ethtool_ops;
    netdev->netdev_ops = &ak_mac_netdev_ops;

    /* ethernet watchdog timeout */
    netdev->watchdog_timeo = msecs_to_jiffies(5000);

    platform_set_drvdata(pdev, netdev);

    /* setup private data */
    priv = netdev_priv(netdev);
    priv->netdev = netdev;
    priv->dev = &pdev->dev;

    spin_lock_init(&priv->lock);
    spin_lock_init(&priv->tx_lock);

    /* map io memory */
    priv->res = request_mem_region(
        res->start, resource_size(res), dev_name(&pdev->dev));
    if (!priv->res) {
        dev_err(&pdev->dev, "Could not reserve memory region\n");
        err = -ENOMEM;
        goto err_req_mem;
    }

    priv->base = ioremap(res->start, resource_size(res));
    if (!priv->base) {
        dev_err(&pdev->dev, "Failed to ioremap ethernet registers\n");
        err = -EIO;
        goto err_ioremap;
    }

    priv->irq = irq;
    netdev->irq = irq;
    netdev->base_addr = (unsigned long)priv->base;

    /**
     *Attach the device to MAC struct This will configure all the required base
     *addresses such as Mac base, configuration base, phy base address(out of 32
     *possible phys ) cdh:here must be initialled inside of ak_mac_probe
     */
    ret = of_property_read_u32(node, "phy-address", &macphy_addr);
    if (ret < 0) {
        macphy_addr = DEFAULT_PHY_BASE;
        printk(KERN_ERR "%s, line:%d, err:phy-address:%d,using default:%d\n",
            __func__, __LINE__, macphy_addr, DEFAULT_PHY_BASE);
    }

    ak_gmac_attach(pdev, &priv->gmacdev, (u32)netdev->base_addr + MACBASE,
        (u32)netdev->base_addr + DMABASE, macphy_addr);
    ak_gmac_if_init(&ak_gmac_if);

    ret = of_property_read_u32(node, "inner-phy-mode", &inner_phy_mode);
    if (ret < 0) {
        inner_phy_mode = 0;
    }
    priv->inner_phy_mode = !!inner_phy_mode;

    if (priv->inner_phy_mode) {
        priv->pinctrl = devm_pinctrl_get(dev);
        if (IS_ERR(priv->pinctrl)) {
            ret = PTR_ERR(priv->pinctrl);
            dev_err(dev, "get pinctrl err!\n");
            goto err_mac_init;
        }

        priv->pinctrl_pins_inner_sz18201
            = pinctrl_lookup_state(priv->pinctrl, "inner_sz18201");
    }

    /* open mac ctrl and phy 50m clock, and set rmii interface all share pin ok
     */
    err = ak_gmac_clk_sharepin_init(pdev, AK_TRUE);
    if (err) {
        dev_err(&pdev->dev, "get mac clk and set sharepin err!\n");
        goto err_mac_init;
    }

    /* register network device */
    err = register_netdev(netdev);
    if (err) {
        dev_err(&pdev->dev, "Failed to register netdev\n");
        goto err_register_netdev;
    }

    netdev_warn(netdev, "irq %d, mapped at %p\n", priv->irq, priv->base);

    /* set mac address
     * get Ethernet mac address from flash area and check valid ,
     * notes first we can get fixed value , not read file
     */
    mac_addr = of_get_mac_address(node);
    if (mac_addr)
        ether_addr_copy(netdev->dev_addr, mac_addr);
    if (!is_valid_ether_addr(netdev->dev_addr)) {
        random_ether_addr(netdev->dev_addr);
        dev_warn(dev, "using random MAC address %pM\n", netdev->dev_addr);
    }

    priv->mac_id = pdev->id;
    save_ether_addr(netdev, netdev->dev_addr);

    if (pdev->id == 0) {
        eth0_proc = proc_create("eth0", 0, NULL, &proc_eth0_operations);
        printk(
            KERN_ERR "%s, line:%d, mac_id:%d \n", __func__, __LINE__, pdev->id);
    } else if (pdev->id == 1) {
        eth1_proc = proc_create("eth1", 0, NULL, &proc_eth1_operations);
        printk(
            KERN_ERR "%s, line:%d, mac_id:%d \n", __func__, __LINE__, pdev->id);
    }

    INIT_WORK(&priv->linkup_work, linkup_work_proc);

    printk(KERN_ERR "insmod mac device ok\n");

    return 0;

err_register_netdev:
    ak_gmac_clk_sharepin_init(pdev, AK_FALSE);
err_mac_init:
    iounmap(priv->base);
err_ioremap:
    release_resource(priv->res);
err_req_mem:
    free_netdev(netdev);
err_alloc_etherdev:
    return err;
}
/*end of ak_mac_probe*/

/**
 * @BRIEF        mac remove function
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    *pdev:pointer to platform device.
 * @RETURN       int
 * @RETVAL       if probe ok,return 0, else return -n
 * @NOTES
 */
static int ak_mac_remove(struct platform_device* pdev)
{
    struct net_device* netdev;
    struct ak_gmac100* priv;
    gmac_device* gmacdev = NULL;

    netdev = platform_get_drvdata(pdev);
    priv = netdev_priv(netdev);
    gmacdev = (gmac_device*)&priv->gmacdev;

    printk(KERN_ERR "released and free mac device ok\n");

    cancel_work_sync(&priv->linkup_work);
    ak_gmac_phy_pwd(gmacdev);

    /* unregister net device */
    unregister_netdev(netdev);

    /* release clk */
    ak_gmac_clk_sharepin_init(pdev, AK_FALSE);

    if (priv->mac_clk)
        clk_put(priv->mac_clk);
    if (priv->phy_clk)
        clk_put(priv->phy_clk);

    /* release ioresource */
    iounmap(priv->base);
    release_resource(priv->res);

    /* release ndev struct */
    free_netdev(netdev);

    if (pdev->id == 0) {
        proc_remove(eth0_proc);
        printk(
            KERN_ERR "%s, line:%d, mac_id:%d \n", __func__, __LINE__, pdev->id);
    } else if (pdev->id == 1) {
        proc_remove(eth1_proc);
        printk(
            KERN_ERR "%s, line:%d, mac_id:%d \n", __func__, __LINE__, pdev->id);
    }

    return 0;
}

#ifdef CONFIG_PM
static int ak_eth_mac_drv_suspend(struct device* dev)
{
    struct platform_device* pdev = to_platform_device(dev);
    struct net_device* ndev = platform_get_drvdata(pdev);
    struct ak_gmac100* priv;
    gmac_device* gmacdev;

    if (ndev) {
        priv = (struct ak_gmac100*)netdev_priv(ndev);
        gmacdev = (gmac_device*)&priv->gmacdev;

        if (netif_running(ndev)) {
            netif_device_detach(ndev);
            ak_mac_ctrl_suspend(ndev);
        }
        pr_debug("reset phy and power down phy\n");
        if ((gmacdev->phy_id1 != 0xffff) && (gmacdev->phy_id2 != 0xffff)) {
            /* reset  phy  */
            if (priv->gmacdev.gpio_index < 0) {
                ak_gmac_phy_software_reset((gmac_device*)&priv->gmacdev);
            } else {
                gpio_set_value(priv->gmacdev.gpio_index, 1);
                mdelay(1);
                gpio_set_value(priv->gmacdev.gpio_index, 0);
                msleep(20);
                gpio_set_value(priv->gmacdev.gpio_index, 1);
            }

            /* power down phy*/
            ak_gmac_phy_pwd(gmacdev);
            /*
             * Configed MAC RMII interface to sleep mode
             */
            ak_gmac_rmii_interface_sharepin(pdev, AK_FALSE);
        }
    }
    return 0;
}

static int ak_eth_mac_drv_resume(struct device* dev)
{
    struct platform_device* pdev = to_platform_device(dev);
    struct net_device* ndev = platform_get_drvdata(pdev);

    struct ak_gmac100* priv;
    gmac_device* gmacdev;

    if (ndev) {
        priv = (struct ak_gmac100*)netdev_priv(ndev);
        gmacdev = (gmac_device*)&priv->gmacdev;

        if ((gmacdev->phy_id1 != 0xffff) && (gmacdev->phy_id2 != 0xffff)) {
            /*
             * Configed MAC RMII interface to defalut mode
             */
            ak_gmac_rmii_interface_sharepin(pdev, AK_TRUE);
        }

        if (netif_running(ndev)) {

            if (ak_mac_ctrl_resume(ndev)) {
                printk("ak_mac_ctrl_resume fail\n");
                return -ENOMEM;
            } else
                printk("ak_mac_ctrl_resume success!\n");

            netif_device_attach(ndev);
        }
    }

    return 0;
}
static struct dev_pm_ops ak_eth_mac_drv_pm_ops = {
    .suspend = ak_eth_mac_drv_suspend,
    .resume = ak_eth_mac_drv_resume,
};
#endif

static const struct of_device_id ak_ethernet_of_ids[] = {
    { .compatible = "anyka,ak39ev330-ethernet" },
    { .compatible = "anyka,ak37d-ethernet" },
    { .compatible = "anyka,ak37e-ethernet" },
    { .compatible = "anyka,ak37e-ethernet1" },
    { .compatible = "anyka,ak3918av100-ethernet" },
    { .compatible = "anyka,km01a-ethernet" },
    { .compatible = "anyka,ak3918ev300l-ethernet" },
    { .compatible = "anyka,ak3918av130-ethernet" },
    {},
};
MODULE_DEVICE_TABLE(of, ak_ethernet_of_ids);

static struct platform_driver ak_mac_driver = {
    .driver = {
        .name    = "ak39_ethernet",
        .of_match_table = of_match_ptr(ak_ethernet_of_ids),
        .owner   = THIS_MODULE,
#ifdef CONFIG_PM
        .pm  = &ak_eth_mac_drv_pm_ops,
#endif
    },
    .probe   = ak_mac_probe,
    .remove  = ak_mac_remove,
};

/**
 * @BRIEF        mac drv init function
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    void.
 * @RETURN       int
 * @RETVAL       if probe ok,return 0, else return -n
 * @NOTES
 */
static int __init ak_mac_init(void)
{
    printk(KERN_ERR "%s Ethernet Driver\n", MACNAME);

    return platform_driver_register(&ak_mac_driver);
}

/**
 * @BRIEF        mac drv exit function
 * @AUTHOR       cao_donghua
 * @DATE         2020-02-26
 * @PARAM[in]    void.
 * @RETURN       void
 * @RETVAL       none
 * @NOTES
 */
static void __exit ak_mac_exit(void)
{
    platform_driver_unregister(&ak_mac_driver);
}

module_init(ak_mac_init);
module_exit(ak_mac_exit);

MODULE_DESCRIPTION("Anyka MAC driver");
MODULE_AUTHOR("Anyka Microelectronic");
MODULE_LICENSE("GPL");
MODULE_VERSION("2.0.01");
