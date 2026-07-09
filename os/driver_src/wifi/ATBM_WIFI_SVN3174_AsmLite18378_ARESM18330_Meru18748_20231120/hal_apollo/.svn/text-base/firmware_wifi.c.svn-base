//#ifdef CONFIG_SUPPORT_ARESB_FIRMWARE_H_
#include "apollo.h"
#include "fwio.h"
#if defined CONFIG_SUPPORT_ARESB_FIRMWARE_H_ || defined CONFIG_SUPPORT_WIFI4_ALL_FIRMWARE_H_

#ifdef CONFIG_ATBM_ONLY_WIFI_BLE_PLATFORM

int load_usb_wifi_firmware(struct firmware_altobeam *fw_altobeam)
{
	return -1;
}


#else //CONFIG_ATBM_ONLY_WIFI_BLE_PLATFORM


#ifdef CONFIG_USE_FW_H




#ifdef USB_BUS
#include "firmware_usb.h"
#endif
#ifdef SDIO_BUS
#include "firmware_sdio.h"
#endif
#ifdef SPI_BUS
#include "firmware_spi.h"
#endif




int load_usb_wifi_firmware(struct firmware_altobeam *fw_altobeam)
{
	fw_altobeam->hdr.iccm_len = sizeof(fw_code);
	fw_altobeam->hdr.dccm_len = sizeof(fw_data);

	fw_altobeam->fw_iccm = &fw_code[0];
	fw_altobeam->fw_dccm = &fw_data[0];


	fw_altobeam->hdr.sram_len = 0;

	return 0;
}

#endif //CONFIG_USE_FW_Hs

#endif//CONFIG_ATBM_ONLY_WIFI_BLE_PLATFORM

#else //defined CONFIG_SUPPORT_ARESB_FIRMWARE_H_ || defined CONFIG_SUPPORT_WIFI4_ALL_FIRMWARE_H_
int load_usb_wifi_firmware(struct firmware_altobeam *fw_altobeam)
{
	return -1;
}


#endif//defined CONFIG_SUPPORT_ARESB_FIRMWARE_H_ || defined CONFIG_SUPPORT_WIFI4_ALL_FIRMWARE_H_

