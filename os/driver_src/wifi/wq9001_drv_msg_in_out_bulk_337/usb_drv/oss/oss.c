#include "oss.h"

/* TODO: should have a way to estimate memory usage in simulation environment */


struct memory_chunk_info_t memory_chunk_info = {
    .member[0]  = {   32 ,   32 , 0},
    .member[1]  = {  128 ,   16 , 0},
    .member[2]  = {  256 ,  108 , 1},
    .member[3] = {   512 ,   50 , 1},
    .member[4] = {  1200 ,    8 , 1},
    .member[5] = {  2400 ,   10 , 0},

};


#if 0
//TODO: Memroy Shrink Temporarily Solution
static struct skbuf_info_t skbuf_info = {
    .class[SKBUF_CLASS_SMALL]       = { 4,  256, 0, 0},
    .class[SKBUF_CLASS_REGULAR]     = { 4, 1024, 0, 0},
    .class[SKBUF_CLASS_BIG]         = { 2, 4096, 0, 0},
    .class[SKBUF_CLASS_HDR_ONLY]    = { 8,    0, 0, 0},
};

void test_malloc(void)
{
    int size=1024;
    iot_printf("%s\n",__func__);
    iot_printf("%s:%d\n",__func__,__LINE__);
    int *ptr=mmal_malloc(size);
    iot_printf("%s:%d, allocate size=%d\n",__func__,__LINE__,size);
    *ptr=0x1234;
    iot_printf("%s:%d, *ptr:0x%X\n",__func__,__LINE__,*ptr);
    iot_printf("%s:%d, ptr:0x%X\n",__func__,__LINE__,ptr);
    mmal_free(ptr);
}

#endif

static uint8_t * g_normal=NULL;
extern void mmal_pool_reset();

void oss_stop(void)
{
    if(g_normal){
        kfree(g_normal);
        g_normal=NULL;
    }
}

void oss_start(void)
{
    int normal_sz = 120 * 1024; //120KB for MMAL

    /* start OSAL */
    osal_start();

    /* attach memory pool for WiFi sub-system */
    g_normal= (uint8_t *)kmalloc(normal_sz,GFP_KERNEL);

    iot_printf("memory pool addr=0x%p size=%d\n", g_normal, normal_sz);

    mmal_pool_reset();

    mmal_pool_attach(MEMPOOL_TYPE_NORMAL, 1, g_normal, normal_sz);

    /* create memory chunk for light-weight malloc/free */
    mmal_chunk_init(&memory_chunk_info);

    //test_malloc();

    iot_printf("after mmal init \n");
#if 0
    /* start task-queue */
    wk_task_init();

    /* create mbuf for data packet */
    mbuf_init(2, 4, 6, 8);

    /* start sock ipc */
    //skipc_init(6, 8, 4, 4, 6, 0);
    //skipc_init(6, 8, 0, 0, 0, 0);
    
    /* init RTM message */
    rt_init();

    /* start net */
    net_init(2);

    /* start packet */
    packet_init();

    /* start ioctl */
    //ioctl_init();

    /* start dev */
    dev_init();

    /* start mfile */
    //mfile_init(3, 200);
#endif
    return;
}
