/*
 * Includes
 *******************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef CFG_HOSTIF

#include "hostif.h"
#include "hostif_cfg.h"

#define HOSTIF_PKT_MEM_NUM_SUB_POOLS 3
#define HOSTIF_PKT_MEM_ALIGNMENT     32

#define HOSTIF_BUFFER_POOL_0_SIZE   (CONFIG_HOSTIF_RX_BUF_SIZE_MSG1 * CONFIG_HOSTIF_RX_BUF_NUM_MSG1 + \
                                     CONFIG_HOSTIF_RX_BUF_SIZE_MSG2 * CONFIG_HOSTIF_RX_BUF_NUM_MSG2 + \
                                     HOSTIF_PKT_MEM_ALIGNMENT * HOSTIF_PKT_MEM_NUM_SUB_POOLS)

#define HOSTIF_BUFFER_POOL_1_SIZE   (1624 * CONFIG_HOSTIF_BUF_NUM_TOTAL)
/*
 * Global variables
 *******************************************************************************
 */
const hostif_cfg_t hostif_cfg_tag = {
    .if_mode = CONFIG_HOSTIF_MODE,
    .filter_mode = CONFIG_HOSTIF_FILTER_MODE,
    .buf_num_total = CONFIG_HOSTIF_BUF_NUM_TOTAL,
    .rxbuf_num_thres = CONFIG_HOSTIF_RX_BUF_NUM_THRESHOLD,
    .rxbuf_num_msg1  = CONFIG_HOSTIF_RX_BUF_NUM_MSG1,
    .rxbuf_num_msg2  = CONFIG_HOSTIF_RX_BUF_NUM_MSG2,
    .rxbuf_size_msg1 = CONFIG_HOSTIF_RX_BUF_SIZE_MSG1,
    .rxbuf_size_msg2 = CONFIG_HOSTIF_RX_BUF_SIZE_MSG2,
};

__HOSTIF_BUF_POOL uint8_t hostif_buffer_pool_0[HOSTIF_BUFFER_POOL_0_SIZE];
__HOSTIF_BUF_POOL uint8_t hostif_buffer_pool_1[HOSTIF_BUFFER_POOL_1_SIZE];

/*
 * Functions
 *******************************************************************************
 */

HOST_TYPE_T hostif_type_get(void)
{
    #if defined(CONFIG_SDIO)
    return SDIO_HOST;
    #elif defined(CONFIG_USB)
    return USB_HOST;
    #endif
}

HOST_MODE_T hostif_mode_get(void)
{
    return hostif_cfg_tag.if_mode;
}

uint8_t hostif_filter_type_get(void)
{
    return hostif_cfg_tag.filter_mode;
}

uint8_t hostif_buf_num_total_get(void)
{
    return hostif_cfg_tag.buf_num_total;
}

uint8_t hostif_rxbuf_num_threshold_get(void)
{
    return hostif_cfg_tag.rxbuf_num_thres;
}

uint8_t hostif_msg1_buf_num_get(void)
{
    return hostif_cfg_tag.rxbuf_num_msg1;
}

uint8_t hostif_msg2_buf_num_get(void)
{
    return hostif_cfg_tag.rxbuf_num_msg2;
}

uint16_t hostif_msg1_buf_size_get(void)
{
    return hostif_cfg_tag.rxbuf_size_msg1;
}

uint16_t hostif_msg2_buf_size_get(void)
{
    return hostif_cfg_tag.rxbuf_size_msg2;
}
#endif /* CFG_HOSTIF */
