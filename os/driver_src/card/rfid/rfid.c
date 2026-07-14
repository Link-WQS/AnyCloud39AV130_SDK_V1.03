#include <linux/module.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <asm/current.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define diff_us(t1, t2) ((u64)t1.tv_sec - (u64)t2.tv_sec) * 1000000 + ((u64)t1.tv_usec - (u64)t2.tv_usec);

#define rfid_code_bit_value(data, index) ((data[(index) / 8] >> (7 - (index) % 8)) & 0x01)
/* 主设备号 */
static int major = 0;
/* 创建设备类的结构体 */
static struct class *rfid_class;
/* 记录设备树信息的结构体*/
static int rfid_gpio_index;

static struct task_struct *rfid_kthread;

static char rfid_data_code[16] = {0};
static int rfid_data_count = 0;

static char rfid_code_number[4] = {0};
static char rfid_code_number_ready = 0;

static DEFINE_MUTEX(rfid_code_mutex);

static ssize_t rfid_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{

	int len = 0;
	if (size < 4)
	{
		return 0;
	}

	len = sizeof(rfid_code_number);
	// printk("read rfid value len:%d,rfid_code_number_ready:%d\n", len, rfid_code_number_ready);
	mutex_lock(&rfid_code_mutex);
	if (rfid_code_number_ready == 0)
	{
		mutex_unlock(&rfid_code_mutex);
		return 0;
	}

	copy_to_user(buf, rfid_code_number, len);
	rfid_code_number_ready = 0;
	mutex_unlock(&rfid_code_mutex);
	return len;
}
/* 创建设备方法 */
static struct file_operations rfid_control_fops = {
	.owner = THIS_MODULE,
	.read = rfid_read,
};

static int rfid_code_leave_wait(int *level, int min, int middle, int max, struct timeval *pre_tv)
{

	u64 us = 0;
	struct timeval tv;

	int cur_level = gpio_get_value(rfid_gpio_index);
	while (gpio_get_value(rfid_gpio_index) == cur_level)
	{

		do_gettimeofday(&tv);
		us = diff_us(tv, (*pre_tv));

		if (us > max)
		{
			return -1;
		}
	}

	do_gettimeofday(&tv);
	us = diff_us(tv, (*pre_tv));
	*level = cur_level ? 0 : 1;
	if (us < min)
	{
		return -1;
	}
	else if ((us > min) && (us < middle))
	{
		return 0;
	}
	return 1;
}

static char rfid_code_jump_flg = 0;

static int rfid_code_read_proc(void)
{

	int i = 0;
	int reslut = 0;
	int level = 0;
	struct timeval tv;
	unsigned char value = 0;

	memset(rfid_data_code, 0, sizeof(rfid_data_code));
	rfid_data_count = 0;
	rfid_code_jump_flg = 0;
	do_gettimeofday(&tv);
	while (1)
	{

		reslut = rfid_code_leave_wait(&level, 125, 375, 625, &tv);
		do_gettimeofday(&tv);
		if ((reslut == 0) && (rfid_code_jump_flg == 0))
		{

			rfid_code_jump_flg = 1;
		}
		else if ((reslut == 1) || ((reslut == 0) && (rfid_code_jump_flg == 1)))
		{

			if (rfid_code_jump_flg == 1)
			{

				value = ((rfid_data_code[(rfid_data_count - 1) / 8] >> (7 - (rfid_data_count - 1) % 8)) & 0x01) ? 1 : 0;
			}
			else
			{
				value = level ? 0 : 1;
			}

			rfid_code_jump_flg = 0;
			rfid_data_code[rfid_data_count / 8] |= (value << (7 - rfid_data_count % 8));
			rfid_data_count++;
			if (rfid_data_count >= 128)
			{

				// printk("rfid_data_count (%d,%d)\n", rfid_data_count, reslut);
				return 1;
			}
		}
		else if (reslut < 0)
		{

			return -1;
		}
	}
	return 0;
}

static int rfid_code_data_check(void)
{

	int index = 0, i = 0, j = 0, n = 0;
	int value = 0;
	for (index = 0; index < 64; index++)
	{

		value = 0;
		for (i = 0; i < 9; i++)
		{

			value <<= 1;
			value |= rfid_code_bit_value(rfid_data_code, index + i);
			// printk("%d",rfid_code_bit_value(rfid_data_code,index + i));
		}

		if (value == 0x1FF)
		{
			index += 9;
			break;
		}
	}

	if (index == 64)
	{

		printk("check rfid data failed (%s,%d)\n", __func__, __LINE__);
		return -1;
	}

	for (i = index; i < (index + 45); i += 5)
	{

		value = 0;
		for (j = 0; j < 4; j++)
		{

			value += rfid_code_bit_value(rfid_data_code, i + j);
		}
		if ((value % 2) != rfid_code_bit_value(rfid_data_code, i + j))
		{

			printk("check rfid data failed (%s,%d)\n", __func__, __LINE__);
			return -1;
		}
	}

	for (i = index; i < (index + 4); i++)
	{

		value = 0;
		for (j = 0; j < 50; j += 5)
		{

			value += rfid_code_bit_value(rfid_data_code, i + j);
		}

		if ((value % 2) != rfid_code_bit_value(rfid_data_code, i + j))
		{

			printk("check rfid data failed (%s,%d)\n", __func__, __LINE__);
			return -1;
		}
	}

	if (rfid_code_bit_value(rfid_data_code, index + 54) != 0)
	{

		printk("stop failed \n");
		return -1;
	}
	n = 0;
	mutex_lock(&rfid_code_mutex);
	rfid_code_number_ready = 0;
	mutex_unlock(&rfid_code_mutex);
	memset(rfid_code_number, 0, sizeof(rfid_code_number));
	for (i = index + 10; i < (index + 50); i += 10)
	{

		value = 0;
		for (j = 0; j < 9; j++)
		{

			if (j == 4)
			{

				continue;
			}
			value <<= 1;
			value |= rfid_code_bit_value(rfid_data_code, i + j);
		}
		rfid_code_number[n++] = value;
		// printk("%02x ", value);
	}
	// printk("\n");

	mutex_lock(&rfid_code_mutex);
	rfid_code_number_ready = 1;
	mutex_unlock(&rfid_code_mutex);
	return 1;
}

static int k_rfid_task(void *data)
{

	int pre_level = gpio_get_value(rfid_gpio_index);
	int cur_level = pre_level;
	char det_start_flag = 0;

	printk("kernel rfid card task success! \n");
	while (1)
	{

		if (kthread_should_stop())
		{
			break;
		}

		if (det_start_flag == 0)
		{
			cur_level = gpio_get_value(rfid_gpio_index);
			if (cur_level == 0)
			{
				det_start_flag = 1;
				pre_level = cur_level;
			}
			else
			{
				msleep(100);
			}
		}
		else
		{

			if (rfid_code_read_proc() == 1)
			{

				// printk("check data \n");
				rfid_code_data_check();
			}
			gpio_set_value(rfid_gpio_index, 1);
			pre_level = gpio_get_value(rfid_gpio_index);
			det_start_flag = 0;
			msleep(10);
		}
	}
	return 0;
}

static int rfid_probe(struct platform_device *pdev)
{

	int irq;
	struct device_node *np = pdev->dev.of_node;
	printk("rfid probe success \n");
	rfid_gpio_index = of_get_named_gpio(np, "gpios", 0);
	if (rfid_gpio_index < 0)
	{

		printk("rfid gpio setting failed\n");
	}
	else
	{
		printk("rfid setting gpio:%d\n", rfid_gpio_index);
	}
	device_create(rfid_class, NULL, MKDEV(major, 0), NULL, "rfid_control");

	gpio_direction_output(rfid_gpio_index, 1);
	msleep(100);
	gpio_direction_input(rfid_gpio_index);

	mutex_init(&rfid_code_mutex);

	rfid_kthread = kthread_create(k_rfid_task, NULL, "rfid_kthread");
	if (IS_ERR(rfid_kthread))
	{
		printk("create rfid_kthread faield\n");
		return 0;
	}
	wake_up_process(rfid_kthread);
	return 0;
}

static int rfid_remove(struct platform_device *pdev)
{
	int irq;
	printk("rfid driver remove\n");
	kthread_stop(rfid_kthread);

	device_destroy(rfid_class, MKDEV(major, 0));
	return 0;
}

static const struct of_device_id rfid_board_control[] = {
	{.compatible = "rfid_control"},
	{}};

static struct platform_driver rfid_platform_drv = {

	.probe = rfid_probe,
	.remove = rfid_remove,
	.driver = {
		.name = "rfid_control",
		.of_match_table = rfid_board_control,
	},
};

static int __init rfid_drv_init(void)
{

	printk("rfid drv init start\n");

	// 注册主字符设备号
	major = register_chrdev(0, "rfid_control", &rfid_control_fops);

	// 创建设备类
	rfid_class = class_create(THIS_MODULE, "rfid_class");
	if (IS_ERR(rfid_class))
	{
		printk("rfid class create failed\n");
		unregister_chrdev(major, "rfid_control");
		return PTR_ERR(rfid_class);
	}

	return platform_driver_register(&rfid_platform_drv);
}

static void __exit rfid_drv_exit(void)
{

	platform_driver_unregister(&rfid_platform_drv);
	class_destroy(rfid_class);
	unregister_chrdev(major, "rfid_control");
}

/*
 * ko创建
 */
module_init(rfid_drv_init);
module_exit(rfid_drv_exit);

MODULE_DESCRIPTION("Anyka MAC driver");
MODULE_AUTHOR("Anyka Microelectronic Ltd.");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.10");
