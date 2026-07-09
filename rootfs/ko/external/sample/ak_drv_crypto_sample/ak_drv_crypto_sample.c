#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include "global.h"
#include "main.h"
#include "printcolor.h"
#include "timecount.h"
#include "print_hex.h"



static char gc_prog_run = true;
static int gi_len_iv   = LEN_IV;
static int gi_len_data = DEFAULT_LEN_DATA;
static int gi_len_key = 256 ;
static int gi_len_adata = LEN_ADATA;
static int crypto_mode = 0;
static int crypt_test=0;
static unsigned int ak_debug =0;

static unsigned int gi_times = DEFAULT_TIMES;
static int gi_align = NUM_MEM_ALIGN;
static int gi_enc_type = CRYPTO_AES_CBC ;
static unsigned long long gi_size_exit = SIZE_ENCRYPT ;
//static pthread_mutex_t g_pthread_mutex_enc = PTHREAD_MUTEX_INITIALIZER;
//static char gc_init_mutex_enc = true;
static int gi_thread_num = NUM_THREAD;

static char gac_option_hint[  ][ LEN_HINT ] = {
    "[crypto mode]  加解密方式mode 0：加密 1：解密",
    "[crypto len]   加密数据长度",
    "[crypto times] 测试次数",
    "[crypto type]  加密类型 \
AES_CBC = 11、AES_CTR = 21、AES_ECB = 23、\
AES_GCM = 50、AES_CCM = 51、AES_CFB = 52、\
AES_OFB = 53、DES_CBC = 1 、DES_ECB = 54、\
DES_CFB = 55 、DES_OFB = 56 、3DES_CBC = 2、\
3DES_ECB = 57、3DES_CFB = 58、3DES_OFB = 59",
    "[mem align]    内存对齐",
    "[crypto size]  加密数据量(MB)",
    "[thread num]   线程数",
    "[test sign]   测试标志test_loop：1测试所有支持类型，0：只测试对应类型",
    "[key_length]   key长度，aes：256、192、128，des：64 3des: 192",
    "[debug]      打开调试信息 1：打开 0：关闭",
    "               帮助",
    "" ,
};

//"[crypto mode]  加解密方式",
//"[crypto len]   加密数据长度",
//"[crypto times] 测试次数",
//"[crypto type]  加密类型 CBC:11 ECB:23 CFB:51
//"[mem align]    内存对齐",
//"[crypto size]  加密数据量(MB)",
//"[thread num]   线程数",
//" 帮助",
//"" ,

static struct option g_option_long[ ] = {
    { "mode"   , required_argument, NULL, 'm' },
    { "len"    , required_argument, NULL, 'a' },
    { "times"  , required_argument, NULL, 'b' },
    { "type"   , required_argument, NULL, 'c' },
    { "align"  , required_argument, NULL, 'd' },
    { "size"   , required_argument, NULL, 'e' },
    { "thread" , required_argument, NULL, 'f' },
    { "test_loop" , required_argument, NULL, 'l' },
    { "key_length" , required_argument, NULL, 'k' },
    { "debug"   , required_argument , NULL, 'g' },
    { "help"   , no_argument      , NULL, 'h' },
    { 0        , 0                , 0   ,  0  } ,

};

static int parse_option( int argc, char **argv )
{
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_option;
    int options_specified = 0;  // 标记是否有选项被指定

    get_option_short( g_option_long, sizeof( g_option_long )/ sizeof( struct option ) ,
        ac_option_short , sizeof( ac_option_short ) );
    while( ( i_option = getopt_long( argc, argv, ac_option_short, g_option_long, NULL ) ) != -1 ) {
         options_specified = 1;  // 有选项被指定
        switch( i_option ) {
            case 'm' :
                crypto_mode = atoi( optarg ) ;
                break;
            case 'a' :
                gi_len_data = atoi( optarg ) ;
                break;
            case 'b' :
                gi_times = atoi( optarg ) ;
                break;
            case 'c' :
                gi_enc_type = atoi( optarg ) ;
                break;
            case 'd' :
                gi_align = atoi( optarg ) ;
                break;
            case 'e' :
                //gi_size_exit = atoi( optarg ) * 1024 * 1024 ;
                //gi_size_exit = atoi( optarg ) * 1024;
                gi_size_exit = atoi( optarg );
                break;
            case 'f' :
                gi_thread_num = atoi( optarg );
                if( gi_thread_num <= 0 ) {
                    gi_thread_num = 1;
                }
                break;

            case 'l' :
                crypt_test = atoi( optarg );
                break;
            case 'k' :
                gi_len_key = atoi( optarg );
                break;
            case 'g' :
                ak_debug = atoi( optarg );
                break;
            case 'h' :
                help( );
                return false;
            default :
                help( );
                return false;
        }

    }

    // 如果没有任何选项被指定，显示帮助信息
        if (!options_specified) {
            help( );
            return false;
        }

    return true;
}


/*
    get_option_short: 根据g_option_long填充短选项字符串
    @p_option[IN]: struct option数组地址
    @i_num_option[IN]: 数组元素个数
    @pc_option_short[IN]: 填充的数组地址
    @i_len_option[IN]: 数组长度
    return: pc_option_short
*/
static char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for( i = 0 ; i < i_num_option ; i ++ ) {
        if( ( c_option = p_option[ i ].val ) == 0 ) {
            continue;
        }
        switch( p_option[ i ].has_arg ){
            case no_argument:
                i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c" , c_option );
                break;
            case required_argument:
                i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c:" , c_option );
                break;
            case optional_argument:
                i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c::" , c_option );
                break;
        }
    }
    return pc_option_short;
}

//根据g_option_long打印帮助信息
int help( )
{
    int i;
    DEBUG_VAL( CM_NORMAL, CB_BLACK, CF_GREEN, "%s\n", "ak_drv_crypto_sample" );
    for( i = 0; i < sizeof( g_option_long ) / sizeof( struct option ); i ++ ) {
        if( g_option_long[ i ].val != 0 ) {
            DEBUG_VAL( CM_NORMAL, CB_BLACK, CF_GREEN,
                       "\t--%-16s -%c %s\n",
                       g_option_long[ i ].name, g_option_long[ i ].val, gac_option_hint[ i ] )
        }
    }
    DEBUG_VAL( CM_NORMAL, CB_BLACK, CF_GREEN,"./ak_drv_crypto_sample --type 11  --mode 0 \
    --len 8192 --key_length 256 --size 8192 --thread 1 --test_loop 0\n")
    DEBUG_VAL( CM_NORMAL, CB_BLACK, CF_GREEN,"./ak_drv_crypto_sample --type 11  --mode 0 \
    --len 16 --key_length 256 --size 16 --thread 1 --test_loop 0\n")
    DEBUG_VAL( CM_NORMAL, CB_BLACK, CF_GREEN,"./ak_drv_crypto_sample --type 11  --mode 0 \
    --len 16 --key_length 256 --size 16 --debug 0 --thread 1 --test_loop 1\n")
    DEBUG_VAL( CM_NORMAL, CB_BLACK, CF_GREEN,"./ak_drv_crypto_sample --type 11  --mode 0 \
    --len 1048576 --key_length 256 --size 1048576 --debug 0 --thread 1 --test_loop 1\n")
    return 0;
}


void prog_exit( int i_sig )
{
    gc_prog_run = false;
    return;
}

/*
*创建会话
*/
int cipherdev_init( struct ak_encrypt *p_ak_encrypt , int i_encrypt_type, char *pc_key, int i_key_bits )
{
    p_ak_encrypt->i_fd = open(CRYPTO_DEV, O_RDWR, 0);
    if (p_ak_encrypt->i_fd < 0) {
        DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED,"open(%s) fail.\n", CRYPTO_DEV);
        return -1;
    }

   // DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_BLUE ,"p_ak_encrypt->i_fd= %d CRIOGET= %d \n", p_ak_encrypt->i_fd, CRIOGET);

    if (fcntl(p_ak_encrypt->i_fd, F_SETFD, 1) == -1) {
        DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED, "strerror(%d)= '%s'\n", errno , strerror( errno ) ) ;
        goto close_out;
    }

    memset(&p_ak_encrypt->session_op_enc, 0, sizeof(struct session_op));
    memset(&p_ak_encrypt->crypt_op_enc, 0, sizeof(struct crypt_op));

    p_ak_encrypt->session_op_enc.cipher = i_encrypt_type;
    p_ak_encrypt->session_op_enc.keylen = i_key_bits / BYTE2BITS;
    p_ak_encrypt->session_op_enc.key = ( unsigned char * )pc_key;


    if (ioctl(p_ak_encrypt->i_fd, CIOCGSESSION, &p_ak_encrypt->session_op_enc)) {
        DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED, "p_ak_encrypt->i_fd= %d strerror(%d)= '%s'\n",
            p_ak_encrypt->i_fd, errno , strerror( errno ) ) ;
        goto close_out;
    }

    return 0;
close_out:
    close(p_ak_encrypt->i_fd);
    return -1;
}

/**/
int cipherdev_enc( struct ak_encrypt *p_ak_encrypt, char *pc_iv,
        char *pc_in, char *pc_out,char *pc_auth, int i_len )
{


    if (gi_enc_type == CRYPTO_AES_GCM || gi_enc_type == CRYPTO_AES_CCM){
    // if (gi_enc_type == CRYPTO_AES_GCM ){
        p_ak_encrypt->crypt_auth_op_enc.ses   = p_ak_encrypt->session_op_enc.ses;
        p_ak_encrypt->crypt_auth_op_enc.len   = i_len;
        p_ak_encrypt->crypt_auth_op_enc.src   = ( unsigned char * )pc_in;
        p_ak_encrypt->crypt_auth_op_enc.dst   = ( unsigned char * )pc_out;
        p_ak_encrypt->crypt_auth_op_enc.iv    = ( unsigned char * )pc_iv;
        p_ak_encrypt->crypt_auth_op_enc.iv_len    = 12;

        //这里传参判断加解密
        /*
        解密：COP_DECRYPT 加密：COP_ENCRYPT
        */
        if(crypto_mode){
              p_ak_encrypt->crypt_auth_op_enc.op    = COP_DECRYPT;
        }else{
            p_ak_encrypt->crypt_auth_op_enc.op    = COP_ENCRYPT;
        }

        p_ak_encrypt->crypt_auth_op_enc.flags = p_ak_encrypt->crypt_auth_op_enc.flags | COP_FLAG_WRITE_IV;

        p_ak_encrypt->crypt_auth_op_enc.auth_len = gi_len_adata;
        p_ak_encrypt->crypt_auth_op_enc.auth_src = (unsigned char *)pc_auth;

        if( ioctl( p_ak_encrypt->i_fd, CIOCAUTHCRYPT, &p_ak_encrypt->crypt_auth_op_enc ) ) {
            DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED, "strerror(%d)= '%s'\n", errno , strerror( errno ) ) ;
            DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED, "i_len= %d pc_in= %p pc_out= %p pc_iv= %p\n",
                i_len, pc_in, pc_out, pc_iv ) ;
            return -1;
        }
    }else{
        p_ak_encrypt->crypt_op_enc.ses   = p_ak_encrypt->session_op_enc.ses;
        p_ak_encrypt->crypt_op_enc.len   = i_len;
        p_ak_encrypt->crypt_op_enc.src   = ( unsigned char * )pc_in;
        p_ak_encrypt->crypt_op_enc.dst   = ( unsigned char * )pc_out;
        p_ak_encrypt->crypt_op_enc.iv    = ( unsigned char * )pc_iv;
        //这里传参判断加解密
        /*
        解密：COP_DECRYPT 加密：COP_ENCRYPT
        */
        if(crypto_mode){
              p_ak_encrypt->crypt_op_enc.op    = COP_DECRYPT;
        }else{
            p_ak_encrypt->crypt_op_enc.op    = COP_ENCRYPT;
        }

       // hex_dump1(p_ak_encrypt->crypt_op_enc.src,gi_len_data);
        // sleep(1);
        p_ak_encrypt->crypt_op_enc.flags = p_ak_encrypt->crypt_op_enc.flags | COP_FLAG_WRITE_IV;

        if( ioctl( p_ak_encrypt->i_fd, CIOCCRYPT, &p_ak_encrypt->crypt_op_enc ) ) {
            DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED, "strerror(%d)= '%s'\n", errno , strerror( errno ) ) ;
            DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED, "i_len= %d pc_in= %p pc_out= %p pc_iv= %p\n",
                i_len, pc_in, pc_out, pc_iv ) ;
            return -1;
        }
        // hex_dump1(p_ak_encrypt->crypt_op_enc.dst,gi_len_data);
        }

    return 0;
}

void cipherdev_release(struct ak_encrypt *p_ak_encrypt)
{
    if (ioctl(p_ak_encrypt->i_fd, CIOCFSESSION, &p_ak_encrypt->session_op_enc.ses)) {
        DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED, "strerror(%d)= '%s'\n", errno , strerror( errno ) ) ;
    }
    //close(p_ak_encrypt->i_fd);
}

void* calloc_align(size_t size, size_t alignd_byte)
{

    size_t offset = sizeof(void *) + alignd_byte - 1;
    void *pv_assign , *pv_align ;

    if ( ( pv_assign = calloc( 1 , size + offset ) ) == NULL) {
        return NULL;
    }
    pv_align = (void *)( ( ( size_t )( pv_assign ) + offset ) & ~( alignd_byte - 1 ) ) ;

    *( ( ( void ** )pv_align ) - 1 ) = pv_assign ;
//    DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_BLUE ,"pv_assign= %p pv_align= %p\n" , pv_assign , pv_align );
    return pv_align;
}

void free_align(void *pv_align )
{
    void *pv_assign = NULL ;

    if ( pv_align != NULL ) {
        pv_assign = ( ( void ** )pv_align )[ - 1 ];
        free( pv_assign );
    }
}

#if 0

 static unsigned char src_iv[]=
{
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,

};

#endif
/**
 * 初始化测试数据缓冲区
 * @param buffer 数据缓冲区指针
 * @param length 缓冲区长度
 * @return 0 成功, -1 失败
 */
int init_buf_data(unsigned char *buffer, int length)
{
    int i;

    if (buffer == NULL || length <= 0) {
        printf("Invalid parameters\n");
        return -1;
    }

    for (i = 0; i < length; i++) {
        buffer[i] = i % 256;  // 循环填充0-255

        // 验证赋值是否正确
        if (buffer[i] != (i & 0xFF)) {
            printf("Error at index %d: expected 0x%02X, got 0x%02X\n",
                   i, i & 0xFF, buffer[i]);
            return -1;
        }
    }

    return 0;
}


static void ak_crypto_key(struct benchmark *p_benchmark)
{
    int  i_color =0;

    switch (gi_enc_type) {
           case CRYPTO_AES_CBC:
           case CRYPTO_AES_ECB:
           case CRYPTO_AES_GCM:
           case CRYPTO_AES_CCM:
           case CRYPTO_AES_CFB:
           case CRYPTO_AES_OFB:
           case CRYPTO_AES_CTR:
               /*
                 判断其秘钥 AES 密钥长度支持128、192、256 位
                */
               if(gi_len_key==256 || gi_len_key==192 ||gi_len_key==128){
                   printf("aes support :key 128、192、256 bit now is :%d\n",gi_len_key);
                }else{
                   printf("warning !!! aes support :key 128、192、256 bit \
                    but now is :%d set default 256\n",gi_len_key);
                   gi_len_key=256;
                }
               break;
           case CRYPTO_DES_ECB:
           case CRYPTO_DES_CFB:
           case CRYPTO_DES_OFB:
           case CRYPTO_DES_CBC:
            /*
            判断其秘钥 选择DES算法时，低64 位数据有效，即KEY[63:0]为有效数据
            */
             gi_len_key=64;
             printf("des support :key 64 bit now is :%d\n",gi_len_key);
               break;
           case CRYPTO_3DES_CBC:
           case CRYPTO_3DES_ECB:
           case CRYPTO_3DES_CFB:
           case CRYPTO_3DES_OFB:
                /*
                判断其秘钥 选择3DES算法时：如果选择3个密钥运算时，
                低192位数据有效 如果选择2个密钥运算时，低128位数据有效，
                */
                gi_len_key=192;
                printf("3des support :key 192 bit now is :%d\n",gi_len_key);
                 break;
           default:
                printf("error ! no support\n");
                return;
       }

#if 0
    memset( p_benchmark->ac_key, 0xFF, gi_len_key / 8 );
#else
    init_buf_data((unsigned char *)p_benchmark->ac_key, gi_len_key / 8);
#endif
    if(ak_debug){
        // print_hex_dump( p_benchmark->ac_key,0);
        DEBUG_PRINT( CM_NORMAL, CB_BLACK, i_color, "秘钥key: %d 位 == %d字节:\n",
            gi_len_key, gi_len_key / 8 );
        print_hex_detail( CM_NORMAL, CB_BLACK, i_color, (unsigned char *)p_benchmark->ac_key,  gi_len_key / 8 );
    }
}


static void ak_crypto_vi(struct benchmark *p_benchmark)
{
    int  i_color =0;

    switch (gi_enc_type) {
           case CRYPTO_AES_CBC:
           case CRYPTO_AES_ECB:
           case CRYPTO_AES_GCM:
           case CRYPTO_AES_CCM:
           case CRYPTO_AES_CFB:
           case CRYPTO_AES_OFB:
           case CRYPTO_AES_CTR:
               /*
               判断其iv值,AES算法时，[127:0]为有效数据即16个字节；
               DES或3DES算法时，低64 位数据有效，即[63:0]为有效数据即8个字节。
               */
                gi_len_iv =16;
               printf("aes support vi: 16 byte now is :%d\n",gi_len_iv);
               break;
           case CRYPTO_DES_ECB:
           case CRYPTO_DES_CFB:
           case CRYPTO_DES_OFB:
           case CRYPTO_DES_CBC:
           case CRYPTO_3DES_CBC:
           case CRYPTO_3DES_ECB:
           case CRYPTO_3DES_CFB:
           case CRYPTO_3DES_OFB:
                gi_len_iv =8;
                printf("des and 3des support vi: 8 byte now is :%d\n",gi_len_iv);
                break;;
           default:
                printf("error ! no support\n");
                return;
       }

    /*
    vi值的配置
    */
    // DEBUG_PRINT("[ lmj %s line:%d]密码iv长度： gi_len_iv:%d赋值: \n",__func__,__LINE__,gi_len_iv);
#if 0
        memset( p_benchmark->ac_iv_hw, 0x00, gi_len_iv );
#else
        init_buf_data(p_benchmark->ac_iv_hw, gi_len_iv);
#endif

    if(ak_debug){
        DEBUG_PRINT( CM_NORMAL, CB_BLACK, i_color, "vi值: %d \n", gi_len_iv );
        print_hex_detail( CM_NORMAL, CB_BLACK, i_color, (unsigned char *)p_benchmark->ac_iv_hw,  gi_len_iv );
    }
}


static void ak_crypto_auth(struct benchmark *p_benchmark)
{
    int  i_color =0;

    switch (gi_enc_type) {
              case CRYPTO_AES_GCM:
              case CRYPTO_AES_CCM:
                  //判断其认证数据 ccm gcm才需要认证数据
                  gi_len_adata = 16;
                   printf("aes-gcm and aes-ccm support vi: 16 byte now is :%d\n",gi_len_adata);
                   break;;
              default:
                   printf("error ! no support\n");
                   return;
          }
    p_benchmark->auth = ( unsigned char * )calloc_align( gi_len_adata , gi_align ) ; //认证数据
    /*判断是申请到内存*/
    if ( p_benchmark->auth == NULL ) {
        DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED,"p_benchmark->auth= %p p_benchmark->auth == NULL \n",
                p_benchmark->auth);
        return;
    }else{
#if 1
       memset( p_benchmark->auth, 0xFF, gi_len_adata);//加密认证数据;
#else
       init_buf_data(p_benchmark->auth, gi_len_adata);
#endif
    }

    if(ak_debug){
        DEBUG_PRINT( CM_NORMAL, CB_BLACK, i_color, "认证数据: %d \n", gi_len_adata );
        print_hex_detail( CM_NORMAL, CB_BLACK, i_color, p_benchmark->auth,  gi_len_adata );
    }
}


/*初始化密码配置和创建一个会话*/
int init_benchmark( int i_idx, struct benchmark *p_benchmark )
{
    memset( p_benchmark, 0, sizeof( struct benchmark ) );
    p_benchmark->i_idx= i_idx;

    /*申请以4k对齐的空间*/
    /*源数据*/
    p_benchmark->pc_src = ( unsigned char * )calloc_align( gi_len_data , gi_align ) ;     //原始数据
    if ( p_benchmark->pc_src == NULL ) {
        DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED,"pc_enc_input_data= %p p_benchmark->pc_src == NULL \n",
                p_benchmark->pc_src);
        return -1;
    }else{
        #if 1
        //  赋值
        init_buf_data(p_benchmark->pc_src, gi_len_data);
        #else
         memset( p_benchmark->pc_src, 0xFF, gi_len_data );
        #endif
    }

     /*目的数据*/
     p_benchmark->pc_enc_hw = ( unsigned char * )calloc_align( gi_len_data , gi_align ) ;  //目的数据
     /*判断是申请到内存*/
     if (p_benchmark->pc_enc_hw == NULL) {
         DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED, "pc_enc_output_data= %p p_benchmark->pc_enc_hw == NULL\n",
                p_benchmark->pc_enc_hw );
          return -1;
     }else{
        //赋值
         memset( p_benchmark->pc_enc_hw, 0x00, gi_len_data );
     }

    /*key值的配置*/
    ak_crypto_key(p_benchmark);

    /*vi值的配置*/
    ak_crypto_vi(p_benchmark);

    /*认证数据的配置*/
     if (gi_enc_type == CRYPTO_AES_GCM || gi_enc_type == CRYPTO_AES_CCM){
        ak_crypto_auth(p_benchmark);
    }

    /*1、硬件加解密的操作，创建一个会话，传输秘钥key*/
    if ( cipherdev_init( &p_benchmark->ak_encrypt_session , gi_enc_type , p_benchmark->ac_key , gi_len_key ) != 0 ) {
        DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED, "strerror(%d)= '%s'\n", errno , strerror( errno ) ) ;
        return false;
    }

    return true;
}

void start_crypto_hw( struct benchmark *p_benchmark )                            // ******** 硬件加密 ********
{
    struct timeval timeval_start, timeval_end;

    timeval_mark( &timeval_start );

    if ( ( p_benchmark->pc_src == NULL ) ||
         ( p_benchmark->pc_enc_hw == NULL )) {
        DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED,
                     "p_benchmark= %p pc_src= %p pc_enc_hw= %p\n",
                     p_benchmark, p_benchmark->pc_src, p_benchmark->pc_enc_hw );
        return ;
    }

#if 1
    for( ;; ) {
        p_benchmark->i_times_hw ++ ;
        if ( cipherdev_enc( &p_benchmark->ak_encrypt_session , ( char * )p_benchmark->ac_iv_hw ,
            ( char * )p_benchmark->pc_src , ( char * )p_benchmark->pc_enc_hw,
            ( char * )p_benchmark->auth , gi_len_data ) != 0 ) {
            break;
        }

        #if 1
        p_benchmark->i_size_enc_hw += gi_len_data ;

        if ( ( p_benchmark->i_size_enc_hw >= gi_size_exit ) ||
             ( ( gi_times > 0 ) && ( p_benchmark->i_times_hw >= gi_times ) ) ) {
            break;
        }
             #endif
    }

    #else
    cipherdev_enc( &p_benchmark->ak_encrypt_session , ( char * )p_benchmark->ac_iv_hw ,
    ( char * )p_benchmark->pc_src , ( char * )p_benchmark->pc_enc_hw , gi_len_data );
 #endif
    timeval_mark( &timeval_end ) ;
    p_benchmark->i_us_hw = timeval_count( &timeval_start, &timeval_end );


}

//判断其加密类型
static void check_crypt_type(void)
{
    /*
     CRYPTO_AES_CBC = 11,
     CRYPTO_AES_CTR = 21,
     CRYPTO_AES_ECB = 23,
     CRYPTO_AES_GCM = 50,
     CRYPTO_AES_CCM = 51,
     CRYPTO_AES_CFB = 52,
     CRYPTO_AES_OFB = 53,

     CRYPTO_DES_CBC = 1,
     CRYPTO_DES_ECB = 54,
     CRYPTO_DES_CFB = 55,
     CRYPTO_DES_OFB = 56,

     CRYPTO_3DES_CBC = 2,
     CRYPTO_3DES_ECB = 57,
     CRYPTO_3DES_CFB = 58,
     CRYPTO_3DES_OFB = 59,
     */
     switch (gi_enc_type) {
            case CRYPTO_AES_CBC:
               printf("*****************ctypto_type is  CRYPTO_AES_CBC\n");
                break;
            case CRYPTO_AES_ECB:
                printf("*****************ctypto_type is  CRYPTO_AES_ECB\n");
                break;
            case CRYPTO_AES_GCM:
                printf("*****************ctypto_type is  CRYPTO_AES_GCM\n");
                break;
            case CRYPTO_AES_CCM:
                printf("*****************ctypto_type is  CRYPTO_AES_CCM\n");
                break;
            case CRYPTO_AES_CFB:
                printf("*****************ctypto_type is  CRYPTO_AES_CFB\n");
                break;
            case CRYPTO_AES_OFB:
                printf("*****************ctypto_type is  CRYPTO_AES_OFB\n");
                break;
            case CRYPTO_AES_CTR:
                printf("*****************ctypto_type is  CRYPTO_AES_CTR\n");
                break;
            case CRYPTO_DES_ECB:
                printf("*****************ctypto_type is  CRYPTO_DES_ECB\n");
                break;
            case CRYPTO_DES_CFB:
                printf("*****************ctypto_type is  CRYPTO_DES_CFB\n");
                break;
            case CRYPTO_DES_OFB:
                printf("*****************ctypto_type is  CRYPTO_DES_OFB\n");
                break;
            case CRYPTO_DES_CBC:
                printf("*****************ctypto_type is  CRYPTO_DES_CBC\n");
                break;
            case CRYPTO_3DES_CBC:
                printf("*****************ctypto_type is  CRYPTO_3DES_CBC\n");
                break;
            case CRYPTO_3DES_ECB:
                printf("*****************ctypto_type is  CRYPTO_3DES_ECB\n");
                break;
            case CRYPTO_3DES_CFB:
                printf("*****************ctypto_type is  CRYPTO_3DES_CFB\n");
                break;
            case CRYPTO_3DES_OFB:
                printf("*****************ctypto_type is  CRYPTO_3DES_OFB\n");
                break;
            default:
                 printf("error ! no support\n");
                 return;
        }
}


 static void ak_ctypt(void)
{

        int i = 0;
        struct timeval timeval_start, timeval_end;
        struct benchmark *p_benchmark;
        long long i_us_hw ;
       // int i_cmp_iv,
        int i_cmp_enc ;
        int i_output , i_color = 0;
        unsigned long long i_size_enc_hw = 0 ;
        i_size_enc_hw = gi_len_data ;
        if( gi_len_data > LEN_OUTPUT ) {
            i_output = LEN_OUTPUT ;
        }
        else {
            i_output = gi_len_data;
        }


        /*判断其密码类型*/
        check_crypt_type();

        /*保存源文件，用于解密后比较*/
         unsigned char *src_date = ( unsigned char * )calloc_align( gi_len_data , gi_align ) ;
         memset( src_date, 0x00, gi_len_data );/*赋值0x00*/

         /*********初始化配置和创建会话**************/
        /*软硬加解密初始化*/
        p_benchmark = ( struct benchmark * )calloc( gi_thread_num, sizeof ( struct benchmark ) );
        for( i = 0 ; i < gi_thread_num ; i ++ ) {
            init_benchmark( i, p_benchmark + i );
            memcpy(src_date, ( p_benchmark + i )->pc_src, gi_len_data );

            if(ak_debug){
#if 1
                DEBUG_PRINT( CM_NORMAL, CB_BLACK, i_color, "硬件加密结果前 %d 字节:\n", i_output );
                print_hex_detail( CM_NORMAL, CB_BLACK, i_color, ( p_benchmark + i )->pc_src, i_output );
#if 0
                if( gi_len_data - i_output > 0 ) {
                DEBUG_PRINT( CM_NORMAL, CB_BLACK, i_color, "硬件加密原文件数据的后 %d 字节:\n", i_output );
                print_hex_detail( CM_NORMAL, CB_BLACK, i_color,
                    ( p_benchmark + i )->pc_src + gi_len_data - i_output, i_output );
                }
#endif
#endif
            }
        }

        /********加密**********/
        /*设置加密*/
        crypto_mode = 0;
        timeval_mark( &timeval_start );
        for( i = 0 ; i < gi_thread_num ; i ++ ) {
            pthread_create( &( p_benchmark + i )->pthread_id , NULL, ( void * )start_crypto_hw , p_benchmark + i ) ;
        }

        for( i = 0 ; i < gi_thread_num ; i ++ ) {
            pthread_join( ( p_benchmark + i )->pthread_id , ( void * )NULL ) ;
        }
        timeval_mark( &timeval_end ) ;
        i_us_hw = timeval_count( &timeval_start, &timeval_end );
        if(ak_debug){

            /*输出加密log*/
            for( i = 0 ; i < gi_thread_num ; i ++ ) {
                DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_BLUE , "编码路数 i_idx= %d\n", ( p_benchmark + i )->i_idx )
                DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_BLUE ,
                "硬件加密: 时间: %f(s) 速度: %f (MB/s) 加密次数: %d\n",
                ( double )( p_benchmark + i )->i_us_hw / SEC2USEC,
            ( double )( p_benchmark + i )->i_size_enc_hw / ( 1024 * 1024 ) / ( p_benchmark + i )->i_us_hw * SEC2USEC,
                ( p_benchmark + i )->i_times_hw );
#if 1
                DEBUG_PRINT( CM_NORMAL, CB_BLACK, i_color, "硬件加密结果的前 %d 字节:\n", i_output );
                print_hex_detail( CM_NORMAL, CB_BLACK, i_color, ( p_benchmark + i )->pc_enc_hw, i_output );
#else
                if( gi_len_data - i_output > 0 ) {
                    DEBUG_PRINT( CM_NORMAL, CB_BLACK, i_color, "硬件加密结果后 %d 字节:\n", i_output );
                    print_hex_detail( CM_NORMAL, CB_BLACK, i_color,
                        ( p_benchmark + i )->pc_enc_hw + gi_len_data - i_output, i_output );
                }
#endif
                DEBUG_PRINT( CM_NORMAL, CB_BLACK, i_color, "硬件加密后IV值:\n");
                print_hex_detail( CM_NORMAL, CB_BLACK, i_color, ( p_benchmark + i )->ac_iv_hw, gi_len_iv );
                }
       }


     DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_PURPLE ,
                     "###### [硬件加密全局速度] 时间: %f (s) 速度: %f (MB/s) 加密字节数: %llu ######\n",
                     ( double )i_us_hw / SEC2USEC,
                     ( double )i_size_enc_hw / ( 1024 * 1024 ) / i_us_hw * SEC2USEC, i_size_enc_hw );
    usleep(50000);    // 延时50毫秒

#if 1
    /*******解密************/
        /*设置解密*/
        crypto_mode = 1;

        /*把加密数据赋值给源文件，设置相同的key和iv值，然后设置解密*/
        for( i = 0 ; i < gi_thread_num ; i ++ ) {
            memset( ( p_benchmark + i )->pc_src,0x00, gi_len_data );
            memcpy(( p_benchmark + i )->pc_src, ( p_benchmark + i )->pc_enc_hw, gi_len_data );
            memset( ( p_benchmark + i )->pc_enc_hw,0x00, gi_len_data );

            /*加密后vi值会变化需配置和加密相同的iv值来解密*/
            /*vi值的配置*/
            ak_crypto_vi(p_benchmark + i );

        }

        /*再次操作硬件密码*/
        timeval_mark( &timeval_start );
        for( i = 0 ; i < gi_thread_num ; i ++ ) {
            pthread_create( &( p_benchmark + i )->pthread_id , NULL, ( void * )start_crypto_hw , p_benchmark + i ) ;
        }

        for( i = 0 ; i < gi_thread_num ; i ++ ) {
            pthread_join( ( p_benchmark + i )->pthread_id , ( void * )NULL ) ;
        }
        timeval_mark( &timeval_end ) ;
        i_us_hw = timeval_count( &timeval_start, &timeval_end );

        DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_PURPLE ,
                     "###### [硬件解密全局速度] 时间: %f (s) 速度: %f (MB/s) 解密字节数: %llu ######\n",
                     ( double )i_us_hw / SEC2USEC,
                     ( double )i_size_enc_hw / ( 1024 * 1024 ) / i_us_hw * SEC2USEC, i_size_enc_hw );
        if(ak_debug){

                for( i = 0 ; i < gi_thread_num ; i ++ ) {

                DEBUG_PRINT( CM_NORMAL, CB_BLACK, i_color, "硬件解密结果前 %d 字节:\n", i_output );
                print_hex_detail( CM_NORMAL, CB_BLACK, i_color, ( p_benchmark + i )->pc_enc_hw, i_output );
#if 0
                if( gi_len_data - i_output > 0 ) {
                    DEBUG_PRINT( CM_NORMAL, CB_BLACK, i_color, "硬件解密结果后 %d 字节:\n", i_output );
                    print_hex_detail( CM_NORMAL, CB_BLACK, i_color,
                        ( p_benchmark + i )->pc_enc_hw + gi_len_data - i_output, i_output );
                }
#endif
               }
        }

        for( i = 0 ; i < gi_thread_num ; i ++ ) {
            /*
            比较数据
            */
            DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_BLUE,
                " 源数据--》加密--》解密--》目的数据 比较源数据和目的数据\n");
            i_cmp_enc = memcmp( ( p_benchmark + i )->pc_enc_hw , src_date , gi_len_data );
            if( i_cmp_enc != 0 ) {
                DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_RED, "比较源数据和目的数据比较结果不正确\n" )
            }
            else {
                DEBUG_PRINT( CM_NORMAL, CB_BLACK, CF_GREEN, "比较源数据和目的数据比较结果正确\n" )
            }
        }

#endif

        /*
        释放内存
        */
        for( i = 0 ; i < gi_thread_num ; i ++ ) {
            free_align( ( p_benchmark + i )->pc_src ) ;
            free_align( ( p_benchmark + i )->pc_enc_hw ) ;
            free_align( src_date ) ;
            free_align( ( p_benchmark + i )->auth ) ;
            cipherdev_release( &( p_benchmark + i )->ak_encrypt_session );
        }
}


int main(int argc, char** argv)
{
     //解释和配置选项
    if( parse_option( argc, argv ) == false ) {
    //打印帮助后退出
        return 0;
    }

    int type_i;
    int type_num[]={ 11 ,23 ,52 ,53 ,21, 50 , 51,1 ,54,55 ,56,2,57,58,59};

  if(crypt_test){
      /*遍历测试所有密码类型*/
      for(type_i =0; type_i < ARRAY_SIZE(type_num);type_i++){
           gi_enc_type = type_num[type_i];
           ak_ctypt();
       }
  }else{
      /*只测试对应的密码类型*/
          ak_ctypt();
  }

    return 0;
}
