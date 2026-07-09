#include <regex.h>

#define ERGEX_HING_BUFFFULL "Target buffer is FULL."
#define CONF_LEN_RES     4096
#define CONF_LEN_PATTERN 1024
#define CONF_LEN_ERROR   1024
#define CONF_REG_STYLE   REG_EXTENDED | REG_NEWLINE
/*
    REG_EXTENDED - 扩展方式匹配
    REG_ICASE    - 忽略大小写
    REG_NOSUB    - 不存储匹配后的结果
    REG_NEWLINE  - 识别换行符
*/
#ifndef FREE_POINT
#define FREE_POINT( POINT )  \
if( POINT != NULL ) {\
    free( POINT ) ;\
    POINT = NULL ;\
}
#endif

enum REGEX_ACTION {
    //pc_pattern , i_regex_option , pc_split , i_split_option
    ACTION_PTRN_MATCH_ALL ,
    ACTION_PTRN_MATCH_ONE ,  //pc_pattern , i_regex_option
    ACTION_PTRN_DELETE_ALL , //pc_pattern , i_regex_option
    ACTION_PTRN_DELETE_ONE , //pc_pattern , i_regex_option
    ACTION_PTRN_REPLACE_ALL ,  //pc_pattern , i_regex_option , pc_replace
    ACTION_PTRN_REPLACE_ONE ,  //pc_pattern , i_regex_option , pc_replace
    ACTION_COMP_MATCH_ALL ,    //p_regex_t , pc_split , i_split_option
    ACTION_COMP_MATCH_ONE ,    //p_regex_t
    ACTION_COMP_DELETE_ALL ,   //p_regex_t
    ACTION_COMP_DELETE_ONE ,   //p_regex_t
    ACTION_COMP_REPLACE_ALL ,  //p_regex_t , pc_replace
    ACTION_COMP_REPLACE_ONE ,  //p_regex_t , pc_replace
};

enum SPLIT_OPTION {
    SPLIT_NONE ,
    SPLIT_FRONT ,
    SPLIT_TAIL ,
    SPLIT_MIDDLE ,
    SPLIT_FULL ,
};

enum EXEC_OPTION {
    EXEC_ALL ,
    EXEC_ONE ,
};

/**
 * ak_regexpr_get - ak_regexpr_get
 * pc_buff[IN] : pc_buff
 * pc_res_in[IN/OUT] : pc_res_in
 * i_len_res[IN] : i_len_res
 * i_regex_num[IN] : i_regex_num
 * return: i_len
 * notes:
 */
int ak_regexpr_get( char *pc_buff , char *pc_res_in , \
    int i_len_res , int i_regex_num , ... ) ;

/**
 * regexpr_match - regexpr_match
 * p_regex_t_in[IN] : p_regex_t_in
 * pc_pattern[IN] : pc_pattern
 * i_flags[IN] : i_flags
 * pc_buff[IN] : pc_buff
 * pc_res[IN] : pc_res
 * i_len_res[IN] : i_len_res
 * pc_split[IN] : pc_split
 * i_split_option[IN] : i_split_option
 * i_exec_option[IN] : i_exec_option
 * return: i_len_return
 * notes:
 */
int regexpr_match( regex_t *p_regex_t_in , char *pc_pattern ,\
    int i_flags , char *pc_buff , char *pc_res , int i_len_res , \
    char *pc_split , int i_split_option , int i_exec_option ) ;

/**
 * regexpr_replace - regexpr_replace
 * p_regex_t_in[IN] : p_regex_t_in
 * pc_pattern[IN] : pc_pattern
 * i_flags[IN] : i_flags
 * pc_buff[IN] : pc_buff
 * pc_res[IN] : pc_res
 * i_len_res[IN] : i_len_res
 * pc_replace[IN] : pc_replace
 * i_exec_option[IN] : i_exec_option
 * return: i_len_return
 * notes:
 */
int regexpr_replace( regex_t *p_regex_t_in , char *pc_pattern , \
    int i_flags , char *pc_buff , char *pc_res , int i_len_res , \
    char *pc_replace , int i_exec_option ) ;

/**
 * read_file - read_file
 * pc_filename[IN] : pc_filename
 * pi_filesize[OUT] : pi_filesize
 * return: pc_buff
 * notes:
 */
char *read_file( char *pc_filename , int *pi_filesize );

/**
 * get_conf_str - get_conf_str
 * pc_conf[IN] : pc_conf
 * pc_key[IN] : pc_key
 * ppc_val[OUT] : ppc_val
 * return: 0 success, otherwise failed
 * notes:
 */
int get_conf_str( char *pc_conf, char *pc_key, char **ppc_val );

/**
 * get_conf_int - get_conf_int
 * pc_conf[IN] : pc_conf
 * pc_key[IN] : pc_key
 * pi_val[OUT] : pi_val
 * return: 0 success, otherwise failed
 * notes:
 */
int get_conf_int( char *pc_conf, char *pc_key, int *pi_val );