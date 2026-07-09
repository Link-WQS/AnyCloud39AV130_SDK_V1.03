#ifdef CFG_TEST_USBH_CDC

#include "usbh_cdc.h"
#include "dbg.h"
#include "console.h"
#include "rtos.h"
#include "ring_buffer.h"
#include "co_main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
               /* File object */
typedef enum
{
    APPLICATION_IDLE = 0,
    APPLICATION_START,
    APPLICATION_RUNNING,
} MSC_ApplicationTypeDef;
#define APPLICATION_STATE_CHANGED           (0x01UL << 0)
#define APPLICATION_STATE_EM          		(0x02UL << 0)
#define APPLICATION_STATE_RM          		(0x04UL << 0)

#define	USBH_TBUFF_MAX					1024
static MSC_ApplicationTypeDef Appli_state = APPLICATION_IDLE;
#if (USBH_USE_OS == 1)
static rtos_task_handle test_task_handle = NULL;
#endif
static co_timer *co_usb_timer = NULL;


/* Private function prototypes -----------------------------------------------*/
//static void SystemClock_Config(void);
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);
uint16_t usbh_cdc_data_push(uint8_t *buff ,uint16_t len);


USBH_HandleTypeDef hUSB_Host; 
static uint8_t CDC_TX_Buffer[USBH_TBUFF_MAX]  __attribute__ ((at(0x160000)));
static uint8_t CDC_R_Buffer[USBH_TBUFF_MAX*2]  __attribute__ ((at(0x160000+USBH_TBUFF_MAX)));

static ring_buffer_t usb_cdc_rbuff;

static rtos_mutex usbh_capture_ring_buffer_mutex = NULL;

#define USBH_RING_BUFFER_LOCK()       rtos_mutex_recursive_lock(usbh_capture_ring_buffer_mutex)
#define USBH_RING_BUFFER_UNLOCK()     rtos_mutex_recursive_unlock(usbh_capture_ring_buffer_mutex)

static int do_usbcdc_send(int argc, char * const argv[])
{
	if(Appli_state == APPLICATION_RUNNING)
	{
		if(argc > 1)
		{
			usbh_cdc_data_push((uint8_t*)argv[1],strlen(argv[1]));
		}
		else
		{
			usbh_cdc_data_push((uint8_t*)"ok,let's go!!",15);
		}
		
		if (test_task_handle)
			rtos_task_notify_setbits(test_task_handle, APPLICATION_STATE_EM, false);
	}
		
	else
		dbg_test_print("USBH not active\r\n");

	return 0;
}

static int do_usbcdc_reenum(int argc, char * const argv[])
{
	if(Appli_state == APPLICATION_RUNNING)
	{
		if (test_task_handle)
			rtos_task_notify_setbits(test_task_handle, APPLICATION_STATE_RM, false);
	}
		
	else
		dbg_test_print("USBH not active\r\n");

	return 0;
}

static void co_usb_timer_handler(void *cb_param)
{
	if(Appli_state == APPLICATION_RUNNING)
	{
		if (test_task_handle)
			rtos_task_notify_setbits(test_task_handle, APPLICATION_STATE_RM, false);
	}
	dbg_test_print("USBH reenum for  timer\r\n");
}

uint16_t usbh_cdc_data_ava(void)
{
	USBH_RING_BUFFER_LOCK();
	uint16_t nlen =  ring_buffer_bytes_used(&usb_cdc_rbuff);
	USBH_RING_BUFFER_UNLOCK();

	return nlen;
}

uint16_t usbh_cdc_data_push(uint8_t *buff ,uint16_t len)
{
	if(Appli_state != APPLICATION_RUNNING)
		return 0;
	
	USBH_RING_BUFFER_LOCK();
	uint16_t nlen = ring_buffer_bytes_free(&usb_cdc_rbuff);
	nlen = nlen < len?nlen:len;

	if(nlen)
	{
		ring_buffer_write(&usb_cdc_rbuff, buff, nlen);
	}

	USBH_RING_BUFFER_UNLOCK();

	if(nlen)	
		rtos_task_notify_setbits(test_task_handle, APPLICATION_STATE_EM, false);
	return nlen;
}

uint16_t usbh_cdc_data_pop(uint8_t *buff)
{
	USBH_RING_BUFFER_LOCK();
	uint16_t nlen = ring_buffer_bytes_used(&usb_cdc_rbuff);
	if(nlen)
	{
		nlen = nlen>USBH_TBUFF_MAX?USBH_TBUFF_MAX:nlen;
		
		ring_buffer_read(&usb_cdc_rbuff, buff, nlen);
	}

	USBH_RING_BUFFER_UNLOCK();

	return nlen;
}


void USBH_CDC_TransmitCallback(USBH_HandleTypeDef *phost)
{
	dbg_test_print("USBH data have sent\r\n");
	if(usbh_cdc_data_ava())
	{
		rtos_task_notify_setbits(test_task_handle, APPLICATION_STATE_EM, false);
	}
	
	if(co_usb_timer != NULL)
		co_timer_stop(co_usb_timer);
}


void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef *phost)
{
	dbg_test_print("USBH have data coming\r\n");
}

/* Private functions ---------------------------------------------------------*/
void usbh_cdc_test()
{
	test_task_handle = rtos_get_task_handle();
	//memset(CDC_TX_Buffer,0,256);
	//memset(CDC_TX_Buffer,'F',128);
    /*##-2- Init Host Library ################################################*/
	USBH_Init(&hUSB_Host, USBH_UserProcess, 0);

    /*##-3- Add Supported Class ##############################################*/
    USBH_RegisterClass(&hUSB_Host, USBH_CDC_CLASS);

    /*##-4- Start Host Process ###############################################*/
    USBH_Start(&hUSB_Host);

	console_init();
	console_cmd_add("ucdct", "usbhcdc_send", 1, 2, do_usbcdc_send);
	console_cmd_add("ucrm", "usbhcdc re enum", 1, 1, do_usbcdc_reenum);
	
	ring_buffer_init(&usb_cdc_rbuff, CDC_R_Buffer, USBH_TBUFF_MAX);
	if (0 != rtos_mutex_recursive_create(&usbh_capture_ring_buffer_mutex)) 
	{
		dbg_test_print("ERR: capture_ring_buffer_mutex create fail!\r\n");
		return ;
	}

    /*##-5- Run Application (Blocking mode) ##################################*/
    while (1)
    {
#if (USBH_USE_OS == 1)
        unsigned int notification = rtos_task_wait_notification(-1);
        if (notification & APPLICATION_STATE_CHANGED) 
		{
			dbg_test_print("USBH_UserProcess sta:%d\r\n",Appli_state);
        }
		else if (notification & APPLICATION_STATE_EM) 
		{
			uint16_t nlen = usbh_cdc_data_pop(CDC_TX_Buffer);
			if(nlen)
			{
				co_timer_start(&co_usb_timer, (nlen/64+1)*100, NULL, co_usb_timer_handler, 0);
				USBH_CDC_Transmit(&hUSB_Host, CDC_TX_Buffer, nlen);
			}
			
		}
		else if (notification & APPLICATION_STATE_RM)
		{
			USBH_ReEnumerate(&hUSB_Host);
		}
#else
        /* USB Host Background task */
        USBH_Process(&hUSB_Host);
#endif
    }
}

static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch (id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
			dbg_test_print("USBH_UserProcess discon\r\n");
			Appli_state = APPLICATION_IDLE;   
			if (test_task_handle)
				rtos_task_notify_setbits(test_task_handle, APPLICATION_STATE_CHANGED, true);

            break;

        case HOST_USER_CLASS_ACTIVE:
			dbg_test_print("USBH_UserProcess class active\r\n");
			Appli_state = APPLICATION_RUNNING;
			if (test_task_handle)
				rtos_task_notify_setbits(test_task_handle, APPLICATION_STATE_EM, true);

			break;
		case HOST_USER_CONNECTION:
			dbg_test_print("USBH_UserProcess connect\r\n");
			Appli_state = APPLICATION_START;
			if (test_task_handle)
				rtos_task_notify_setbits(test_task_handle, APPLICATION_STATE_CHANGED, true);

			break;

        default:
            break;
    }


}

#endif // CFG_TEST_USB_HOST
