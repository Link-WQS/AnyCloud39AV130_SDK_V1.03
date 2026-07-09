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

/* Default write timeout in milliseconds */
#define RPMSG_WRITE_TIMEOUT_MS	3000

/* Default read timeout in milliseconds */
#define RPMSG_READ_TIMEOUT_MS	3000

#define dev_to_eptdev(dev) container_of(dev, struct rpmsg_eptdev, dev)
#define cdev_to_eptdev(i_cdev) container_of(i_cdev, struct rpmsg_eptdev, cdev)

/**
 * struct rpmsg_eptdev - endpoint device context
 * @dev:	endpoint device
 * @cdev:	cdev for the endpoint device
 * @rpdev:	underlaying rpmsg device
 * @chinfo:	info used to open the endpoint
 * @ept_lock:	synchronization of @ept modifications
 * @ept:	rpmsg endpoint reference, when open
 * @queue_lock:	synchronization of @queue operations
 * @queue:	incoming message queue
 * @readq:	wait object for incoming queue
 * @write_timeout_ms: write timeout in milliseconds (0 = use default)
 * @read_timeout_ms: read timeout in milliseconds (0 = use default)
 */
struct rpmsg_eptdev {
	struct device dev;
	struct cdev cdev;

	struct rpmsg_device *rpdev;
	u32 ctrl_id;
	u32 id;

	struct mutex ept_lock;
	struct rpmsg_endpoint *ept;

	spinlock_t queue_lock;
	struct sk_buff_head queue;
	wait_queue_head_t readq;

	u32 write_timeout_ms;
	u32 read_timeout_ms;
};

static int rpmsg_ept_cb(struct rpmsg_device *rpdev, void *buf, int len,
			void *priv, u32 addr)
{
	struct rpmsg_eptdev *eptdev = priv;
	struct sk_buff *skb;

	skb = alloc_skb(len, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	memcpy(skb_put(skb, len), buf, len);

	spin_lock(&eptdev->queue_lock);
	skb_queue_tail(&eptdev->queue, skb);
	spin_unlock(&eptdev->queue_lock);

	/* wake up any blocking processes, waiting for new data */
	wake_up_interruptible(&eptdev->readq);

	return 0;
}

static int rpmsg_eptdev_open(struct inode *inode, struct file *filp)
{
	struct rpmsg_eptdev *eptdev = cdev_to_eptdev(inode->i_cdev);
	struct device *dev = &eptdev->dev;

	get_device(dev);

	filp->private_data = eptdev;

	return 0;
}

static int rpmsg_eptdev_release(struct inode *inode, struct file *filp)
{
	struct rpmsg_eptdev *eptdev = cdev_to_eptdev(inode->i_cdev);
	struct device *dev = &eptdev->dev;

	put_device(dev);

	return 0;
}

static ssize_t rpmsg_eptdev_read(struct file *filp, char __user *buf,
				 size_t len, loff_t *f_pos)
{
	struct rpmsg_eptdev *eptdev = filp->private_data;
	unsigned long flags;
	struct sk_buff *skb;
	int use;
	u32 timeout_ms;
	long ret;

	if (!eptdev->ept)
		return -EPIPE;

	spin_lock_irqsave(&eptdev->queue_lock, flags);

	/* Wait for data in the queue */
	if (skb_queue_empty(&eptdev->queue)) {
		spin_unlock_irqrestore(&eptdev->queue_lock, flags);

		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		/*
		 * Blocking mode with timeout:
		 * Use wait_event_interruptible_timeout() to wait for data
		 * with a configurable timeout.
		 */
		timeout_ms = eptdev->read_timeout_ms ? 
			     eptdev->read_timeout_ms : RPMSG_READ_TIMEOUT_MS;

		ret = wait_event_interruptible_timeout(eptdev->readq,
					     !skb_queue_empty(&eptdev->queue) ||
					     !eptdev->ept,
					     msecs_to_jiffies(timeout_ms));

		/* Check for signal interrupt */
		if (ret < 0)
			return -ERESTARTSYS;

		/* Check for timeout (ret == 0) */
		if (ret == 0)
			return -ETIMEDOUT;

		/* We lost the endpoint while waiting */
		if (!eptdev->ept)
			return -EPIPE;

		spin_lock_irqsave(&eptdev->queue_lock, flags);
	}

	skb = skb_dequeue(&eptdev->queue);
	spin_unlock_irqrestore(&eptdev->queue_lock, flags);
	if (!skb)
		return -EFAULT;

	use = min_t(size_t, len, skb->len);
	if (copy_to_user(buf, skb->data, use))
		use = -EFAULT;

	kfree_skb(skb);

	return use;
}

static ssize_t rpmsg_eptdev_write(struct file *filp, const char __user *buf,
				  size_t len, loff_t *f_pos)
{
	struct rpmsg_eptdev *eptdev = filp->private_data;
	void *kbuf;
	int ret;
	unsigned long timeout;
	unsigned long start_jiffies;
	u32 timeout_ms;

	kbuf = memdup_user(buf, len);
	if (IS_ERR(kbuf)) {
		dev_info(&eptdev->dev, "rpmsg_eptdev_write: memdup_user failed\n");
		return PTR_ERR(kbuf);
	}

	if (mutex_lock_interruptible(&eptdev->ept_lock)) {
		dev_info(&eptdev->dev, "rpmsg_eptdev_write: mutex_lock_interruptible failed\n");
		ret = -ERESTARTSYS;
		goto free_kbuf;
	}

	if (!eptdev->ept) {
		dev_info(&eptdev->dev, "rpmsg_eptdev_write: ept is NULL\n");
		ret = -EPIPE;
		goto unlock_eptdev;
	}

	if (filp->f_flags & O_NONBLOCK) {
		/* Non-blocking mode: try to send immediately */
		ret = rpmsg_trysend(eptdev->ept, kbuf, len);
	} else {
		/*
		 * Blocking mode with timeout:
		 * Use rpmsg_trysend() in a loop with timeout.
		 * This allows us to have a configurable timeout instead of
		 * the hardcoded 15 seconds in rpmsg_send().
		 *
		 * Note: We use a simple retry mechanism with short sleeps
		 * because there's no dedicated wait queue for TX buffer
		 * availability. The rpmsg_poll() can be used to check if
		 * TX is possible (POLLOUT), but we need to poll periodically.
		 */
		//timeout_ms = eptdev->write_timeout_ms ? 
		//	     eptdev->write_timeout_ms : RPMSG_WRITE_TIMEOUT_MS;
		timeout_ms = RPMSG_WRITE_TIMEOUT_MS;
		start_jiffies = jiffies;
		timeout = msecs_to_jiffies(timeout_ms);

		while ((ret = rpmsg_trysend(eptdev->ept, kbuf, len)) == -ENOMEM) {
			/* No TX buffer available */
			/* Check if endpoint was closed */
			if (!eptdev->ept) {
				ret = -EPIPE;
				goto unlock_eptdev;
			}

			/* Check if timeout has elapsed */
			if (time_after(jiffies, start_jiffies + timeout)) {
				dev_err(&eptdev->dev, 
					"rpmsg_eptdev_write: timeout (%u ms) waiting for TX buffer\n",
					timeout_ms);
				ret = -ETIMEDOUT;
				goto unlock_eptdev;
			}

			/*
			 * Sleep for a short time before retrying.
			 * Use schedule_timeout_interruptible() to allow
			 * signal interruption.
			 */
			set_current_state(TASK_INTERRUPTIBLE);
			ret = schedule_timeout(msecs_to_jiffies(10));
			
			/* Check for signal interrupt */
			if (signal_pending(current)) {
				ret = -ERESTARTSYS;
				goto unlock_eptdev;
			}
		}
	}

unlock_eptdev:
	mutex_unlock(&eptdev->ept_lock);

free_kbuf:
	kfree(kbuf);
	return ret < 0 ? ret : len;
}

static unsigned int rpmsg_eptdev_poll(struct file *filp, poll_table *wait)
{
	struct rpmsg_eptdev *eptdev = filp->private_data;
	unsigned int mask = 0;

	if (!eptdev->ept)
		return POLLERR;

	poll_wait(filp, &eptdev->readq, wait);

	if (!skb_queue_empty(&eptdev->queue))
		mask |= POLLIN | POLLRDNORM;

	mask |= rpmsg_poll(eptdev->ept, filp, wait);

	return mask;
}

static const struct file_operations rpmsg_eptdev_fops = {
	.owner = THIS_MODULE,
	.open = rpmsg_eptdev_open,
	.release = rpmsg_eptdev_release,
	.read = rpmsg_eptdev_read,
	.write = rpmsg_eptdev_write,
	.poll = rpmsg_eptdev_poll,
};

static ssize_t name_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct rpmsg_eptdev *eptdev = dev_get_drvdata(dev);

	return sprintf(buf, "%s\n", dev_name(&eptdev->dev));
}
static DEVICE_ATTR_RO(name);

static ssize_t src_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct rpmsg_eptdev *eptdev = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", eptdev->rpdev->src);
}
static DEVICE_ATTR_RO(src);

static ssize_t dst_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct rpmsg_eptdev *eptdev = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", eptdev->rpdev->dst);
}
static DEVICE_ATTR_RO(dst);

static ssize_t write_timeout_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct rpmsg_eptdev *eptdev = dev_get_drvdata(dev);
	u32 timeout_ms = eptdev->write_timeout_ms ? 
			 eptdev->write_timeout_ms : RPMSG_WRITE_TIMEOUT_MS;

	return sprintf(buf, "%u\n", timeout_ms);
}

static ssize_t write_timeout_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct rpmsg_eptdev *eptdev = dev_get_drvdata(dev);
	u32 timeout_ms;
	int ret;

	ret = kstrtou32(buf, 10, &timeout_ms);
	if (ret)
		return ret;

	/* 0 means use default timeout */
	eptdev->write_timeout_ms = timeout_ms;

	return count;
}
static DEVICE_ATTR_RW(write_timeout);

static ssize_t read_timeout_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct rpmsg_eptdev *eptdev = dev_get_drvdata(dev);
	u32 timeout_ms = eptdev->read_timeout_ms ? 
			 eptdev->read_timeout_ms : RPMSG_READ_TIMEOUT_MS;

	return sprintf(buf, "%u\n", timeout_ms);
}

static ssize_t read_timeout_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct rpmsg_eptdev *eptdev = dev_get_drvdata(dev);
	u32 timeout_ms;
	int ret;

	ret = kstrtou32(buf, 10, &timeout_ms);
	if (ret)
		return ret;

	/* 0 means use default timeout */
	eptdev->read_timeout_ms = timeout_ms;

	return count;
}
static DEVICE_ATTR_RW(read_timeout);

static struct attribute *rpmsg_eptdev_attrs[] = {
	&dev_attr_name.attr,
	&dev_attr_src.attr,
	&dev_attr_dst.attr,
	&dev_attr_write_timeout.attr,
	&dev_attr_read_timeout.attr,
	NULL
};
ATTRIBUTE_GROUPS(rpmsg_eptdev);

static void rpmsg_eptdev_release_device(struct device *dev)
{
	struct rpmsg_eptdev *eptdev = dev_to_eptdev(dev);

	dev_dbg(&eptdev->dev, "release device rpmsg%d\n", eptdev->dev.devt);
	rpmsg_ctrldev_put_devt(eptdev->dev.devt);
	rpmsg_ctrldev_notify(eptdev->ctrl_id, eptdev->id);

	kfree(eptdev);
}

static int rpmsg_eptdev_create(struct rpmsg_eptdev *eptdev, int master, int id)
{
	struct device *dev;
	int ret;

	dev = &eptdev->dev;

	dev->devt = rpmsg_ctrldev_get_devt();
	dev->id = id;
	dev_set_name(dev, "rpmsg%d", id);

	ret = cdev_add(&eptdev->cdev, dev->devt, 1);
	if (ret)
		goto free_ept_devt;

	/* We can now rely on the release function for cleanup */
	dev->release = rpmsg_eptdev_release_device;

	ret = device_add(dev);
	if (ret) {
		dev_err(dev, "device_add failed: %d\n", ret);
		put_device(dev);
	}

	return ret;

free_ept_devt:
	rpmsg_ctrldev_put_devt(dev->devt);

	return ret;
}

#ifdef DEBUG
static void rpmsg_dump_msg(struct rpmsg_ctrl_msg *msg)
{
	pr_info("message info:\n");
	pr_info("\t name = %s\n", msg->name);
	pr_info("\t id = %d\n", msg->id);
	pr_info("\t ctrl_id = %d\n", msg->ctrl_id);
	pr_info("\t cmd = 0x%x\n", msg->cmd);
}
#endif

static int rpmsg_ept_temp_cb(struct rpmsg_device *rpdev, void *buf, int len,
			void *priv, u32 addr)
{
	struct rpmsg_eptdev *eptdev = priv;
	struct rpmsg_ctrl_msg *msg = buf;
	struct rpmsg_ctrl_msg_ack ack;

	dev_info(&rpdev->dev, "Rx len=%d\n", len);
#if 1
	if (len != sizeof(*msg))
		return 0;
#ifdef DEBUG
	rpmsg_dump_msg(msg);
#endif

	if (msg->cmd != RPMSG_CREATE_CLIENT) {
		dev_dbg(&rpdev->dev, "Invalid Data,Create rpmsg%d failed\n", msg->id);
		return 0;
	}

	rpdev->ept->cb = rpmsg_ept_cb;
	/* create device file */
	rpmsg_eptdev_create(eptdev, msg->ctrl_id, msg->id);
	/* send ack */
	ack.id = msg->id;
	ack.ack = RPMSG_ACK_OK;
	eptdev->ctrl_id = msg->ctrl_id;
	eptdev->id = msg->id;
	rpmsg_send(eptdev->rpdev->ept, &ack, sizeof(ack));
#endif
	return 0;
}

static int rpmsg_trans_probe(struct rpmsg_device *rpdev)
{
	struct rpmsg_eptdev *eptdev;
	struct device *dev;
	
	eptdev = kzalloc(sizeof(*eptdev), GFP_KERNEL);
	if (!eptdev)
		return -ENOMEM;

	dev = &eptdev->dev;

	/* init eptdev member */
	eptdev->rpdev = rpdev;
	eptdev->ept = rpdev->ept;
	mutex_init(&eptdev->ept_lock);
	spin_lock_init(&eptdev->queue_lock);
	skb_queue_head_init(&eptdev->queue);
	init_waitqueue_head(&eptdev->readq);
	cdev_init(&eptdev->cdev, &rpmsg_eptdev_fops);
	eptdev->cdev.owner = THIS_MODULE;

	/* init device */
	device_initialize(dev);
	dev->class = rpmsg_ctrldev_get_class();
	dev->groups = rpmsg_eptdev_groups;
	dev->release = NULL;

	dev_set_drvdata(&eptdev->dev, eptdev);
	dev_set_drvdata(&rpdev->dev, eptdev);

	eptdev->ept->priv = eptdev;
	rpdev->announce = rpdev->src != RPMSG_ADDR_ANY;

	return 0;
}

static void rpmsg_trans_remove(struct rpmsg_device *rpdev)
{
	struct rpmsg_eptdev *eptdev = dev_get_drvdata(&rpdev->dev);
	struct device *dev = &eptdev->dev;
	struct sk_buff *skb;

	/* Discard all SKBs */
	mutex_lock(&eptdev->ept_lock);
	while (!skb_queue_empty(&eptdev->queue)) {
		skb = skb_dequeue(&eptdev->queue);
		kfree_skb(skb);
	}
	mutex_unlock(&eptdev->ept_lock);

	eptdev->ept = NULL;

	wake_up_interruptible(&eptdev->readq);

	if (dev->release == NULL) {
		rpmsg_ctrldev_put_devt(eptdev->dev.devt);
		rpmsg_ctrldev_notify(eptdev->ctrl_id, eptdev->id);
		put_device(dev);
		kfree(eptdev);
	} else {
		device_del(dev);
		put_device(dev);
		cdev_del(&eptdev->cdev);
	}
}

static struct rpmsg_device_id rpmsg_driver_trans_id_table[] = {
	{ .name = "anyka,rpmsg_client0" },
	{ .name = "anyka,rpmsg_client1" },
	{ .name = "anyka,rpmsg_client2" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, rpmsg_driver_trans_id_table);

static struct rpmsg_driver rpmsg_chrtrans_driver = {
	.probe = rpmsg_trans_probe,
	.remove = rpmsg_trans_remove,
	.callback	= rpmsg_ept_temp_cb,
	.drv = {
		.name = "rpmsg_chr_trans",
	},
	.id_table = rpmsg_driver_trans_id_table,
};

module_rpmsg_driver(rpmsg_chrtrans_driver);

MODULE_ALIAS("rpmsg:rpmsg_chr_trans");
MODULE_LICENSE("GPL v2");
