/*
 * Remote processor machine-specific module for DA8XX
 *
 * Copyright (C) 2013 Texas Instruments, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <mach/map.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_reserved_mem.h>
#include <linux/dma-mapping.h>
#include <linux/remoteproc.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/delay.h>

#include "remoteproc_internal.h"

#define AK_HOST_SEND         0x164
#define AK_HOST_RECV         0x168

/*use l2 reversed mem*/
#define REMOTE_CPU_BOOT_ADDR_OFFSET  0x14FC

#define AHBHANT_VERSION   0x0
#define AHBHANT_CTRL      0x4 //set clear
#define AHBHANT_INTR_EN   0x8
#define AHBHANT_INTR_STA  0xc
#define AHBHANT_MODE      0x10
#define AHBHANT_MODE      0x10
#define AHBHANT_AREA4_L   0x38
#define AHBHANT_AREA4_H   0x3c

/*cpu ahbhant crtl bit*/
#define AHBHANT_CTRL_WORK      0x0
#define AHBHANT_CTRL_PAUSE     0x1
#define AHBHANT_CTRL_STOP      0x2
#define AHBHANT_CTRL_RUN       0x3
#define AHBHANT_CTRL_TRIG      0x5


/*cpu ahbhant interrupt enable bit*/
#define AHBHANT_INTR_WORK      0x0
#define AHBHANT_INTR_STOP      0x1
#define AHBHANT_INTR_PAUSE     0x2
#define AHBHANT_INTR_TRIG      0x3

#define RPROC_IOCTL_START       _IO('R', 1)
#define RPROC_IOCTL_SHUTDOWN    _IO('R', 2)
#define RPROC_IOCTL_GET_STATE   _IOR('R', 3, int)

struct ak_resource_map_table {
	u64 pa; /* Address of cpu's address */
	u64 da; /* Address of rproc's address */
	u32 len;
	void __iomem *va;
};

struct ak_rproc {
	struct rproc *rproc;
	const char *core_name;
	int mem_maps_cnt;
	struct ak_resource_map_table *mem_maps;
	void __iomem * va_b_ahbhant;   
    void __iomem * va_l_ahbhant;  
	struct workqueue_struct *workqueue;
	struct work_struct vq_work; 
	int irq;
};

static void trigger_intr_to_remote_cpu(struct rproc *rproc)
{
	struct ak_rproc *ak_rproc_data = rproc->priv;
    __raw_writel(1 << AHBHANT_CTRL_TRIG, ak_rproc_data->va_l_ahbhant + AHBHANT_CTRL);
}


static int ak_rproc_start(struct rproc *rproc)
{
	struct ak_rproc *ak_rproc_data = rproc->priv;
	uint32_t regval;

	/*set boot addr*/
	__raw_writel(rproc->bootaddr,AK_VA_L2MEM + REMOTE_CPU_BOOT_ADDR_OFFSET);
	/*set start flag*/
	__raw_writel(rproc->bootaddr,ak_rproc_data->va_l_ahbhant + AHBHANT_AREA4_H);

	/*clk open*/
	regval = __raw_readl(AK_VA_SYSCTRL + 0x28);
	regval &= ~(0x1 << 31);
	__raw_writel(regval, AK_VA_SYSCTRL + 0x28);
	udelay(10);
	/*reset release*/
	regval &= ~(0x1 << 30);
	__raw_writel(regval, AK_VA_SYSCTRL + 0x28);
	udelay(10);
	/*clear ahbhant stop*/
	__raw_writel(0x1 << AHBHANT_CTRL_RUN,ak_rproc_data->va_l_ahbhant + AHBHANT_CTRL);

	pr_info("%s start\n",ak_rproc_data->core_name);

	return 0;
}

static int ak_rproc_stop(struct rproc *rproc)
{
	struct ak_rproc *ak_rproc_data = rproc->priv;
	uint32_t regval;

	//clear boot addr
	__raw_writel(0,AK_VA_L2MEM + REMOTE_CPU_BOOT_ADDR_OFFSET);

	/*ahbhant stop*/
	__raw_writel(0x1 << AHBHANT_CTRL_STOP,ak_rproc_data->va_l_ahbhant + AHBHANT_CTRL);
	udelay(10);
	
	/*reset assert*/
	regval = __raw_readl(AK_VA_SYSCTRL + 0x28);
	regval |= (0x1 << 30);
	__raw_writel(regval, AK_VA_SYSCTRL + 0x28);
	udelay(10);
	/*clk close*/
	regval |= (0x1 << 31);
	__raw_writel(regval, AK_VA_SYSCTRL + 0x28);

	/*清除小核中断以及关闭使能，否者小核会一直有中断*/
	__raw_writel(0x3FFF,ak_rproc_data->va_l_ahbhant + AHBHANT_INTR_STA);
	__raw_writel(0x0,ak_rproc_data->va_l_ahbhant + AHBHANT_INTR_EN);
	
	pr_info("%s stop \n",ak_rproc_data->core_name);
	return 0;
}

/* kick a virtqueue */
static void ak_rproc_kick(struct rproc *rproc, int vqid)
{
	// dev_info(&rproc->dev, "vqid=%d kick\n", vqid);
	// __raw_writel(vqid,AK_VA_SYSCTRL + AK_HOST_SEND);
	//由于stop后会destory rpmsg bus,会触发rpmsg_sendto
	if (atomic_read(&rproc->power) > 0)
	{
		trigger_intr_to_remote_cpu(rproc);
	}	
}

static void *ak_da_to_va(struct rproc *rproc, u64 da, int len)
{
	struct device *dev = &rproc->dev;
	struct ak_rproc *ak_rproc_data = rproc->priv;
	const struct ak_resource_map_table *t;
	int i;
	void *va;

	for (i = 0; i < ak_rproc_data->mem_maps_cnt; i++) {
		t = &ak_rproc_data->mem_maps[i];

		if (da >= t->da && (da + len) <= (t->da + t->len)) {
			va = t->va + (da - t->da);
			return va;
		}
	}

	dev_err(dev, "failed da_to_va\n");
	return NULL;
}

static irqreturn_t ak_rproc_intr_handle(int irq, void* args)
{
	struct ak_rproc* ak_rproc_data = (struct ak_rproc*)args;
    uint32_t status;

    status = __raw_readl(ak_rproc_data->va_b_ahbhant + AHBHANT_INTR_STA);
    __raw_writel(status,ak_rproc_data->va_b_ahbhant + AHBHANT_INTR_STA);

	queue_work(ak_rproc_data->workqueue, &ak_rproc_data->vq_work);

	return IRQ_HANDLED;
}

static void ak_rproc_vq_work(struct work_struct *work)
{
	struct ak_rproc *ak_rproc_data = container_of(work, struct ak_rproc, vq_work);
	struct rproc  *rproc = ak_rproc_data->rproc;
	int notifyid = 0;

	// notifyid = __raw_readl(AK_VA_SYSCTRL + AK_HOST_RECV);
	// asm("MMU_Clean_Invalidate_Dcache:\n" "mrc  p15,0,r15,c7,c14,3\n"
    // "bne MMU_Clean_Invalidate_Dcache");
	if (rproc_vq_interrupt(rproc, notifyid) == IRQ_NONE)
	{
		dev_info(&rproc->dev,"no message found in vq%d\n",notifyid);
	}
}

static int ak_rproc_ahbhant_init(struct rproc *rproc)
{
	struct ak_rproc *ak_rproc_data = rproc->priv;

	/*enable interrupt from cpul*/
    __raw_writel(0x1 << AHBHANT_INTR_TRIG,ak_rproc_data->va_b_ahbhant + AHBHANT_INTR_EN);

	INIT_WORK(&ak_rproc_data->vq_work, ak_rproc_vq_work);

	return 0;
}

static int ak_rproc_parse_dt(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rproc *rproc = platform_get_drvdata(pdev);
	struct ak_rproc *ak_rproc_data = rproc->priv;
	struct device_node *np = dev->of_node;
	struct resource* mem;
	u32 *map_array;
	int ret = 0;
	int i;

	ret = of_property_read_string(np, "core-name", &ak_rproc_data->core_name);
	if (ret < 0) {
		dev_err(dev, "fial to get core-name\n");
		return -EINVAL;
	}

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!mem)
        return -ENODEV;

    ak_rproc_data->va_b_ahbhant = devm_ioremap_resource(&pdev->dev, mem);

    pr_info("cpu big ahbhant start 0x%x to 0x%x\n",mem->start,mem->end);

    mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if (!mem)
        return -ENODEV;

    ak_rproc_data->va_l_ahbhant = devm_ioremap_resource(&pdev->dev, mem);

	pr_info("cpu little ahbhant start 0x%x to 0x%x\n",mem->start,mem->end);

	ak_rproc_data->irq = platform_get_irq(pdev, 0);

	ret = of_property_count_elems_of_size(np, "memory-mappings", sizeof(u32) * 3);
	if (ret < 0) {
		dev_err(dev, "fail to get memory-mappings\n");
		return -ENXIO;
	}

	if (ret < 1) {
		dev_err(dev, "Please define memory-mappings property in dts\n");
		return -EINVAL;
	}

	ak_rproc_data->mem_maps_cnt = ret;
	ak_rproc_data->mem_maps = devm_kcalloc(dev, ret,
				sizeof(struct ak_resource_map_table), GFP_KERNEL);
	if (!ak_rproc_data->mem_maps) {
		dev_err(dev, "fail to alloc memory-mappings\n");
		return -ENOMEM;
	}

	map_array = devm_kcalloc(dev, ret * 3, sizeof(u32), GFP_KERNEL);
	if (!map_array) {
		dev_err(dev, "fail to alloc map_array\n");
		return -ENOMEM;
	}
	
	ret = of_property_read_u32_array(np, "memory-mappings", map_array,
			ak_rproc_data->mem_maps_cnt * 3);
	if (ret < 0) {
		dev_err(dev, "fail to read memory-mappings\n");
		return -ENXIO;
	}
	
	for (i = 0; i < ak_rproc_data->mem_maps_cnt; i++) {
		ak_rproc_data->mem_maps[i].da = map_array[i * 3 + 0];
		ak_rproc_data->mem_maps[i].len = map_array[i * 3 + 1];
		ak_rproc_data->mem_maps[i].pa = map_array[i * 3 + 2];
		ak_rproc_data->mem_maps[i].va = devm_ioremap_wc(dev, ak_rproc_data->mem_maps[i].pa,
				ak_rproc_data->mem_maps[i].len);

		if (!PTR_ERR(ak_rproc_data->mem_maps[i].va)) {
			dev_err(dev, "ioremap failed %llx - %llx\n", ak_rproc_data->mem_maps[i].pa,
				ak_rproc_data->mem_maps[i].pa + ak_rproc_data->mem_maps[i].len - 1);
			ret = -ENOMEM;
			goto free_mem_maps;
		}

		dev_info(dev, "memory-mappings[%d]: da: 0x%llx, len: 0x%x, pa: 0x%llx va: 0x%p\n",
				i, ak_rproc_data->mem_maps[i].da, ak_rproc_data->mem_maps[i].len,
				ak_rproc_data->mem_maps[i].pa, ak_rproc_data->mem_maps[i].va);
	}
	devm_kfree(dev, map_array);

	return 0;

free_mem_maps:
	devm_kfree(dev, ak_rproc_data->mem_maps);

	return ret;
}

static int ak_rproc_register_mem(struct rproc *rproc)
{
	struct device *dev = rproc->dev.parent;
	struct device_node *np;
	struct resource r;
	void *va;
	u32 da;
	int len = 0,i = 0;
	int ret;

	while ((np = of_parse_phandle(dev->of_node, "memory-region", i))) {
		ret = of_address_to_resource(np, 0, &r);
		if (ret) {
			dev_err(dev, "Failed to get memory-region(ret=%d)\n", ret);
			return ret;
		}

		if (strcmp(np->name, "vdev0buffer") == 0) {
			/* Initial reserved memory resources */
			ret = of_reserved_mem_device_init_by_idx(dev, dev->of_node, i);
			dev_info(dev, "Register a shared buffer(start:0x%08x len:0x%x)\n",
							r.start, r.end - r.start + 1);
			if (ret) {
				dev_err(dev, "Failed to get shared-memory(ret=%d)\n", ret);
				return ret;
			}
			dma_set_coherent_mask(dev, 0xffffffff);
			va = NULL;
		} else {
			len = resource_size(&r);
			va = devm_ioremap_wc(dev, r.start, len);
			// va = memremap(r.start,len,MEMREMAP_WB);
			if (!PTR_ERR(va)) {
				dev_err(dev, "Fialed to remap memory-region\n");
				return -ENOMEM;
			}
		}

		da = r.start;

		rproc_add_mem_entry(rproc, np->name, r.start, da, va, len);

		dev_info(dev, "memory-region%d:%s 0x%08x [0x%08x - 0x%08x]\n", i,
						np->name, (uint32_t)va, r.start, r.end);

		i++;
	}

	return 0;
}

static void ak_rproc_unregister_mem(struct rproc *rproc)
{
	struct device *dev = rproc->dev.parent;

	rproc_clean_mem_entry(rproc);

	of_reserved_mem_device_release(dev);
}

static struct rproc_ops ak_rproc_ops = {
	.start = ak_rproc_start,
	.stop = ak_rproc_stop,
	.kick = ak_rproc_kick,
	.da_to_va = ak_da_to_va,
};

static int ak_rproc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct rproc *rproc;
	struct ak_rproc *ak_rproc_data;
	const char *fw_name;
	int ret;

	/* we need to read firmware name at first. */
	ret = of_property_read_string(np, "firmware-name", &fw_name);
	if (ret < 0) {
		dev_err(dev, "failed to get firmware-name\n");
		return -EINVAL;
	}

	rproc = rproc_alloc(dev, np->name, &ak_rproc_ops, fw_name, sizeof(*ak_rproc_data));
	if (!rproc)
		return -ENOMEM;

	rproc->has_iommu = false;  
	rproc->auto_boot = false;

	ak_rproc_data = rproc->priv;
	ak_rproc_data->rproc = rproc;

	platform_set_drvdata(pdev, rproc);

	ret = ak_rproc_parse_dt(pdev);
	if (ret)
		goto free_rproc;

	ret = ak_rproc_register_mem(rproc);
	if (ret < 0) {
		dev_err(dev, "Fialed to parser memory-region\n");
		goto free_rproc;
	}
	
	ak_rproc_data->workqueue = create_workqueue(dev_name(dev));
	if (!ak_rproc_data->workqueue) {
		dev_err(dev, "Cannot create workqueue\n");
		ret = -ENOMEM;
		goto free_rproc;
	}

	ak_rproc_ahbhant_init(rproc);
	
	if (ak_rproc_data->irq > 0) {
        ret = devm_request_irq(&pdev->dev,ak_rproc_data->irq, ak_rproc_intr_handle, 0, "rproc", ak_rproc_data);
        if (ret) {
            pr_err("failed to request npu interrupt.\n");
            ret = -ENOENT;
            return ret;
        }
    }

	ret = rproc_add(rproc);
	if (ret) {
		dev_err(dev, "rproc_add failed: %d\n", ret);
		goto destroy_workqueue;
	}

	return 0;

destroy_workqueue:
	destroy_workqueue(ak_rproc_data->workqueue);

free_rproc:
	rproc_put(rproc);

	return ret;
}

static int ak_rproc_remove(struct platform_device *pdev)
{
	struct rproc *rproc = platform_get_drvdata(pdev);
	struct ak_rproc *ak_rproc_data = (struct ak_rproc *)rproc->priv;

	if (atomic_read(&rproc->power) > 0)
		rproc_shutdown(rproc);

	ak_rproc_unregister_mem(rproc);

	rproc_del(rproc);
	
	destroy_workqueue(ak_rproc_data->workqueue);

	rproc_free(rproc);

	return 0;
}

static const struct of_device_id ak_rproc_match[] = {
    { .compatible = "anyka,ak3918av130-remoteproc"},
    { /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, ak_rproc_match);

static struct platform_driver ak_rproc_driver = {
	.probe = ak_rproc_probe,
	.remove = ak_rproc_remove,
	.driver = {
		.name = "ak-rproc",
		.of_match_table = ak_rproc_match,
	},
};

static int __init ak_rproc_init(void)
{
	int err;

	err = platform_driver_register(&ak_rproc_driver);
	if (err)
		return err;
	
	return 0;
}

static void __exit ak_rproc_exit(void)
{
	platform_driver_unregister(&ak_rproc_driver);
}
#ifdef CONFIG_ANYKA_FASTSYS
postcore_initcall(ak_rproc_init);
#else
module_init(ak_rproc_init);
#endif
module_exit(ak_rproc_exit);

MODULE_AUTHOR("Anyka Microelectronic");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ANYKA Remote Processor control driver");
MODULE_VERSION("1.0.00");

