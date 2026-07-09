#ifndef SEC2USEC
#define SEC2USEC 1000000
#endif

#ifndef SEC2NSEC
#define SEC2NSEC 1000000000
#endif

#define LEN_PERF 64
#define NUM_PERF_NOTINIT -1

#define TIME_PERF_START \
{ \
    static int i_time_perf_mark = NUM_PERF_NOTINIT ;\
    time_perf_start( &i_time_perf_mark,  __FILE__ , __LINE__ , __func__ ) ;

#define TIME_PERF_END \
    time_perf_end( i_time_perf_mark ); \
}

struct timeperf {
    char c_mark ;
    char ac_file[ LEN_PERF ];
    unsigned int i_line;
    char ac_func[ LEN_PERF ];
    unsigned long long i_us_min, i_us_max;
    unsigned long long i_us_total;
    unsigned int i_times;
    struct timeval timeval_start, timeval_end;
};



/**
 * 初始化性能计时器系统
 * 分配指定数量的计时器结构体内存
 * @param i_num_perf 需要初始化的计时器数量
 * @return 0表示成功，负值表示失败
 */
int time_perf_init(int i_num_perf);

/**
 * 启动指定索引的性能计时器
 * 记录函数执行开始时间，用于性能测试
 * @param _pi_num 指向计时器索引的指针，如果为负数则自动分配
 * @param pc_file 调用此函数的源文件名
 * @param i_line 调用此函数的行号
 * @param pc_func 调用此函数的函数名
 * @return 0表示成功，其他值表示失败
 */
int time_perf_start(int *_pi_num, char *pc_file, unsigned int i_line, const char *pc_func);

/**
 * 停止指定索引的性能计时器
 * 记录函数执行结束时间并计算耗时
 * @param i_num 要停止的计时器索引
 * @return 0表示成功，负值表示失败
 */
int time_perf_end(int i_num);

/**
 * 打印所有性能计时器的统计结果
 * 显示各函数的执行时间和调用信息
 * @param pp_timeperf 指向计时器数组指针的指针
 * @return 0表示成功，负值表示失败
 */
int time_perf_print(struct timeperf **pp_timeperf);

/**
 * 释放性能计时器系统资源
 * 释放分配的内存并清理计时器状态
 * @return 0表示成功，负值表示失败
 */
int time_perf_exit(void);

/**
 * 重置所有性能计时器
 * 清零所有计时器的统计数据，保留内存分配
 * @return 0表示成功，负值表示失败
 */
int time_perf_reset(void);

/*
    timeval_mark: 对一个timeval结构体时间赋予当前时间戳
    @p_timeval[IN]: 传入的struct timeval结构体指针
    return: 传入的结构体指针地址
*/
struct timeval *timeval_mark( struct timeval *p_timeval ) ;

/*
    timeval_count: 对两个timeval之间的时间差进行统计
    @p_timeval_begin[IN]: 传入的struct timeval结构体指针,记录开始值
    @p_timeval_end[IN]: 传入的struct timeval,记录结束值
    return: 两个struct timeval的usec差值
*/
long long timeval_count( struct timeval *p_timeval_begin , struct timeval *p_timeval_end ) ;

/*
    timespec_mark: 对一个timespec结构体时间赋予当前时间戳
    @p_timespec[IN]: 传入的struct timespec结构体指针
    return: 传入的结构体指针地址
*/
struct timespec *timespec_mark( struct timespec *p_timespec ) ;

/*
    timespec_count: 对两个timespec之间的时间差进行统计
    @p_timespec_begin[IN]: 传入的struct timespec结构体指针,记录开始值
    @p_timespec_end[IN]: 传入的struct timespec,记录结束值
    return: 两个struct timespec的nsec差值
*/
long long timespec_count( struct timespec *p_timespec_begin , struct timespec *p_timespec_end ) ;