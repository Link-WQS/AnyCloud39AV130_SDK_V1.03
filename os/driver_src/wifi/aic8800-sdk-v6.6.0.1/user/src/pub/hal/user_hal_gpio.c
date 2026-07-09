#include "user_hal_gpio.h"
#include "reg_gpio.h"

void gpio_init(PinName pn)
{
    uint8_t pin_type;
    uint8_t pin_idx;

    pin_type = PIN_TYPE(pn);
    pin_idx = PIN_IDX(pn);
    if (pin_type == PIN_TYPE_A) {
        gpioa_init(pin_idx);
    } else {
        gpiob_init(pin_idx);
    }
}

void gpio_deinit(PinName pn)
{
    uint8_t pin_type;
    uint8_t pin_idx;

    pin_type = PIN_TYPE(pn);
    pin_idx = PIN_IDX(pn);
    if (pin_type == PIN_TYPE_A) {
        gpioa_deinit(pin_idx);
    } else {
        gpiob_deinit(pin_idx);
    }
}

void gpio_dir_out(PinName pn)
{
    uint8_t pin_type;
    uint8_t pin_idx;

    pin_type = PIN_TYPE(pn);
    pin_idx = PIN_IDX(pn);
    if (pin_type == PIN_TYPE_A) {
        gpioa_dir_out(pin_idx);
    } else {
        gpiob_dir_out(pin_idx);
    }
}

void gpio_dir_in(PinName pn)
{
    uint8_t pin_type;
    uint8_t pin_idx;

    pin_type = PIN_TYPE(pn);
    pin_idx = PIN_IDX(pn);
    if (pin_type == PIN_TYPE_A) {
        gpioa_dir_in(pin_idx);
    } else {
        gpiob_dir_in(pin_idx);
    }
}

void gpio_set(PinName pn)
{
    uint8_t pin_type;
    uint8_t pin_idx;

    pin_type = PIN_TYPE(pn);
    pin_idx = PIN_IDX(pn);
    if (pin_type == PIN_TYPE_A) {
        gpioa_set(pin_idx);
    } else {
        gpiob_set(pin_idx);
    }
}

void gpio_clr(PinName pn)
{
    uint8_t pin_type;
    uint8_t pin_idx;

    pin_type = PIN_TYPE(pn);
    pin_idx = PIN_IDX(pn);
    if (pin_type == PIN_TYPE_A) {
        gpioa_clr(pin_idx);
    } else {
        gpiob_clr(pin_idx);
    }
}

int gpio_get(PinName pn)
{
    uint8_t pin_type;
    uint8_t pin_idx;

    pin_type = PIN_TYPE(pn);
    pin_idx = PIN_IDX(pn);
    if (pin_type == PIN_TYPE_A) {
        return gpioa_get(pin_idx);
    } else {
        return gpiob_get(pin_idx);
    }
}

void gpio_mask(PinName pn)
{
    uint8_t pin_type;
    uint8_t pin_idx;
    unsigned int gpmsk;

    pin_type = PIN_TYPE(pn);
    pin_idx = PIN_IDX(pn);
    gpmsk = 0x01UL << pin_idx;
    if (pin_type == PIN_TYPE_A) {
        AIC_GPIOA->MR |=  gpmsk;
    } else {
        AIC_GPIOB->MR |=  gpmsk;
    }
}

