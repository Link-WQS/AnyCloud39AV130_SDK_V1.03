#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/videodev2.h>
#include <../drivers/staging/android/ion/ion.h>
#include <../drivers/staging/android/ion/ion_priv.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/syscalls.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/mtd/partitions.h>
#include <linux/initrd.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <generated/compile.h>
#include <linux/uts.h>
#include <linux/reboot.h>
#include <asm-generic/sections.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/libfdt.h>
#include <linux/io.h>
#include <asm/io.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
//#include "../driver/ak_quick_start/quick_start.h"
//#include "../drivers/media/platform/ak_isp/isp_quick_ioctl.h"
#include <soc/anyka/ak_ipi.h>


#define FAST_AE_ENABLE

#define USE_CFG_BUF  1
#define DYNAMIC_MALLOC 1
#define FILES_MAX     96
#define FILENAME_LEN  32
#define SPLIT_READ   1
#define UPDATE_PART  1
#define FASTFS_KTHREAD 0
#define ZERO_CPY_TEST  0
#define ADC_TEST       0
#define FAST_LAUNCH_VERSION  "Anyka-Fast-V1.0.08"
#define FAST_BUILD_TIME      UTS_VERSION
#define PRELOAD_LEN   (100*1024)


#define CHECKID_PATH "/etc/config/checkID"

#define ISPCONF_HEAD_LEN 64

#if 0
static int CONFIG_LEN  = 0;
static int FAST_FS_LEN = 0;
#else
int CONFIG_LEN  = 0;
int FAST_FS_LEN = 0;

#endif

static char *isp_cfg_buf = NULL,*head_data=NULL,*org_data=NULL,*conf_name=NULL,*real_conf_name=NULL,*real_conf_len_str=NULL, *fast_run_param=NULL;
static int   isp_cfg_len = 0, isp_drop_frame = 0, fast_ae_enable = 1;
static int   data_off = 0, param_offset = 0, param_len = 0, real_conf_len = 0, allow_conf_len = 0;
static int   sensor_allow_reinit = 0, conf_sensor_id = 0, sensor_init_id = 0, update_flag = 0, dual_mode = 0, day_night_mode = 0, adc_fd = -1;
static char *config_part = NULL, *config_path = NULL, *config_file = NULL;

typedef struct isp_partition_header
{
    uint32_t magic;
    uint32_t hcrc;//从dcrc到isp_partition_header最后一个字节的校验。累加和。
    uint32_t dcrc;//所有 {headerX+ispX}总的校验。累加和。
    uint32_t isps_offset[8];//表示每个isp文件的偏移。如第一个是偏移为0，表示header0的开头
    uint8_t valid_index[8];//用于标识isps_offset[]中哪些属于正在使用的文件。重新匹配后可以修改。制作镜像时相同SensorID的文件挨一起。
    uint8_t valid_num;//表示isps_offset[]中前几个的数据是有效的。制作镜像时已确定好，系统不可更改。
    uint8_t reserved[11];//凑够64字节大小
} isp_partition_header_t;

#define IH_NMLEN		32	/* Image Name Length		*/
typedef struct image_header {
	__be32		ih_magic;	/* Image Header Magic Number	*/
	__be32		ih_hcrc;	/* Image Header CRC Checksum	*/
	__be32		ih_time;	/* Image Creation Timestamp	*/
	__be32		ih_size;	/* Image Data Size		*/
	__be32		ih_load;	/* Data	 Load  Address		*/
	__be32		ih_ep;		/* Entry Point Address		*/
	__be32		ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;

uint32_t checksum(const uint8_t *data, size_t len)
{
    uint32_t sum = 0;
    size_t i;

    for (i = 0; i < len; i++) {
        sum += data[i];
    }

    return sum;
}

#ifdef CONFIG_SYS_FAST_LAUNCH
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/dirent.h>
#include <linux/utime.h>

static int module_insmod(const char *path, const char *uargs);

static ssize_t xwrite(int fd, const char *p, size_t count)
{
    ssize_t out = 0;

    /* sys_write only can write MAX_RW_COUNT aka 2G-4K bytes at most */
    while (count) {
        ssize_t rv = sys_write(fd, p, count);

        if (rv < 0) {
            if (rv == -EINTR || rv == -EAGAIN)
                continue;
            return out ? out : rv;
        } else if (rv == 0)
            break;

        p += rv;
        out += rv;
        count -= rv;
    }

    return out;
}

static  char *message;
static void  error(char *x)
{
    if (!message)
        message = x;
}

/* link hash */

#define N_ALIGN(len) ((((len) + 1) & ~3) + 2)

static  struct hash {
    int ino, minor, major;
    umode_t mode;
    struct hash *next;
    char name[N_ALIGN(PATH_MAX)];
} *head[32];

static inline int hash(int major, int minor, int ino)
{
    unsigned long tmp = ino + minor + (major << 3);
    tmp += tmp >> 5;
    return tmp & 31;
}

static char  *find_link(int major, int minor, int ino,
        umode_t mode, char *name)
{
    struct hash **p, *q;
    for (p = head + hash(major, minor, ino); *p; p = &(*p)->next) {
        if ((*p)->ino != ino)
            continue;
        if ((*p)->minor != minor)
            continue;
        if ((*p)->major != major)
            continue;
        if (((*p)->mode ^ mode) & S_IFMT)
            continue;
        return (*p)->name;
    }
    q = kmalloc(sizeof(struct hash), GFP_KERNEL);
    if (!q)
        panic("can't allocate link hash entry");
    q->major = major;
    q->minor = minor;
    q->ino = ino;
    q->mode = mode;
    strcpy(q->name, name);
    q->next = NULL;
    *p = q;
    return NULL;
}

static void  free_hash(void)
{
    struct hash **p, *q;
    for (p = head; p < head + 32; p++) {
        while (*p) {
            q = *p;
            *p = q->next;
            kfree(q);
        }
    }
}
//use time 5ms
static long  do_utime(char *filename, time_t mtime)
{
    struct timespec t[2];

    t[0].tv_sec = mtime;
    t[0].tv_nsec = 0;
    t[1].tv_sec = mtime;
    t[1].tv_nsec = 0;

    return do_utimes(AT_FDCWD, filename, t, AT_SYMLINK_NOFOLLOW);
}

static  LIST_HEAD(dir_list);
struct dir_entry {
    struct list_head list;
    char *name;
    time_t mtime;
};

static void  dir_add(const char *name, time_t mtime)
{
    struct dir_entry *de = kmalloc(sizeof(struct dir_entry), GFP_KERNEL);
    if (!de)
        panic("can't allocate dir_entry buffer");
    INIT_LIST_HEAD(&de->list);
    de->name = kstrdup(name, GFP_KERNEL);
    de->mtime = mtime;
    list_add(&de->list, &dir_list);
}

static void  dir_utime(void)
{
    struct dir_entry *de, *tmp;
    list_for_each_entry_safe(de, tmp, &dir_list, list) {
        list_del(&de->list);
        do_utime(de->name, de->mtime);
        kfree(de->name);
        kfree(de);
    }
}

static  time_t mtime;

/* cpio header parsing */

static  unsigned long ino, major, minor, nlink;
static  umode_t mode;
static  unsigned long body_len, name_len;
static  uid_t uid;
static  gid_t gid;
static  unsigned rdev;

static void  parse_header(char *s)
{
    unsigned long parsed[12];
    char buf[9];
    int i;

    buf[8] = '\0';
    for (i = 0, s += 6; i < 12; i++, s += 8) {
        memcpy(buf, s, 8);
        parsed[i] = simple_strtoul(buf, NULL, 16);
    }
    ino = parsed[0];
    mode = parsed[1];
    uid = parsed[2];
    gid = parsed[3];
    nlink = parsed[4];
    mtime = parsed[5];
    body_len = parsed[6];
    major = parsed[7];
    minor = parsed[8];
    rdev = new_encode_dev(MKDEV(parsed[9], parsed[10]));
    name_len = parsed[11];
}

/* FSM */

static  enum state {
    Start,
    Collect,
    GotHeader,
    SkipIt,
    GotName,
    CopyFile,
    GotSymlink,
    Reset
} state, next_state;

static  char *victim;
static unsigned long byte_count;
static  loff_t this_header, next_header;

static inline void  eat(unsigned n)
{
    victim += n;
    this_header += n;
    byte_count -= n;
}

static  char *vcollected;
static  char *collected;
static long remains;
static  char *collect;

static void  read_into(char *buf, unsigned size, enum state next)
{
    if (byte_count >= size) {
        collected = victim;
        eat(size);
        state = next;
    } else {
        collect = collected = buf;
        remains = size;
        next_state = next;
        state = Collect;
    }
}

static  char *header_buf, *symlink_buf, *name_buf;

static int  do_start(void)
{
    read_into(header_buf, 110, GotHeader);
    return 0;
}

static int  do_collect(void)
{
    unsigned long n = remains;
    if (byte_count < n)
        n = byte_count;
    memcpy(collect, victim, n);
    eat(n);
    collect += n;
    if ((remains -= n) != 0)
        return 1;
    state = next_state;
    return 0;
}

static int  do_header(void)
{
    if (memcmp(collected, "070707", 6)==0) {
        error("incorrect cpio method used: use -H newc option");
        return 1;
    }
    if (memcmp(collected, "070701", 6)) {
        error("no cpio magic");
        return 1;
    }
    parse_header(collected);
    next_header = this_header + N_ALIGN(name_len) + body_len;
    next_header = (next_header + 3) & ~3;
    state = SkipIt;
    if (name_len <= 0 || name_len > PATH_MAX)
        return 0;
    if (S_ISLNK(mode)) {
        if (body_len > PATH_MAX)
            return 0;
        collect = collected = symlink_buf;
        remains = N_ALIGN(name_len) + body_len;
        next_state = GotSymlink;
        state = Collect;
        return 0;
    }
    if (S_ISREG(mode) || !body_len)
        read_into(name_buf, N_ALIGN(name_len), GotName);
    return 0;
}

static int  do_skip(void)
{
    if (this_header + byte_count < next_header) {
        eat(byte_count);
        return 1;
    } else {
        eat(next_header - this_header);
        state = next_state;
        return 0;
    }
}

static int  do_reset(void)
{
    while (byte_count && *victim == '\0')
        eat(1);
    if (byte_count && (this_header & 3))
        error("broken padding");
    return 1;
}

static int  maybe_link(void)
{
    if (nlink >= 2) {
        char *old = find_link(major, minor, ino, mode, collected);
        if (old)
            return (sys_link(old, collected) < 0) ? -1 : 1;
    }
    return 0;
}

static void  clean_path(char *path, umode_t fmode)
{
    struct stat st;

    if (!sys_newlstat(path, &st) && (st.st_mode ^ fmode) & S_IFMT) {
        if (S_ISDIR(st.st_mode))
            sys_rmdir(path);
        else
            sys_unlink(path);
    }
}

static  int wfd;

static int  do_name(void)
{
    state = SkipIt;
    next_state = Reset;
    if (strcmp(collected, "TRAILER!!!") == 0) {
        free_hash();
        return 0;
    }
    clean_path(collected, mode);
    if (S_ISREG(mode)) {
        int ml = maybe_link();
        if (ml >= 0) {
            int openflags = O_WRONLY|O_CREAT;
            if (ml != 1)
                openflags |= O_TRUNC;
#if 0 //temp disable save file reduce mem use				
            if ( strstr(collected, "ak_") && !strstr(collected, ".ko") && !strstr(collected, "font") )
            {
                printk("skip %s\n",collected);
                return 0;
            }	
#endif			
            wfd = sys_open(collected, openflags, mode);

            if (wfd >= 0) {
                //sys_fchown(wfd, uid, gid);
                //sys_fchmod(wfd, mode);
                if (body_len)
                    sys_ftruncate(wfd, body_len);
                vcollected = kstrdup(collected, GFP_KERNEL);
                state = CopyFile;
            }
        }
    } else if (S_ISDIR(mode)) {
        sys_mkdir(collected, mode);
        //sys_chown(collected, uid, gid);
        //sys_chmod(collected, mode);
        dir_add(collected, mtime);
    } else if (S_ISBLK(mode) || S_ISCHR(mode) ||
            S_ISFIFO(mode) || S_ISSOCK(mode)) {
        if (maybe_link() == 0) {
            sys_mknod(collected, mode, rdev);
            //sys_chown(collected, uid, gid);
            //sys_chmod(collected, mode);
            do_utime(collected, mtime);
        }
    }
    return 0;
}

DECLARE_COMPLETION(unpack_done);
//DECLARE_COMPLETION(busybox_done);
static int unpack_wait_cnt = -1;
static int  do_copy(void)
{
    int target_fd =0;
    int j = 0;
    int sensor_id = 0, is_match_file = 0;
    char tmp_conf_name[50], tmp_conf_size[20];
    char *short_name = NULL;
    if (byte_count >= body_len) {
#if 1	
        //find sensor's match config file
        if(update_flag==1 && strstr(collected,"conf"))
        {
            is_match_file = 1;
            memcpy(&sensor_id, (victim+20), 4);	
            printk("len=%lu,get sensor_id:%04x\n",body_len,sensor_id);	
            if(sensor_id==sensor_init_id)
            {
                printk("find match sensor_id:%04x's file_name:%s\n",sensor_id,collected);
                if(body_len>allow_conf_len)
                {
                    printk("new conf file size too big\n");
                    goto MATCHED_END;   
                }
                //get new config id, update ramfs using's config file
                conf_sensor_id = sensor_id;
                //update isp_cfg_buf head_data
                memcpy(head_data+data_off+param_len,victim,(PRELOAD_LEN-data_off+param_len));
                //create new cfg file to ramfs replace old file
                sys_unlink(conf_name);
                target_fd = sys_open(conf_name,O_WRONLY|O_CREAT,0755);
                if(target_fd>=0)
                {
                    sys_write(target_fd, victim,body_len);
                    sys_close(target_fd);
                    target_fd = -1;
                }

                //update backup area config file name and file size
#if 1 			
                printk("old real_conf_name:%s,name_len:%d,name_space_en:%d,real_conf_len:%d,off:%d\n",real_conf_name, strlen(real_conf_name), (real_conf_len_str-real_conf_name-2), real_conf_len, (real_conf_len_str-head_data));
                //update new file name and file size to head_data buf		   
                short_name = strstr(collected,"/");
                if(short_name != NULL)
                {
                    short_name++;
                }
                else
                {
                    short_name = collected;
                }
                j=strlen(short_name);
                printk("collected short_name:%s,strlen:%d\n",short_name, j);	 
                if(j > (real_conf_len_str-real_conf_name-2))//new filename len> orignal space
                {		      
                    *(short_name + (real_conf_len_str-real_conf_name-3) ) = 0;//cut shortname at space end position,void update partition 
                    printk("new conf_name too long, max strlen:%d, new short_name:%s\n",(real_conf_len_str-real_conf_name-2), short_name);	 
                }
                else
                    *(short_name + j ) = 0; 		

                memset(tmp_conf_name, 0, sizeof(tmp_conf_name));
                memset(tmp_conf_size, 0, sizeof(tmp_conf_size));
                sprintf(tmp_conf_name,"%s",short_name);
                sprintf(tmp_conf_size,"%lu",body_len);
                //update head_data buf
                memcpy(real_conf_name, tmp_conf_name, (strlen(tmp_conf_name)+1) );		   
                memcpy(real_conf_len_str, tmp_conf_size, strlen(tmp_conf_size) );		   
                //update org_data buf
                //file name off
                j = (real_conf_name-head_data);
                memcpy(org_data+j, tmp_conf_name, (strlen(tmp_conf_name)+1) );
                //file size off
                j = (real_conf_len_str-head_data);
                memcpy(org_data+j, tmp_conf_size, (strlen(tmp_conf_size)) );

                printk("new conf_name:%s,strlen:%d,conf_size:%s\n",tmp_conf_name, strlen(tmp_conf_name), tmp_conf_size);	

                CONFIG_LEN = real_conf_len = body_len; 	   		   
#endif

                //restore org_data to sys cache
                memcpy((void *)initrd_start,org_data,data_off);
                //update head_data
                memcpy(head_data+data_off+param_len,victim,(PRELOAD_LEN-(data_off+param_len)));		   
                //update org_data
                memcpy(org_data+data_off+param_len,victim,(8192-(data_off+param_len)));
                //reset fast ae awb param 
#if 0
                memset(org_data+data_off, 0xff, param_len);
                memset(head_data+data_off,0xff, param_len);		   		   
                memset((char *)initrd_start+data_off, 0xff, param_len);
#endif

                //copy cfg data to cache fill old file data area
                memcpy((char *)initrd_start+data_off+param_len,victim,body_len);

                j=0;
                while( j<body_len && (victim+j) && (*(victim+j) == *((char *)initrd_start+data_off+param_len+j)) )
                    j++;

                printk("write before check j=%d,verfiy-[%s]!\n",j, (j==body_len)?"ok":"fail");	
                if(j==body_len)
                    is_match_file |= 0x10;	   
            }
        }
MATCHED_END:
        if(update_flag==1 && is_match_file==1) //reduce ramfs space
            printk("skip:%s\n",collected);
        else
#endif		
            if (xwrite(wfd, victim, body_len) != body_len)
                error("write error");
        sys_close(wfd);
        if(unpack_wait_cnt>=0 && body_len>307200 && strstr(collected, "bin/busybox"))
        {
            printk("xwrite [%s] ok\n",collected);
            unpack_wait_cnt++;
            if(unpack_wait_cnt>=2)
                complete(&unpack_done); 
        }
        do_utime(vcollected, mtime);
        kfree(vcollected);
        eat(body_len);
        state = SkipIt;
        return 0;
    } else {
        if (xwrite(wfd, victim, byte_count) != byte_count)
            error("write error");
        body_len -= byte_count;
        eat(byte_count);
        return 1;
    }
}

static int  do_symlink(void)
{
    collected[N_ALIGN(name_len) + body_len] = '\0';
    clean_path(collected, 0);
    sys_symlink(collected + N_ALIGN(name_len), collected);
    if(unpack_wait_cnt>=0 && strstr(collected, "sbin/init"))
    {
        printk("xwrite [%s] ok\n",collected);
        unpack_wait_cnt++;
        if(unpack_wait_cnt>=2)
            complete(&unpack_done); 
    }
    //sys_lchown(collected, uid, gid);
    do_utime(collected, mtime);
    state = SkipIt;
    next_state = Reset;
    return 0;
}

static  int (*actions[])(void) = {
    [Start]		= do_start,
    [Collect]	= do_collect,
    [GotHeader]	= do_header,
    [SkipIt]	= do_skip,
    [GotName]	= do_name,
    [CopyFile]	= do_copy,
    [GotSymlink]	= do_symlink,
    [Reset]		= do_reset,
};

static long  write_buffer(char *buf, unsigned long len)
{
    byte_count = len;
    victim = buf;

    while (!actions[state]())
        ;
    return len - byte_count;
}

static long  flush_buffer(void *bufv, unsigned long len)
{
    char *buf = (char *) bufv;
    long written;
    long origLen = len;
    if (message)
        return -1;
    while ((written = write_buffer(buf, len)) < len && !message) {
        char c = buf[written];
        if (c == '0') {
            buf += written;
            len -= written;
            state = Start;
        } else if (c == 0) {
            buf += written;
            len -= written;
            state = Reset;
        } else
        {
            error("junk2 in compressed archive");
        }
    }
    return origLen;
}
static unsigned long my_inptr; /* index of next byte to be processed in inbuf */
#include <linux/decompress/generic.h>
#include <linux/decompress/unlz4.h>
#include <linux/decompress/unlzma.h>
static char * unpack_to_fs(char *buf, unsigned long len)
{
    long written;
    decompress_fn decompress;
    const char *compress_name;
    static  char msg_buf[64];
    char *outp;
    int res;

    header_buf = kmalloc(110, GFP_KERNEL);
    symlink_buf = kmalloc(PATH_MAX + N_ALIGN(PATH_MAX) + 1, GFP_KERNEL);
    name_buf = kmalloc(N_ALIGN(PATH_MAX), GFP_KERNEL);

    if (!header_buf || !symlink_buf || !name_buf)
        panic("can't allocate buffers");

    state = Start;
    this_header = 0;
    message = NULL;

    while (!message && len) {
        loff_t saved_offset = this_header;
        if (*buf == '0' && !(this_header & 3)) {
            state = Start;
            written = write_buffer(buf, len);
            buf += written;
            len -= written;
            continue;
        }
        if (!*buf) {
            buf++;
            len--;
            this_header++;
            continue;
        }
        this_header = 0;
#if 1 // support compress cpio
        if(*buf==0x02 && *(buf+1)==0x21)
        {
            decompress = unlz5; //decompress_method(buf, len, &compress_name);
            compress_name = "lz4";
        }
        else
            decompress = NULL;
        pr_info("Detected %s compressed data, decompress:%p\n", compress_name,decompress);
        if (decompress) {
            outp = (char *)(((int)(buf+len+4095))/4096 * 4096); //(char *)(phys_to_virt(0x83800000));	
            printk("outp-init:%p,len*1.5:%lu\n",outp,(len*3/2));		
            if( (conf_sensor_id != sensor_init_id && !update_flag) || ((outp+(len*3/2)) > (char *)(initrd_end)) || len>4096000 ) //avoid cache overflow
                outp = NULL;			

#if 1 // isp update first can't save aov
            if(!outp && update_flag) //ispcfg unpack must continue memory
            {
                outp = (char *)(phys_to_virt(0x83E00000));
                if( (buf+len) > outp )
                    outp = NULL;
            }
#endif
            printk("outp:%p\n",outp);
            res = decompress(buf, len, NULL, flush_buffer, outp, &my_inptr, error);
            if (res)
                error("decompressor failed");
        } else if (compress_name) {
            if (!message) {
                snprintf(msg_buf, sizeof msg_buf,
                        "compression method %s not configured",
                        compress_name);
                message = msg_buf;
            }
        } else
#endif	
        {
            if(*buf==0xff && *(buf+1)==0xff)
            {
                printk("uncompress data,to end, saved_len:%lld\n",saved_offset);
                break;
            }
            error("junk1 in compressed archive");
        }
        if (state != Reset)
            error("junk0 in compressed archive");
        this_header = saved_offset + my_inptr;
        buf += my_inptr;
        len -= my_inptr;
        printk("len:%lu,my_inptr:%lu\n",len,my_inptr);
    }
    dir_utime();
    kfree(name_buf);
    kfree(symlink_buf);
    kfree(header_buf);
    return message;
}
#endif

#define APP_ADDR  CONFIG_FAST_FS_HEAD_ADDR //0x81200000


int get_sensor_mode(void)
{
    char *file_data = NULL;
    int i = 0;

    if(head_data == NULL)//
        file_data = phys_to_virt(APP_ADDR);
    else
        file_data = head_data;
    if( !(dual_mode&0x10) )	  
    {
        file_data = phys_to_virt(APP_ADDR);

        //find file list head start mark 'S' 
        while(i<32)
        {
            if( *(file_data+i) == 0x53 && *(file_data+i+1) == 0x0a )
            {
                i += 2;
                break;
            }
            i++;
        }

        if(i==32)
        {
            printk("[%s]:%02x %02x %02x %02x\n", file_data, *(file_data+0), *(file_data+1), *(file_data+2), *(file_data+3));	 
            printk("fast fs start mark not found,i=%d\n",i);
            return -1;
        }

        //find dual_mode config option
        while( *(file_data+i) != 'd' && *(file_data+i) != '[' && i<=32)
            i++;

        if( *(file_data+i) == 'd' )
        {
            if( !strncmp((file_data+i), "dual:1", strlen("dual:1")) )
            { 
                dual_mode |= 1;
                printk("dual_mode=1\n");
            }
            else
            {
                printk("dual_mode=0\n");
            }
        }
        dual_mode |= 0x10; //update dual_mode config status
        //find dual_mode option end	   
    }

    return (dual_mode&0xf);   
}
EXPORT_SYMBOL_GPL(get_sensor_mode);

char *get_sensor_preg(int *preg_len)
{
    char *preg_info = NULL, *ptr_cfg = NULL;
    int ptr_offset = 0;
    int sensor_len = 0;

    if(head_data == NULL)//
    {
        ptr_cfg = phys_to_virt(APP_ADDR);
        while( ptr_offset<512 )
        {
            //find file head end 'e'.
            if( *(ptr_cfg+ptr_offset) == 0x65 && *(ptr_cfg+ptr_offset+1) == 0x0a )
            {
                ptr_offset += 2;
                break;
            }
            else
                //find param file ']$' . allow param file before isp_conf
                if( *(ptr_cfg+ptr_offset) == ']' && *(ptr_cfg+ptr_offset+1) == '$' && *(ptr_cfg+ptr_offset+2) == 0x0a )
                {
                    //skip param file name to param file size
                    ptr_offset += 3;
                    //parse param file size
                    for(; *(ptr_cfg+ptr_offset) != 0x0a; ptr_offset++)
                    {
                        param_len = param_len*10 + (*(ptr_cfg+ptr_offset) - 0x30);
                    }
                }

            ptr_offset++;
        }
        if(ptr_offset==512)
        {
            *preg_len=0; 
            return NULL;
        }

        //before init head_data, data_off;after init param_offset
        head_data = ptr_cfg;
        data_off = ptr_offset;	   	      

        if(param_len>0)
        {
            param_offset = ptr_offset; //param file start position
            ptr_offset += param_len;	 //skip param file  
            printk("param_offset:%d,param_len:%d\n",param_offset, param_len);  
        }

    }
    else
    {
        ptr_cfg = head_data;
        ptr_offset += data_off + param_len;   
        printk("param_offset:%d,param_len:%d\n",param_offset, param_len);  
    }

    ptr_offset += 512; //skip cfg header
    preg_info = (ptr_cfg+ptr_offset + 93108);
    memcpy(&sensor_len, (ptr_cfg+ptr_offset + 93108)-2, 2);
    *preg_len = sensor_len;
    printk("%s:[%p][%d]\n",__func__,preg_info,sensor_len);
    return preg_info;   
}
EXPORT_SYMBOL_GPL(get_sensor_preg);

char *get_isp_cfg_buf(int *buf_len)
{
    char *isp_cfg = NULL;
    *buf_len = isp_cfg_len;
    printk("dual_mode[%d]:[%p][%d]\n",(dual_mode&0xf),isp_cfg_buf,isp_cfg_len);
    isp_cfg = isp_cfg_buf;
    //RESET
    if((dual_mode&0xf)==0)
    {
        isp_cfg_buf = NULL;
        isp_cfg_len = 0;
    }
    else
        dual_mode &= 0xf0;
    return isp_cfg;
}
EXPORT_SYMBOL_GPL(get_isp_cfg_buf);

int get_init_drop(void)
{
    isp_drop_frame = 1;
    printk("isp_drop_frame:%d\n",isp_drop_frame);
    return isp_drop_frame;
}
EXPORT_SYMBOL_GPL(get_init_drop);

char *get_param_buf(int *buf_len, int *offset, int flag)
{
    char *head_ptr = NULL;
    char *param_buf = NULL;

    if(head_data == NULL)
    { 
        //head_ptr = phys_to_virt(APP_ADDR);
        get_sensor_preg(buf_len);
        if(head_data && data_off>0)
            head_ptr = head_data;
        else
        {
            printk("%s:head[%p][%d]\n",__func__,head_data,data_off);
            return NULL;	  
        }	  
    }
    else
        head_ptr = head_data;

    if(param_len>0)
    {
        param_buf = (head_ptr+param_offset);
        *buf_len = param_len;
        *offset = param_offset;
    }
    else // add param buf to head_data
    {
        param_len = 4154; //lumi_ae_table.bin size
        param_buf = kmalloc(param_len,GFP_KERNEL);
        memset(param_buf,0,param_len);
        param_offset = 0;
        *buf_len = param_len;
        *offset = param_offset;
    }
    //   printk("%s:buf-addr[%p],buf-len[%d],data-off[%d]\n",__func__,param_buf,param_len,param_offset);
    return param_buf;   
}
EXPORT_SYMBOL_GPL(get_param_buf);

int get_allow_reinit(void)
{
    return sensor_allow_reinit;
}
EXPORT_SYMBOL_GPL(get_allow_reinit);
void set_allow_reinit(void)
{
    sensor_allow_reinit = 1; 
}
EXPORT_SYMBOL_GPL(set_allow_reinit);

int enable_fast_ae(void)
{
    return fast_ae_enable;
}
EXPORT_SYMBOL_GPL(enable_fast_ae);

void save_partition_data(void)
{
    // int len = 8192; //write block min 4K, there write 8K >= param_len
    // memcpy(org_data+data_off, (head_data+data_off), param_len);

    if(update_flag==0) //only sensor matched need save fast param
    {
        // get_partition_data("APP2", 0, org_data, &len, 1);

        int target_fd = sys_open("/mnt/fast_awb.bin", O_CREAT | O_WRONLY, 0755);	
        if(target_fd>=0)
        {
            sys_write(target_fd, head_data+data_off,param_len);
            sys_close(target_fd);            
        }
    }

    printk("saved fast ae data param_len:%d\n",param_len);   
}
EXPORT_SYMBOL_GPL(save_partition_data);

#define MAX_SENSOR_NUM 2
typedef int (*sensor_init_cb)(void *arg, void *para);
typedef int (*sensor_param_cb)(void *arg, int param, void *value);
static void *sensor_init_arg[MAX_SENSOR_NUM] = {NULL,NULL};
static int sensor_init_argc = 0;
static sensor_init_cb _sensor_init_cb[1] = {0};
static sensor_param_cb _sensor_param_cb[1] = {0};

int sys_sensor_param_func(int dev,int param, void *value)
{
    //printk("inited_flag:%d\n",sensor_init_argc,inited_flag);

    if(_sensor_param_cb[0])
    {
        if(dev == 0){
            _sensor_param_cb[0](sensor_init_arg[0], param,value);
        }
        if(sensor_init_argc>1 && dev == 1)
        {
            _sensor_param_cb[0](sensor_init_arg[1], param,value);
        }
    }
    return 0;
}
EXPORT_SYMBOL_GPL(sys_sensor_param_func);

static int inited_flag = 0;
int sys_sensor_init_func(void)
{
#ifdef FAST_AE_ENABLE
    return 0;
#endif	 
    if(inited_flag==sensor_init_argc)
    {
        printk("inited-%d finish\n",inited_flag);
        return 0;
    }
    //allow _sensor_init_cb is null, only to do fast ae init
    if(enable_fast_ae())
    {
        day_night_mode = 0;//ygh get_ae_fast_def();
        //adc_fd = sys_open("/sys/bus/iio/devices/iio:device0/in_voltage1_raw", O_RDONLY, 0);
        printk("day_night_mode=%d,adc_fd=%d\n", day_night_mode,adc_fd);

    }
    if(_sensor_init_cb[0])
    {
        //avoid dual frame split bad
        //if( (dual_mode&0xf) || sensor_init_id==0x4653 || sensor_init_id==0x2253  || sensor_init_id==0x1346 )
        return 0; //20fps or spec sensor init do in sensor_probe

        if( _sensor_init_cb[0](sensor_init_arg[inited_flag], NULL) < 0)
            return -1;
        inited_flag++;
        printk("inited-%d\n",inited_flag);
        if( sensor_init_argc>1 )
        {	
            if( _sensor_init_cb[0](sensor_init_arg[inited_flag], NULL) < 0 )
                return -1;
            inited_flag++;
            printk("inited-%d\n",inited_flag);
        }			 
    }
    return 0;
}
EXPORT_SYMBOL_GPL(sys_sensor_init_func);
int get_sensor_fast_inited(void)
{
    return ((inited_flag>=sensor_init_argc)?1:0);
}
EXPORT_SYMBOL_GPL(get_sensor_fast_inited);

int sys_sensor_init_set(void *init_func, void *arg, void *param_func, int sensor_id)
{
#if 0//ygh 咨询过zhangguomao，之前为了让sensor提前出帧，没必要了
    if(sensor_init_argc>0)
    {
        if(sensor_init_argc == MAX_SENSOR_NUM)
        {
            printk("sensor_init_argc:%d,set full\n",sensor_init_argc);
            return 0;
        }

        if(_sensor_init_cb[0] == init_func && sensor_init_id == sensor_id )
        {
            sensor_init_arg[sensor_init_argc] = arg;		 
            sensor_init_argc++;
            printk("sensor_init_arg[%d]:%p,sensor_id:%04x same to sensor0 info\n",sensor_init_argc-1, sensor_init_arg[sensor_init_argc-1],sensor_id);		 
            return sensor_init_argc;
        }
        else
        {
            printk("sensor_init_arg[%d]:%p,sensor_id:%04x diff sensor0 id\n",sensor_init_argc, arg,sensor_id);		 	  
        }

        return 0;	  
    }
    _sensor_init_cb[sensor_init_argc] = (sensor_init_cb )init_func;
    sensor_init_arg[sensor_init_argc]    = arg; 
    _sensor_param_cb[sensor_init_argc] = (sensor_param_cb )param_func;
    sensor_init_argc++;
#endif
    sensor_init_id = sensor_id;
    printk("sensor_init_id:%04x\n",sensor_init_id);
    return sensor_init_argc;
}
EXPORT_SYMBOL_GPL(sys_sensor_init_set);

#if ZERO_CPY_TEST //mmap debug
#define VIDEO_DATA_ADDR  0x81350000
#define VIDEO_DATA_LEN   0x40000
static int fastfs_proc_mmap(struct file *filp, struct vm_area_struct *vma)
{
    int ret = 0, size = 0;
    size = vma->vm_end - vma->vm_start;
    pr_err("%s vma->vm_start:0x%x, filp:%p, max_size:%x\n", __func__, vma->vm_start, filp, size);

    if (!(vma->vm_flags & VM_SHARED)) {
        pr_err( "invalid vma flags, VM_SHARED needed.");
        return -EINVAL;
    }

    vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
    vma->vm_pgoff = 0;
#if 0
    ret = vm_iomap_memory(vma, virt_to_phys( run_proc_addr ), run_proc_size);
#else
    //  if(size>304*1024)
    //	size = 304*1024;
    ret = vm_iomap_memory(vma, VIDEO_DATA_ADDR, VIDEO_DATA_LEN);
#endif	
    if (ret) {
        pr_err("vm_iomap_memory 0x%x failed \n",VIDEO_DATA_ADDR);
        return ret;
    } else {
        pr_info("mmap [0x%x] to [0x%x] ok\n", VIDEO_DATA_ADDR, vma->vm_start);
    }
    if (ret) {
        pr_err("Remapping memory failed, error: %d\n", ret);
        return ret;
    }

    vma->vm_flags       |= VM_DONTEXPAND | VM_DONTDUMP;
    vma->vm_private_data    = NULL;
    vma->vm_ops     = NULL;

    return 0;
}

static int fast_log_thread(void *data)
{

    char *log_line = NULL, *ptr_log = NULL;
    int line_len = 0, line_cnt = 0, log_total_len = 0;
    log_line = ptr_log = phys_to_virt(VIDEO_DATA_ADDR);
    printk("%s-%p\n",__func__,log_line);	
    memset(ptr_log, 0x00, VIDEO_DATA_LEN);
#if 1
    while(log_total_len<VIDEO_DATA_LEN)
    {
        msleep(100);
        if(*log_line != 0)
        {
            if((line_cnt++%10) == 0)
                printk("L[%d],B[%d]\n",line_cnt,log_total_len);

            line_len = strlen(log_line);
            //printk("%s",log_line);		
            log_line += line_len;
            log_total_len += line_len;
        }
    };
#endif	

    return 0;
}

static int fastfs_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m,"fastfs version:%s-%s\n",FAST_LAUNCH_VERSION,FAST_BUILD_TIME);
    return 0;
}

static int fastfs_proc_open(struct inode *inode, struct file *filp)
{
    //printk("[0x%x]=%s\n",APP_ADDR, (char *)phys_to_virt(APP_ADDR));
    return single_open(filp, fastfs_proc_show, NULL);
}

static const struct file_operations fastfs_proc_fops = {
    .open		= fastfs_proc_open,
    .read		= seq_read,
    .llseek		= seq_lseek,
    .release	= seq_release,
    .mmap       = fastfs_proc_mmap,
};
#endif

#if FASTFS_KTHREAD
static int unpack_addr=0, unpack_len = 0;
static char *unpack_name = NULL;
static int unpack_kthread(void *data)
{
    int cnt=0;
    if(data)
    {	
        char *file_name = (char *)unpack_name;
        int input[2];
        input[0] = unpack_addr;//*(int *)data;
        input[1] = unpack_len;//*(int *)(data+1);   
        // char *file_name = (char *)(data+2);

        printk("%s, input[0]=%x, input[1]=%d\n", file_name, input[0], input[1] );
        sys_chroot("/");
        unpack_to_fs( (char *)input[0], input[1]);
        sys_chroot("/");   
        printk("unpack[%s],size:%d ok\n",file_name,input[1]);
    }
    free_initrd_mem(initrd_start, initrd_end); 
#if 1
    md5_mod_init();
    sha1_generic_mod_init();
    crc32c_mod_init();
    arc4_init();
    aes_init();
    slab_sysfs_init();
#endif	
}
#endif
/////////////////////////////////////////////////////////////////////////

static int video_daemon_open(char *run_cmd, int argc, char **arg)
{
    int ret=0,i=0;    
    char cmd[]="/mnt/ak_vi_sample1";
    char *argv[10] = {NULL}; 
    char *envp[]={NULL};

    if(run_cmd == NULL)
        argv[0]=cmd;
    else
        argv[0]=run_cmd;

    for(i=0;i<argc;i++)
    {
        argv[i+1]=arg[i];
        printk("argv[%d]=%s\n", i+1,argv[i+1]);  
    }
    if(i>0 && i<8 && fast_run_param != NULL)
    {
        argv[i+1]=fast_run_param;
        printk("argv[%d]=%s\n", i+1,argv[i+1]);
        i++;  	
    }
    argv[i+1]=NULL;


    ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    printk("%s ret=%d\n", argv[0], ret);  
    return ret;	
}

#ifdef FAST_AE_ENABLE
void set_qs_root(const char *quick_part, const char *quick_path, const char *quick_file)
{
    if(quick_part)
        config_part = (char *)quick_part;
    if(quick_path)
        config_path = (char *)quick_path;
    if(quick_file)
        config_file = (char *)quick_file;

    pr_err("config_part:%s,config_path:%s,config_file:%s\n",config_part,config_path,config_file);

}
EXPORT_SYMBOL(set_qs_root);

char *get_qs_part(void)
{
    return config_part;
}
EXPORT_SYMBOL(get_qs_part);

char *get_qs_path(void)
{
    return config_path;
}
EXPORT_SYMBOL(get_qs_path);


#if 0
int fast_ae_quick_start_init(char *qs_isp_conf_path)
{
    struct qs_info qs  = {0};
    int fd  = sys_open("/dev/quick_start", O_RDWR, 0);
    printk("quick_start fd=%d,  qs_isp_conf_path:%s\n", fd,  qs_isp_conf_path);
    memcpy(qs.isp_conf_name, qs_isp_conf_path, strlen(qs_isp_conf_path));
    sys_ioctl(fd, IO_ISP_QUICK_START_SET_INT, (unsigned long)&qs);
    sys_close(fd);
    return 0;
}
#endif

#endif


/*
 * had_check_sensorid - 表示是否匹配过sensorID。来自上次的匹配结果。
 */
struct fast4_info {
    int i;
    int file_index;//-1
    char *file_data;//NULL
    char *file_name[FILES_MAX];
    int  file_type[FILES_MAX];
    int  file_size[FILES_MAX];
    int  run_proc_seq;
    int run_flag;
    int load_flag;
    char *check_code[FILES_MAX];
    int cfg_seq ;
    int total_size;
    char  run_proc_param[6];
    unsigned long file_len ;
    int file_len1;
    int file_off[FILES_MAX];
    int file_offset;
    int split_read;
    int bigmem_used;
    int bigmem_used_offset;
    int fs_readed;
    int fast_used_off;
    int file_seq;
    int  split_offset;
    char save_filename[64];
    char qs_isp_conf[64];
    int conf_index ;
    int conf_size[4];
    unsigned long conf_tosize;
    char *con_name[4];
    int split_write_index;
    char had_check_sensorid;
};
#if 0
#define KERN_RED "\033[31m"
#define KERN_BLUE "\033[34m"

#define KERN_RESET "\033[0m"
#endif

static void camera_get_checkid_file(struct fast4_info *fs)
{
    int idfd;
    int len;
    char checkid = 0;

    fs->had_check_sensorid = 0;

    idfd = sys_open(CHECKID_PATH, O_RDONLY, 0644);
    if (idfd >= 0) {
        sys_lseek(idfd, 0, SEEK_SET);
        len = sys_read(idfd, &checkid, 1);
        if (len == 1) {
            pr_err("%s read ok, checkid=%d len:%d\n", CHECKID_PATH, checkid,len);
            fs->had_check_sensorid = checkid;
        }

        sys_close(idfd);
    }

    pr_err("fs->had_check_sensorid:%d\n", fs->had_check_sensorid);
}

static void camera_update_checkid_file(struct fast4_info *fs, char flag)
{
    int idfd;
    int len;
    char checkid = 0;

    idfd = sys_open(CHECKID_PATH, O_CREAT | O_RDWR, 0755);
    if (idfd >= 0) {
        len = sys_read(idfd, &checkid, 1);
        if(len < 1 || checkid != flag){
            pr_err("%s write flag:%d\n",__func__,flag);
            len = sys_write(idfd, &flag, 1);
        }
        sys_close(idfd);
    }

    pr_err("%s idfd:%d\n",__func__,idfd);
}

int head_date_resolve(struct fast4_info * fs)
{
    int num = 0;
    int cp_cnt = 0;


    while(fs->i<32){
        if( *(fs->file_data+fs->i) == 'F' && *(fs->file_data+fs->i+1) == 0x0a ) //fast3.fs size 'F' mark
        {
            fs->i += 2;
            break;
        }
        fs->i++;
    }

    if(fs->i>0 && fs->i<32){
        //parse list file size
        fs->fs_readed = 0;
        for(; *(fs->file_data+fs->i) != 0x0a; fs->i++){
            fs->fs_readed = fs->fs_readed*10 + (*(fs->file_data+fs->i) - 0x30);
        }
        fs->i++;//skip 0x0a
        fs->fs_readed += fs->i;//fs size + offset 
    }
    if(fs->fs_readed==0) fs->i=0; //reset start position
    printk("fs->i=%d,fs->fs_readed:%d\n",fs->i,fs->fs_readed);

    /*在前32个字节中寻找字符‘S’开始标志，对应的值为0x53*/
    while(fs->file_index==-1 && fs->i<32){
        if( *(fs->file_data+fs->i) == 0x53 && *(fs->file_data+fs->i+1) == 0x0a )
        {
            fs->i += 2;
            fs->file_index = 0;
            break;
        }
        fs->i++;
    }
    /*没找到则报错返回*/
    if(fs->i==32){
        printk("fast fs start mark not found,fs->i=%d\n",fs->i);
        printk("[%s]:%02x %02x %02x %02x\n", fs->file_data, *(fs->file_data+0), 
                *(fs->file_data+1), *(fs->file_data+2), *(fs->file_data+3));  
        return -1;
    }

    //printk("[lmj][%s %d] fast4.fs start [fs->file_index:%d][file_data_i:%d]\n",__func__,__LINE__,fs->file_index,fs->i);

    /*初始化这个fast4.fs 文件系统中各个文件大小的数组*/
    memset(fs->file_size,0,sizeof(fs->file_size));

    //find dual_mode config option
    if( !(dual_mode&0x10) ){ 
        while( *(fs->file_data+fs->i) != 'd' && *(fs->file_data+fs->i) != '[' && fs->i<=32)
            fs->i++;

        if( *(fs->file_data+fs->i) == 'd' ){
            if( !strncmp((fs->file_data+fs->i), "dual:1", strlen("dual:1")) ) {
                dual_mode |= 1;
                printk("dual_mode=1\n");
            }else{
                printk("dual_mode=0\n");
            }
        }
        dual_mode |= 0x10; //update dual_mode config status
    }
    //find dual_mode option end

    /*开始解析各个文件的类型，从出现的第一个[开始*/
    //find file info '[' start, skip before other info
    while( *(fs->file_data+fs->i) != '[' )
        fs->i++;

    //printk("[lmj][%s %d] fast4.fs [fs->file_index:%d][file_data_i:%d]\n",__func__,__LINE__,fs->file_index,fs->i);

    /* 遍历剩下的1k字节*/
    while( fs->i<1024 ){

        fs->file_size[fs->file_index] = 0;//init fs->file_size
        //parse list file name
        //find '[' start mark
        if ( *(fs->file_data+fs->i) == '[' )
            fs->i++;

        /*1、解析文件名*/
        // fs->file_name[fs->file_index] = fs->file_data+fs->i;
        //skip ']'
        cp_cnt = 0;
        for(; *(fs->file_data+fs->i) != ']'; fs->i++,cp_cnt++){
        }

        /*打印文件名*/
        // file_name = (char **)kzalloc(sizeof(char *)*cp_cnt,GFP_KERNEL);
        fs->file_name[fs->file_index] = (char *)kzalloc(sizeof(char *)*cp_cnt,GFP_KERNEL);
        if (fs->file_name[fs->file_index] == NULL)
            pr_err(" kzalloc fs->file_name[fs->file_index] fail\n");

        strncpy(fs->file_name[fs->file_index], fs->file_data+fs->i - cp_cnt,cp_cnt);

        // printk("[lmj][%s %d] fast4.fs cp_cnt:%d fs->file_name[%d]:%s\n",
        //    __func__,__LINE__,cp_cnt,fs->file_index,fs->file_name[fs->file_index]);

        *(fs->file_data+fs->i) = 0;// '\0'; //at ']' or 0x0a end file name
        fs->i++;

        /*2、解析文件类型：list头标识*/
        ///file type check begin
        fs->file_type[fs->file_index] = 0;// bin or data

        //skip '*'
        if ( *(fs->file_data+fs->i) == '*' ){
            *(fs->file_data+fs->i) = '\0';
            fs->i++;
            fs->run_proc_seq = fs->file_index;
            fs->file_type[fs->file_index] = 1;// fast-run-prog
        }

        //skip '&'
        else if ( *(fs->file_data+fs->i) == '&' ){
            *(fs->file_data+fs->i) = '\0';
            fs->i++;
            fs->file_type[fs->file_index] = 2;// pro-load ko file
        }

        //skip '#'
        else if ( *(fs->file_data+fs->i) == '#' ){
            *(fs->file_data+fs->i) = '\0';
            fs->i++;
            fs->file_type[fs->file_index] = 3;// cpio packge file
        }

        //skip '$'
        else if ( *(fs->file_data+fs->i) == '$' ){
            *(fs->file_data+fs->i) = '\0';
            fs->i++;
            fs->file_type[fs->file_index] = 4;// param file,need save cache
        }

        //skip '!'
        else if ( *(fs->file_data+fs->i) == '!' ){
            *(fs->file_data+fs->i) = '\0';
            fs->i++;
            fs->file_type[fs->file_index] = 5;// backup cpio packge file
        }

        //skip '@'
        /*APP1，CONFIG分区包*/
        else if ( *(fs->file_data+fs->i) == '@' ){
            *(fs->file_data+fs->i) = '\0';
            fs->i++;
            fs->file_type[fs->file_index] = 6;// usr partition cpio packge file
        }

        //skip '.'
        else if ( *(fs->file_data+fs->i) == '.' ){
            *(fs->file_data+fs->i) = '\0';
            fs->i++;
            fs->file_type[fs->file_index] = 7; // sec fast program
            fs->run_flag |= 0x40; //set has second fast program flag
        }

        //file type check end
        //skip 0x0a
        if ( *(fs->file_data+fs->i) == 0x0a ){
            *(fs->file_data+fs->i) = '\0';
            fs->i++;
        }

        // printk("[lmj][%s %d] fast4.fs [fs->file_index:%d]fs->file_type[%d]:%d\n",
        //  __func__,__LINE__,fs->file_index,fs->file_index,fs->file_type[fs->file_index]);


        //file data check code get begin
        /*
           3、判断是否ram，也就是#,校准ram根文件系统压缩包。
           中间的乱码为该文件的头4Byte字节的数据，
           用于启动校验，不能省略
           */
        //find '?' Tag as check code MARK
        if ( *(fs->file_data+fs->i) == '?' ){
            //skip '?'
            fs->i++;
            //skip 0x0a and fs->check_code area 4 BYTE
            if ( *(fs->file_data+fs->i) == 0x0a ){
                fs->check_code[fs->file_index] = fs->file_data+fs->i; //get fs->check_code
                *(fs->file_data+fs->i) = 1; //mark fs->check_code flag
                //skip 0x0a
                fs->i++;              
                //skip check code 4 BYTE
                fs->i += 4;
                printk(" this file is ram chk[%d][%x]:%02x %02x %02x %02x\n",
                        fs->file_index, fs->check_code[fs->file_index][0],
                        fs->check_code[fs->file_index][1],fs->check_code[fs->file_index][2],
                        fs->check_code[fs->file_index][3],fs->check_code[fs->file_index][4] );
            }
        }

        //parse list file size
        /*4、解析文件的大小*/
        for(; *(fs->file_data+fs->i) != 0x0a; fs->i++){
            fs->file_size[fs->file_index] = fs->file_size[fs->file_index]*10 + (*(fs->file_data+fs->i) - 0x30);
        }
        fs->i++; //skip 0x0a

        /*5、解析.conf文件*/
        if( fs->cfg_seq<0 && strstr(fs->file_name[fs->file_index], ".conf") ){
            fs->cfg_seq = fs->file_index;
            printk(" this file is .conf  [fs->cfg_seq:%d]\n",fs->cfg_seq);
        }

        /*6、判断文件类型是否是 ae参数*/
        else if(fs->file_type[fs->file_index] == 4){
            param_len = fs->file_size[fs->file_index];
            printk("[lmj] this file is $ ae param [param_len:%d]\n", param_len);
        }
        //if it is config file, get backup file info begin
        //find '~' Tag as read config file MARK
        if( (fs->file_index == fs->cfg_seq) && *(fs->file_data+fs->i) == '~' && *(fs->file_data+fs->i+1) == '[')
        {
            //skip '~'
            fs->i++;
            //skip '['
            fs->i++;
            real_conf_name = fs->file_data+fs->i;
            printk("real_conf off:%d\n",fs->i);
            //find file name end ' ' or ']'
            while( *(fs->file_data+fs->i) != ' ' && *(fs->file_data+fs->i) != ']' ) fs->i++;

            //skip ' ' and ']'
            if( *(fs->file_data+fs->i) == ' ' ){
                *(fs->file_data+fs->i) = 0;
                //skip ' '
                fs->i++;

                //skip all ' ' and find ']'
                while( *(fs->file_data+fs->i) != ']' ) fs->i++;
                //skip ']'
                fs->i++; 
            } else if( *(fs->file_data+fs->i) == ']' ){
                *(fs->file_data+fs->i) = 0;
                //skip ']'
                fs->i++;
            }

            //skip 0x0a 
            if( *(fs->file_data+fs->i) == 0x0a )
                fs->i++;

            //find file size
            if ( *(fs->file_data+fs->i) <= 0x39 && *(fs->file_data+fs->i) >= 0x30 ){
                real_conf_len_str = fs->file_data+fs->i;
                real_conf_len = 0;
                for(; *(fs->file_data+fs->i) != 0x0a; fs->i++){
                    real_conf_len = real_conf_len*10 + (*(fs->file_data+fs->i) - 0x30);
                }

                //skip 0x0a
                *(fs->file_data+fs->i) = 0;
                fs->i++;

                printk("real_conf_name:%s,pos:%d,real_conf_len:%d,pos:%d\n", 
                        real_conf_name, (real_conf_name-fs->file_data), real_conf_len, (real_conf_len_str-fs->file_data));
            }else{
                printk("not found %s size\n",real_conf_name);
            }
        }
        //if it is config file, get backup file info end

        if(fs->file_type[fs->file_index] == 6) {
            if(fs->conf_index<4){
                fs->con_name[fs->conf_index] = fs->file_name[fs->file_index];
                fs->conf_size[fs->conf_index] = fs->file_size[fs->file_index];
                fs->conf_tosize += fs->conf_size[fs->conf_index];
                printk("fs->conf_size[%d]=%d\n", fs->conf_index,fs->conf_size[fs->conf_index]);
                fs->conf_index++;
            }else
                printk("config file list total to max %d\n", fs->conf_index);
        } else{
            fs->total_size += fs->file_size[fs->file_index];    
            fs->file_index++;
        }

        if( fs->file_index == FILES_MAX ){
            printk("fast file list total to max %d\n", FILES_MAX);
            //move to list end
            for(; *(fs->file_data+fs->i) != 0x45 || *(fs->file_data+fs->i+1) != 0x0a; fs->i++){
            }
        }else{
            fs->file_size[fs->file_index] = 0;
        }

        //check file list end mark 'E'
        if( *(fs->file_data+fs->i) == 0x45 && *(fs->file_data+fs->i+1) == 0x0a ){
            fs->i += 2;//skip end mark
            printk("[%s %d] fast total file[%d],size[%d]\n",
                    __func__,__LINE__,fs->file_index, fs->total_size);
        }

        /***************视频快启程序（第一段）的启动参数开始标志****************/
        //check fast run var mark 'v'
        if( *(fs->file_data+fs->i) == 0x76 && *(fs->file_data+fs->i+1) == 0x0a ){
            fs->i += 2;//skip var mark

            /*
# main_res, frame_fmt,  venc_mode,  thd[en],sub_res, venc_fps, drop_frame, fast_ae_enable
# main[2~8] h265[0~3] , mode[1 or 2], 0/1, sub[0~2], fps[0~30], drop[0~30], fast_ae[0,1]
*/
            /*对echo -e -n "\x05\x02\x02\x04\x00\x1e\x00\x01" >> len.bin进行解析*/
            if( *(fs->file_data+fs->i)<=10 && *(fs->file_data+fs->i+1)<=3 && *(fs->file_data+fs->i+2)<=2 ){
                printk("[%s %d]fast run param [%02x %02x %02x %02x]\n",
                        __func__,__LINE__,*(fs->file_data+fs->i),*(fs->file_data+fs->i+1),*(fs->file_data+fs->i+2),*(fs->file_data+fs->i+3));
                fs->run_proc_param[0]=*(fs->file_data+fs->i);
                fs->run_proc_param[1]=*(fs->file_data+fs->i+1);
                fs->run_proc_param[2]=*(fs->file_data+fs->i+2);
                fs->run_proc_param[3]=*(fs->file_data+fs->i+3);

                fs->i += 4;//skip param data
                //sub_res
                //0x65是“e”也就是表示不是尾
                if( *(fs->file_data+fs->i) != 0x65 ) {
                    //if( *(fs->file_data+fs->i) < 10 ){
                    fs->run_proc_param[4]=*(fs->file_data+fs->i);
                    printk("sub_res:%02x\n",fs->run_proc_param[4]);
                    //}
                    fs->i++;
                }
                //init fps
                if( *(fs->file_data+fs->i) != 0x65 )  {
                    if( *(fs->file_data+fs->i) <= 0x1e ){
                        fs->run_proc_param[5]=*(fs->file_data+fs->i);
                        printk("init fps:%d\n",fs->run_proc_param[5]);
                    }
                    fs->i++;
                }

                //init drop_frame
                if( *(fs->file_data+fs->i) != 0x65 ) {
                    if( *(fs->file_data+fs->i) <= 0x1e ) {
                        isp_drop_frame = *(fs->file_data+fs->i);
                        printk("drop_frame:%d\n",isp_drop_frame);
                    }
                    fs->i++;
                }

                //enable fast ae/awb
                if( *(fs->file_data+fs->i) != 0x65 ){
                    //allow disable fast ae/awb
                    if( *(fs->file_data+fs->i) == 1 || *(fs->file_data+fs->i) == 0 ) {
                        fast_ae_enable= *(fs->file_data+fs->i);
                        printk("fast_ae_enable:%d\n",fast_ae_enable);
                    }
                    fs->i++;
                }
            }
            //other reserved param 
            while( *(fs->file_data+fs->i) != 0x65 )  
                fs->i++;
        }
        /*视频快启程序（第一段）的启动参数结束标志*/
        //check fast run var end mark 'e'
        if( *(fs->file_data+fs->i) == 0x65 && *(fs->file_data+fs->i+1) == 0x0a ){
            fs->i += 2;//skip end mark
            printk("fast fs data offset:%d\n",fs->i);
            break;
        }
        /********视频快启程序（第一段）的启动参数结束标志***************/
        /********这里也退出了所有参数的解析while**************/
    }

    FAST_FS_LEN = fs->total_size + fs->i;
    //printk(KERN_BLUE"[FAST_FS_LEN:%d]"KERN_RESET"\n",FAST_FS_LEN);

    /*对应文件的偏移*/
    data_off = fs->i; // data are a start offset
    //calculate file_off
    for(num=0; num<fs->file_index; num++){
        /*非@APP1，CONFIG分区包*/
        if(fs->file_type[num] != 6) {
            if(num==0)
                fs->file_off[num] = data_off; 
            else
                fs->file_off[num] = fs->file_off[num-1]+fs->file_size[num-1];

            // printk(KERN_BLUE "[lmj] [%s %d] [fs->file_index:%d][num:%d] [fs->file_name:%s] [fs->file_type:%d],[fs->file_off:%d][fs->file_size:%d]" KERN_RESET "\n",
            //   __func__,__LINE__,fs->file_index,num,fs->file_name[num],fs->file_type[num],fs->file_off[num],fs->file_size[num]);

        }
    }

    // printk(KERN_RED "This is red text! " KERN_RESET "\n");

    if(param_len>0)
        param_offset = data_off;

    camera_get_checkid_file(fs);

    return 0;
}

static void filename_split_to_args(char *filename, char *uargs)
{
    char *p;
    char *first_empty;
    int filename_len = strlen(filename);

    uargs[0] = '\0';

    first_empty = strchr(filename, ' ');
    if (first_empty) {
        *first_empty = '\0';//截断留下真实的ko名称
        //pr_err("ygh ko:%s\n",filename);
        for (p = first_empty + 1; p < filename + filename_len; p++) {
            /*搜索ko的参数部分*/
            if (*p != ' ') {
                sprintf(uargs, "%s", p);
                //pr_err("ygh ko args:%s\n",uargs);
                break;
            }
        }
    }
}

/*preload_ko_stage1 - 加载除快启驱动quick_start之外的其他驱动*/
static void preload_ko_stage1(struct fast4_info *fs)
{
    int tmp_file_seq;
    char tmp_filename[FILES_MAX];
    char tmp_uargs[FILES_MAX];

    for (tmp_file_seq = 0; tmp_file_seq < fs->file_index; tmp_file_seq++) {
        memset(tmp_filename,0,sizeof(tmp_filename));
        if (fs->file_type[tmp_file_seq] == 2 && fs->file_size[tmp_file_seq] == 0 && fs->load_flag&0x2) {
            if (!strstr(fs->file_name[tmp_file_seq], "ak_quick_start")) {
                sprintf(tmp_filename,"/lib/%s",fs->file_name[tmp_file_seq]);
                if (strstr(fs->file_name[tmp_file_seq], "sensor") && fs->had_check_sensorid == 0) {
                    /*如果要匹配sensorID则加上check_id参数*/
                    sprintf(tmp_filename,"%s check_id=1",tmp_filename);
                }
                printk("kload[%d]:%s\n",tmp_file_seq,tmp_filename);
                filename_split_to_args(tmp_filename, tmp_uargs);
                module_insmod(tmp_filename, tmp_uargs);
            }
        }
    }
}

static void preload_ko_stage2(struct fast4_info *fs)
{
    int tmp_file_seq;
    char tmp_filename[FILES_MAX];
    char tmp_uargs[FILES_MAX];

    for (tmp_file_seq = 0; tmp_file_seq < fs->file_index; tmp_file_seq++) {
        memset(tmp_filename,0,sizeof(tmp_filename));
        if (fs->file_type[tmp_file_seq] == 2 && fs->file_size[tmp_file_seq] == 0 && fs->load_flag&0x2) {
            if (strstr(fs->file_name[tmp_file_seq], "ak_quick_start")) {
                sprintf(tmp_filename,"/lib/%s",fs->file_name[tmp_file_seq]);
                printk("kload[%d]:%s\n",tmp_file_seq,tmp_filename);
                filename_split_to_args(tmp_filename, tmp_uargs);
                module_insmod(tmp_filename, tmp_uargs);
            }
        }
    }
}

void get_isp_conf_from_little_core(struct fast4_info *fs, char *save_path)
{
    struct mtd_info *mtd;
    void *file_data = NULL;
    /*isp分区的header*/
    isp_partition_header_t isp_partition_header;
    /*isp文件的header*/
    image_header_t isp_conf_header;
    char name[16] = "ISPCONF";
    int target_fd = -1;
    int offset = 0, index = 0, num = 0, sum = 0;

    // 根据名称获取MTD设备
    mtd = get_mtd_device_nm(name);
    if (IS_ERR(mtd)) {
        pr_err("%s, Cannot find MTD device %s\n", __func__, name);
        return PTR_ERR(mtd);
    }

    if (!fs || !save_path) {
        printk("%s, Invalid parameters\n", __func__);
        return -EINVAL;
    }

    file_data = ioremap(ISPCONF_MEM_BASE, mtd->size);
    if (!file_data) {
        printk("%s, Failed to ioremap ISP config memory\n", __func__);
        return -ENOMEM;
    }

    /*获取isp分区header*/
    memcpy(&isp_partition_header, file_data, ISPCONF_HEAD_LEN);

    /*解析isp分区header，定位到有效的isp.conf，目前多目使用相同的isp.conf*/
    num = isp_partition_header.valid_num;
    index = isp_partition_header.valid_index[num];
    offset = isp_partition_header.isps_offset[index] + ISPCONF_HEAD_LEN;
    // pr_err("[zltc][%s][%d] num[%d] index[%d] offset[%d]\n", __func__, __LINE__, num, index, offset);

    memcpy(&isp_conf_header, file_data+offset, ISPCONF_HEAD_LEN);

    sum = checksum(file_data+offset+ISPCONF_HEAD_LEN, be32_to_cpu(isp_conf_header.ih_size));
    if (sum != isp_conf_header.ih_dcrc) {
        pr_err("isp.conf checksum err, sum[0x%x] ih_dcrc[0x%x] ih_size[0x%x]\n",
         sum, isp_conf_header.ih_dcrc, be32_to_cpu(isp_conf_header.ih_size));
        return;
    }

    target_fd = sys_open(save_path, O_CREAT | O_WRONLY, 0755);
    if (target_fd >= 0) {
        sys_write(target_fd, (char *)file_data+offset+ISPCONF_HEAD_LEN, be32_to_cpu(isp_conf_header.ih_size));
        sys_close(target_fd);
    }

    iounmap(file_data);

    // 使用完后释放引用
    put_mtd_device(mtd);

    /*
     * 跑到这isp文件已经创建好了，可以跑采集了
     */
    /*insmod pre-load ko stage2*/
    preload_ko_stage2(fs);

    /*更新匹配标志到flash*/
    camera_update_checkid_file(fs, 1);

    /*更新标志*/
    fs->had_check_sensorid = 1;
}

void sensor_conf_resolve(struct fast4_info *fs)
{
    /**************对isp_cfg/isp_sc2336p_mipi_2lane_av100.conf进行解析 -- start*************/
    allow_conf_len = fs->file_size[fs->cfg_seq];
    //cfg len
    CONFIG_LEN = fs->file_size[fs->cfg_seq];

    if(real_conf_len>0)
        CONFIG_LEN = real_conf_len; //real config file size

    conf_name = fs->file_name[fs->cfg_seq];

    /*获取sensor的id进行匹配*/
    memcpy(&conf_sensor_id, (head_data+fs->i+param_len+20), 4);
    // printk(KERN_BLUE"[conf_name:%s] [CONFIG_LEN:%d] [CONF_ID:%04x] [sensor_init_id:%d]"KERN_RESET"\n",
    // conf_name,CONFIG_LEN,conf_sensor_id,sensor_init_id);

    if (fs->had_check_sensorid) {
        /*
         * 如果已经匹配过sensorID，证明之前的sensorID就是对的了，
         * 无需其他匹配动作了
         */
        sensor_init_id = conf_sensor_id;
    }


    if(conf_sensor_id == sensor_init_id){
        printk("sensor_init_id match conf ok\n");
    }else{
        printk("sensor_init_id match conf fail %x&%x, wait conf file update!\n",
                conf_sensor_id, sensor_init_id);
        fs->run_flag &= ~0x100; //clear kthread load fastfs, avoid goto application at sensor matched before
    }

#if USE_CFG_BUF
    //export sensor reg init data, need sensor_id match config
    if( (conf_sensor_id == sensor_init_id) && (fs->run_flag&0xf) && fs->run_proc_seq>0 && head_data && CONFIG_LEN>0)
    {
        isp_cfg_buf = head_data + fs->i + 512; //cfg_header len 512B, header info
        if(fs->cfg_seq>0)
            isp_cfg_buf += fs->file_size[0];//offset add param file len
        isp_cfg_len = CONFIG_LEN - 512;
        memcpy(&fs->file_offset, (isp_cfg_buf + 93108)-2, 2);
        printk("sensor_id=%04x,reg_num=%d\n",conf_sensor_id,fs->file_offset/4);
        fs->file_offset = 0;
    }
#endif

}

int config_file_fun(struct fast4_info *fs){

    int target_fd = -1;
    int target_fd1= -1;
    char file_name[64];
    int size = 0;
    int ret = 0; 
    char *data = NULL;

    char *conf_data = NULL;
    if( fs->fs_readed==FAST_FS_LEN ) {  
        printk("fastfs readed, goto loading\n");
        fs->file_len = FAST_FS_LEN;
        if(fs->bigmem_used){
            fs->bigmem_used_offset = 0;
            fs->file_data = (char *)(phys_to_virt(0x83A00000)+fs->bigmem_used_offset);
            fs->bigmem_used_offset += fs->file_len;
            printk("fs->file_data:%p,mem_offset[%d]\n",fs->file_data,fs->bigmem_used_offset);
            return -1;
        }//has config partition
    }else if(fs->conf_index>0)  {
        fs->conf_tosize = ((fs->conf_tosize + 4095)/4096) * 4096;
        conf_data = kmalloc(fs->conf_tosize, GFP_KERNEL); //phys_to_virt(APP_ADDR); //use app dmabuf
        if(get_partition_data("CONFIG2", 0, conf_data, &fs->conf_tosize,0)<0){
            get_partition_data("CONFIG", 0, conf_data, &fs->conf_tosize,0);
        }
        fs->file_offset = 0;
        fs->file_seq = 0;
        while(fs->file_seq < fs->conf_index) {
            memset(fs->save_filename,0,sizeof(fs->save_filename));
            sprintf(fs->save_filename,"/mnt/%s",fs->con_name[fs->file_seq]);
            target_fd = sys_open(fs->save_filename, O_CREAT | O_WRONLY, 0755);
            if(target_fd>=0){
                sys_write(target_fd, conf_data+fs->file_offset,fs->conf_size[fs->file_seq]);
                sys_close(target_fd);
                printk("load[%d] %s,size[%d],fs->file_offset[%d] ok\n",
                        fs->file_seq,fs->save_filename,fs->conf_size[fs->file_seq],fs->file_offset);
                fs->file_offset += fs->conf_size[fs->file_seq];
            }
            fs->file_seq++; 
        }
        kfree(conf_data);
        conf_data = NULL;
    }else{

        memset(file_name,0,64);
        if(  config_file && config_path  ){
            if(strstr(config_file,config_path))
                sprintf(file_name, "%s", config_file);
            else
                sprintf(file_name, "%s/%s", config_path,config_file);
        } else{
            sprintf(file_name, "%s", "/etc/config/fast_config.ini");
        }

        //printk(KERN_BLUE"[sys_read file_name:%s]"KERN_RESET"\n",file_name);

        target_fd = sys_open(file_name, O_RDONLY, 0644);
        pr_err("qs_config:%s target_fd:%d\n", file_name, target_fd);
        if(target_fd>=0) {
            ret = sys_lseek(target_fd, 0, SEEK_END);
            pr_err("fast_config.ini size:%d\n",ret);
            if(ret>0){
                size = ret;
                data = kmalloc(size, GFP_KERNEL);
            }
            sys_lseek(target_fd, 0, SEEK_SET);
            sys_read(target_fd, data, size);

            memset(fs->save_filename,0,sizeof(fs->save_filename));
            sprintf(fs->save_filename,"/mnt/%s","fast_config.ini");
            //printk(KERN_BLUE"[sys_write fs->save_filename:%s]"KERN_RESET"\n",fs->save_filename);

            target_fd1 = sys_open(fs->save_filename, O_CREAT | O_WRONLY, 0755);
            if(target_fd1>0) {
                sys_write(target_fd1, data, size);
                pr_err("/mnt/fast_config.ini target_fd1:%d\n",target_fd1);
                sys_close(target_fd1);
            }    
            sys_close(target_fd); 
            if(data)
                kfree(data);
        }
    }
    return 0;
}


int load_less_than_2m_data(struct fast4_info *fs)
{
    int cp_cnt = 0;
    //fs->file_len = FAST_FS_LEN;
    /*判断文件长度是否大于fs->file_len1:100k(102400)*/
    if(fs->file_len1 < FAST_FS_LEN){
#if SPLIT_READ
        fs->split_read = 0; //split next file index
        fs->file_len = fs->i; //head info len 
        while( fs->split_read < fs->file_index && (fs->file_len<=fs->i || (fs->file_len + fs->file_size[fs->split_read]) < (2*1024*1024)) ){

            fs->file_len += fs->file_size[fs->split_read++];

            // printk(KERN_BLUE"[fs->run_proc_seq:%d][fs->split_read-1:%d][fs->run_flag:0x%x]"KERN_RESET"\n",
            // fs->run_proc_seq, fs->split_read-1,fs->run_flag);

            if( (fs->run_proc_seq == fs->split_read-1) && (fs->run_flag&0xf) ){ //first load fast run program

                printk("first load fast run program file[%d]\n",fs->run_proc_seq);

                //sensor reg init func, need sensor_id match config
                if( (conf_sensor_id == sensor_init_id) && !enable_fast_ae() ) {
                    cp_cnt = sys_sensor_init_func();
                    if(cp_cnt<0) {//support sensor can't match, no app run!
                        printk("sensor init error, can't run fast app\n");
                        fs->run_flag = 0;
                    }
                }
                break;
            }
        }
#endif

        //printk(KERN_BLUE"[%s %d][fs->split_read:%d]"KERN_RESET"\n",__func__,__LINE__,fs->split_read);

        if(fs->file_len>FAST_FS_LEN){
            printk("fs->file_len error %lu\n",fs->file_len);
            fs->file_len = FAST_FS_LEN;
        }else if(fs->file_len == fs->i){
            printk("no preload file\n");
            return -1;
        }

        //set aligned 4K-block imporve read speed
        fs->file_len = ((fs->file_len + 4095)/4096) * 4096;

#if UPDATE_PART
        // printk(KERN_BLUE"[0xc1000000 phys:0x%x ]"KERN_RESET"\n",
        //     virt_to_phys(0xc1000000));/*[0xc1000000 phys:0x81000000 ]*/

        if(fs->bigmem_used){
            fs->bigmem_used_offset = 0;
            //printk(KERN_BLUE"[initrd_start:0x%x][fs->file_data:0x%x]"KERN_RESET"\n",initrd_start,fs->file_data);
            fs->file_data = (char *)(initrd_start+fs->bigmem_used_offset);

            fs->bigmem_used_offset += fs->file_len;

            fs->fast_used_off = fs->bigmem_used_offset; //save fs->fast_used_off reserved can't overlay !!!

            printk("fs->file_data:%p,mem_offset[%d]\n",fs->file_data,fs->bigmem_used_offset);
        }else{
            fs->file_data = kmalloc(fs->file_len, GFP_KERNEL);
            if(!fs->file_data)
                pr_err("kmalloc %lu fail\n",fs->file_len);
        }
#endif
        fs->file_offset = 0;//start read data position

        //cp fs->file_len1 data, avoid repeat read
        memcpy(fs->file_data, head_data, fs->file_len1);

        if(fs->file_len > fs->file_len1){
            fs->file_len -= fs->file_len1;

            cp_cnt = get_partition_data("APP2", fs->file_offset+fs->file_len1, fs->file_data+fs->file_len1, &fs->file_len, 0);
            if(cp_cnt<0){
                printk("get_partition_data :%d\n",cp_cnt);
            }
            fs->file_len += fs->file_len1;
        }

    }else{
        printk("preread-fs->file_len1:%d \n",fs->file_len1);
    }


    return 0;
}

/*更新isp的配置*/
void update_isp_conf(struct fast4_info *fs)
{

    //update isp conf file
    if( (update_flag==2 && fs->file_seq == fs->file_index-1) ){
        if(update_flag==2 && head_data && CONFIG_LEN>0){
            memcpy(&conf_sensor_id, (head_data+fs->i+param_len+20), 4);
            isp_cfg_buf = head_data + fs->i + 512; //cfg_header len 512B, header info
            //if(fs->cfg_seq>0)
            isp_cfg_buf += param_len;//offset add param file len            
            isp_cfg_len = CONFIG_LEN - 512;
            memcpy(&fs->i, (isp_cfg_buf + 93108)-2, 2);
            printk("conf_sensor_id:%04x, sensor reg num=%d\n",conf_sensor_id, fs->i/4);
            update_flag++;
            if(((fs->run_flag&0xf)==1) && fs->run_proc_seq>0)
                fs->run_flag |= 0x2; //allow updated re-run fast program 
        }

        if(update_flag>=2) {
            printk("fastfs need update:%d\n",update_flag);
            if(fs->bigmem_used){
                fs->file_data = (char *)initrd_start;
                fs->file_len=0;
                for(fs->i=0;fs->i<=fs->cfg_seq;fs->i++)
                    fs->file_len += fs->file_size[fs->i];
                fs->file_len = ((fs->file_len + 4095)/4096) * 4096 + 64*1024;
            }
            memcpy(fs->file_data, org_data, 8192);
            get_partition_data("APP2", 0, fs->file_data, (unsigned long *)&fs->file_len, 1); 
            printk("update write:%p,len[%lu] ok!\n",fs->file_data,fs->file_len);
            msleep(10);
            update_flag = 0;
            if(((fs->run_flag&0xf)==1) && fs->run_proc_seq>0)
                fs->run_flag |= 0x2; //allow updated re-run fast program 
            printk("fs->run_flag:%x,fs->run_proc_seq:%d\n",fs->run_flag,fs->run_proc_seq);
            //def FAST_AE_ENABLE
            if( (conf_sensor_id == sensor_init_id) ){
#if 0
                memset(fs->save_filename,0,sizeof(fs->save_filename));
                sprintf(fs->save_filename,"/mnt/%s",fs->file_name[fs->cfg_seq]);
                memset(fs->qs_isp_conf, 0x0, strlen(fs->qs_isp_conf));
                memcpy(fs->qs_isp_conf, fs->save_filename, strlen(fs->save_filename));
                printk("fs->qs_isp_conf:%s\n",fs->qs_isp_conf);
                fast_ae_quick_start_init(fs->qs_isp_conf);
                printk("fast_ae_quick_start_init done.\n");
#endif
            }
        }
    }

}

/*快启参数的处理*/
void fast_param_handle(struct fast4_info *fs)
{
    char *argx[6];
    char tmp[6][10];

    /**********************************************************************************************************/
    // fast_video run condition, it's very important, effect ouput stream time. support
    //1) fast_video first run [(flag&1)==1]
    //2) fast_video run wait while all file load finish  [(flag&2)==2]
    //3) fast_video run wait stream on     [(fs->run_flag&0x10)]
    //4) 1) 、2) same to must be sensor_id match ok, and fast_video index exist!
    /*******************************************************************************************************/
    if( ( ( ((fs->run_flag&0xf)==1) && (fs->run_proc_seq==fs->file_seq) )
                || ((fs->run_flag&0xf)>1 && fs->run_proc_seq>0 && fs->file_seq == fs->file_index-1) 
        ) && (conf_sensor_id == sensor_init_id) ) {

        //daemon process args input
        memset(fs->save_filename,0,sizeof(fs->save_filename));
        sprintf(fs->save_filename,"/mnt/%s",fs->file_name[fs->run_proc_seq]);
        printk("fast run program:%s\n",fs->save_filename);

        //first get sensor reg init 
        if(enable_fast_ae())
            sys_sensor_init_func();

        if(fs->run_proc_param[0]==0 && fs->run_proc_param[1]==0 && fs->run_proc_param[2]==0 ){
            for(fs->i=0;fs->i<4;fs->i++) {
                memset(tmp[fs->i],0,10);

                if(fs->i==0)
                    sprintf(tmp[fs->i],"%02x",4);
                else if(fs->i==1)
                    sprintf(tmp[fs->i],"%02x",2);
                else if(fs->i==2)
                    sprintf(tmp[fs->i],"%02x",1);
                else if(fs->i==3)
                    sprintf(tmp[fs->i],"%02x",4);
                argx[fs->i]=tmp[fs->i];
                printk("argx[%d]=%s\n", fs->i, argx[fs->i]);  
            }
        }else{
            for(fs->i=0;fs->i<4;fs->i++){
                memset(tmp[fs->i],0,10);
                sprintf(tmp[fs->i],"%02x",*(fs->run_proc_param+fs->i));
                argx[fs->i]=tmp[fs->i];
            }
        }

        //allow add 2 param
        while( (fs->i<6) && (fs->run_proc_param[fs->i]>0) ){
            memset(tmp[fs->i],0,4);
            sprintf(tmp[fs->i],"%02x",*(fs->run_proc_param+fs->i));
            argx[fs->i]=tmp[fs->i];
            printk("argx[%d]=%s\n", fs->i, argx[fs->i]);
            fs->i++;
        }

        //fast program run
        video_daemon_open(fs->save_filename, fs->i, argx);

        if( 1 ) 
            msleep(45); //delay wait isp init finish
        else
            wait_for_completion(&frame_done);  //may be isp init fail forever can't return
        //delete runned file
        sys_unlink(fs->save_filename);
        fs->run_flag |= 0x20; //first fast running flag
    }

    //sec fast run program, must after first program
    if( (fs->run_flag&0x60)==0x60 && (fs->file_type[fs->file_seq] == 7 || fs->file_seq == fs->file_index-1 ) )  {
        if( fs->file_type[fs->file_seq] == 7 )  
            fs->i =  fs->file_seq;
        else if(fs->file_seq == fs->file_index-1) {
            for(fs->i=0; fs->i<fs->file_index; fs->i++){
                if( fs->file_type[fs->i] == 7 )
                    break;
            }
        }

        if(fs->i < fs->file_index){
            memset(fs->save_filename,0,sizeof(fs->save_filename));
            sprintf(fs->save_filename,"/mnt/%s",fs->file_name[fs->i]);
            // printk(KERN_BLUE"[%s %d][ fs->save_filename:%s]"KERN_RESET"\n",__func__,__LINE__,fs->save_filename);
            //fast program run
            video_daemon_open(fs->save_filename, 0, NULL);
            sys_unlink(fs->save_filename);
        }
        fs->run_flag &= ~0x40; //clear has sec program file flag
    }

}

/*把超过2m的数据读下来*/
int load_more_than_2m_data(struct fast4_info *fs)
{
    int cp_cnt = 0;

#if SPLIT_READ
    if(fs->split_read>0 && fs->file_seq == fs->split_read){

        //printk(KERN_BLUE"[%s %d][fs->split_read:%d][fs->file_seq:%d][fs->file_name[fs->file_seq]:%s]"KERN_RESET"\n",
        //__func__,__LINE__,fs->split_read,fs->file_seq,fs->file_name[fs->file_seq]);

        if(!fs->bigmem_used)
            kfree(fs->file_data);

        fs->file_offset = fs->file_len;

        if(fs->file_type[fs->file_seq-1] == 6){
            printk("file[%d] at APP1\n",fs->file_seq-1);
        }else{
            fs->split_offset += fs->file_offset;
        }

        fs->file_len = 0;
        fs->split_write_index = fs->file_seq;

        // printk(KERN_BLUE"[%s %d][fs->split_read:%d][fs->file_index :%d]"KERN_RESET"\n",
        //__func__,__LINE__,fs->split_read,fs->file_index );
        while( fs->split_read<fs->file_index &&  (fs->file_len==0 || 
                    (fs->file_len + fs->file_size[fs->split_read]) < 2*1024*1024) )
        {
            fs->file_len += fs->file_size[fs->split_read++];
            //index to max or next file is cpio packge
            /*ramfs4.cpio.lz4*/
            if(fs->split_read == fs->file_index || fs->file_type[fs->split_read] == 3) 
                break;
        }

        //set aligned 4K-block imporve read speed
        if(fs->split_write_index <= fs->file_index-1) //current read index < max
            fs->file_len = ((fs->file_len + 4095)/4096) * 4096;
        printk("read size:%lu\n",fs->file_len);

        if(fs->bigmem_used){
#if UPDATE_PART
            if( (initrd_start+fs->bigmem_used_offset+fs->file_len)>initrd_end ){
                fs->bigmem_used_offset = fs->fast_used_off;
                fs->split_offset = fs->file_off[fs->file_seq];

                printk("fs->bigmem_used_offset reset to [%d],fs->split_offset:%d\n",fs->bigmem_used_offset, fs->split_offset);
            }
            fs->file_data = (char *)(initrd_start+fs->bigmem_used_offset);
            fs->bigmem_used_offset += fs->file_len;
            printk("fs->file_data:%p,used_offset[%d]\n",fs->file_data,fs->bigmem_used_offset);
#else
            fs->file_data = (char *)initrd_start;
#endif

        }else{
            fs->file_data = kmalloc(fs->file_len, GFP_KERNEL);
            if(!fs->file_data)
                printk("kmalloc %lu fail\n",fs->file_len);
        }

        //for sensor match config ok needn't read backup config file         
        if((fs->file_type[fs->file_seq]==5 ) && (conf_sensor_id == sensor_init_id)){
            printk("skip:%s\n",fs->file_name[fs->file_seq]);
        }else if(fs->file_type[fs->file_seq] == 6) {
            cp_cnt = get_partition_data("APP1", 0, fs->file_data, (unsigned long *)&fs->file_len,0);
        } else{
            cp_cnt = get_partition_data("APP2", fs->split_offset, fs->file_data, (unsigned long *)&fs->file_len,0);
        }
        printk("get_part-fs->file_len:%lu,fs->split_offset=%d cp_cnt:%d\n",fs->file_len,fs->split_offset,cp_cnt);
        fs->file_offset = 0;
    }

#endif
    return 0;

}



int load_file_start_fun(struct fast4_info *fs)
{
    int target_fd =-1;
#if FASTFS_KTHREAD
    int inbuf[3] = {0,0,0};
#endif
    char *err;
    fs->split_write_index = 0;

    for(fs->file_seq=0; fs->file_seq<fs->file_index; fs->file_seq++){

        //printk(KERN_BLUE"[%s %d][fs->split_read:%d][fs->file_seq:%d][fs->file_name[fs->file_seq]:%s]"KERN_RESET"\n",
        //      __func__,__LINE__,fs->split_read,fs->file_seq,fs->file_name[fs->file_seq]);
        load_more_than_2m_data(fs);

        memset(fs->save_filename,0,sizeof(fs->save_filename));
        if (fs->file_type[fs->file_seq] == 2 && fs->file_size[fs->file_seq] == 0)
            sprintf(fs->save_filename,"/lib/%s",fs->file_name[fs->file_seq]);
        else
            sprintf(fs->save_filename,"/mnt/%s",fs->file_name[fs->file_seq]);
        //printk(KERN_BLUE"[%s %d][ fs->save_filename:%s]"KERN_RESET"\n",__func__,__LINE__,fs->save_filename);

        //file data position calculate
        //split first file 
        if(fs->split_write_index > 0 && fs->file_seq == fs->split_write_index){
            fs->file_offset = fs->file_off[fs->file_seq-1]+fs->file_size[fs->file_seq-1]-(fs->file_data-(char *)initrd_start);
            //fs->bigmem_used_offset reset to fs->fast_used_off;
            if(fs->file_offset>0) {
                printk("fs->file_offset[%d] fs->bigmem_used_offset reset to [%d]\n", fs->file_offset, fs->fast_used_off);
                fs->file_offset = 0;
            }
            printk("cur fs->file_seq:%d,fs->file_off[%d]:%d,fs->file_size:%d,file_data_off:%d,fs->file_offset:%d\n", 
                    fs->file_seq, fs->file_seq-1, fs->file_off[fs->file_seq-1], fs->file_size[fs->file_seq-1], (fs->file_data-(char *)initrd_start), fs->file_offset);
        }else{
            if(fs->file_seq>0)
                fs->file_offset += fs->file_size[fs->file_seq-1];   
            else 
                fs->file_offset = fs->i;    //start from head tail
        }

        //file data right check !!!
        //if file has check code,to do data info check
        //has check code and flag is 1
        if(fs->check_code[fs->file_seq] && fs->check_code[fs->file_seq][0]==1) {
            if( (fs->check_code[fs->file_seq][1] == *(fs->file_data+fs->file_offset+0)) 
                    && (fs->check_code[fs->file_seq][2] == *(fs->file_data+fs->file_offset+1)) 
                    && (fs->check_code[fs->file_seq][3] == *(fs->file_data+fs->file_offset+2)) 
                    && (fs->check_code[fs->file_seq][4] == *(fs->file_data+fs->file_offset+3)) ){
                printk("chk[%d] ok\n",fs->file_seq);
            }else{
                printk("chk[%d] can't match: %02x %02x %02x %02x\n",fs->file_seq,*(fs->file_data+fs->file_offset+0),
                        *(fs->file_data+fs->file_offset+1),*(fs->file_data+fs->file_offset+2),*(fs->file_data+fs->file_offset+3));
                printk("%s check error!\n",fs->save_filename);
                return -1;
            } 
        } //file data right check end


        /*rootfs cpio file need uncompress to*/
        if(fs->file_type[fs->file_seq] == 3 || fs->file_type[fs->file_seq] == 6){
#if FASTFS_KTHREAD
            if( (fs->run_flag&0x100) && fs->file_type[fs->file_seq] == 3){
                unpack_addr = fs->file_data+fs->file_offset;
                unpack_len = fs->file_size[fs->file_seq];
                unpack_name = fs->file_name[fs->file_seq];
                inbuf[0] = fs->file_data+fs->file_offset;
                inbuf[1] = fs->file_size[fs->file_seq];
                inbuf[2] = (int)(char *)unpack_name;
                unpack_wait_cnt = 0;
                printk("unpack_start [%s],[%x]:[%d],%s\n",unpack_name, inbuf[0], inbuf[1], (char *)inbuf[2]);
                kthread_run(unpack_kthread, &inbuf[0], "unpack_kth");
            }
            else
#endif
            {
                schedule();
                /*
                   unsigned long timeout = jiffies+5, t1 = jiffies;
                   pr_err("delay unpack_to_fs %d0ms\n", timeout-t1);
                   while(jiffies<timeout) usleep_range(100,200);
                   */
                sys_chroot("/");
                printk("unpack_to_fs:%p,%d\n",fs->file_data+fs->file_offset,fs->file_size[fs->file_seq]);
                err = unpack_to_fs(fs->file_data+fs->file_offset,fs->file_size[fs->file_seq]);
                if (err)
                    printk("%s\n",err);
                sys_chroot("/");
                printk("unpack[%d] %s,size[%d],fs->file_offset[%d] ok\n",
                        fs->file_seq,fs->save_filename,fs->file_size[fs->file_seq],fs->file_offset);
            }
        }
        else if( (fs->file_type[fs->file_seq] != 4) && !((fs->file_type[fs->file_seq] == 5 ) && (conf_sensor_id == sensor_init_id)) ){
#if 0
            //param file not save /mnt
            if(fs->file_type[fs->file_seq] != 5 && fs->file_type[fs->file_seq] != 2){
                //cpio backup file needn't save, ko init from buf
                target_fd = sys_open(fs->save_filename, O_CREAT | O_WRONLY, 0755);			
                if(target_fd>=0) {
                    if(fs->file_seq == fs->cfg_seq && (conf_sensor_id == sensor_init_id) ){
                        sys_write(target_fd, fs->file_data+fs->file_offset,CONFIG_LEN);
                    } else{
                        sys_write(target_fd, fs->file_data+fs->file_offset,fs->file_size[fs->file_seq]);
                        // printk(KERN_BLUE"[%s %d][ fs->save_filename:%s]"KERN_RESET"\n",__func__,__LINE__,fs->save_filename);
                    }
                    sys_close(target_fd);
                    target_fd = -1;
                    printk("load[%d] %s,size[%d],fs->file_offset[%d] ok\n",
                            fs->file_seq,fs->save_filename,fs->file_size[fs->file_seq],fs->file_offset);
                    //def FAST_AE_ENABLE
                    if(fs->file_seq == fs->cfg_seq && (conf_sensor_id == sensor_init_id) ){
#if 0
                        memset(fs->qs_isp_conf, 0x0, strlen(fs->qs_isp_conf));
                        memcpy(fs->qs_isp_conf, fs->save_filename, strlen(fs->save_filename));
                        printk("fs->qs_isp_conf:%s\n",fs->qs_isp_conf);
                        fast_ae_quick_start_init(fs->qs_isp_conf);
                        printk("fast_ae_quick_start_init done.\n");
#endif
                    }
                }

                /*
                 * 跑到这isp文件已经创建好了，可以跑采集了
                 */
                /*insmod pre-load ko stage2*/
                preload_ko_stage2(fs);

                /*更新匹配标志到flash*/
                camera_update_checkid_file(fs, 1);

                /*更新标志*/
                fs->had_check_sensorid = 1;
            }

            //cpio backup file need uncompress
            if(fs->file_type[fs->file_seq] == 5 && (conf_sensor_id != sensor_init_id) ){
                sys_chdir("/mnt");
                printk("unpack_to_fs:%p,%d\n",fs->file_data+fs->file_offset,fs->file_size[fs->file_seq]);
                update_flag = 1;
                err = unpack_to_fs(fs->file_data+fs->file_offset,fs->file_size[fs->file_seq]);
                if (err)
                    printk("%s\n",err);
                sys_chroot("/");
                if(conf_sensor_id != sensor_init_id){
                    fs->run_flag = 0;		
                    printk("sensor matched fail, can't run fast programe\n");		  
                }else{
                    update_flag++;
                }
                printk("unpack[%d] %s,size[%d],fs->file_offset[%d] ok\n",fs->file_seq,fs->save_filename,fs->file_size[fs->file_seq],fs->file_offset);
            }
#endif
        }

        //update isp conf file
        update_isp_conf(fs);

        fast_param_handle(fs);

        if( (fs->load_flag&0x2) && (fs->file_type[fs->file_seq] == 2) ) //run ko module file
        {
            if (fs->file_size[fs->file_seq] == 0) {
                //ygh do none
            } else {
                sys_kinit_mod((char *)fs->file_data+fs->file_offset, fs->file_size[fs->file_seq], NULL);
            }
        }
    }

    fs->file_offset += fs->file_size[fs->file_seq-1] + fs->split_offset;
    printk("load end file_offset[%d],files[%d]\n",fs->file_offset,fs->file_seq);

    return 0;
}


void load_file_end_fun(struct fast4_info *fs)
{
#if DYNAMIC_MALLOC
#if UPDATE_PART // write partition 
        if(update_flag>=2){
            printk("fastfs need update:%d\n",update_flag);
            //update fast4.fs partition test
            if(fs->bigmem_used ){
                fs->file_data = (char *)initrd_start;
                //adjust write size to min
                fs->file_len=0;
                for(fs->i=0;fs->i<=fs->cfg_seq;fs->i++)
                    fs->file_len += fs->file_size[fs->i];
                fs->file_len = ((fs->file_len + 4095)/4096) * 4096;
            }
            memcpy(fs->file_data, org_data, 8192);
            get_partition_data("APP2", 0, fs->file_data, &fs->file_len, 1); 
            printk("update write:%p,len[%lu] ok!\n",fs->file_data,fs->file_len);
            update_flag = 0;
            //update test end
        }
#endif
        if( !(fs->run_flag&0x100) ){
            free_initrd_mem(initrd_start, initrd_end);
            free_initrd_mem((unsigned long)phys_to_virt(APP_ADDR),
                    (unsigned long)phys_to_virt(APP_ADDR+CONFIG_FAST_FS_HEAD_SIZE));
            fs->file_data = NULL;
            initrd_start = 0;
            initrd_end = 0;
            fs->bigmem_used = 0;
        }
#endif
}

//fastfs loading function

static int daemon_run(int run_flag, int load_flag)
{
    int ret = 0;
    struct fast4_info *fs;

    printk("%s-%s\n", FAST_LAUNCH_VERSION,FAST_BUILD_TIME);

    fs = kmalloc(sizeof(struct fast4_info), GFP_KERNEL);
    if(!fs){
        pr_err("struct fast4_info *fs kmalloc err!\n");
        return -ENOMEM;
    }

    /*对成员初始化*/
    fs->i = 0;
    fs->file_index = -1;
    fs->file_data = NULL;
    memset( fs->file_name, 0, sizeof(fs->file_name));
    memset( fs->file_type, 0, sizeof(fs->file_type));
    memset( fs->file_size, 0, sizeof(fs->file_size));
    memset( fs->run_proc_param, 0, sizeof(fs->run_proc_param));
    fs->run_proc_seq = -1;
    fs->run_flag = run_flag;
    fs->load_flag = load_flag;
    memset( fs->check_code, 0, sizeof(fs->check_code));
    fs->cfg_seq = -1;
    fs->total_size = 0;
    fs->file_len = 0;
    memset( fs->file_off, 0, sizeof(fs->file_off));
    fs->file_offset = 0;
    fs->file_len1 = PRELOAD_LEN;
    memset( fs->save_filename, 0, sizeof(fs->save_filename));
    memset( fs->qs_isp_conf, 0, sizeof(fs->qs_isp_conf));
    fs->split_offset = 0;
    memset( fs->con_name, 0, sizeof(fs->con_name));
    fs->conf_index = 0;
    memset( fs->conf_size, 0, sizeof(fs->conf_size));
    fs->conf_tosize = 0;
    fs->fs_readed =0;

    fs->split_read = 0;
    fs->split_offset = 0 ;
    fs->bigmem_used = 1;
    fs->bigmem_used_offset = 0;

#if !defined(CONFIG_MACH_AK3918AV130)
    /*将物理地址0x81200000转为虚拟地址*/
    fs->file_data = phys_to_virt(APP_ADDR);
#endif

    if( !(initrd_start >= (unsigned long)phys_to_virt(0x80a00000) &&
        initrd_end <= (unsigned long)phys_to_virt(CONFIG_FAST_FS_HEAD_ADDR)) ){
       pr_err("[%s %d] error ! initrd must 0x80a00000 ~ 0x%08x but current initrd is:0x%lx ~ 0x%lx \n",
           __func__,__LINE__,CONFIG_FAST_FS_HEAD_ADDR,
           (unsigned long)virt_to_phys((void *)initrd_start),
           (unsigned long)virt_to_phys((void *)initrd_end));
       return -ENOMEM;
    }

    /*
       printk(KERN_BLUE"[initrd_start phys:0x%x virt:0x%x] \n[initrd_end phys:0x%x virt:0x%x][file_data  phys:0x%x virt:0x%x]"KERN_RESET"\n",
       virt_to_phys(initrd_start),initrd_start,
       virt_to_phys(initrd_end),initrd_end,
       virt_to_phys(fs->file_data),fs->file_data);
       */

    //use new kmem avoid overlay
    /*申请长度100*1024字节的内存*/
    head_data = kmalloc(fs->file_len1, GFP_KERNEL);
    if(!head_data){
        pr_err("head_data kmalloc err!\n");
        return -ENOMEM;
    }

    /*申请长度8*1024字节的内存*/
    org_data = kmalloc(8192, GFP_KERNEL);
    if(!org_data){
        pr_err("org_data kmalloc err!\n");
        return -ENOMEM;
    }

#if !defined(CONFIG_MACH_AK3918AV130)
    /*将file_data（0x81200000）的100*1024字节的内存拷贝到head_data*/
    memcpy(head_data,fs->file_data,fs->file_len1);
#else
    get_partition_data("APP2", 0, head_data, &fs->file_len1, 0); 
    fs->file_data = head_data;
#endif
    /*将file_data（0x81200000）的8*1024字节的内存拷贝到org_data*/
    memcpy(org_data,fs->file_data,8192);

    fs->file_data = head_data; //move to new head data addr

    if( (fs->load_flag&0x1) ){
        /*解析fast.fs头部数据的格式*/
        head_date_resolve(fs);

        /*insmod pre-load ko stage1*/
        preload_ko_stage1(fs);

        /*对isp_cfg/isp_sc2336p_mipi_2lane_av100.conf解析*/
        // sensor_conf_resolve(fs);
        get_isp_conf_from_little_core(fs, "/mnt/isp.conf");

        /*fast_config.ini解析函数*/
        ret = config_file_fun(fs);
        if(ret < 0)
            goto LOAD_FILE_START;

        /*把不超过2m的数据一次性读下来*/
        ret = load_less_than_2m_data(fs);
        if(ret < 0)
            goto LOAD_FILE_END;

LOAD_FILE_START:
        /*读其余数据并解压*/
        ret = load_file_start_fun(fs);
        if(ret < 0)
            goto LOAD_FILE_END;

LOAD_FILE_END:
        /*结束*/
        load_file_end_fun(fs);
    }

    //no fast push stream, allow re-init video modules 
    //!run_flag or no fast_app          
    if( !(fs->run_flag&0xf) || fs->run_proc_seq<0) {
#ifndef FAST_AE_ENABLE
        if(enable_fast_ae());//ygh get_ae_fast_def();
#endif
        set_allow_reinit();
    }

    if( (fs->run_flag&0x100) ){
        wait_for_completion(&unpack_done);
        //wait_for_completion(&busybox_done);
    }

    printk("%s finish\n",__func__);

    /*释放申请的动态内存*/
    if(!fs)
        kfree(fs);

    if(!head_data)
        kfree(head_data);

    if(!org_data)
        kfree(org_data);

    return 0;
}


int video_daemon_init(int fastfs_len, int conf_len)
{
    return daemon_run(fastfs_len, conf_len);
}

static int module_insmod(const char *path, const char *uargs)
{
    int fd;
    int ret;

    fd = sys_open(path, O_RDONLY, 0644);
    if(fd < 0){
        pr_err("%s:%d, open %s failed", __func__, __LINE__, path);
        return -1;
    }
    ret = sys_finit_module(fd, uargs, 0);
    if (ret < 0) {
        pr_err("finit_module failed ret=%d \n", ret);
        sys_close(fd);
        return -1;
    }
    sys_close(fd);

    sys_unlink(path);
    return 0;
}

#if !defined(CONFIG_FAST_FS)

static void load_one_ko(char *name)
{
    char tmp_filename[FILES_MAX];
    char tmp_uargs[FILES_MAX];

    sprintf(tmp_filename,"/init/%s",name);
    printk("kload:%s\n", tmp_filename);
    filename_split_to_args(tmp_filename, tmp_uargs);
    module_insmod(tmp_filename, tmp_uargs);
}

static void parse_len_bin(char *file_data, int file_len)
{
    int i = 0;
    int cp_cnt = 0;
    char ko_name[256];

    //find file info '[' start, skip before other info
    while( i < file_len ){
        if (file_data[i++] != '[' )
            continue;

        cp_cnt = 0;        
        for(; file_data[i] != ']'; i++, cp_cnt++){
        }

        /*打印文件名*/
        strncpy(ko_name, file_data + i - cp_cnt, cp_cnt);
        ko_name[cp_cnt] = 0;

        load_one_ko(ko_name);
        
        i++;
    }
}

void romfs_load_ko(void)
{
    long source_fd, target_fd;
    char *file_data;
    int rlen, wlen;
    char len_bin[512];

    file_data = (char *)(phys_to_virt(0x83A00000));

    source_fd = sys_open("/init/isp.conf", O_CREAT | O_RDONLY, 0755);			
    rlen = sys_read(source_fd, file_data, 200*1024);
    sys_close(source_fd);

    target_fd = sys_open("/mnt/isp.conf", O_CREAT | O_WRONLY, 0755);			
    wlen = sys_write(target_fd, file_data, rlen);
    sys_close(target_fd);

    //read len.bin
    source_fd = sys_open("/init/len.bin", O_CREAT | O_RDONLY, 0755);			
    rlen = sys_read(source_fd, len_bin, 512);
    sys_close(source_fd);
    printk("[len_bin size: %d]\n", rlen);

    parse_len_bin(len_bin, rlen);
}

#endif
MODULE_DESCRIPTION("Anyka Video daemon");
MODULE_AUTHOR("Anyka Microelectronic");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("2.0.00");
