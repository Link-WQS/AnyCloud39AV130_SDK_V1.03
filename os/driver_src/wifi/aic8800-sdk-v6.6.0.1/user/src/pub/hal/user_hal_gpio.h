#ifndef _USER_HAL_GPIO_H_
#define _USER_HAL_GPIO_H_

#include "pinname.h"
#include "gpio_api.h"

void gpio_init(PinName pn);
void gpio_deinit(PinName pn);
void gpio_dir_out(PinName pn);
void gpio_dir_in(PinName pn);
void gpio_set(PinName pn);
void gpio_clr(PinName pn);
int gpio_get(PinName pn);
 void gpio_mask(PinName pn);

#endif
