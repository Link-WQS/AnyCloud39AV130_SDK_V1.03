#ifndef _AK_UUID_WR_H_
#define _AK_UUID_WR_H_


/**
 * ak_efuse get custom_id bit length  - get custom_id git length
 * @pathname[OUT]: length
 * return: success return 0  or -1 on failed
 * notes:
 */
int ak_uuid_get_custom_id_length(int* lenght);

/**
 * ak_efuse get custom_id   - get custom_id
 * @pathname[OUT]: buf
 * return: return 0 on success   or -1 on failed
 * notes:
 */
int ak_uuid_get_custom_id(unsigned char** buf);

/**
 * ak_efuse set custom_id   - set custom_id
 * @pathname[IN]: buf_len
 * @pathname[IN]: buf
 * return: return 0 on success   or -1 on failed
 * notes:
 */
int ak_uuid_set_custom_id(unsigned char *buf,int buf_len);

#endif

/* end of file */
