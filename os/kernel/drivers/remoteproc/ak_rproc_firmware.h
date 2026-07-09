#ifndef  __AK_RPROC_FIRMWARE_H__
#define  __AK_RPROC_FIRMWARE_H__

int ak_request_firmware_nowait(const char *name, struct device *device, gfp_t gfp, void *context,
	void (*cont)(const struct firmware *fw, void *context));

#endif/*__AK_RPROC_FIRMWARE_H__*/
