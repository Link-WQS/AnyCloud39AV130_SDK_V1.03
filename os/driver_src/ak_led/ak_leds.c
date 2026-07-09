/*
 * LEDs driver for GPIOs
 *
 * Copyright (C) 2007 8D Technologies inc.
 * Raphael Assenat <raph@8d.com>
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

static ssize_t ak_led_info_show_gpio_num(struct device *dev,
                struct device_attribute *attr,
                char *buf);

static ssize_t ak_led_info_show_gpio_status(struct device *dev,
                struct device_attribute *attr,
                char *buf);

// ATTRIBUTES: /sys/devices/platform/leds/leds/
// ***led/ak_led_info/gpio_status [ro] 
static DEVICE_ATTR(gpio_status, S_IWUSR | S_IRUGO,
           ak_led_info_show_gpio_status,
           NULL);

// ATTRIBUTES: /sys/devices/platform/leds/leds/
// ***led/ak_led_info/gpio_num [ro] 
static DEVICE_ATTR(gpio_num, S_IWUSR | S_IRUGO,
           ak_led_info_show_gpio_num,
           NULL);

static struct attribute *ak_leds_attrs[] = {
    &dev_attr_gpio_num.attr,
    &dev_attr_gpio_status.attr,
    NULL,
};

static struct attribute_group ak_leds_attr_group = {
    .name  = "ak_led_info",
    .attrs = ak_leds_attrs,
};

struct gpio_led_data {
    struct led_classdev cdev;
    struct gpio_desc *gpiod;
    struct work_struct work;
    u8 new_level;
    u8 can_sleep;
    u8 blinking;
    int (*platform_gpio_blink_set)(struct gpio_desc *desc, int state,
            unsigned long *delay_on, unsigned long *delay_off);
};

/**
*
*@brief: /sys/devices/platform/leds/leds/xxxled/ak_led_info/gpio_status [ro]
*@param[in] struct device *dev
*@param[in] struct device_attribute *attr
*@param[in] char *buf
*@return: ssize_t
*
**/
static ssize_t ak_led_info_show_gpio_status(struct device *dev,
                struct device_attribute *attr,
                char *buf)
{
    struct led_classdev  *cdev    = (struct led_classdev *)dev_get_drvdata(dev);
    struct gpio_led_data *led_dat = 
        container_of(cdev, struct gpio_led_data, cdev);

    sprintf(buf, "%d\n", 
                 gpiod_get_value(led_dat->gpiod)
                 );
    return strlen(buf);
}

/**
*
*@brief: /sys/devices/platform/leds/leds/xxxled/ak_led_info/gpio_num [ro]
*@param[in] struct device *dev
*@param[in] struct device_attribute *attr
*@param[in] char *buf
*@return: ssize_t
*
**/
static ssize_t ak_led_info_show_gpio_num(struct device *dev,
                struct device_attribute *attr,
                char *buf)
{
    struct led_classdev  *cdev    = (struct led_classdev *)dev_get_drvdata(dev);
    struct gpio_led_data *led_dat = 
        container_of(cdev, struct gpio_led_data, cdev);

    sprintf(buf, "%d\n", 
                 desc_to_gpio(led_dat->gpiod)
                 );
    return strlen(buf);
}

/**
*
*@brief: gpio_led_work
*@param[in] struct work_struct *work
*@return: void
*
**/
static void gpio_led_work(struct work_struct *work)
{
    struct gpio_led_data *led_dat =
        container_of(work, struct gpio_led_data, work);

    if (led_dat->blinking) {
        led_dat->platform_gpio_blink_set(led_dat->gpiod,
                    led_dat->new_level, NULL, NULL);
        led_dat->blinking = 0;
    } else
        gpiod_set_value_cansleep(led_dat->gpiod, led_dat->new_level);
}

/**
*
*@brief: gpio_led_set
*@param[in] struct led_classdev *led_cdev
*@param[in] enum led_brightness value
*@return: void
*
**/
static void gpio_led_set(struct led_classdev *led_cdev,
    enum led_brightness value)
{
    struct gpio_led_data *led_dat =
        container_of(led_cdev, struct gpio_led_data, cdev);
    int level;

    if (value == LED_OFF)
        level = 0;
    else
        level = 1;

    pr_info("gpio_led_set value=%d, level=%d\n", value, level);

    /* Setting GPIOs with I2C/etc requires a task context, and we don't
     * seem to have a reliable way to know if we're already in one; so
     * let's just assume the worst.
     */
    if (led_dat->can_sleep) {
        led_dat->new_level = level;
        schedule_work(&led_dat->work);
    } else {
        if (led_dat->blinking) {
            led_dat->platform_gpio_blink_set(led_dat->gpiod, level,
                             NULL, NULL);
            led_dat->blinking = 0;
        } else
            gpiod_set_value(led_dat->gpiod, level);
    }
}

static int gpio_blink_set(struct led_classdev *led_cdev,
    unsigned long *delay_on, unsigned long *delay_off)
{
    struct gpio_led_data *led_dat = 
        container_of(led_cdev, struct gpio_led_data, cdev);

    led_dat->blinking = 1;
    return led_dat->platform_gpio_blink_set(led_dat->gpiod, GPIO_LED_BLINK,
                        delay_on, delay_off);
}

/**
*
*@brief: create_gpio_led
*@param[in] const struct gpio_led *template
*@param[in] struct gpio_led_data *led_dat
*@param[in] struct device *parent
*@param[in] int (*blink_set)(struct gpio_desc *, int, unsigned long *,
             unsigned long *)
*@return: void
*
**/
static int create_gpio_led(const struct gpio_led *template,
    struct gpio_led_data *led_dat, struct device *parent,
    int (*blink_set)(struct gpio_desc *, int, unsigned long *,
             unsigned long *))
{
    int ret, state, error;

    led_dat->gpiod = template->gpiod;
    if (!led_dat->gpiod) {
        /*
         * This is the legacy code path for platform code that
         * still uses GPIO numbers. Ultimately we would like to get
         * rid of this block completely.
         */
        unsigned long flags = 0;

        /* skip leds that aren't available */
        if (!gpio_is_valid(template->gpio)) {
            dev_info(parent, "Skipping unavailable LED gpio %d (%s)\n",
                    template->gpio, template->name);
            return 0;
        }

        if (template->active_low)
            flags |= GPIOF_ACTIVE_LOW;

        ret = devm_gpio_request_one(parent, template->gpio, flags,
                        template->name);
        if (ret < 0)
            return ret;

        led_dat->gpiod = gpio_to_desc(template->gpio);
        if (!led_dat->gpiod)
            return -EINVAL;
    }

    led_dat->cdev.name = template->name;
    led_dat->cdev.default_trigger = template->default_trigger;
    led_dat->can_sleep = gpiod_cansleep(led_dat->gpiod);
    led_dat->blinking = 0;
    if (blink_set) {
        led_dat->platform_gpio_blink_set = blink_set;
        led_dat->cdev.blink_set = gpio_blink_set;
    }
    led_dat->cdev.brightness_set = gpio_led_set;
    if (template->default_state == LEDS_GPIO_DEFSTATE_KEEP)
        state = !!gpiod_get_value_cansleep(led_dat->gpiod);
    else
        state = (template->default_state == LEDS_GPIO_DEFSTATE_ON);
    led_dat->cdev.brightness = state ? LED_FULL : LED_OFF;
    if (!template->retain_state_suspended)
        led_dat->cdev.flags |= LED_CORE_SUSPENDRESUME;

#if !(defined(CONFIG_SYS_FAST_LAUNCH) && defined(CONFIG_MACH_AK3918AV130))
    ret = gpiod_direction_output(led_dat->gpiod, state);
    if (ret < 0)
        return ret;
#endif

    INIT_WORK(&led_dat->work, gpio_led_work);

    /* register classdev */
    ret = led_classdev_register(parent, &led_dat->cdev);
    if (ret < 0)
        return ret;

    error = sysfs_create_group(&led_dat->cdev.dev->kobj, &ak_leds_attr_group);
    if (error) {
        dev_err(led_dat->cdev.dev, "Unable to export ak_leds, error: %d\n",
            error);
    }

    return ret;
}

static void delete_gpio_led(struct gpio_led_data *led)
{
    if (ak_leds_attr_group.name) {
        if (kernfs_find_and_get(led->cdev.dev->kobj.sd,
            ak_leds_attr_group.name)) {
            sysfs_remove_group(&led->cdev.dev->kobj, &ak_leds_attr_group);
        }
    }

    led_classdev_unregister(&led->cdev);
    cancel_work_sync(&led->work);
}

struct gpio_leds_priv {
    int num_leds;
    struct gpio_led_data leds[];
};

static inline int sizeof_gpio_leds_priv(int num_leds)
{
    return sizeof(struct gpio_leds_priv) +
        (sizeof(struct gpio_led_data) * num_leds);
}

static struct gpio_leds_priv *gpio_leds_create(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct fwnode_handle *child;
    struct gpio_leds_priv *priv;
    int count, ret;
    struct device_node *np;

    count = device_get_child_node_count(dev);
    if (!count)
        return ERR_PTR(-ENODEV);

    priv = devm_kzalloc(dev, sizeof_gpio_leds_priv(count), GFP_KERNEL);
    if (!priv)
        return ERR_PTR(-ENOMEM);

    device_for_each_child_node(dev, child) {
        struct gpio_led led = {};
        const char *state = NULL;

        led.gpiod = devm_get_gpiod_from_child(dev, NULL, child);
        if (IS_ERR(led.gpiod)) {
            fwnode_handle_put(child);
            ret = PTR_ERR(led.gpiod);
            goto err;
        }

        np = to_of_node(child);

        if (fwnode_property_present(child, "label")) {
            fwnode_property_read_string(child, "label", &led.name);
        } else {
            if (IS_ENABLED(CONFIG_OF) && !led.name && np)
                led.name = np->name;
            if (!led.name) {
                ret = -EINVAL;
                goto err;
            }
        }
        fwnode_property_read_string(child, "linux,default-trigger",
                        &led.default_trigger);

        if (!fwnode_property_read_string(child, "default-state",
                         &state)) {
            if (!strcmp(state, "keep"))
                led.default_state = LEDS_GPIO_DEFSTATE_KEEP;
            else if (!strcmp(state, "on"))
                led.default_state = LEDS_GPIO_DEFSTATE_ON;
            else
                led.default_state = LEDS_GPIO_DEFSTATE_OFF;
        }

        if (fwnode_property_present(child, "retain-state-suspended"))
            led.retain_state_suspended = 1;

        ret = create_gpio_led(&led, &priv->leds[priv->num_leds],
                      dev, NULL);
        if (ret < 0) {
            fwnode_handle_put(child);
            goto err;
        }
        priv->num_leds++;
    }

    return priv;

err:
    for (count = priv->num_leds - 1; count >= 0; count--)
        delete_gpio_led(&priv->leds[count]);
    return ERR_PTR(ret);
}

static const struct of_device_id ak_led_match[] = {
    { .compatible = "anyka,ak37d-leds" },
    { .compatible = "anyka,ak39ev330-leds" },
    { .compatible = "anyka,ak37e-leds" },
    { .compatible = "anyka,ak3918av100-leds" },
    { .compatible = "anyka,ak3918av130-leds" },
    { .compatible = "anyka,ak3918ev300l-leds" },
    { .compatible = "anyka,km01a-leds" },
    {}
};
MODULE_DEVICE_TABLE(of, ak_led_match);


static int ak_gpio_led_probe(struct platform_device *pdev)
{
    struct gpio_led_platform_data *pdata = dev_get_platdata(&pdev->dev);
    struct gpio_leds_priv *priv;
    int i, ret = 0;

    pr_info("ak_led_probe  enter...\n");

    if (pdata && pdata->num_leds) {
        priv = devm_kzalloc(&pdev->dev,
                sizeof_gpio_leds_priv(pdata->num_leds),
                    GFP_KERNEL);
        if (!priv)
            return -ENOMEM;

        priv->num_leds = pdata->num_leds;
        for (i = 0; i < priv->num_leds; i++) {
            ret = create_gpio_led(&pdata->leds[i],
                          &priv->leds[i],
                          &pdev->dev, pdata->gpio_blink_set);
            if (ret < 0) {
                /* On failure: unwind the led creations */
                for (i = i - 1; i >= 0; i--)
                    delete_gpio_led(&priv->leds[i]);
                return ret;
            }
        }
    } else {
        priv = gpio_leds_create(pdev);
        if (IS_ERR(priv))
            return PTR_ERR(priv);
    }

    platform_set_drvdata(pdev, priv);

    pr_info("ak_led_probe  exit...\n");
    return 0;
}

static int ak_gpio_led_remove(struct platform_device *pdev)
{
    struct gpio_leds_priv *priv = platform_get_drvdata(pdev);
    int i;

    for (i = 0; i < priv->num_leds; i++)
        delete_gpio_led(&priv->leds[i]);

    return 0;
}

static void ak_gpio_led_shutdown(struct platform_device *pdev)
{
    struct gpio_leds_priv *priv = platform_get_drvdata(pdev);
    int i;

    for (i = 0; i < priv->num_leds; i++) {
        struct gpio_led_data *led = &priv->leds[i];

        gpio_led_set(&led->cdev, LED_OFF);
    }
}

#ifdef CONFIG_PM
/*
* !!NOTICE:DONOT care about the led status when suspend and resume
* The led-class will take charge of this moment.
* Please refer to led_suspend and led_resume@kernel/drivers/leds/led-class.c
*/
static int ak_gpio_led_suspend(struct platform_device *pdev, pm_message_t state)
{
    pr_info("%s\n", __func__);

    pinctrl_pm_select_sleep_state(&pdev->dev);

    return 0;
}

static int ak_gpio_led_resume(struct platform_device *pdev)
{
    pr_info("%s\n", __func__);

    pinctrl_pm_select_default_state(&pdev->dev);

    return 0;
}
#else
#define ak_gpio_led_suspend NULL
#define ak_gpio_led_resume NULL
#endif

static struct platform_driver ak_gpio_led_driver = {
    .probe      = ak_gpio_led_probe,
    .remove     = ak_gpio_led_remove,
    .suspend    = ak_gpio_led_suspend,
    .resume     = ak_gpio_led_resume,
    .shutdown   = ak_gpio_led_shutdown,
    .driver     = {
        .name   = "ak_led",
        .of_match_table = of_match_ptr(ak_led_match),
        .owner      = THIS_MODULE,
    },
};

static int __init ak_led_init(void)
{
    return platform_driver_register(&ak_gpio_led_driver);
}

static void __exit ak_led_exit(void)
{
    platform_driver_unregister(&ak_gpio_led_driver);
}

module_init(ak_led_init);
module_exit(ak_led_exit);

MODULE_AUTHOR("Anyka Microelectronic Ltd.");
MODULE_DESCRIPTION("GPIO LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:leds-gpio");
MODULE_VERSION("2.0.00");
