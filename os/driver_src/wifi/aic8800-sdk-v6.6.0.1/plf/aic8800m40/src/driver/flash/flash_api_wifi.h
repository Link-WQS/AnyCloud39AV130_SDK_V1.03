#ifndef _FLASH_API_WIFI_H_
#define _FLASH_API_WIFI_H_

/**
 * Includes
 */
#include "flash_api.h"

/**
 * Structs
 */
/* wifi info */
typedef struct {
    char ssid[48];
    char passwd[64];
} wifi_ssidpw_t;

typedef struct {
    unsigned int  config;
    unsigned char mac_addr[6];
    unsigned char PADDING0[2];
    wifi_ssidpw_t ssidpw;
    //unsigned char pmk[32];
} wifi_sta_info_t;

typedef struct {
    unsigned int  config;
    unsigned char mac_addr[6];
    unsigned char PADDING0[2];
    wifi_ssidpw_t ssidpw;
    unsigned int channel;
} wifi_ap_info_t;

typedef struct {
    unsigned int lease_time;
    unsigned int dhcp_start;
    unsigned int dhcp_end;
} wifi_lwip_dhcps_t;

typedef struct {
    signed int   time_zone;
} wifi_lwip_sntp_t;

typedef struct {
    unsigned int config;
    wifi_lwip_dhcps_t dhcps;
    unsigned int dns_server;
    wifi_lwip_sntp_t sntp;
} wifi_lwip_info_t;

/* wifi sta config */
#define WIFI_STA_CONFIG_STA_AUTO_CONNECT_EN             CO_BIT(0)

/* wifi ap config */
#define WIFI_AP_CONFIG_FORCED_AP_MODE_EN                CO_BIT(0)

/* wifi lwip config */
#define WIFI_LWIP_CONFIG_SNTP_CLIENT_AUTO_CONNECT_EN    CO_BIT(0)

typedef struct {
    unsigned int magic_num; /* "WIFI" */
    unsigned int info_flag;
    unsigned int RESERVED_08;
    unsigned int RESERVED_0C;
    wifi_sta_info_t  sta_info;
    wifi_ap_info_t   ap_info;
    wifi_lwip_info_t lwip_info;
} wifi_info_t;

enum {
    DPD_CHAN_GRP_2G4 = 0,
    DPD_CHAN_GRP_5G_CH_42,
    DPD_CHAN_GRP_5G_CH_58,
    DPD_CHAN_GRP_5G_CH_106,
    DPD_CHAN_GRP_5G_CH_122,
    DPD_CHAN_GRP_5G_CH_138,
    DPD_CHAN_GRP_5G_CH_155,
    DPD_CHAN_GRP_2G4_5G_NUM,
};

enum {
    DPD_TEMP_LVL_0 = 0, // <-15
    DPD_TEMP_LVL_1,     // < 5
    DPD_TEMP_LVL_2,     // < 25
    DPD_TEMP_LVL_3,     // < 45
    DPD_TEMP_LVL_4,     // < 65
    DPD_TEMP_LVL_5,     // > 85
    DPD_TEMP_LVL_6,     // > 105
    DPD_TEMP_LVL_7,     // >=105
    DPD_TEMP_LVL_NUM,
};

typedef struct {
    unsigned char ch_cur_idx; // 2.4g + 5g : 1 + 6 ch
    unsigned char dpd_temp_lvl;
    signed char   dpd_degree_val;
    unsigned char RESERVED_03;
} rf_dpd_info_t;

typedef struct {
    unsigned int bit_mask[3];
    unsigned int reserved;
    unsigned int dpd_11b[96];
    unsigned int dpd_high[96];
    unsigned int dpd_low[96];
    unsigned int loft_res[24];
} rf_misc_ram_t;

typedef struct {
    unsigned int magic_num; /* "DCR0", "DCR1", "DCR2" */
    unsigned int info_flag;
    unsigned int RESERVED_08;
    unsigned int RESERVED_0C;
    rf_dpd_info_t rf_dpd_info[3];
    unsigned int RESERVED_1C;
    rf_misc_ram_t rf_misc_ram[3];
} wifi_rf_cal_info_t;

/**
 * wifi info api
 */
int flash_wifi_sta_config_read(unsigned int *config);
void flash_wifi_sta_config_write(unsigned int *config);
int flash_wifi_sta_macaddr_read(unsigned char *addr);
void flash_wifi_sta_macaddr_write(unsigned char *addr);
int flash_wifi_sta_ssidpw_read(char *ssid, char *pass);
void flash_wifi_sta_ssidpw_write(char *ssid, char *pass);
int flash_wifi_ap_config_read(unsigned int *config);
void flash_wifi_ap_config_write(unsigned int *config);
int flash_wifi_ap_macaddr_read(unsigned char *addr);
void flash_wifi_ap_macaddr_write(unsigned char *addr);
int flash_wifi_ap_ssidpw_read(char *ssid, char *pass);
void flash_wifi_ap_ssidpw_write(char *ssid, char *pass);
int flash_wifi_ap_channel_read(unsigned int *channel);
void flash_wifi_ap_channel_write(unsigned int channel);
int flash_wifi_lwip_config_read(unsigned int *config);
void flash_wifi_lwip_config_write(unsigned int *config);
int flash_wifi_lwip_dhcps_read(unsigned int *lease_time, unsigned int *dhcp_start, unsigned int *dhcp_end);
void flash_wifi_lwip_dhcps_write(unsigned int lease_time, unsigned int dhcp_start, unsigned int dhcp_end);
int flash_wifi_lwip_dnsserv_read(unsigned int *dns_server);
void flash_wifi_lwip_dnsserv_write(unsigned int dns_server);
int flash_wifi_lwip_sntp_read(signed int *time_zone);
void flash_wifi_lwip_sntp_write(signed int time_zone);
void flash_wifi_info_remove_all(void);
void flash_wifi_info_remove_ssidpw(void);
int flash_wifi_dpd_cal_res_valid(int ch_grp_idx, int dpd_temp_lvl, int *dpd_degree_out);
int flash_wifi_dpd_cal_res_read(int ch_grp_idx, int dpd_temp_lvl, void *buf_out);
int flash_wifi_dpd_cal_res_write(int ch_grp_idx, int dpd_temp_lvl, int dpd_degree_val, void *buf_tmp, void *buf_in);

#endif /* _FLASH_API_WIFI_H_ */
