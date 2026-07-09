
#define pr_fmt(fmt)    "%s: " fmt, __func__

#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/crc32.h>
#include <linux/remoteproc.h>

#include "remoteproc_internal.h"

#define RSC_TABLE_START_OFFSET   0x40
#define RSC_TABLE_END_OFFSET   0x44

#define IH_NMLEN		32	/* Image Name Length		*/
typedef struct image_header {
	__be32		ih_magic;	/* Image Header Magic Number	*/
	__be32		ih_hcrc;	/* Image Header CRC Checksum	*/
	__be32		ih_time;	/* Image Creation Timestamp	*/
	__be32		ih_size;	/* Image Data Size		*/
	__be32		ih_load;	/* Data	 Load  Address		*/
	__be32		ih_ep;		/* Entry Point Address		*/
	__be32		ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;

/**
 * rproc_bin_sanity_check() - Sanity Check uImage firmware image
 * @rproc: the remote processor handle
 * @fw: the uImage firmware image
 *
 * Make sure this fw image is sane.
 */
static int rproc_bin_sanity_check(struct rproc *rproc, const struct firmware *fw)
{
	image_header_t header;
	image_header_t *hdr = &header;
	const u8 *data;
	uint32_t hcrc = 0,dcrc = 0,ih_hcrc;

	memcpy(hdr, fw->data, sizeof(image_header_t));

	ih_hcrc = be32_to_cpu(hdr->ih_hcrc);
	hdr->ih_hcrc = 0;
	hcrc = crc32(0xffffffff, hdr, sizeof(image_header_t)) ^ 0xffffffff;
    if (hcrc != ih_hcrc) {
        pr_err("image head checksum err, sum[0x%x] ih_hcrc[0x%x] hdr_size[%d]\n",
         hcrc, ih_hcrc, sizeof(image_header_t));
        return -ENOEXEC;
    }

	data = fw->data + sizeof(image_header_t);
	dcrc = crc32(0xffffffff, data, be32_to_cpu(hdr->ih_size)) ^ 0xffffffff;
    if (dcrc != be32_to_cpu(hdr->ih_dcrc)) {
        pr_err("image data checksum err, sum[0x%x] ih_dcrc[0x%x] ih_size[%d]\n",
         dcrc, be32_to_cpu(hdr->ih_dcrc), be32_to_cpu(hdr->ih_size));
        return -ENOEXEC;
    }

	return 0;
}

static u32 rproc_bin_get_boot_addr(struct rproc *rproc, const struct firmware *fw)
{
	image_header_t *image = (image_header_t *)fw->data;
	
	return (u32)(be32_to_cpu(image->ih_ep));
}

/**
 * rproc_bin_load() - load firmware to memory
 * @rproc: remote processor which will be booted using uImage
 * @fw: the BIN firmware image
 */
static int rproc_bin_load(struct rproc *rproc, const struct firmware *fw)
{	
	struct device *dev = &rproc->dev;
	image_header_t *image = (image_header_t *)fw->data;
	const u8 *bin_data = (fw->data + sizeof(image_header_t));
	void *ptr;
	// uint32_t i = 0;

	ptr = rproc_da_to_va(rproc, be32_to_cpu(image->ih_ep), be32_to_cpu(image->ih_size));
	if (!ptr) {
		dev_err(dev, "bad image da 0x%x mem 0x%x\n", be32_to_cpu(image->ih_ep), be32_to_cpu(image->ih_size));
		return -EINVAL;
	}

	memcpy(ptr,bin_data,be32_to_cpu(image->ih_size));
	// printk("ptr:0x%x,bin_data:0x%x,size:0x%x\n",ptr,bin_data,be32_to_cpu(image->ih_size));

	// bin_data = ptr;
	// for(i=0x0;i<1024;i++)
	// {
	// 	if(i%16 == 0)
	// 	{
	// 		printk("\n");
	// 		printk("%x:",0x81400000 + i);
	// 	}
	// 	printk("%02X ",bin_data[i]);
	// }
	return 0;
}

/**
 * rproc_bin_find_rsc_table() - find the resource table
 * @rproc: the rproc handle
 * @fw: the BIN firmware image
 * @tablesz: place holder for providing back the table size
 */
static struct resource_table *rproc_bin_find_rsc_table(struct rproc *rproc, const struct firmware *fw,
			 int *tablesz)
{
	struct resource_table *table = NULL;
	image_header_t *image = (image_header_t *)fw->data;
	uint32_t rsc_table_start = 0,rsc_table_end = 0;
	const u8 *ptr = fw->data;

	rsc_table_start = *(volatile uint32_t *)(ptr + sizeof(image_header_t) + RSC_TABLE_START_OFFSET);
	rsc_table_end = *(volatile uint32_t *)(ptr + sizeof(image_header_t) + RSC_TABLE_END_OFFSET);
	// pr_err("rsc_table_start:0x%x,rsc_table_end:0x%x\n",
	// 	rsc_table_start+ sizeof(image_header_t),rsc_table_end+ sizeof(image_header_t));

	*tablesz = rsc_table_end - rsc_table_start;
	table = (struct resource_table *)(ptr + sizeof(image_header_t) + (rsc_table_start - be32_to_cpu(image->ih_ep)));

	return table;
}

/**
 * rproc_bin_find_loaded_rsc_table() - find the loaded resource table
 * @rproc: the rproc handle
 * @fw: the BIN firmware image
 */
static struct resource_table *
rproc_bin_find_loaded_rsc_table(struct rproc *rproc, const struct firmware *fw)
{
	uint32_t rsc_table_start = 0,rsc_table_end = 0;
	const void *ptr = fw->data;
	
	rsc_table_start = *(volatile uint32_t *)(ptr + sizeof(image_header_t) + RSC_TABLE_START_OFFSET);
	rsc_table_end = *(volatile uint32_t *)(ptr + sizeof(image_header_t) + RSC_TABLE_END_OFFSET);
	// pr_err("rsc_table_start:0x%x,rsc_table_end:0x%x\n",rsc_table_start,rsc_table_end);
	
	return rproc_da_to_va(rproc, rsc_table_start, rsc_table_end - rsc_table_start);
}

const struct rproc_fw_ops rproc_bin_fw_ops = {
	.load = rproc_bin_load,
	.find_rsc_table = rproc_bin_find_rsc_table,
	.find_loaded_rsc_table = rproc_bin_find_loaded_rsc_table,
	.sanity_check = rproc_bin_sanity_check,
	.get_boot_addr = rproc_bin_get_boot_addr
};
