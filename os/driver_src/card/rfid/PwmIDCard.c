/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-12-25 15:31:54
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-04-12 11:13:23
 * @FilePath: /project_3/src/PwmIdCard.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "GeneralInterface.h"
#include "DrvSwipeCard.h"
#include "PwmControl.h"
#include "PwmIdCard.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define RFID_DEV_PATH "/dev/rfid_control"
#define RFID_MODULE_KO "/usr/modules/rfid.ko"

static int FilterDuplicareCard(const char *Data)
{
    static char CardCache[CARD_CACHE_BUFFER_SIZE] = {0};

    static struct timespec time;

    if (memcmp(CardCache, Data, PWM_ID_CARD_LEN) == 0)
    {
        int DiffMs = DiffClockTimeMs(&time);
        GetClockTimeMs(&time);
        return DiffMs < 1000 ? -1 : 0;
    }
    GetClockTimeMs(&time);
    memcpy(CardCache, Data, CARD_CACHE_BUFFER_SIZE);
    return 0;
}
#if 1
/**********************************************************弱函数重定义************************************************************8*/
int PwmIdCardModuleInit(int *DrvFd, int *DrvLen)
{
    if (AkDrvPwmOpen(3) != 0)
    {
        printf("AkDrvPwmOpen fail\n");
        return -1;
    }
    if (AkDrvPwmSet(3, 4000, 8000) != 0)
    {
        AkDrvPwmClose(3);
        printf("AkDrvPwmSet fail\n");
        return -1;
    }

    if (access(RFID_MODULE_KO, F_OK) != 0)
    {
        AkDrvPwmClose(3);
        printf("%s Not Found!!!\n", RFID_MODULE_KO);
        return -1;
    }

    system("insmod " RFID_MODULE_KO);

    if ((*DrvFd = open(RFID_DEV_PATH, O_RDONLY)) < 0)
    {
        return -1;
    }
    *DrvLen = PWM_ID_CARD_LEN;
    return *DrvFd;
}

int PwmIdCardModuleHandle(int *DrvFd, char *Data)
{
    if (FilterDuplicareCard(Data) == -1)
    {
        return -1;
    }

    printf("CARD : [%02x %02x %02x %02x]\n", Data[0], Data[1], Data[2], Data[3]);
    return 0;
}
/***********************************************************************************************************************/

#endif