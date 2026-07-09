#ifndef __BT_USER_DRIVER_H__
#define __BT_USER_DRIVER_H__


enum aicbt_btport_type {
    AICBT_BTPORT_NULL,
    AICBT_BTPORT_MB,
    AICBT_BTPORT_UART,
};

/*  btmode
 * used for force bt mode,if not AICBSP_MODE_NULL
 * efuse valid and vendor_info will be invalid, even has beed set valid
*/
#if 0
enum aicbt_btmode_type {
    AICBT_BTMODE_BT_ONLY_SW = 0x0,    // bt only mode with switch
    AICBT_BTMODE_BT_WIFI_COMBO,       // wifi/bt combo mode
    AICBT_BTMODE_BT_ONLY,             // bt only mode without switch
    AICBT_BTMODE_BT_ONLY_TEST,        // bt only test mode
    AICBT_BTMODE_BT_WIFI_COMBO_TEST,  // wifi/bt combo test mode
    AICBT_BTMODE_BT_ONLY_COANT,       // bt only mode with no external switch
    AICBT_MODE_NULL = 0xFF,           // invalid value
};
#endif
#define AICBT_BTMODE_BT_ONLY_SW          0// bt only mode with switch
#define AICBT_BTMODE_BT_WIFI_COMBO       1// wifi/bt combo mode
#define AICBT_BTMODE_BT_ONLY             2// bt only mode without switch
#define AICBT_BTMODE_BT_ONLY_TEST        3// bt only test mode
#define AICBT_BTMODE_BT_WIFI_COMBO_TEST  4// wifi/bt combo test mode
#define AICBT_BTMODE_BT_ONLY_COANT       5// bt only mode with no external switch
#define AICBT_MODE_NULL                  0xFF// invalid value


/*  uart_baud
 * used for config uart baud when btport set to uart,
 * otherwise meaningless
*/
enum aicbt_uart_baud_type {
    AICBT_UART_BAUD_115200     = 115200,
    AICBT_UART_BAUD_921600     = 921600,
    AICBT_UART_BAUD_1_5M       = 1500000,
    AICBT_UART_BAUD_3_25M      = 3250000,
};

enum aicbt_uart_flowctrl_type {
    AICBT_UART_FLOWCTRL_DISABLE = 0x0,    // uart without flow ctrl
    AICBT_UART_FLOWCTRL_ENABLE,           // uart with flow ctrl
};

enum aicbsp_cpmode_type {
    AICBSP_CPMODE_WORK,
    AICBSP_CPMODE_TEST,
    AICBSP_CPMODE_MAX,
};

///aic bt tx pwr lvl :lsb->msb: first byte, min pwr lvl; second byte, max pwr lvl;
///pwr lvl:20(min), 30 , 40 , 50 , 60(max)
#define AICBT_TXPWR_LVL                   0x00006020
#define AICBT_TXPWR_LVL_8800dc            0x00006f2f
#define AICBT_TXPWR_LVL_8800d80           0x00006f2f

struct aicbsp_info_t {
    int hwinfo;
    int hwinfo_r;
    uint32_t cpmode;
    uint32_t chip_rev;
    bool fwlog_en;
};

struct aicbt_info_t {
    uint32_t btmode;
    uint32_t btport;
    uint32_t uart_baud;
    uint32_t uart_flowctrl;
    uint32_t lpm_enable;
    uint32_t txpwr_lvl;
};
/*
 * Launch the bt
 */
void bt_launch(void);

#endif