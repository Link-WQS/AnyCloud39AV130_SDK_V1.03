#define LEN_HINT          512
#define LEN_RES           4096

#define DEFAULT_KEY       "00001111222233334444555566667777"
#define DEFAULT_IV        "ABCDEFGHIJKLMNOP"
#define DEFAULT_DATA      "EEEEFFFFGGGGHHHH"
#define LEN_KEY_128       128
#define LEN_KEY_192       192
#define LEN_KEY_256       256
#define LEN_KEY_BYTE      32                                                    //最长key所占字节数
#define BYTE2BITS         8
#define DEFAULT_LEN_DATA  256
#define LEN_IV            16
#define DEFAULT_TIMES     1
#define LEN_CRYPT         16
#define LEN_ADATA		  16

#define NUM_MEM_ALIGN     4096                                                  //内存对齐
#define HEX_OUTPUT_VIEW   64
#define LEN_HEX_OUTPUT    129

#define TEST_COUNT        100
#define SEC2USEC          1000000
#define BYTE2MB           1000000
#define LEN_OPTION_SHORT  512
#define SIZE_ENCRYPT      4 * 1024 * 1024                                       //加密累计字节数后退出(mb)

#define NUM_THREAD        3                                                     //加密线程数

/*
#define DEFAULT_DATA "ABCDEFGHIJKLMNOP"
#define DEFAULT_KEY  "abcdefghijklmnop"
#define DEFAULT_IV   "0123456789012345"
*/
#define ALG_PRIORITY_MIN 		(0)
#define ALG_PRIORITY_MAX 		(32)
#define ALG_PRIORITY_DEFAULT 	(ALG_PRIORITY_MIN + 1)
#define MAX_IV_BLK_LEN 		32
#define CRYPTODEV_MAX_ALG_NAME		64

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(t) 	(sizeof(t)/sizeof(t[0]))
#endif

#define CRYPTO_DEV 		"/dev/crypto"

#define CRIOGET         _IOWR('c', 101, __u32)
#define CIOCGSESSION    _IOWR('c', 102, struct session_op)
#define CIOCFSESSION    _IOW('c', 103, __u32)
#define CIOCCRYPT       _IOWR('c', 104, struct crypt_op)
#define CIOCKEY         _IOWR('c', 105, struct crypt_kop)
#define CIOCASYMFEAT    _IOR('c', 106, __u32)

/* additional ioctls for AEAD */
#define CIOCAUTHCRYPT   _IOWR('c', 109, struct crypt_auth_op)

//#define CIOCGSESSINFO	_IOWR('c', 107, struct session_info)

#define	COP_ENCRYPT	0
#define COP_DECRYPT	1
#define COP_FLAG_WRITE_IV (1 << 2)                                              // update the IV during operation
#define DEBUG_MUTEX 1                                                           //打印使用锁方式打印
#define LEN_OUTPUT 256
#define MUTEX_IF_INIT_ENC    \
if( gc_init_mutex_enc == false ) { \
    pthread_mutex_init( &g_pthread_mutex_enc, NULL ); \
    gc_init_mutex_enc = true; \
}
#define MUTEX_LOCK_ENC    pthread_mutex_lock( &g_pthread_mutex_enc );
#define MUTEX_UNLOCK_ENC  pthread_mutex_unlock( &g_pthread_mutex_enc );
#define MUTEX_IF_DESTORY_ENC \
if( gc_init_mutex_enc == true ) { \
    pthread_mutex_destroy( &g_pthread_mutex_enc );\
    gc_init_mutex_enc = false; \
}

enum PINGPONT {
    NUM_PING,
    NUM_PONG,
    NUM_PINGPONG,
};

enum cryptodev_crypto_op_t {
        CRYPTO_DES_CBC = 1,
        CRYPTO_3DES_CBC = 2,
        CRYPTO_BLF_CBC = 3,
        CRYPTO_CAST_CBC = 4,
        CRYPTO_SKIPJACK_CBC = 5,
        CRYPTO_MD5_HMAC = 6,
        CRYPTO_SHA1_HMAC = 7,
        CRYPTO_RIPEMD160_HMAC = 8,
        CRYPTO_MD5_KPDK = 9,
        CRYPTO_SHA1_KPDK = 10,
        CRYPTO_RIJNDAEL128_CBC = 11,
        CRYPTO_AES_CBC = CRYPTO_RIJNDAEL128_CBC,
        CRYPTO_ARC4 = 12,
        CRYPTO_MD5 = 13,
        CRYPTO_SHA1 = 14,
        CRYPTO_DEFLATE_COMP = 15,
        CRYPTO_NULL = 16,
        CRYPTO_LZS_COMP = 17,
        CRYPTO_SHA2_256_HMAC = 18,
        CRYPTO_SHA2_384_HMAC = 19,
        CRYPTO_SHA2_512_HMAC = 20,
        CRYPTO_AES_CTR = 21,
        CRYPTO_AES_XTS = 22,
        CRYPTO_AES_ECB = 23,
        CRYPTO_AES_GCM = 50,
        CRYPTO_AES_CCM = 51,
        CRYPTO_AES_CFB ,
        CRYPTO_AES_OFB ,
        CRYPTO_DES_ECB ,
        CRYPTO_DES_CFB ,
        CRYPTO_DES_OFB ,
        CRYPTO_3DES_ECB,
        CRYPTO_3DES_CFB,
        CRYPTO_3DES_OFB,

        CRYPTO_CAMELLIA_CBC = 101,
        CRYPTO_RIPEMD160,
        CRYPTO_SHA2_224,
        CRYPTO_SHA2_256,
        CRYPTO_SHA2_384,
        CRYPTO_SHA2_512,
        CRYPTO_SHA2_224_HMAC,
        CRYPTO_TLS11_AES_CBC_HMAC_SHA1,
        CRYPTO_TLS12_AES_CBC_HMAC_SHA256,
        CRYPTO_ALGORITHM_ALL, /* Keep updated - see below */
};


struct session_op {
    /* Specify either cipher or mac
     */
    __u32	cipher;		/* cryptodev_crypto_op_t */
    __u32	mac;		/* cryptodev_crypto_op_t */

    __u32	keylen;
    __u8	__user *key;
    __u32	mackeylen;
    __u8	__user *mackey;

    __u32	ses;		/* session identifier */
};

/*
struct session_info {
    __u32 ses;		// session identifier

    struct alg_info {
        char cra_name[CRYPTODEV_MAX_ALG_NAME];
        char cra_driver_name[CRYPTODEV_MAX_ALG_NAME];
    } cipher_info, hash_info;

    __u16	alignmask;	// alignment constraints
    __u32   flags;          // SIOP_FLAGS
};
*/

struct crypt_op {
    __u32	ses;		/* session identifier */
    __u16	op;		/* COP_ENCRYPT or COP_DECRYPT */
    __u16	flags;		/* see COP_FLAG_* */
    __u32	len;		/* length of source data */
    __u8	__user *src;	/* source data */
    __u8	__user *dst;	/* pointer to output data */
    __u8	__user *mac;    	/* pointer to output data for hash/MAC operations */
    __u8	__user *iv;  /* initialization vector for encryption operations */
};



/* input of CIOCAUTHCRYPT */
struct crypt_auth_op {
    __u32	ses;		/* session identifier */
    __u16	op;		/* COP_ENCRYPT or COP_DECRYPT */
    __u16	flags;		/* see COP_FLAG_AEAD_* */
    __u32	len;		/* length of source data */
    __u32	auth_len;	/* length of auth data */
    __u8	__user *auth_src;	/* authenticated-only data */

    /* The current implementation is more efficient if data are
     * encrypted in-place (src==dst). */
    __u8	__user *src;	/* data to be encrypted and authenticated */
    __u8	__user *dst;	/* pointer to output data. Must have
                             * space for tag. For TLS this should be at least
                             * len + tag_size + block_size for padding */

    __u8    __user *tag;    /* where the tag will be copied to. TLS mode
                                 * doesn't use that as tag is copied to dst.
                                 * SRTP mode copies tag there. */
    __u32	tag_len;	/* the length of the tag. Use zero for digest size or max tag. */

    /* initialization vector for encryption operations */
    __u8	__user *iv;
    __u32   iv_len;
};

struct ak_encrypt
{
    int i_fd;

    struct session_op session_op_enc;
    struct crypt_op crypt_op_enc;
    struct crypt_auth_op crypt_auth_op_enc;
};

struct benchmark {
    int i_idx;
    pthread_t pthread_id;
    struct ak_encrypt ak_encrypt_session ;
    //AES_KEY AES_KEY_enc ;
    //AES_KEY AES_KEY_dec ;
    unsigned char *pc_src ;
    unsigned char *pc_enc_hw ;
    unsigned char *pc_enc_ssl ;
    unsigned char *pc_dec_ssl ;
    char ac_key[ LEN_KEY_BYTE ] ;
    unsigned char ac_iv_hw[ LEN_IV ];
    unsigned char ac_iv_enc[ LEN_IV ];
    unsigned char ac_iv_dec[ LEN_IV ];
    int i_times_hw;
    int i_times_ssl;
    long long i_us_hw;
    long long i_us_ssl;
    unsigned long long i_size_enc_hw;
    unsigned long long i_size_enc_ssl;
    unsigned char *auth;//adata 认证数据
};
