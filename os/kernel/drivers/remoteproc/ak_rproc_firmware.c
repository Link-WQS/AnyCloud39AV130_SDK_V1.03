#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include <linux/remoteproc.h>
#include "remoteproc_internal.h"
#include "ak_rproc_firmware.h"

#define RSC_TABLE_START_OFFSET   0x40
#define RSC_TABLE_END_OFFSET   0x44

int ak_request_fw_from_mem(const struct firmware **fw,const char *name, struct device *dev,struct rproc *rproc)
{
	struct firmware *fw_p = NULL;
	struct rproc_mem_entry *mem;
	uint32_t rsc_table_start = 0,rsc_table_end = 0;

	fw_p = kmalloc(sizeof(struct firmware), GFP_KERNEL);
	if (!fw_p) {
		dev_err(dev, "failed to alloc Firmware\n");
		return -ENOMEM;
	}

	mem = rproc_find_carveout_by_name(rproc,"arm9l_dram");
	if(mem == NULL)
	{
		pr_err("can not find arm9l dram\n");
		return -ENOMEM;
	}

	rsc_table_start = *(volatile uint32_t *)(mem->va + RSC_TABLE_START_OFFSET);
	rsc_table_end = *(volatile uint32_t *)(mem->va + RSC_TABLE_END_OFFSET);
	// pr_err("rsc_table_start:0x%x,rsc_table_end:0x%x\n",rsc_table_start,rsc_table_end);

	fw_p->size = rsc_table_end - rsc_table_start;
	fw_p->priv = mem->va + (rsc_table_start - mem->da);
	fw_p->pages = NULL;
	fw_p->data = NULL;
	*fw = fw_p;

	return 0;
}


int ak_request_firmware(const struct firmware **fw, const char *name, struct device *dev, void *context)
{
	int ret;
	
	ret = ak_request_fw_from_mem(fw,name,dev,(struct rproc *)context);
	if (ret) {
		pr_err( "can not find in memory.\n");
		return ret;
	}
	return 0;
}

int ak_request_firmware_nowait(const char *name, struct device *device, gfp_t gfp, void *context,
	void (*cont)(const struct firmware *fw, void *context))
{
	const struct firmware *fw;
	int ret;

#ifdef CONFIG_ANYKA_FASTSYS
	ret = ak_request_firmware(&fw, name, device,context);
	if (!ret) {
		cont(fw, context);
		return 0;
	}
#endif

	// fw_work = kzalloc(sizeof(*fw_work), gfp);
	// if (!fw_work)
	// 	return -ENOMEM;

	// fw_work->name = kstrdup_const(name, gfp);
	// if (!fw_work->name) {
	// 	kfree(fw_work);
	// 	return -ENOMEM;
	// }
	// fw_work->device = device;
	// fw_work->context = context;
	// fw_work->cont = cont;

	// get_device(fw_work->device);
	// INIT_DELAYED_WORK(&fw_work->work, request_firmware_work_func);
	// schedule_delayed_work(&fw_work->work, 0);

	return -1;
}
EXPORT_SYMBOL(ak_request_firmware_nowait);