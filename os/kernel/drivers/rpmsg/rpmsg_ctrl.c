/*
 * anyka rpmsg clinet driver
 *
 * Copyright (C) 2025 Anyka(Guangzhou) Microelectronics Technology Co., Ltd.
 *
 * Author: Yang Xiao <xiao_yang@anyka.oa>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/idr.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/rpmsg.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <uapi/linux/rpmsg.h>

#include "rpmsg_internal.h"

#define WAIT_TIMEOUT	(msecs_to_jiffies(2000))

static dev_t rpmsg_major;
static struct class *rpmsg_class;

static DEFINE_MUTEX(g_list_lock);
static LIST_HEAD(g_rpmsg_ctrldev_list);

static DEFINE_IDA(rpmsg_ctrl_ida);
static DEFINE_IDA(rpmsg_ept_ida);
static DEFINE_IDA(rpmsg_minor_ida);

#define dev_to_ctrldev(dev) container_of(dev, struct rpmsg_ctrldev, dev)
#define cdev_to_ctrldev(i_cdev) container_of(i_cdev, struct rpmsg_ctrldev, cdev)

/**
 * struct rpmsg_ctrldev - control device for instantiating endpoint devices
 * @rpdev:	underlaying rpmsg device
 * @cdev:	cdev for the ctrl device
 * @dev:	device for the ctrl device
 */
struct rpmsg_ctrldev {
	struct rpmsg_device *rpdev;
	struct device dev;
	struct cdev cdev;
	struct list_head list;
	u32    alive:1;

	struct mutex lock;
	struct list_head epts;
	struct list_head wait;
};

struct rpmsg_ept_notify {
	struct rpmsg_ctrldev *ctrl;
	char name[32];
	int id;
	struct list_head list;
	struct list_head notify;
	struct completion complete;
	uint32_t status;
};

struct class *rpmsg_ctrldev_get_class(void)
{
	return rpmsg_class;
}
EXPORT_SYMBOL(rpmsg_ctrldev_get_class);

dev_t rpmsg_ctrldev_get_devt(void)
{
	int ret;
	ret = ida_simple_get(&rpmsg_minor_ida, 0, RPMSG_DEV_MAX, GFP_KERNEL);
	if (ret < 0)
		return ret;
	return MKDEV(MAJOR(rpmsg_major), ret);
}
EXPORT_SYMBOL(rpmsg_ctrldev_get_devt);

void rpmsg_ctrldev_put_devt(dev_t devt)
{
	ida_simple_remove(&rpmsg_minor_ida, MINOR(devt));
}
EXPORT_SYMBOL(rpmsg_ctrldev_put_devt);

static struct rpmsg_ctrldev *rpmsg_ctrldev_get_ctrl(int id)
{
	struct rpmsg_ctrldev *pos, *tmp;

	mutex_lock(&g_list_lock);
	list_for_each_entry_safe(pos, tmp, &g_rpmsg_ctrldev_list, list) {
		if (id == pos->dev.id) {
			mutex_unlock(&g_list_lock);
			return pos;
		}
	}
	mutex_unlock(&g_list_lock);

	return NULL;
}

static void rpmsg_print_ack_str(struct device *dev, uint32_t ack)
{
	switch (ack) {
	case RPMSG_ACK_FAILED:
		dev_err(dev, "Operation failed\r\n");
		break;
	case RPMSG_ACK_NOLISTEN:
		dev_err(dev, "Remote don't listen this name\r\n");
		break;
	case RPMSG_ACK_BUSY:
		dev_err(dev, "Remote is busy\r\n");
		break;
	case RPMSG_ACK_NOMEM:
		dev_err(dev, "Remote not enought memory\r\n");
		break;
	case RPMSG_ACK_NOENT:
		dev_err(dev, "Remote is full\r\n");
		break;
	}
}

static struct rpmsg_ept_notify *
rpmsg_ctrldev_notify_find(struct rpmsg_ctrldev *ctrldev, int id)
{
	struct rpmsg_ept_notify *pos, *tmp;

	mutex_lock(&ctrldev->lock);
	list_for_each_entry_safe(pos, tmp, &ctrldev->wait, notify) {
		if (id == pos->id) {
			mutex_unlock(&ctrldev->lock);
			return pos;
		}
	}
	mutex_unlock(&ctrldev->lock);

	return NULL;
}

static void rpmsg_ctrldev_notify_add(struct rpmsg_ctrldev *ctrldev,
				struct rpmsg_ept_notify *notify)
{
	mutex_lock(&ctrldev->lock);
	list_add(&notify->notify, &ctrldev->wait);
	mutex_unlock(&ctrldev->lock);
}

static void rpmsg_ctrldev_notify_del(struct rpmsg_ctrldev *ctrldev,
				struct rpmsg_ept_notify *notify)
{
	mutex_lock(&ctrldev->lock);
	list_del(&notify->notify);
	mutex_unlock(&ctrldev->lock);
}

static struct rpmsg_ept_notify *
rpmsg_ctrldev_epts_find(struct rpmsg_ctrldev *ctrldev, int id)
{
	struct rpmsg_ept_notify *pos, *tmp;

	mutex_lock(&ctrldev->lock);
	list_for_each_entry_safe(pos, tmp, &ctrldev->epts, list) {
		if (id == pos->id) {
			mutex_unlock(&ctrldev->lock);
			return pos;
		}
	}
	mutex_unlock(&ctrldev->lock);

	return NULL;
}

static void rpmsg_ctrldev_epts_add(struct rpmsg_ctrldev *ctrldev,
				struct rpmsg_ept_notify *notify)
{
	mutex_lock(&ctrldev->lock);
	list_add(&notify->list, &ctrldev->epts);
	mutex_unlock(&ctrldev->lock);
}

static void rpmsg_ctrldev_epts_del(struct rpmsg_ctrldev *ctrldev,
				struct rpmsg_ept_notify *notify)
{
	mutex_lock(&ctrldev->lock);
	list_del(&notify->list);
	mutex_unlock(&ctrldev->lock);
}

int rpmsg_ctrldev_notify(int ctrl_id, int id)
{
	struct rpmsg_ctrldev *ctrldev;
	struct rpmsg_ept_notify *notify;

	ctrldev = rpmsg_ctrldev_get_ctrl(ctrl_id);
	if (!ctrldev) {
		/* 
		 * ctrldev may have been removed during remoteproc shutdown.
		 * The ida was already cleaned up in rpmsg_ctrldev_remove.
		 */
		pr_debug("/dev/rpmsg_ctrl%d dev not exits (may be removed during shutdown)\n", ctrl_id);
		return 0;
	}

	notify = rpmsg_ctrldev_epts_find(ctrldev, id);
	if (!notify) {
		/* 
		 * notify may have been cleaned up already in rpmsg_ctrldev_remove.
		 * The ida was already cleaned up there.
		 */
		dev_dbg(&ctrldev->dev, "/dev/rpmsg%d dev not exits (may be cleaned up already)\n", id);
		return 0;
	}

	rpmsg_ctrldev_epts_del(ctrldev, notify);

	devm_kfree(&notify->ctrl->dev, notify);

	ida_simple_remove(&rpmsg_ept_ida, id);

	return 0;
}
EXPORT_SYMBOL(rpmsg_ctrldev_notify);

static int rpmsg_eptdev_release(struct rpmsg_ctrldev *ctrldev, int id)
{
	int ret;
	struct rpmsg_ctrl_msg msg;
	struct rpmsg_ept_notify *notify;

	/* Check if rpdev and ept are valid */
	if (!ctrldev->rpdev || !ctrldev->rpdev->ept) {
		dev_err(&ctrldev->dev, "rpmsg_eptdev_release: rpmsg device or endpoint is not valid\n");
		return -ENODEV;
	}

	msg.ctrl_id = ctrldev->dev.id;
	msg.id = id;
	msg.cmd = RPMSG_CLOSE_CLIENT;

	notify = rpmsg_ctrldev_epts_find(ctrldev, id);
	if (!notify) {
		dev_err(&ctrldev->dev, "/dev/rpmsg%d dev not exits \n", id);
		return -ENODEV;
	}

	/* add to notify chain */
	rpmsg_ctrldev_notify_add(ctrldev, notify);

	strncpy(msg.name, notify->name, 32);
	/* tell remote close this endpoint */
	ret = rpmsg_trysend(ctrldev->rpdev->ept, &msg, sizeof(msg));
	if (ret)
		goto out;

	/* wait endpoint notify */
	ret = wait_for_completion_timeout(&notify->complete, WAIT_TIMEOUT);
	if (!ret) {
		dev_err(&ctrldev->dev, "close %s eptdev failed(timeout)\n", notify->name);
		ret = -ETIMEDOUT;
		goto out;
	}

	if (notify->status != RPMSG_ACK_OK) {
		rpmsg_print_ack_str(&ctrldev->dev, notify->status);
		ret = -EFAULT;
		goto out;
	}

	ret = 0;
	/* delete will complete by rpmsg_ctrldev_notify */

out:
	rpmsg_ctrldev_notify_del(ctrldev, notify);
	return ret;
}

static int rpmsg_eptdev_create(struct rpmsg_ctrldev *ctrldev, const char *name, int id)
{
	int ret;
	struct rpmsg_ctrl_msg msg;
	struct rpmsg_ept_notify *notify = NULL;

	/* Check if rpdev and ept are valid */
	if (!ctrldev->rpdev || !ctrldev->rpdev->ept) {
		dev_err(&ctrldev->dev, "rpmsg_eptdev_create: rpmsg device or endpoint is not valid\n");
		return -ENODEV;
	}

	notify = devm_kzalloc(&ctrldev->dev, sizeof(*notify), GFP_KERNEL);
	if (!notify)
		return -ENOMEM;

	notify->ctrl = ctrldev;
	notify->id = id;
	strncpy(notify->name, name, 32);
	init_completion(&notify->complete);

	/* fill message struct */
	strncpy(msg.name, name, 32);
	msg.ctrl_id = ctrldev->dev.id;
	msg.id = id;
	msg.cmd = RPMSG_CREATE_CLIENT;

	/* add ctrl to notify chain */
	rpmsg_ctrldev_notify_add(ctrldev, notify);

	/* tell remote to create a new endpoint */
	ret = rpmsg_send(ctrldev->rpdev->ept, &msg, sizeof(msg));
	if (ret) {
		dev_err(&ctrldev->dev, "send data failed\n");
		ret = -EFAULT;
		goto del_notify;
	}

	/* wait endpoint notify */
	notify->status = RPMSG_ACK_FAILED;
	ret = wait_for_completion_timeout(&notify->complete, WAIT_TIMEOUT);
	if (!ret) {
		dev_err(&ctrldev->dev, "create %s eptdev failed(timeout)\n", name);
		ret = -ETIMEDOUT;
		goto del_notify;
	}

	if (notify->status != RPMSG_ACK_OK) {
		rpmsg_print_ack_str(&ctrldev->dev, notify->status);
		ret = -EFAULT;
		goto del_notify;
	}

	rpmsg_ctrldev_notify_del(ctrldev, notify);

	/* success create client,add it to epts list */
	rpmsg_ctrldev_epts_add(ctrldev, notify);

	return 0;

del_notify:
	rpmsg_ctrldev_notify_del(ctrldev, notify);

	devm_kfree(&notify->ctrl->dev, notify);

	return ret;
}

static int rpmsg_ctrldev_open(struct inode *inode, struct file *filp)
{
	struct rpmsg_ctrldev *ctrldev = cdev_to_ctrldev(inode->i_cdev);

	get_device(&ctrldev->dev);
	filp->private_data = ctrldev;

	return 0;
}

static int rpmsg_ctrldev_release(struct inode *inode, struct file *filp)
{
	struct rpmsg_ctrldev *ctrldev = cdev_to_ctrldev(inode->i_cdev);

	put_device(&ctrldev->dev);

	return 0;
}

static long rpmsg_ctrldev_ioctl(struct file *fp, unsigned int cmd,
				unsigned long arg)
{
	int ret;
	struct rpmsg_ctrldev *ctrldev = fp->private_data;
	void __user *argp = (void __user *)arg;
	struct rpmsg_ept_info info;

	if (cmd != RPMSG_CREATE_EPT_IOCTL  && cmd != RPMSG_DESTROY_EPT_IOCTL)
		return -EINVAL;

	/* Check if rpdev is still valid before processing ioctl */
	mutex_lock(&ctrldev->lock);
	if (!ctrldev->rpdev) {
		mutex_unlock(&ctrldev->lock);
		dev_err(&ctrldev->dev, "rpmsg device has been removed\n");
		return -ENODEV;
	}
	mutex_unlock(&ctrldev->lock);

	if (copy_from_user(&info, argp, sizeof(info)))
		return -EFAULT;

	if (cmd == RPMSG_CREATE_EPT_IOCTL) {

		/* get unique id for ept */
		ret = ida_simple_get(&rpmsg_ept_ida, 0, 0, GFP_KERNEL);
		if (ret < 0) {
			dev_err(&ctrldev->dev, "ida_simple_get error\n");
			return -EFAULT;
		}
		info.id = ret;
		/* notify remote create endpoint */
		ret = rpmsg_eptdev_create(ctrldev, info.name, info.id);
		if (ret) {
			dev_err(&ctrldev->dev, "rpmsg_eptdev_create error\n");
			return ret;
		}

		if (copy_to_user(argp, &info, sizeof(info))) {
			dev_err(&ctrldev->dev, "copy_to_user error\n");
			return -EFAULT;
		}
		return 0;
	} else if (cmd == RPMSG_DESTROY_EPT_IOCTL) {
		dev_info(&ctrldev->dev, "close /dev/rpmsg%d endpoint\r\n", info.id);
		ret = rpmsg_eptdev_release(ctrldev, info.id);
		if (ret) {
			dev_err(&ctrldev->dev, "rpmsg_eptdev_release error\n");
			return ret;
		}
		return 0;
	}

	dev_warn(&ctrldev->dev, "Undown konw cmd=0x%x\r\n", cmd);

	return -EINVAL;
};

static const struct file_operations rpmsg_ctrldev_fops = {
	.owner = THIS_MODULE,
	.open = rpmsg_ctrldev_open,
	.release = rpmsg_ctrldev_release,
	.unlocked_ioctl = rpmsg_ctrldev_ioctl,
	.compat_ioctl = rpmsg_ctrldev_ioctl,
};

static int rpmsg_ctrldev_cb(struct rpmsg_device *rpdev, void *data, int len,
						void *priv, u32 src)
{
	struct rpmsg_ctrldev *ctrldev = dev_get_drvdata(&rpdev->dev);
	struct rpmsg_ctrl_msg_ack *cell = data;
	struct rpmsg_ept_notify *notify = NULL;

	if (len != sizeof(*cell)) {
		dev_err(&rpdev->dev, "Invalid len:expect:%d,Rx:%d\n", sizeof(*cell), len);
		return 0;
	}

	dev_dbg(&ctrldev->dev, "cb rpmsg%d ack=0x%x\n", cell->id, cell->ack);

	notify = rpmsg_ctrldev_notify_find(ctrldev, cell->id);
	if (!notify) {
		dev_err(&ctrldev->dev, "/dev/rpmsg%d dev not exits \n", cell->id);
		return -ENOENT;
	}

	notify->status = cell->ack;
	complete(&notify->complete);

	return 0;
}

static void rpmsg_ctrldev_release_device(struct device *dev)
{
	struct rpmsg_ctrldev *ctrldev = dev_to_ctrldev(dev);

	/*
	 * Note: rpmsg_ctrl_ida is already removed in rpmsg_ctrldev_remove
	 * to avoid id=0 being reused before device is fully released.
	 * So we don't remove it here again.
	 */
	rpmsg_ctrldev_put_devt(MINOR(dev->devt));

	kfree(ctrldev);
}

static ssize_t open_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;
	struct rpmsg_ctrldev *ctrl = dev_get_drvdata(dev);
	char name[32];

	if (count == 0)
		return count;

	memcpy(name, buf, count > 32 ? 32:count);
	if (name[count - 1] == '\n')
		name[count - 1] = '\0';
	else
		name[count] = '\0';

	/* get unique id for ept */
	ret = ida_simple_get(&rpmsg_ept_ida, 0, 0, GFP_KERNEL);
	if (ret < 0)
		return -EFAULT;

	dev_dbg(&ctrl->dev, "create /dev/rpmsg%d endpoint\r\n", ret);

	/* notify remote create endpoint */
	ret = rpmsg_eptdev_create(ctrl, name, ret);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR_WO(open);

static ssize_t close_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;
	struct rpmsg_ctrldev *ctrl = dev_get_drvdata(dev);
	int id;

	if (count == 0)
		return count;

	ret = sscanf(buf, "%d", &id);;

	if (ret != 1)
		return -EINVAL;

	if (id < 0 || id == -1)
		return -EINVAL;

	dev_dbg(&ctrl->dev, "close /dev/rpmsg%d endpoint\r\n", id);

	/* notify remote close endpoint */
	ret = rpmsg_eptdev_release(ctrl, id);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR_WO(close);


static int rpmsg_ctrldev_probe(struct rpmsg_device *rpdev)
{
	struct rpmsg_ctrldev *ctrldev;
	struct device *dev;
	int ret;

	ctrldev = kzalloc(sizeof(*ctrldev), GFP_KERNEL);
	if (!ctrldev)
		return -ENOMEM;

	ctrldev->rpdev = rpdev;

	dev = &ctrldev->dev;
	device_initialize(dev);
	dev->parent = &rpdev->dev;
	dev->class = rpmsg_class;
	INIT_LIST_HEAD(&ctrldev->epts);
	INIT_LIST_HEAD(&ctrldev->wait);
	mutex_init(&ctrldev->lock);

	cdev_init(&ctrldev->cdev, &rpmsg_ctrldev_fops);
	ctrldev->cdev.owner = THIS_MODULE;

	ret = rpmsg_ctrldev_get_devt();
	if (ret < 0)
		goto free_ctrldev;
	dev->devt = ret;

	ret = ida_simple_get(&rpmsg_ctrl_ida, 0, 0, GFP_KERNEL);
	if (ret < 0)
		goto free_minor_ida;
	dev->id = ret;
	dev_set_name(&ctrldev->dev, "rpmsg_ctrl%d", ret);

	ret = cdev_add(&ctrldev->cdev, dev->devt, 1);
	if (ret)
		goto free_ctrl_ida;

	/* We can now rely on the release function for cleanup */
	dev->release = rpmsg_ctrldev_release_device;

	ret = device_add(dev);
	if (ret) {
		dev_err(&rpdev->dev, "device_add failed: %d\n", ret);
		put_device(dev);
	}

	device_create_file(&ctrldev->dev, &dev_attr_open);
	device_create_file(&ctrldev->dev, &dev_attr_close);

	dev_set_drvdata(&rpdev->dev, ctrldev);
	dev_set_drvdata(&ctrldev->dev, ctrldev);

	list_add(&ctrldev->list, &g_rpmsg_ctrldev_list);
	if (mutex_is_locked(&g_list_lock))
			mutex_unlock(&g_list_lock);

	/* wo need to announce the new ept to remote */
	rpdev->announce = rpdev->src != RPMSG_ADDR_ANY;

	return ret;

free_ctrl_ida:
	ida_simple_remove(&rpmsg_ctrl_ida, dev->id);
free_minor_ida:
	rpmsg_ctrldev_put_devt(MINOR(dev->devt));
free_ctrldev:
	put_device(dev);
	kfree(ctrldev);

	return ret;
}

static void rpmsg_ctrldev_remove(struct rpmsg_device *rpdev)
{
	struct rpmsg_ctrldev *ctrldev = dev_get_drvdata(&rpdev->dev);
	struct rpmsg_ept_notify *pos, *tmp;
	struct list_head cleanup_list;

	dev_dbg(&rpdev->dev, "%s is removed\n", dev_name(&rpdev->dev));

	/* Clear rpdev pointer to prevent use after free */
	mutex_lock(&ctrldev->lock);
	ctrldev->rpdev = NULL;
	mutex_unlock(&ctrldev->lock);

	/* Clean up any pending notifies in wait list */
	INIT_LIST_HEAD(&cleanup_list);
	mutex_lock(&ctrldev->lock);
	list_for_each_entry_safe(pos, tmp, &ctrldev->wait, notify) {
		list_del(&pos->notify);
		list_add(&pos->notify, &cleanup_list);
		/* Complete any waiting operations with error */
		pos->status = RPMSG_ACK_FAILED;
		complete(&pos->complete);
	}
	mutex_unlock(&ctrldev->lock);

	/* Clean up epts list and free notify structures */
	INIT_LIST_HEAD(&cleanup_list);
	mutex_lock(&ctrldev->lock);
	list_for_each_entry_safe(pos, tmp, &ctrldev->epts, list) {
		list_del(&pos->list);
		ida_simple_remove(&rpmsg_ept_ida, pos->id);
		devm_kfree(&ctrldev->dev, pos);
	}
	mutex_unlock(&ctrldev->lock);

	/* Remove from global list */
	list_del(&ctrldev->list);

	/* 
	 * Remove ida here instead of in release_device, because release_device
	 * may not be called immediately if device is still open.
	 */
	ida_simple_remove(&rpmsg_ctrl_ida, ctrldev->dev.id);

	device_del(&ctrldev->dev);
	put_device(&ctrldev->dev);
	cdev_del(&ctrldev->cdev);
}

static struct rpmsg_device_id rpmsg_driver_ctrldev_id_table[] = {
	{ .name = "anyka,rpmsg_ctrl" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, rpmsg_driver_crtldev_id_table);

static struct rpmsg_driver rpmsg_ctrldev_driver = {
	.probe = rpmsg_ctrldev_probe,
	.remove = rpmsg_ctrldev_remove,
	.callback	= rpmsg_ctrldev_cb,
	.drv = {
		.name = "anyka,rpmsg_ctrl",
	},
	.id_table = rpmsg_driver_ctrldev_id_table,
};

static int rpmsg_ctrldev_init(void)
{
	int ret;
	
	ret = alloc_chrdev_region(&rpmsg_major, 0, RPMSG_DEV_MAX, "rpmsg");
	if (ret < 0) {
		pr_err("rpmsg: failed to allocate char dev region\n");
		return ret;
	}

	rpmsg_class = class_create(THIS_MODULE, "rpmsg");
	if (IS_ERR(rpmsg_class)) {
		pr_err("failed to create rpmsg class\n");
		unregister_chrdev_region(rpmsg_major, RPMSG_DEV_MAX);
		return PTR_ERR(rpmsg_class);
	}

	ret = register_rpmsg_driver(&rpmsg_ctrldev_driver);
	if (ret < 0) {
		pr_err("rpmsgchr: failed to register rpmsg driver\n");
		class_destroy(rpmsg_class);
		unregister_chrdev_region(rpmsg_major, RPMSG_DEV_MAX);
	}

	return ret;
}
postcore_initcall(rpmsg_ctrldev_init);

static void rpmsg_ctrldev_exit(void)
{
	unregister_rpmsg_driver(&rpmsg_ctrldev_driver);
	class_destroy(rpmsg_class);
	unregister_chrdev_region(rpmsg_major, RPMSG_DEV_MAX);
}
module_exit(rpmsg_ctrldev_exit);

MODULE_ALIAS("rpmsg:rpmsg_ctrl");
MODULE_LICENSE("GPL v2");
