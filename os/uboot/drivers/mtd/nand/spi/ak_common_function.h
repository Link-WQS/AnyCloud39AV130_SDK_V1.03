/**
* @FILENAME: common_function.h
* @BRIEF:  
* @Copyright (C) 2025 Anyka Micro-Electronic Technology Co., LTD
* @AUTHOR  ZouTianxiang 
* @DATA 2025-3-1
* @VERSION 1.0
* @DELAY:  
* @REF please refer to...
*/
#ifndef __COMMON_FUN_H_
#define __COMMON_FUN_H_


#define LEN_MB(x)  (x*0x100000)
#define LEN_KB(x)  (x*1024)


//«ÂAHB –¥ª∫¥Ê
#define CLEAR_AHB_WRITE_BUF     {REG32(0x00000000) = REG32(0x00000000);}



#endif
