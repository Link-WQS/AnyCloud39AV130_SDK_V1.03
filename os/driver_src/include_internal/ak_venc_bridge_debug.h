#ifndef __AK_VENC_BRIDGE_DEBUG_H__
#define __AK_VENC_BRIDGE_DEBUG_H__

enum debug_type {
    DBG_TYEP_NEW_SLICE = 0,
    DBG_TYPE_ENC_START,
    DBG_TYPE_ENC_FINISH,
    DBG_TYPE_ERROR,
};

enum debug_error_type {
    DBG_ERR_TYPE_NO_ERR = 0,
    DBG_ERR_TYPE_LOSS_SLICE,
    DBG_ERR_TYPE_ENC_BUSY,
    DBG_ERR_TYPE_CAP_EQ_ENC,
    DBG_ERR_TYPE_CAP_EQ_ENCP1,
};

enum debug_enc_mode {
    DBG_ENC_MODE_KRN = 0,   /*k2k*/
    DBG_ENC_MODE_USR,       /*u2k*/
};

struct debug_struct {
    unsigned long long current_time;
    unsigned int frame_index;
    signed char slice_index;
    signed char block_index;
    signed char enc_slice;
    signed char enc_block;

    enum debug_error_type error;
    enum debug_type type;

    enum debug_enc_mode enc_m;
    unsigned int bridge_handle;

    int hw_time;
    int total_time;

    signed char chn;
    signed char res[7];
};

#endif
