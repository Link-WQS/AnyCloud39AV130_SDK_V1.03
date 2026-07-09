#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <regex.h>

#include "ak_common.h"
#include "ak_conf.h"

char ac_buff_val[ CONF_LEN_RES ];
int i_offset = 0;

unsigned int get_file_size( char *pc_filename )
{
    struct stat stat_buf;
    if( stat( pc_filename , &stat_buf ) < 0 ){
        return 0 ;
    }
    return ( unsigned int )stat_buf.st_size ;
}

/**
 * read_file - read_file
 * pc_filename[IN] : pc_filename
 * pi_filesize[OUT] : pi_filesize
 * return: pc_buff
 * notes:
 */
char *read_file( char *pc_filename , int *pi_filesize )
{
    int i_filesize , i_fd_conf , i_read ;
    char *pc_buff ;

    /*初始化保存字符串信息的缓冲区*/
    memset( ac_buff_val, 0, CONF_LEN_RES );

    if ( ( i_filesize = get_file_size( pc_filename ) ) == 0 ){
        return NULL ;
    }
    i_fd_conf = open( pc_filename , O_RDONLY ) ;
    if ( i_fd_conf > 0 ) {
        *pi_filesize = i_filesize ;
        pc_buff = ( char * )calloc( 1, i_filesize * sizeof( char ) + 1 ) ;
        i_read = read( i_fd_conf , pc_buff , i_filesize ) ;
        close( i_fd_conf ) ;
        if ( i_read == i_filesize ){
            return pc_buff ;
        }
        else {
            free( pc_buff ) ;
            return NULL ;
        }
    }
    return NULL ;
}

/**
 * get_conf_str - get_conf_str
 * pc_conf[IN] : pc_conf
 * pc_key[IN] : pc_key
 * ppc_val[OUT] : ppc_val
 * return: 0 success, otherwise failed
 * notes:
 */
int get_conf_str( char *pc_conf, char *pc_key, char **ppc_val )
{
    char ac_res[ CONF_LEN_RES ], ac_pattern[ CONF_LEN_PATTERN ];
    int i_res = 0;

    /*创建键值的正则表达式**/
    snprintf( ac_pattern, CONF_LEN_PATTERN, "^%s\\s*=.*", pc_key );

    i_res = ak_regexpr_get( pc_conf , ac_res , CONF_LEN_RES, 3 ,
                ACTION_PTRN_MATCH_ONE , ac_pattern , CONF_REG_STYLE,
                ACTION_PTRN_MATCH_ONE , "=[^=]+$" , CONF_REG_STYLE,
                ACTION_PTRN_MATCH_ONE , "[0-9a-zA-Z/._]+" , CONF_REG_STYLE);
    if ( ( i_res > 0 ) && ( i_offset + i_res < CONF_LEN_RES ) ) {
        *ppc_val = ac_buff_val + i_offset;
        memcpy( *ppc_val, ac_res, i_res );
        i_offset += ( i_res + 1 ) ;
        printf( "key= '%s' val= '%s'\n", pc_key, *ppc_val );
        return AK_SUCCESS;
    }
    else {
        return AK_FAILED;
    }
}

/**
 * get_conf_int - get_conf_int
 * pc_conf[IN] : pc_conf
 * pc_key[IN] : pc_key
 * pi_val[OUT] : pi_val
 * return: 0 success, otherwise failed
 * notes:
 */
int get_conf_int( char *pc_conf, char *pc_key, int *pi_val )
{
    char ac_res[ CONF_LEN_RES ], ac_pattern[ CONF_LEN_PATTERN ];
    int i_res = 0;

    /*创建键值的正则表达式**/
    snprintf( ac_pattern, CONF_LEN_PATTERN, "^%s\\s*=.*", pc_key );

    i_res = ak_regexpr_get( pc_conf , ac_res , CONF_LEN_RES, 3 ,
                ACTION_PTRN_MATCH_ONE , ac_pattern , CONF_REG_STYLE,
                ACTION_PTRN_MATCH_ONE , "=[^=]+$" , CONF_REG_STYLE,
                ACTION_PTRN_MATCH_ONE , "[0-9]+" , CONF_REG_STYLE);
    if ( i_res > 0 ) {
        *pi_val = atoi( ac_res );
        printf( "key= '%s' val= %d\n", pc_key, *pi_val );
        return AK_SUCCESS;
    }
    else {
        return AK_FAILED;
    }
}
/*获取全部匹配正则表达式的字符串**/
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
int regexpr_match( regex_t *p_regex_t_in , char *pc_pattern , int i_flags , \
    char *pc_buff , char *pc_res , int i_len_res , char *pc_split , \
    int i_split_option , int i_exec_option )
{
    int i_status , i_len_match = 0 , i_len_return = 0 , i_len_split = 0 ;
    regex_t regex_t_pattern , *p_regex_t_now = NULL ;
    char ac_error[ CONF_LEN_ERROR ] , *pc_now = pc_buff , c_buff_full = AK_FALSE ;
    regmatch_t regmatch_t_res ;

    if ( p_regex_t_in != NULL ) {
        p_regex_t_now = p_regex_t_in ;
    }
    else if ((i_status = regcomp( &regex_t_pattern , pc_pattern , i_flags)) \
        == REG_NOERROR) {
        p_regex_t_now = &regex_t_pattern ;
    }
    else {
        regerror( i_status , &regex_t_pattern , ac_error , CONF_LEN_ERROR ) ;
        printf( "i_status= %d pc_pattern= '%s' ac_error= '%s'\n" ,
                i_status , pc_pattern , ac_error );
        return 0 ;
    }

    if ( i_split_option != SPLIT_NONE ) {
        i_len_split = strlen( pc_split ) ;
        if ( i_len_split <= 0 ) {
            i_split_option = SPLIT_NONE ;
        }
    }

    /*返回的字符串长度*/
    i_len_return = 0 ;
    while ( ( i_status = regexec( p_regex_t_now , pc_now , 1 , \
        &regmatch_t_res , 0 ) ) == 0 ) {
        if ( ( i_len_match = regmatch_t_res.rm_eo - \
            regmatch_t_res.rm_so ) <= 0 ) {
            /*匹配到的字符串长度为0*/
            break ;
        }

        switch( i_split_option ) {
            /*分隔符前后都有**/
            case SPLIT_FULL :
                if ( ( i_len_return + i_len_split ) < i_len_res ) {
                    /*目标缓冲区未满**/
                    if ( i_len_return == 0 ) {
                        /*判断是否存在数据,
                                           没有有数据才拷贝分隔符**/
                        /*拷贝分隔符**/
                        memcpy( pc_res + i_len_return, pc_split, i_len_split);
                        /*设置返回的字符串长度*/
                        i_len_return += i_len_split;
                    }
                }
                else {
                    c_buff_full = AK_TRUE ;
                }
                break ;
            /*分隔符在前**/
            case SPLIT_FRONT :
                if ( ( i_len_return + i_len_split ) < i_len_res ) {
                    /*目标缓冲区未满**/
                    /*拷贝分隔符**/
                    memcpy( pc_res + i_len_return , pc_split , i_len_split);
                    /*设置返回的字符串长度*/
                    i_len_return += i_len_split;
                }
                else {
                    c_buff_full = AK_TRUE ;
                }
                break ;
            /*分隔符在结果之间*/
            case SPLIT_MIDDLE :
                if ( ( i_len_return + i_len_split ) < i_len_res ) {
                    /*目标缓冲区未满**/
                    if ( i_len_return != 0 ) {
                        /*判断是否存在数据,
                                           有数据才拷贝分隔符**/
                        /*拷贝分隔符**/
                        memcpy( pc_res + i_len_return, pc_split, i_len_split);
                        /*设置返回的字符串长度*/
                        i_len_return += i_len_split;
                    }
                }
                else {
                    c_buff_full = AK_TRUE ;
                }
                break ;
        }

        if( c_buff_full != AK_TRUE ) {
            /*判断缓冲区是否已满,未满则拷贝数据**/
            if ( ( i_len_return + i_len_match ) < i_len_res ) {
                /*目标缓冲区未满**/
                memcpy( pc_res + i_len_return , \
                    pc_now + regmatch_t_res.rm_so , i_len_match ) ;
                /*设置返回的字符串长度*/
                i_len_return += i_len_match ;
                /*跳转到替换结果后的地址*/
                pc_now += regmatch_t_res.rm_so + i_len_match ;
            }
            else {
                /*目标缓冲区已满,部分拷贝*/
                c_buff_full = AK_TRUE ;
                memcpy( pc_res + i_len_return , \
                    pc_now + regmatch_t_res.rm_so, i_len_res - 1 - i_len_return);
                i_len_return = i_len_res - 1 ;
            }
        }

        if( c_buff_full != AK_TRUE ) {
            /*分隔符在后**/
            switch( i_split_option ) {
                case SPLIT_TAIL :
                /*分隔符前后都有**/
                case SPLIT_FULL :
                    if ( ( i_len_return + i_len_split ) < i_len_res ) {
                        /*目标缓冲区未满**/
                        /*拷贝分隔符**/
                        memcpy( pc_res + i_len_return, pc_split, i_len_split);
                        /*设置返回的字符串长度*/
                        i_len_return += i_len_split;
                    }
                    else {
                        c_buff_full = AK_TRUE ;
                    }
                    break ;
            }
        }

        pc_res[ i_len_return ] = 0x0 ;
        if ( ( i_exec_option == EXEC_ONE ) || ( c_buff_full == AK_TRUE ) ) {
            break ;
        }
    }
    if( p_regex_t_now == &regex_t_pattern ) {
        regfree( &regex_t_pattern ) ;
    }
    return i_len_return ;
}
/*根据正则表达式替换字符串*/
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
    int i_flags , char *pc_buff , char *pc_res , int i_len_res ,\
    char *pc_replace , int i_exec_option )
{
    int i_status , i_len_replace = 0 , i_len_match = 0 , i_len_now = 0 , \
        i_len_return = 0 , i_len_move = 0 , i_len_limit = 0 , i_len_cpy = 0 ;;
    regex_t regex_t_pattern , *p_regex_t_now = NULL ;
    char ac_error[ CONF_LEN_ERROR ] , *pc_now = pc_res , \
        ac_match_tmp[ CONF_LEN_ERROR ] ;
    regmatch_t regmatch_t_res ;

    if ( p_regex_t_in != NULL ) {
        p_regex_t_now = p_regex_t_in ;
    }
    else if ( ( i_status = regcomp( &regex_t_pattern , \
        pc_pattern , i_flags ) ) == REG_NOERROR ) {
        p_regex_t_now = &regex_t_pattern ;
    }
    else {
        regerror( i_status , &regex_t_pattern , ac_error , CONF_LEN_ERROR ) ;
        printf( "i_status= %d pc_pattern= '%s' ac_error= '%s'\n" ,
                i_status , pc_pattern , ac_error );
        return 0 ;
    }
    if( pc_replace != NULL ) {
        /*判断是否有replace的字符串*/
        /*获取替换字符串的长度*/
        i_len_replace = strlen( pc_replace ) ;
    }

    /*将源字符串先拷贝到返回结果中去**/
    strncpy( pc_res , pc_buff , i_len_res - 1 ) ;
    pc_res[ i_len_res - 1 ] = 0x0 ;
    /*pc_now指向地址后的字符串长度**/
    i_len_now = strlen( pc_res ) ;

    /*返回的字符串长度*/
    i_len_return = i_len_now ;

    while ( ( i_len_now > 0 ) && ( i_status = regexec( p_regex_t_now , \
        pc_now , 1 , &regmatch_t_res , 0 ) ) == 0 ) {
        if ( ( i_len_match = regmatch_t_res.rm_eo - \
            regmatch_t_res.rm_so ) <= 0 ) {
            /*匹配到的字符串长度为0*/
            break ;
        }

        memcpy( ac_match_tmp , pc_now + regmatch_t_res.rm_so , i_len_match ) ;
        ac_match_tmp[ i_len_match ] = 0 ;
        /*设置返回的字符串长度*/
        i_len_return = i_len_return - i_len_match + i_len_replace ;
        if( i_len_return < i_len_res ) {
            /*当前返回长度小于于缓冲区长度*/
            memmove( pc_now + regmatch_t_res.rm_so + i_len_replace  , \
                pc_now + regmatch_t_res.rm_eo, i_len_now - regmatch_t_res.rm_eo);
            if ( i_len_replace != 0 ) {
                /*有替换字符串*/
                memcpy( pc_now + regmatch_t_res.rm_so , \
                pc_replace , i_len_replace ) ;
            }
            /*跳转到替换结果后的地址*/
            pc_now += regmatch_t_res.rm_so + i_len_replace ;
            /*剩下没检索的长度*/
            i_len_now -= regmatch_t_res.rm_eo ;
        }
        else {
            /*返回结果缓冲区已满**/
            /*计算可以移动的长度**/
            i_len_move = i_len_now - regmatch_t_res.rm_eo - \
                ( i_len_return + 1 - i_len_res ) ;
            if ( i_len_move > 0 ) {
                /*移动字符串**/
                memmove( pc_now + regmatch_t_res.rm_so + i_len_replace  ,\
                    pc_now + regmatch_t_res.rm_eo , i_len_move ) ;
            }
            /*剩余缓冲区长度**/
            i_len_limit = i_len_res - 1 - \
                ( pc_now + regmatch_t_res.rm_so - pc_res ) ;
            if( i_len_limit >= i_len_replace ) {
                /*获取拷贝的替换字符串的长度**/
                i_len_cpy = i_len_replace ;
            }
            else {
                i_len_cpy = i_len_limit ;
            }
            /*拷贝替换字符串**/
            memcpy( pc_now + regmatch_t_res.rm_so, pc_replace, i_len_cpy);
            /*跳转到替换结果后的地址*/
            pc_now += regmatch_t_res.rm_so + i_len_cpy ;
            /*剩下没检索的长度*/
            i_len_now = i_len_res - 1 - ( pc_now - pc_res ) ;
            i_len_return = i_len_res - 1 ;
            printf( "%s\n" , ERGEX_HING_BUFFFULL );
        }
        pc_res[ i_len_return ] = 0x0 ;
        if( i_exec_option == EXEC_ONE ) {
            break ;
        }
    }

    if( p_regex_t_now == &regex_t_pattern ) {
        regfree( &regex_t_pattern ) ;
    }
    return i_len_return ;
}

/*通过多个正则表达式获取最终的字符串结果**/
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
    int i_len_res , int i_regex_num , ... )
{
    va_list va_list_regex ;
    char *pc_pattern = NULL , *pc_split = NULL , *pc_replace = NULL , \
        *pc_res_tmp = NULL , *pc_source = NULL , *pc_target = NULL ;
    int i , i_len = 0 , i_action , i_regex_option , i_split_option ;
    regex_t *p_regex_t ;

    if ( i_len_res > 0 ) {
        pc_res_tmp = ( char * )calloc( 1 , i_len_res ) ;
    }
    else {
        return 0 ;
    }
    pc_res_in[ 0 ] = 0 ;
    va_start( va_list_regex, i_regex_num ) ;

    for( i = 0 ; i < i_regex_num ; i ++ ) {
        if ( ( pc_source == NULL ) || ( pc_target == NULL ) ) {
            pc_source = pc_buff ;
            pc_target = pc_res_in ;
        }
        else {
            if( pc_target == pc_res_in ) {
                pc_target = pc_res_tmp ;
                pc_source = pc_res_in ;
            }
            else {
                pc_target = pc_res_in ;
                pc_source = pc_res_tmp ;
            }
        }
        i_action = ( int )va_arg( va_list_regex , int ) ;
        switch( i_action ) {
            /*提取全部符合的字符串*/
            case ACTION_PTRN_MATCH_ALL :
                pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
                i_regex_option = ( int )va_arg( va_list_regex , int ) ;
                pc_split = ( char * )va_arg( va_list_regex , char * ) ;
                i_split_option = ( int )va_arg( va_list_regex , int ) ;
                i_len = regexpr_match( NULL , pc_pattern , i_regex_option , \
                    pc_source , pc_target , i_len_res , pc_split , \
                    i_split_option , EXEC_ALL ) ;
                break ;
            /*提取一个字符串*/
            case ACTION_PTRN_MATCH_ONE :
                pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
                i_regex_option = ( int )va_arg( va_list_regex , int ) ;
                i_len = regexpr_match( NULL , pc_pattern , i_regex_option ,\
                    pc_source , pc_target , i_len_res , NULL , SPLIT_NONE , \
                    EXEC_ONE ) ;
                break ;
            /*删除全部字符串**/
            case ACTION_PTRN_DELETE_ALL :
                pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
                i_regex_option = ( int )va_arg( va_list_regex , int ) ;
                i_len = regexpr_replace( NULL , pc_pattern , i_regex_option , \
                    pc_source , pc_target , i_len_res , NULL , EXEC_ALL ) ;
                break ;
            /*删除一个字符串*/
            case ACTION_PTRN_DELETE_ONE :
                pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
                i_regex_option = ( int )va_arg( va_list_regex , int ) ;
                i_len = regexpr_replace( NULL , pc_pattern , i_regex_option , \
                    pc_source , pc_target , i_len_res , NULL , EXEC_ONE ) ;
                break ;
            /*替换全部字符串**/
            case ACTION_PTRN_REPLACE_ALL :
                pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
                i_regex_option = ( int )va_arg( va_list_regex , int ) ;
                pc_replace = ( char * )va_arg( va_list_regex , char * ) ;
                i_len = regexpr_replace( NULL , pc_pattern , i_regex_option , \
                    pc_source , pc_target , i_len_res , pc_replace , EXEC_ALL);
                break ;
            /*替换一个字符串*/
            case ACTION_PTRN_REPLACE_ONE :
                pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
                i_regex_option = ( int )va_arg( va_list_regex , int ) ;
                pc_replace = ( char * )va_arg( va_list_regex , char * ) ;
                i_len = regexpr_replace( NULL , pc_pattern , i_regex_option , \
                    pc_source , pc_target , i_len_res , pc_replace , EXEC_ONE);
                break ;
            /*提取全部符合的字符串*/
            case ACTION_COMP_MATCH_ALL :
                p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
                pc_split = ( char * )va_arg( va_list_regex , char * ) ;
                i_split_option = ( int )va_arg( va_list_regex , int ) ;
                i_len = regexpr_match( p_regex_t , NULL , 0  , pc_source , \
                    pc_target , i_len_res , pc_split , i_split_option , EXEC_ALL);
                break ;
            /*提取一个字符串*/
            case ACTION_COMP_MATCH_ONE :
                p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
                i_len = regexpr_match( p_regex_t , NULL , 0  , pc_source , \
                    pc_target , i_len_res , NULL , SPLIT_NONE , EXEC_ONE ) ;
                break ;
            /*删除全部字符串**/
            case ACTION_COMP_DELETE_ALL :
                p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
                i_len = regexpr_replace( p_regex_t , NULL , 0  , pc_source , \
                    pc_target , i_len_res , NULL , EXEC_ALL ) ;
                break ;
            /*删除一个字符串*/
            case ACTION_COMP_DELETE_ONE :
                p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
                i_len = regexpr_replace( p_regex_t , NULL , 0  , pc_source , \
                    pc_target , i_len_res , NULL , EXEC_ONE ) ;
                break ;
            /*替换全部字符串**/
            case ACTION_COMP_REPLACE_ALL :
                p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
                pc_replace = ( char * )va_arg( va_list_regex , char * ) ;
                i_len = regexpr_replace( p_regex_t , NULL , 0  , pc_source , \
                    pc_target , i_len_res , pc_replace , EXEC_ALL) ;
                break ;
            /*替换一个字符串*/
            case ACTION_COMP_REPLACE_ONE :
                p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
                pc_replace = ( char * )va_arg( va_list_regex , char * ) ;
                i_len = regexpr_replace( p_regex_t , NULL , 0  , pc_source , \
                    pc_target , i_len_res , pc_replace , EXEC_ONE ) ;
                break ;
            default :
                printf( "error\n" );
        }

        if ( i_len <= 0 ) {
            break ;
        }
    }
    va_end( va_list_regex ) ;
    if( pc_target != pc_res_in ) {
        memcpy( pc_res_in , pc_target , i_len_res ) ;
    }
    FREE_POINT( pc_res_tmp ) ;
    return i_len ;
}
