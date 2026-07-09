#ifndef _AK_NPU_H_
#define _AK_NPU_H_

#include "ak_common.h"

enum ak_npu_error_type {
    ERROR_NPU_DEV_INIT = (MODULE_ID_NPU << 24) + 0,
    ERROR_NPU_DEV_DEINIT,
    ERROR_NPU_RESET_HW,
    ERROR_NPU_COMMAND,
    ERROR_NPU_DEV_NOT_OPEN,
    ERROR_NPU_IOCTL,
};


/**
 * ak_npu_get_version: Get npu module version info
 * return: npu module version info
 */
const char* ak_npu_get_version(void);

/**
 * ak_npu_init - Init npu module
 * return: 0 success, other failed
 * notes:
 */
int ak_npu_init(void);

/**
 * ak_npu_deinit - Deinit npu module
 * return: 0 success, other failed
 * notes:
 */
int ak_npu_deinit(void);

/* ak_npu_reset_hw	-	reset SVP HW
 * return AK_SUCCESS is successful, error code otherwise.
 */
int ak_npu_reset_hw(void);

/**
 * ak_npu_get_nne_version - get nne in version
 * return: version string
 * notes:
 */

const char* ak_npu_get_nne_version(void);

#endif
