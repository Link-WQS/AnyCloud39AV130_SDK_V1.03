#ifndef __ANYKA_IPI_H__
#define __ANYKA_IPI_H__

/**
 * small core state-machine state table
 */
enum machine_state{
    MACHINE_UNKNOWN = 0,
    MACHINE_STARTUP,

    /* vicap */
    /* small windows */
    MACHINE_SMALL_WINDOWS_INIT,
    MACHINE_SMALL_WINDOWS_FASTAE_STABLE,
    /* small to big windows */
    MACHINE_SMALL2BIG_SWITCH_START,
    MACHINE_SMALL2BIG_SWITCH_COMPLETE,
    /* big windows */
    MACHINE_BIG_WINDOWS_INIT,
    MACHINE_BIG_WINDOWS_FASTAE_STABLE,
    MACHINE_BIG_WINDOWS_CAPTURING,
    /* end */
    MACHINE_VICAP_CLOSE_START,
    MACHINE_VICAP_CLOSE_END,

    /* communication with xxx */
    MACHINE_COMMUNICATION,

    MACHINE_SHUTDOWN,
    MACHINE_MAX,
};

/**
 * ipi
 */
#define FASTSYS_MEM_BASE    0x81400000
#define FASTSYS_MEM_SIZE    0x300000        /* 3M */

#define SYS_VEC_MEM_SIZE    0x10000         // 64K
#define SYS_VEC_MEM_BASE    (FASTSYS_MEM_BASE + FASTSYS_MEM_SIZE - SYS_VEC_MEM_SIZE)

#define IPI_MEM_SIZE        0x10000         // 64K
#define IPI_MEM_BASE        (SYS_VEC_MEM_BASE - IPI_MEM_SIZE)
//
#define PARAM_MEM_SIZE      0x2000          // 8K
#define PARAM_MEM_BASE      (IPI_MEM_BASE - PARAM_MEM_SIZE)

#define LOG_MEM_SIZE        0x40000         // 256k
#define LOG_MEM_BASE        (PARAM_MEM_BASE - LOG_MEM_SIZE)

#define ISPCONF_MEM_SIZE    0x30000
#define ISPCONF_MEM_BASE    (LOG_MEM_BASE - ISPCONF_MEM_SIZE)

#define IPI_CHANNEL_FASTSYS             0
#define IPI_CHANNEL_ISP                 1
#define IPI_CHANNEL_VENC                2
#define FASTSYS_IPI_MEM_SIZE            0x10000
#define PARAM_MEM_SIZE                  0x2000

/**
 * struct ipi_chan_zone - tx or rx memory zone
 *
 * @status:        tx or rx commnad status
 * @cmd:    tx or rx commnad
 * @len: data len
 * @data: data memory
 */
struct ipi_chan_zone {
#define IPI_CHAN_RX             (0x1 << 0)
#define IPI_CHAN_RX_COMPLETE    (0x1 << 1)

#define IPI_CHAN_TX             (0x1 << 0)
#define IPI_CHAN_TX_COMPLETE    (0x1 << 1)
    unsigned int status;
    unsigned int cmd;
    unsigned int len;

#define IPI_ZONE_DATA_SIZE          0x1000
//    char data[IPI_ZONE_DATA_SIZE];
    char data[0];
};

/**
 * struct ipi_chan -
 */
struct ipi_chan {
    unsigned int index;
    struct ipi_client *cl;
    phys_addr_t cz_addr;
    spinlock_t lock;
    struct completion tx_complete;
};

/**
 * struct ipi_msg -
 */
struct ipi_msg {
    void *data;
    unsigned int len;
    unsigned int cmd;
};

/**
 * struct ipi_client -
 */
struct ipi_client {
    unsigned int tx_block;
    void *data;
    void (*rx_callback)(struct ipi_client *client, struct ipi_msg *msg);
};

struct ipi_common_client {
    struct list_head node;
    void *data;
    void (*cb)(void *data);
};


#define IPI_FASTSYS_CMD_GLASS       0x100
#define IPI_FASTSYS_DATA_SHARE      0x200
#define IPI_FASTSYS_CMD_LIST        0x400
#define IPI_FASTSYS_FILE_OPERATE    0x1000
#define IPI_MASTER_NOTIFY           0x2000
#define IPI_MASTER_GLASS_CMD_ACK    0x3
#define IPI_FASTSYS_FILE_OPEN  		0x4
#define IPI_FASTSYS_FILE_WRITE 		0x5
#define IPI_FASTSYS_FILE_READ  		0x6
#define IPI_MASTER_FILE_ACK   		0x7
#define IPI_FASTSYS_FILE_SEEK       0x8
#define IPI_FASTSYS_FILE_CLOSE      0x9

#define CMD_NO_CMD          0x0F
#define CMD_PHOTO           0x01
#define CMD_RECORD          0x02
#define CMD_SNAPSHOT        0x03
#define CMD_SMARTCONFIG     0x04
#define CMD_OTA             0x05
#define CMD_IMPORT          0x06
#define CMD_P2P             0x07
#define CMD_STOP_RECORD                 0x08
#define CMD_RTSP                        0x09
#define CMD_USBCAM                      0x0a
#define CMD_POWER_OFF_REQ               0x0b
#define CMD_SET_TIME                    0x0c
#define CMD_SET_TIMEZONE                0x0d
#define CMD_CHK_STORAGE_IS_FULL         0x0e
#define CMD_CHK_STORAGE_SPC             0x10
#define CMD_CHK_STORAGE_FILE_COUNT      0x11
#define CMD_AUDIO_RECORD                0x12
#define CMD_RECOVERY_FACTORY            0x13
#define CMD_FACTORY                     0x14
#define CMD_FEEDBACK                    0x15

// INDICTION
#define IND_READY           0x70
#define IND_ACK             0x71
#define IND_BAUDRATE        0x72
#define IND_UPLOAD          0x73
#define IND_UPLOAD_PACK     0x74
#define IND_COMP            0x75

//BIT7 as LINUX APP/SYS COMM MASK, RESERVED FOR LINUX
#define FILE_OPERATE         0x80

#define MAX_PARAM_LEN       255
typedef struct {
    unsigned int cmd;
    unsigned int param_len;
    unsigned int crc;
    uint8_t parameter[MAX_PARAM_LEN];  
} ak_glass_packet_t;
/*
 *
 */
int ak_ipi_send_msg(struct ipi_chan *chan, struct ipi_msg *msg);

/*
 *
 */
struct ipi_chan *ak_ipi_request_channel(struct ipi_client *cl, int index);

/*
 *
 */
int ak_ipi_register_client(struct ipi_client *client);

/*
 *
 */
int ak_register_common_client(struct ipi_common_client *client);

/*
 *
 */
void ak_unregister_common_client(struct ipi_common_client *client);

/*
 *
 */
int fastsys_get_used_ion_mem(phys_addr_t *base, unsigned int *size);

/*
 *
 */
int ak_get_fastsys_state(unsigned int *state, unsigned int *param);

/*
 *
 */
void ak_fastsys_free_resource(void);

void ak_kload_complete(void);
void ak_wait_kload_complete(void);
void ak_venc_start(void);
void ak_wait_venc_start(void);
void *ak_wait_fast_vi_close(void);
void *ak_get_vi_dev_status(void);
void *ak_get_glass_packet(void);
int ak_ack_glass_packet(void);
int ak_get_glass_mode(void);
void *ak_get_dss_remap(void);
void *ak_get_ispconf_remap(void);
#endif
