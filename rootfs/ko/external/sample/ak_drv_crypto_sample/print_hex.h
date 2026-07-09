/**
 * 以十六进制格式详细打印数据
 * 支持彩色显示，用于调试和数据分析
 * @param i_color_mode 彩色模式开关（0=关闭，1=开启）
 * @param i_color_back 背景颜色代码
 * @param i_color_front 前景颜色代码
 * @param pc_data 要打印的数据指针
 * @param i_len 数据长度
 * @return 0表示成功，负值表示失败
 */
int print_hex_detail( int i_color_mode , int i_color_back , int i_color_front, unsigned char *pc_data , int i_len );
