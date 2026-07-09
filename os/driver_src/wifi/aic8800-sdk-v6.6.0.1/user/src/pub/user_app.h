#ifndef _USER_APP_H_
#define _USER_APP_H_

#include "user_cfg.h"

void user_init(void);
void user_soc_power_on(void);
void user_soc_power_off(void);
int user_setup_wakeup_src(gpio_irq_handler_t handler);
void user_clear_wakeup_src(void);
int user_do_tcp_client_wakeup(void);

#endif
