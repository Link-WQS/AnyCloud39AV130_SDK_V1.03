/**
 * 程序主入口函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 程序退出状态码
 */
int main(int argc, char** argv);

/**
 * 显示程序帮助信息
 * 打印程序使用方法和参数说明
 * @return 0表示成功
 */
int help(void);

/**
 * 程序退出处理函数
 * 用于捕获信号并进行程序清理工作
 * @param i_sig 接收到的信号编号
 */
void prog_exit(int i_sig);

/**
 * 启动加密性能测试
 * 根据索引选择不同的测试模式并执行加密操作
 * @param pi_idx 指向测试模式索引的指针
 * @return 0表示成功，负值表示失败
 */
int start_crypto(int *pi_idx);

/**
 * 初始化加密设备
 * 配置加密算法、密钥等参数
 * @param _p_ak_encrypt 指向加密设备结构体的指针
 * @param i_encrypt_type 加密算法类型
 * @param pc_key 加密密钥指针
 * @param i_key_bits 密钥长度（比特数）
 * @return 0表示成功，负值表示失败
 */
int cipherdev_init(struct ak_encrypt *_p_ak_encrypt, int i_encrypt_type, char *pc_key, int i_key_bits);

/**
 * 执行加密/解密操作
 * 使用指定的IV、输入数据生成输出数据
 * @param _p_ak_encrypt 指向加密设备结构体的指针
 * @param pc_iv 初始化向量指针
 * @param _pc_in 输入数据指针
 * @param pc_out 输出数据指针
 * @param pc_auth 认证数据指针（用于AEAD算法）
 * @param i_len 数据长度
 * @return 0表示成功，负值表示失败
 */
int cipherdev_enc(struct ak_encrypt *_p_ak_encrypt, char *pc_iv, char *_pc_in, char *pc_out, char *pc_auth, int i_len);

/**
 * 释放加密设备资源
 * 清理分配的内存和关闭设备句柄
 * @param p_ak_encrypt 指向加密设备结构体的指针
 */
void cipherdev_release(struct ak_encrypt *p_ak_encrypt);

/**
 * 分配对齐内存
 * 分配指定大小且按指定字节对齐的内存
 * @param size 需要分配的内存大小
 * @param alignd_byte 对齐字节数（如16、32、64等）
 * @return 成功返回内存指针，失败返回NULL
 */
void* calloc_align(size_t size, size_t alignd_byte);

/**
 * 释放对齐内存
 * 释放由calloc_align分配的内存
 * @param p 需要释放的内存指针
 */
void free_align(void *p);

/**
 * 以十六进制格式打印数据
 * 用于调试和结果显示
 * @param pc_data 要打印的数据指针
 * @param i_len 数据长度
 * @return 0表示成功
 */
int print_hex(unsigned char *pc_data, int i_len);

/**
 * 生成短选项字符串
 * 根据长选项数组生成对应的短选项字符串
 * @param p_option 长选项数组指针
 * @param i_num_option 选项数量
 * @param pc_option_short 短选项字符串缓冲区
 * @param i_len_option 缓冲区长度
 * @return 生成的短选项字符串长度
 */
static char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option );

/**
 * 初始化基准测试环境
 * 分配内存、准备测试数据等
 * @param i_idx 测试模式索引
 * @param p_benchmark 指向基准测试结构体的指针
 * @return 0表示成功，负值表示失败
 */
int init_benchmark(int i_idx, struct benchmark *p_benchmark);

/**
 * 启动硬件加密测试
 * 执行硬件加速的加密性能测试
 * @param p_benchmark 指向基准测试结构体的指针
 */
void start_crypto_hw(struct benchmark *p_benchmark);
