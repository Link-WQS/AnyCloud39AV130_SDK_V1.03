/**
 ****************************************************************************************
 *
 * @file fhost_console_moniror.c
 *
 * @brief Implementation of the pre-moniror source code.
 *
 ****************************************************************************************
 */

#include "rtos_al.h"

#if NX_FHOST_MONITOR
void dump_b(const uint8_t *buf, uint16_t len)
{
    int i;

    for (i=0;i<len;i++) {
        if((i%8 == 0))
            dbg("  ");
        dbg("%02x ", buf[i]);
        if((i+1)%16 == 0)
            dbg("\n");
    }
    dbg("\n");
}
/**
 ****************************************************************************************
 * @brief callback function
 *
 * Extract received packet informations (frame length, type, mac addr ...) in monitor mode
 *
 * @param[in] info  RX Frame information.
 * @param[in] arg   Not used
 ****************************************************************************************
 */
static void fhost_console_monitor_cb(struct fhost_frame_info *info, void *arg)
{
    if (info->payload == NULL) {
        dbg("Unsupported frame: length = %d\r\n", info->length);
    } else {
        struct mac_hdr *hdr = (struct mac_hdr *)info->payload;
        uint8_t *adr1 = ((uint8_t *)(hdr->addr1.array));
        uint8_t *adr2 = ((uint8_t *)(hdr->addr2.array));
        uint8_t *adr3 = ((uint8_t *)(hdr->addr3.array));
        dbg("a1=%02x:%02x:%02x:%02x:%02x:%02x a2=%02x:%02x:%02x:%02x:%02x:%02x "
            "a3=%02x:%02x:%02x:%02x:%02x:%02x fc=%04X SN:%d len=%d\n",
           adr1[0], adr1[1], adr1[2], adr1[3], adr1[4], adr1[5],
            adr2[0], adr2[1], adr2[2], adr2[3], adr2[4], adr2[5],
            adr3[0], adr3[1], adr3[2], adr3[3], adr3[4], adr3[5],
            hdr->fctl, hdr->seq >> 4, info->length);
    }
}
/**
 ****************************************************************************************
 * @brief Process function for 'monitor' command
 *
 * monitor command can be used to start the monitor mode
 *
   @verbatim
     monitor start <freq> <20|40|80|80+80|160> <center freq 1> <center freq 2>
     monitor stop
   @endverbatim
 *
 * @param[in] params monitor_start/stop commands above
 * @return 0 on success and !=0 if error occurred
 * @e.g   monitor start wl1 2412 20 2412 2412
 ****************************************************************************************
 */
static int do_monitor(int argc, char * const argv[])
{
    #define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

    const struct {
        const char *name;
        u8_t val;
    } bwmap[] = {
        { .name = "20", .val = PHY_CHNL_BW_20, },
        { .name = "40", .val = PHY_CHNL_BW_40, },
        { .name = "80", .val = PHY_CHNL_BW_80, },
        { .name = "80+80", .val = PHY_CHNL_BW_80P80, },
        { .name = "160", .val = PHY_CHNL_BW_160, },
    };

    ipc_host_cntrl_start();

    console_cntrl_link = fhost_cntrl_cfgrwnx_link_open();
    if (console_cntrl_link == NULL) {
        dbg(D_ERR "Failed to open link with control task\n");
        ASSERT_ERR(0);
    }

    int ret = 0;

    char *subcmd;
    subcmd = argv[1];
    if (!strcmp("start", subcmd)) {
        struct fhost_vif_monitor_cfg cfg;
        struct mac_chan_def *chan;
        unsigned int i, freq_offset;
        char /* *itf, */ *bw_str;
        int fhost_vif_idx = 0;

        if (argc < 6) {
            dbg("wrong # of args\n");
            ret = -1;
            goto err;
        }

        #if 0
        itf = argv[2];
        // get the interface index
        fhost_vif_idx = fhost_console_search_itf(itf);
        if (fhost_vif_idx < 0) {
            ret = -2;
            goto err;
        }
        #endif

        // frequency
        cfg.chan.prim20_freq = console_cmd_strtoul(argv[2], NULL, 0);
        chan = fhost_chan_get(cfg.chan.prim20_freq);
        if (chan == NULL) {
            dbg("Invalid freq %d\n", cfg.chan.prim20_freq);
            ret = -3;
            goto err;
        }
        cfg.chan.band = chan->band;
        cfg.chan.tx_power = chan->tx_power;

        // by default 20Mhz bandwidth
        cfg.chan.type = PHY_CHNL_BW_20;
        cfg.chan.center1_freq = cfg.chan.prim20_freq;
        cfg.chan.center2_freq = 0;

        // bw
        bw_str = argv[3];
        for (i = 0; i < ARRAY_SIZE(bwmap); i++) {
            if (strcmp(bwmap[i].name, bw_str) == 0) {
                cfg.chan.type = bwmap[i].val;
                break;
            }
        }

        // center1_freq
        cfg.chan.center1_freq = console_cmd_strtoul(argv[4], NULL, 0);

        if (cfg.chan.center1_freq > cfg.chan.prim20_freq)
            freq_offset = cfg.chan.center1_freq - cfg.chan.prim20_freq;
        else
            freq_offset = cfg.chan.prim20_freq - cfg.chan.center1_freq;

        switch(cfg.chan.type) {
            case PHY_CHNL_BW_20:
                if (freq_offset != 0) {
                    dbg("monitor_start :"
                        "Center frequency of primary channel different from "
                        "frequency of primary channel in 20MHz (%d != %d)\n",
                        cfg.chan.center1_freq, cfg.chan.prim20_freq);
                    ret = -4;
                    goto err;
                }
                break;
            case PHY_CHNL_BW_40:
                if (freq_offset != 10) {
                    dbg("monitor_start :"
                        "Center frequency of primary channel different from "
                        "frequency of primary channel +/- 10 in 40MHz (%d != %d)\n",
                        cfg.chan.center1_freq, cfg.chan.prim20_freq);
                    ret = -5;
                    goto err;
                }
                break;
            case PHY_CHNL_BW_80P80:
                //center2_freq
                if (argc < 7) {
                    dbg("monitor_start :"
                        "Center frequency of secondary channel must be set\n");
                    ret = -6;
                    goto err;
                }
                cfg.chan.center2_freq = console_cmd_strtoul(argv[5], NULL, 0);

                //adjacent channel rejection
                if ((cfg.chan.center1_freq - cfg.chan.center2_freq == 80) ||
                    (cfg.chan.center2_freq - cfg.chan.center1_freq == 80)) {
                    dbg("monitor_start :"
                        "Adjacent channel is not allowed, use 160MHz bandwidth\n");
                    ret = -7;
                    goto err;
                }

                // no break
            case PHY_CHNL_BW_80:
                if ((freq_offset != 10) && (freq_offset != 30)) {
                    dbg("monitor_start :"
                        "Center frequency of primary channel different from "
                        "frequency of primary channel +/- 10 and frequency of"
                        "primary channel +/- 30 (%d != %d)\n",
                        cfg.chan.center1_freq, cfg.chan.prim20_freq);
                    ret = -8;
                    goto err;
                }
                break;
            case PHY_CHNL_BW_160:
                if ((freq_offset != 10) && (freq_offset != 30) &&
                    (freq_offset != 50) && (freq_offset != 70)) {
                    dbg("monitor_start :"
                        "Center frequency of primary channel must belong to the range:"
                        "frequency of primary channel +/- [10, 30, 50, 70]\n");
                    ret = -9;
                    goto err;
                }
                break;
            default:
                dbg("monitor_start :"
                    "Invalid bandwidth %d\n", cfg.chan.type);
                ret = -10;
                goto err;
        }

        if (fhost_set_vif_type(console_cntrl_link, fhost_vif_idx, VIF_MONITOR, false)) {
            dbg("Error while enabling monitor mode\n");
            ret = -11;
            goto err;
        }

        cfg.uf = true;
        cfg.cb = fhost_console_monitor_cb;
        cfg.cb_arg = NULL;

        if (fhost_cntrl_monitor_cfg(console_cntrl_link, fhost_vif_idx, &cfg)) {
            dbg("Error while configuring monitor mode\n");
            ret = -12;
            goto err;
        }
    } else if (!strcmp("stop", subcmd)) {
        //char *itf;
        int fhost_vif_idx = 0;

        #if 0
        itf = argv[2];
        // get the interface index
        fhost_vif_idx = fhost_console_search_itf(itf);
        if (fhost_vif_idx < 0) {
            ret = -13;
            goto err;
        }
        #endif

        if (fhost_set_vif_type(console_cntrl_link, fhost_vif_idx, VIF_STA, false)) {
            dbg("Error while disabling monitor mode\n");
            ret = -14;
            goto err;
        }
    } else {
        dbg("monitor: invalid subcmd\n");
        ret = -15;
        goto err;
    }

err:
    fhost_cntrl_cfgrwnx_link_close(console_cntrl_link);
    return ret;
}
static int do_monitor_tx(int argc, char * const argv[])
{
    uint8_t pkt[] = {
                    0x40,0x00,0x00,0x00,                  // probe request
                    0xff,0xff,0xff,0xff,0xff,0xff,
                    0x82,0xc6,0x2f,0xad,0x5d,0xc1,        //SA
                    0xff,0xff,0xff,0xff,0xff,0xff,
                    0xe0,0xd3,0x00,0x00,0x01,0x08,0x82,0x84,0x8b,0x0c,
                    0x12,0x96,0x18,0x24,0x03,0x01,0x01,0x32,0x04,0x30,0x48,0x60,0x6c,0x7f,0x0a,0x01,
                    0x00,0x08,0x00,0x00,0x00,0x00,0x40,0x70,0x20,0x2d,0x1a,0xef,0x01,0x17,0xff,0xff,
                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,
                    0x00,0x00,0x00,0x00,0x00,0xdd,0x08,0x00,0xe0,0xfc,0x81,0xf1,0x1e,0x05,0x00,0xdd,
                    0x07,0x00,0x50,0xf2,0x08,0x00,0x19,0x00
    };
    memcpy(&pkt[10], get_mac_address(), 6);
    fhost_send_80211_frame(0, pkt, sizeof(pkt), NULL, NULL);
    return 0;
}
static int do_monitor_set_filter(int argc, char * const argv[])
{
    uint32_t val = console_cmd_strtoul(argv[1], NULL, 10);

    fhost_cntrl_mm_set_filter(val);

    return 0;
}

#endif // NX_FHOST_MONITOR
static void fhost_monitor_command_add(void)
{
    #if NX_FHOST_MONITOR
    console_cmd_add("monitor",  "start <freq> <20|40|80|80+80|160> <center freq 1> <center freq 2>\n"
                                "stop", 6, 6, do_monitor);
    console_cmd_add("m_tx", "- Monitor Tx", 1, 1, do_monitor_tx);
    console_cmd_add("m_filter", "- Monitor Filter", 1, 1, do_monitor_set_filter);
    #endif /* NX_FHOST_MONITOR */
}

