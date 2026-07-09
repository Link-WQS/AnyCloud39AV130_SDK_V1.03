#ifndef _AK_DBG_H_
#define _AK_DBG_H_

#define DBG_SUPPORT

enum dbg_type {
    DBG_TYPE_PARAM,
    DBG_TYPE_STAT,
};

enum dbg_act {
    DBG_ACT_ADD_STR ,
    DBG_ACT_ADD_INT ,
    DBG_ACT_ADD_DOUBLE ,
    DBG_ACT_ADD_OBJ ,
    DBG_ACT_DROP ,
    DBG_ACT_READ ,
};

/**
 * ak_debug_info - write debug information such as "title=string"
 * @module_id_param[IN]: module id
 * @dbg_type_param[IN]:  debug type
 * @str[IN]: such as "title=string"
 * return: 0 success, other failed
 * notes:
 */
int ak_debug_info( enum module_id module_id_in, \
    enum dbg_type dbg_type_param, char *pc_data );

/**
 * ak_dbg_get_version: Get dbg module version info
 * return: dbg module version info
 */
const char* ak_dbg_get_version(void);

/**
 * ak_set_debug_str_info - write debug information such as "title=string"
 * @module_id_param[IN]: module id
 * @dbg_type_param[IN]:  debug type
 * @title[IN]:  debug information title
 * @str[IN]:  debug information string
 * return: 0 success, other failed
 * notes:
 */
int ak_set_debug_str_info(enum module_id module_id_param, \
    enum dbg_type dbg_type_param, char *title, char *str);

/**
 * ak_set_debug_num_info - write debug information such as "title=number"
 * @module_id_param[IN]: module id
 * @dbg_type_param[IN]:  debug type
 * @title[IN]:  debug information title
 * @num[IN]:  debug information number
 * return: 0 success, other failed
 * notes:
 */
int ak_set_debug_num_info(enum module_id module_id_param, \
    enum dbg_type dbg_type_param, char *title, int num);

/**
 * ak_dbg_json - multipurpose interface with extensible parameters of dbg module.
 * @module_id_param[IN]: module id
 * @dbg_type_param[IN]:  debug type
 * @action[IN]: enum dbg_action type
 * @other parameters [IN|OUT]
 * return: 0 success, other failed
 * notes:
 */
int ak_dbg_json( enum module_id module_id_param, \
    enum dbg_type dbg_type_param, ... );

#endif

/* end of file */
