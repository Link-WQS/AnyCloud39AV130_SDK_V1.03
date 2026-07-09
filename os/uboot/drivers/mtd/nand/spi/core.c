// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2017 Micron Technology, Inc.
 *
 * Authors:
 *	Peter Pan <peterpandong@micron.com>
 *	Boris Brezillon <boris.brezillon@bootlin.com>
 */

#define pr_fmt(fmt)	"spi-nand: " fmt

#ifndef __UBOOT__
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/spinand.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi-mem.h>
#else
#include <common.h>
#include <errno.h>
#include <spi.h>
#include <spi-mem.h>
#include <linux/mtd/spinand.h>
#endif

/* SPI NAND index visible in MTD names */
static int spi_nand_idx;

#if 1
#include "AK_types.h"
#include "ak_spi_nand.h"

extern void spi_nand_reset(void);
extern void spi_nand_rdid(T_U8 *data);
extern T_BOOL spi_nand_page_read(T_U32 page_addr, T_U8 *read_buf, T_U32 read_count, T_READ_MODE mode);
extern T_BOOL spi_nand_page_program(T_U32 page_addr, T_U8 *write_buf, T_U32 write_count, T_PROGRAM_MODE mode);




#endif

static void spinand_cache_op_adjust_colum(struct spinand_device *spinand,
					  const struct nand_page_io_req *req,
					  u16 *column)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int shift;

	if (nand->memorg.planes_per_lun < 2)
		return;

	/* The plane number is passed in MSB just above the column address */
	shift = fls(nand->memorg.pagesize);
	*column |= req->pos.plane << shift;
}

static int spinand_read_reg_op(struct spinand_device *spinand, u8 reg, u8 *val)
{
	struct spi_mem_op op = SPINAND_GET_FEATURE_OP(reg,
						      spinand->scratchbuf);
	int ret;
  //  printf("[%s %d]\n",__func__,__LINE__);

#if 0
	ret = spi_mem_exec_op(spinand->slave, &op);
	if (ret)
		return ret;

#endif


#if 0
    if(spi_nand_page_read(op.addr.val, (unsigned char *)op.data.buf.in, 2048 , op.cmd.buswidth) == AK_FALSE)
    {
        printf("Read failed.\n");
        while(1);
    }
    printf("[%s %d] success \n",__func__,__LINE__);
#endif

    if(reg == REG_CFG){
    	T_U8 feature_protection;
    	T_U8 features_feature;

    	//get feature feature
    	features_feature = spi_nand_get_features_feature();
    	//printf("feature_feature = 0x%x\n", features_feature);
        *spinand->scratchbuf = features_feature;
         //printf("[%s %d] 0x%x success\n",__func__,__LINE__,reg);
    }else if(reg == REG_STATUS){
        T_U8 status;
        status = spi_nand_get_features_status();
          *spinand->scratchbuf = status;
           //printf("[%s %d] 0x%x success\n",__func__,__LINE__,reg);
    }

	*val = *spinand->scratchbuf;
	return 0;
}


void print_spi_mem_op_data(const struct spi_mem_op *op)
{
    int i;
    
    if (op->data.dir == SPI_MEM_DATA_OUT && op->data.buf.out) {
        printf("SPI TX data (%u bytes): ", op->data.nbytes);
        for (i = 0; i < op->data.nbytes; i++) {
            if (i % 16 == 0)
                printf("\n%04x: ", i);
            printf("%02x ", ((u8 *)op->data.buf.out)[i]);
        }
        printf("\n");
    }
}

static int spinand_write_reg_op(struct spinand_device *spinand, u8 reg, u8 val)
{
	struct spi_mem_op op = SPINAND_SET_FEATURE_OP(reg,
						      spinand->scratchbuf);

	*spinand->scratchbuf = val;
    #if 0
	return spi_mem_exec_op(spinand->slave, &op);
    #else
    
//    printf("[%s %d]op.data.dir:%d,op.addr.val:0x%08x op.cmd.buswidth:%d op.data.nbytes:%d\n",__func__,__LINE__,
//    op.data.dir,op.addr.val ,op.cmd.buswidth,op.data.nbytes);
    //print_spi_mem_op_data(&op);
#if 0
    if( spi_nand_page_program(op.addr.val,(unsigned char *)op.data.buf.out, op.data.nbytes*2048, op.cmd.buswidth) == AK_FALSE)
		{
			printf("Program failed.\n");
            return 1;
		}

    
        printf("[%s %d] success \n",__func__,__LINE__);
#endif


      if(reg == REG_BLOCK_LOCK){
           spi_nand_set_features_protection(val);
         //  printf("[%s %d] 0x%x success\n",__func__,__LINE__,reg);
      }else if(reg == REG_CFG){
          spi_nand_set_features_feature(val);
//          printf("[%s %d] 0x%x success\n",__func__,__LINE__,reg);
      }

      return 0;



    #endif
}

static int spinand_read_status(struct spinand_device *spinand, u8 *status)
{
	return spinand_read_reg_op(spinand, REG_STATUS, status);
}

static int spinand_get_cfg(struct spinand_device *spinand, u8 *cfg)
{
	struct nand_device *nand = spinand_to_nand(spinand);

	if (WARN_ON(spinand->cur_target < 0 ||
		    spinand->cur_target >= nand->memorg.ntargets))
		return -EINVAL;
//     printf("[%s line:%d]spinand->cfg_cache[spinand->cur_target]:%d \n",__func__,__LINE__,spinand->cfg_cache[spinand->cur_target]);
	*cfg = spinand->cfg_cache[spinand->cur_target];
	return 0;
}

static int spinand_set_cfg(struct spinand_device *spinand, u8 cfg)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	int ret;

	if (WARN_ON(spinand->cur_target < 0 ||
		    spinand->cur_target >= nand->memorg.ntargets))
		return -EINVAL;
//    printf("[%s line:%d]spinand->cfg_cache[spinand->cur_target]:%d cfg:%d\n",__func__,__LINE__,spinand->cfg_cache[spinand->cur_target],cfg);

	if (spinand->cfg_cache[spinand->cur_target] == cfg)
		return 0;

	ret = spinand_write_reg_op(spinand, REG_CFG, cfg);
	if (ret)
		return ret;

	spinand->cfg_cache[spinand->cur_target] = cfg;
	return 0;
}

/**
 * spinand_upd_cfg() - Update the configuration register
 * @spinand: the spinand device
 * @mask: the mask encoding the bits to update in the config reg
 * @val: the new value to apply
 *
 * Update the configuration register.
 *
 * Return: 0 on success, a negative error code otherwise.
 */
int spinand_upd_cfg(struct spinand_device *spinand, u8 mask, u8 val)
{
	int ret;
	u8 cfg;

	ret = spinand_get_cfg(spinand, &cfg);
//     printf("[%s line:%d]ret:%d \n",__func__,__LINE__,ret);
	if (ret)
		return ret;

	cfg &= ~mask;
	cfg |= val;

	return spinand_set_cfg(spinand, cfg);
}

/**
 * spinand_select_target() - Select a specific NAND target/die
 * @spinand: the spinand device
 * @target: the target/die to select
 *
 * Select a new target/die. If chip only has one die, this function is a NOOP.
 *
 * Return: 0 on success, a negative error code otherwise.
 */
int spinand_select_target(struct spinand_device *spinand, unsigned int target)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	int ret;

	if (WARN_ON(target >= nand->memorg.ntargets))
		return -EINVAL;

	if (spinand->cur_target == target)
		return 0;

	if (nand->memorg.ntargets == 1) {
		spinand->cur_target = target;
		return 0;
	}

	ret = spinand->select_target(spinand, target);
	if (ret)
		return ret;

	spinand->cur_target = target;
	return 0;
}

static int spinand_init_cfg_cache(struct spinand_device *spinand)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	struct udevice *dev = spinand->slave->dev;
	unsigned int target;
	int ret;

	spinand->cfg_cache = devm_kzalloc(dev,
					  sizeof(*spinand->cfg_cache) *
					  nand->memorg.ntargets,
					  GFP_KERNEL);
	if (!spinand->cfg_cache)
		return -ENOMEM;

	for (target = 0; target < nand->memorg.ntargets; target++) {
		ret = spinand_select_target(spinand, target);
		if (ret)
			return ret;

		/*
		 * We use spinand_read_reg_op() instead of spinand_get_cfg()
		 * here to bypass the config cache.
		 */
//        printf("[%s line:%d]spinand->cfg_cache[target]:0x%x \n",__func__,__LINE__,spinand->cfg_cache[target]);

		ret = spinand_read_reg_op(spinand, REG_CFG,
					  &spinand->cfg_cache[target]);
        //  printf("[%s line:%d]spinand->cfg_cache[target]:0x%x \n",__func__,__LINE__,spinand->cfg_cache[target]);
		if (ret)
			return ret;
	}

	return 0;
}

static int spinand_init_quad_enable(struct spinand_device *spinand)
{
	bool enable = false;

	if (!(spinand->flags & SPINAND_HAS_QE_BIT))
		return 0;

	if (spinand->op_templates.read_cache->data.buswidth == 4 ||
	    spinand->op_templates.write_cache->data.buswidth == 4 ||
	    spinand->op_templates.update_cache->data.buswidth == 4)
		enable = true;

	return spinand_upd_cfg(spinand, CFG_QUAD_ENABLE,
			       enable ? CFG_QUAD_ENABLE : 0);
}

static int spinand_ecc_enable(struct spinand_device *spinand,
			      bool enable)
{
 //   printf("[%s line:%d] \n",__func__,__LINE__);

	return spinand_upd_cfg(spinand, CFG_ECC_ENABLE,
			       enable ? CFG_ECC_ENABLE : 0);
}

static int spinand_write_enable_op(struct spinand_device *spinand)
{
	struct spi_mem_op op = SPINAND_WR_EN_DIS_OP(true);
   
    printf("[%s %d]\n",__func__,__LINE__);
	return spi_mem_exec_op(spinand->slave, &op);
}


void hexdump_spi_data(const u8 *data, int size)
{
   // const u8 *data;
    unsigned int i, j;
    #if 0
    if (!op || op->data.dir != SPI_MEM_DATA_IN || 
        !op->data.buf.in || op->data.nbytes == 0) {
        return;
    }
    #endif
    //data = (const u8 *)op->data.buf.in;

           
    for (i = 0; i < size; i += 16) {
        /* 打印偏移地址 */
        printf("%08x: ", i);
        
        /* 打印十六进制值 */
        for (j = 0; j < 16; j++) {
            if (i + j <size) {
                printf("%02x ", data[i + j]);
            } else {
                printf("   ");  /* 填充空格 */
            }
            
            /* 在第8个字节后添加额外空格 */
            if (j == 7) {
                printf(" ");
            }
        }
        
        printf(" ");
        
        /* 打印ASCII字符 */
        for (j = 0; j < 16 && i + j < size; j++) {
            if (data[i + j] >= 32 && data[i + j] < 127) {
                printf("%c", data[i + j]);
            } else {
                printf(".");
            }
        }
        
        printf("\n");
    }
}


static int spinand_load_page_op(struct spinand_device *spinand,
				const struct nand_page_io_req *req)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int row = nanddev_pos_to_row(nand, &req->pos);
	struct spi_mem_op op = SPINAND_PAGE_READ_OP(row);



#if 0
    return spi_mem_exec_op(spinand->slave, &op);
#endif
#if 0

    //if(spi_nand_page_read(op.addr.val, (unsigned char *)op.data.buf.in, 2048 , 0) == AK_FALSE)
    if(spi_nand_page_read(0x0, (unsigned char *)op.data.buf.in, 2048 , 0) == AK_FALSE)
    {
        printf("Read failed.\n");
        while(1);
    }
    printf("[%s %d] success \n",__func__,__LINE__);
    hexdump_spi_data(op.data.buf.in,"read nand");
    return 0;
#endif

#if 0
    T_U8 test_buf[2048];

    //if(spi_nand_page_read(op.addr.val, (unsigned char *)op.data.buf.in, 2048 , 0) == AK_FALSE)
    if(spi_nand_page_read(op.addr.val, test_buf, 2048 , 0) == AK_FALSE)
    {
        printf("Read failed.\n");
        while(1);
    }
    printf("[%s %d] success \n",__func__,__LINE__);
    hexdump_spi_data(test_buf,"read nand");
#endif

#if 0
T_U8 test_buf[2048];

if(spi_nand_page_read(op.addr.val, req->databuf.in, 2048 , 0) == AK_FALSE)
//if(spi_nand_page_read(op.addr.val, test_buf, 2048 , 0) == AK_FALSE)
{
    printf("Read failed.\n");
    while(1);
}
printf("[%s %d] success \n",__func__,__LINE__);
hexdump_spi_data( req->databuf.in,"read nand");


#endif

#if 0
if(op.addr.val < 0x200 ){
    printf("[%s %d] op.addr.val:0x%x \n",__func__,__LINE__,op.addr.val);
}
#endif

#if 1
ak_spinand_load_page_op(op.addr.val);
return 0;

#endif


}

static int spinand_read_from_cache_op(struct spinand_device *spinand,
				      const struct nand_page_io_req *req)
{
	struct spi_mem_op op = *spinand->op_templates.read_cache;
	struct nand_device *nand = spinand_to_nand(spinand);
	struct mtd_info *mtd = nanddev_to_mtd(nand);
	struct nand_page_io_req adjreq = *req;
	unsigned int nbytes = 0;
	void *buf = NULL;
	u16 column = 0;
	int ret;

	if (req->datalen) {
		adjreq.datalen = nanddev_page_size(nand);
		adjreq.dataoffs = 0;
		adjreq.databuf.in = spinand->databuf;
		buf = spinand->databuf;
		nbytes = adjreq.datalen;
	}

	if (req->ooblen) {
		adjreq.ooblen = nanddev_per_page_oobsize(nand);
		adjreq.ooboffs = 0;
		adjreq.oobbuf.in = spinand->oobbuf;
		nbytes += nanddev_per_page_oobsize(nand);
		if (!buf) {
			buf = spinand->oobbuf;
			column = nanddev_page_size(nand);
		}
	}

	spinand_cache_op_adjust_colum(spinand, &adjreq, &column);
	op.addr.val = column;

	/*
	 * Some controllers are limited in term of max RX data size. In this
	 * case, just repeat the READ_CACHE operation after updating the
	 * column.
	 */

	while (nbytes) {
		op.data.buf.in = buf;
		op.data.nbytes = nbytes;

		ret = spi_mem_adjust_op_size(spinand->slave, &op);
		if (ret)
			return ret;

         #if 0 

		ret = spi_mem_exec_op(spinand->slave, &op);
		if (ret)
			return ret;

      #else
 
#if 0
      if(spi_nand_page_read(op.addr.val, (unsigned char *)op.data.buf.in, 2048 ,0) == AK_FALSE)
      //if(spi_nand_page_read(op.addr.val, req->databuf.in, 2048 ,0) == AK_FALSE)
      {
          printf("Read failed.\n");
          while(1);
      }
#endif
      if (req->ooblen) {
            T_U8 test_buf[2112];
              ak_spinand_read_from_cache_op(test_buf, 2112 ,0);
              //hexdump_spi_data(test_buf,2112);
             // printf("[%s %d]test_buf:0x%x 0x%x\n",__func__,__LINE__,*(test_buf+2048),*(test_buf+2049));

              /*判断是否是坏块*/
              if(*(test_buf+2048) !=0xff || *(test_buf+2049) !=0xff){
                 // printf("[%s %d]test_buf:0x%x 0x%x\n",__func__,__LINE__,*(test_buf+2048),*(test_buf+2049));
                //  printf(" bad eraseblock is:  %u\n", req->pos.eraseblock);
              }
              memcpy(spinand->oobbuf, test_buf+2048, req->ooblen);
              
        }
      if (req->datalen){
          T_U8 test_buf[2048];
            //printf("[%s %d] op.addr.val:0x%x \n",__func__,__LINE__,op.addr.val);
            ak_spinand_read_from_cache_op(test_buf, 2048 ,0);
             // if(op.addr.val ==0x140 ){
                 // printf("[%s %d] op.addr.val:0x%x \n",__func__,__LINE__,op.addr.val);
                 // hexdump_spi_data(test_buf,2048);
             // }

            
           // hexdump_spi_data(test_buf,2048);
            memcpy( spinand->databuf, test_buf, req->datalen);

      }


      #endif
        
		buf += op.data.nbytes;
		nbytes -= op.data.nbytes;
		op.addr.val += op.data.nbytes;
	}

	if (req->datalen)
		memcpy(req->databuf.in, spinand->databuf + req->dataoffs,
		       req->datalen);

	if (req->ooblen) {
		if (req->mode == MTD_OPS_AUTO_OOB)
			mtd_ooblayout_get_databytes(mtd, req->oobbuf.in,
						    spinand->oobbuf,
						    req->ooboffs,
						    req->ooblen);
		else
			memcpy(req->oobbuf.in, spinand->oobbuf + req->ooboffs,
			       req->ooblen);

    //   printf("[%s %d]req->oobbuf.in:0x%x 0x%x\n",__func__,__LINE__, ((u8 *)req->oobbuf.in)[0], ((u8 *)req->oobbuf.in)[1]);
	}

    

	return 0;
}

static int spinand_write_to_cache_op(struct spinand_device *spinand,
				     const struct nand_page_io_req *req)
{
	struct spi_mem_op op = *spinand->op_templates.write_cache;
	struct nand_device *nand = spinand_to_nand(spinand);
	struct mtd_info *mtd = nanddev_to_mtd(nand);
	struct nand_page_io_req adjreq = *req;
	unsigned int nbytes = 0;
	void *buf = NULL;
	u16 column = 0;
	int ret;

	memset(spinand->databuf, 0xff,
	       nanddev_page_size(nand) +
	       nanddev_per_page_oobsize(nand));

	if (req->datalen) {
		memcpy(spinand->databuf + req->dataoffs, req->databuf.out,
		       req->datalen);
		adjreq.dataoffs = 0;
		adjreq.datalen = nanddev_page_size(nand);
		adjreq.databuf.out = spinand->databuf;
		nbytes = adjreq.datalen;
		buf = spinand->databuf;
	}

	if (req->ooblen) {
		if (req->mode == MTD_OPS_AUTO_OOB)
			mtd_ooblayout_set_databytes(mtd, req->oobbuf.out,
						    spinand->oobbuf,
						    req->ooboffs,
						    req->ooblen);
		else
			memcpy(spinand->oobbuf + req->ooboffs, req->oobbuf.out,
			       req->ooblen);

		adjreq.ooblen = nanddev_per_page_oobsize(nand);
		adjreq.ooboffs = 0;
		nbytes += nanddev_per_page_oobsize(nand);
		if (!buf) {
			buf = spinand->oobbuf;
			column = nanddev_page_size(nand);
		}
	}

	spinand_cache_op_adjust_colum(spinand, &adjreq, &column);

	op = *spinand->op_templates.write_cache;
	op.addr.val = column;

	/*
	 * Some controllers are limited in term of max TX data size. In this
	 * case, split the operation into one LOAD CACHE and one or more
	 * LOAD RANDOM CACHE.
	 */
	while (nbytes) {
		op.data.buf.out = buf;
		op.data.nbytes = nbytes;

		ret = spi_mem_adjust_op_size(spinand->slave, &op);
		if (ret)
			return ret;

		ret = spi_mem_exec_op(spinand->slave, &op);
		if (ret)
			return ret;

		buf += op.data.nbytes;
		nbytes -= op.data.nbytes;
		op.addr.val += op.data.nbytes;

		/*
		 * We need to use the RANDOM LOAD CACHE operation if there's
		 * more than one iteration, because the LOAD operation resets
		 * the cache to 0xff.
		 */
		if (nbytes) {
			column = op.addr.val;
			op = *spinand->op_templates.update_cache;
			op.addr.val = column;
		}
	}

	return 0;
}

static int spinand_program_op(struct spinand_device *spinand,
			      const struct nand_page_io_req *req)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int row = nanddev_pos_to_row(nand, &req->pos);
	struct spi_mem_op op = SPINAND_PROG_EXEC_OP(row);
    printf("[%s %d]\n",__func__,__LINE__);

	return spi_mem_exec_op(spinand->slave, &op);
}

static int spinand_erase_op(struct spinand_device *spinand,
			    const struct nand_pos *pos)
{
	struct nand_device *nand = &spinand->base;
	unsigned int row = nanddev_pos_to_row(nand, pos);
	struct spi_mem_op op = SPINAND_BLK_ERASE_OP(row);
    printf("[%s %d]\n",__func__,__LINE__);

	return spi_mem_exec_op(spinand->slave, &op);
}

static int spinand_wait(struct spinand_device *spinand, u8 *s)
{
	unsigned long start, stop;
	u8 status;
	int ret;

	start = get_timer(0);
	stop = 400;
	do {
		ret = spinand_read_status(spinand, &status);
		if (ret)
			return ret;

		if (!(status & STATUS_BUSY))
			goto out;
	} while (get_timer(start) < stop);

	/*
	 * Extra read, just in case the STATUS_READY bit has changed
	 * since our last check
	 */
	ret = spinand_read_status(spinand, &status);
	if (ret)
		return ret;

out:
	if (s)
		*s = status;

	return status & STATUS_BUSY ? -ETIMEDOUT : 0;
}

static int spinand_read_id_op(struct spinand_device *spinand, u8 *buf)
{
	struct spi_mem_op op = SPINAND_READID_OP(1, spinand->scratchbuf,
						 SPINAND_MAX_ID_LEN);
	int ret;

   #if 0
	ret = spi_mem_exec_op(spinand->slave, &op);
    if (!ret)
        memcpy(buf, spinand->scratchbuf, SPINAND_MAX_ID_LEN);
   #else
     spi_nand_rdid(spinand->scratchbuf);
     memcpy(buf, spinand->scratchbuf, SPINAND_MAX_ID_LEN);

   #endif

	return ret;
}

static int spinand_reset_op(struct spinand_device *spinand)
{
	struct spi_mem_op op = SPINAND_RESET_OP;
	int ret;
//    printf("[%s %d]\n",__func__,__LINE__);
    #if 0
	ret = spi_mem_exec_op(spinand->slave, &op);
     if (ret)
		return ret;
    #else
    spi_nand_reset();
       return ret;
    #endif

    //printf("[%s %d]\n",__func__,__LINE__);

	return spinand_wait(spinand, NULL);
}

static int spinand_lock_block(struct spinand_device *spinand, u8 lock)
{
	return spinand_write_reg_op(spinand, REG_BLOCK_LOCK, lock);
}

static int spinand_check_ecc_status(struct spinand_device *spinand, u8 status)
{
	struct nand_device *nand = spinand_to_nand(spinand);

	if (spinand->eccinfo.get_status){
//        printf("[%s %d]\n",__func__,__LINE__);
		return spinand->eccinfo.get_status(spinand, status);

    }


	switch (status & STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case STATUS_ECC_HAS_BITFLIPS:
		/*
		 * We have no way to know exactly how many bitflips have been
		 * fixed, so let's return the maximum possible value so that
		 * wear-leveling layers move the data immediately.
		 */
		return nand->eccreq.strength;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	default:
		break;
	}

	return -EINVAL;
}

static int spinand_read_page(struct spinand_device *spinand,
			     const struct nand_page_io_req *req,
			     bool ecc_enabled)
{
	u8 status;
	int ret;
    int i;
    #if 0
    /* 打印位置信息 */
    printf("打印所在nand的位置信息 Position:\n");
    printf("  target: %u\n", req->pos.target);
    printf("  lun:    %u\n", req->pos.lun);
    printf("  plane:  %u\n", req->pos.plane);
    printf("  eraseblock:  %u\n", req->pos.eraseblock);
    printf("  page:   %u\n", req->pos.page);
    /* 打印数据区域信息 */
    printf("Data Area:\n");
    printf("  offset: %u\n", req->dataoffs);
    printf("  length: %u\n", req->datalen);

    /* 打印OOB区域信息 */
    printf("OOB Area:\n");
    printf("  offset: %u\n", req->ooboffs);
    printf("  length: %u\n", req->ooblen);

    /* 模式 */
    printf("Mode: 0x%x", req->mode);
    printf("\n");
#endif
#if 1

	ret = spinand_load_page_op(spinand, req);
	if (ret)
		return ret;


	ret = spinand_wait(spinand, &status);
	if (ret < 0)
		return ret;

	ret = spinand_read_from_cache_op(spinand, req);
	if (ret)
		return ret;

	if (!ecc_enabled)
		return 0;

	return spinand_check_ecc_status(spinand, status);
#else

struct nand_device *nand = spinand_to_nand(spinand);
unsigned int row = nanddev_pos_to_row(nand, &req->pos);
struct spi_mem_op op = SPINAND_PAGE_READ_OP(row);

/*这里的页地址对应CONFIG_ENV_OFFSET*/
printf("[%s %d]op.data.dir:%d,op.addr.val:0x%08x op.cmd.buswidth:%d op.data.nbytes:%d\n",__func__,__LINE__,
op.data.dir,op.addr.val ,op.cmd.buswidth,op.data.nbytes);

/*处理坏块*/
if(req->ooblen){
    T_U8 test_buf[2112];
    if(ak_oob_spi_nand_page_read(op.addr.val, test_buf, 2112 , 0) == AK_FALSE)
    //if(spi_nand_page_read(op.addr.val, test_buf, 2048 , 0) == AK_FALSE)
    {
        printf("Read failed.\n");
        while(1); 
    }
    hexdump_spi_data(test_buf,2112);

}else{
        if(spi_nand_page_read(op.addr.val, req->databuf.in, 2048 , 0) == AK_FALSE)
        //if(spi_nand_page_read(op.addr.val, test_buf, 2048 , 0) == AK_FALSE)
        {
            printf("Read failed.\n");
            while(1); 
        }
        printf("[%s %d] success \n",__func__,__LINE__);
        //hexdump_spi_data( req->databuf.in,"read nand");

}






#endif


    
}

static int spinand_write_page(struct spinand_device *spinand,
			      const struct nand_page_io_req *req)
{
	u8 status;
	int ret;

	ret = spinand_write_enable_op(spinand);
	if (ret)
		return ret;

	ret = spinand_write_to_cache_op(spinand, req);
	if (ret)
		return ret;

	ret = spinand_program_op(spinand, req);
	if (ret)
		return ret;

	__udelay(100);
	ret = spinand_wait(spinand, &status);
	if (!ret && (status & STATUS_PROG_FAILED))
		ret = -EIO;

	return ret;
}

static int spinand_mtd_read(struct mtd_info *mtd, loff_t from,
			    struct mtd_oob_ops *ops)
{
	struct spinand_device *spinand = mtd_to_spinand(mtd);
	struct nand_device *nand = mtd_to_nanddev(mtd);
	unsigned int max_bitflips = 0;
	struct nand_io_iter iter;
	bool enable_ecc = false;
	bool ecc_failed = false;
	int ret = 0;

	if (ops->mode != MTD_OPS_RAW && spinand->eccinfo.ooblayout)
		enable_ecc = true;

#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif

	nanddev_io_for_each_page(nand, from, ops, &iter) {
		ret = spinand_select_target(spinand, iter.req.pos.target);
		if (ret)
			break;

		ret = spinand_ecc_enable(spinand, enable_ecc);
		if (ret)
			break;
//        printf("[%s line:%d] \n",__func__,__LINE__);

		ret = spinand_read_page(spinand, &iter.req, enable_ecc);
		if (ret < 0 && ret != -EBADMSG)
			break;

		if (ret == -EBADMSG) {
			ecc_failed = true;
			mtd->ecc_stats.failed++;
			ret = 0;
		} else {
			mtd->ecc_stats.corrected += ret;
			max_bitflips = max_t(unsigned int, max_bitflips, ret);
		}

		ops->retlen += iter.req.datalen;
		ops->oobretlen += iter.req.ooblen;
	}

#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif
	if (ecc_failed && !ret)
		ret = -EBADMSG;

	return ret ? ret : max_bitflips;
}

static int spinand_mtd_write(struct mtd_info *mtd, loff_t to,
			     struct mtd_oob_ops *ops)
{
	struct spinand_device *spinand = mtd_to_spinand(mtd);
	struct nand_device *nand = mtd_to_nanddev(mtd);
	struct nand_io_iter iter;
	bool enable_ecc = false;
	int ret = 0;

	if (ops->mode != MTD_OPS_RAW && mtd->ooblayout)
		enable_ecc = true;

#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif

	nanddev_io_for_each_page(nand, to, ops, &iter) {
		ret = spinand_select_target(spinand, iter.req.pos.target);
		if (ret)
			break;

		ret = spinand_ecc_enable(spinand, enable_ecc);
		if (ret)
			break;

		ret = spinand_write_page(spinand, &iter.req);
		if (ret)
			break;

		ops->retlen += iter.req.datalen;
		ops->oobretlen += iter.req.ooblen;
	}

#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif

	return ret;
}

static bool spinand_isbad(struct nand_device *nand, const struct nand_pos *pos)
{
	struct spinand_device *spinand = nand_to_spinand(nand);
	u8 marker[2] = { };
	struct nand_page_io_req req = {
		.pos = *pos,
		.ooblen = sizeof(marker),
		.ooboffs = 0,
		.oobbuf.in = marker,
		.mode = MTD_OPS_RAW,
	};
	int ret;

	ret = spinand_select_target(spinand, pos->target);
	if (ret)
		return ret;
   // printf("[%s line:%d]\n",__func__,__LINE__);

	ret = spinand_ecc_enable(spinand, false);
	if (ret)
		return ret;
  //  printf("[%s line:%d]\n",__func__,__LINE__);
    //get feature protection
    //printf("feature_protection A0 = 0x%08x\n", spi_nand_get_features_protection());
    //get feature feature
    //printf("feature_feature B0 = 0x%08x\n", spi_nand_get_features_feature());


	ret = spinand_read_page(spinand, &req, false);
	if (ret)
		return ret;
  

	if (marker[0] != 0xff || marker[1] != 0xff){  
       // printf(" [%s line:%d]fail!!!  %d is bad blockmarker[0]:0x%x marker[1]:0x%x\n",__func__,__LINE__,pos->eraseblock,marker[0],marker[1]);
		return true;
    }


	return false;
}

static int spinand_mtd_block_isbad(struct mtd_info *mtd, loff_t offs)
{
	struct nand_device *nand = mtd_to_nanddev(mtd);
#ifndef __UBOOT__
	struct spinand_device *spinand = nand_to_spinand(nand);
#endif
	struct nand_pos pos;
	int ret;

	nanddev_offs_to_pos(nand, offs, &pos);
#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif
	ret = nanddev_isbad(nand, &pos);
#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif
	return ret;
}

static int spinand_markbad(struct nand_device *nand, const struct nand_pos *pos)
{
	struct spinand_device *spinand = nand_to_spinand(nand);
	u8 marker[2] = { };
	struct nand_page_io_req req = {
		.pos = *pos,
		.ooboffs = 0,
		.ooblen = sizeof(marker),
		.oobbuf.out = marker,
		.mode = MTD_OPS_RAW,
	};
	int ret;

	ret = spinand_select_target(spinand, pos->target);
	if (ret)
		return ret;

	return spinand_write_page(spinand, &req);
}

static int spinand_mtd_block_markbad(struct mtd_info *mtd, loff_t offs)
{
	struct nand_device *nand = mtd_to_nanddev(mtd);
#ifndef __UBOOT__
	struct spinand_device *spinand = nand_to_spinand(nand);
#endif
	struct nand_pos pos;
	int ret;

	nanddev_offs_to_pos(nand, offs, &pos);
#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif
	ret = nanddev_markbad(nand, &pos);
#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif
	return ret;
}

static int spinand_erase(struct nand_device *nand, const struct nand_pos *pos)
{
	struct spinand_device *spinand = nand_to_spinand(nand);
	u8 status;
	int ret;

	ret = spinand_select_target(spinand, pos->target);
	if (ret)
		return ret;

	ret = spinand_write_enable_op(spinand);
	if (ret)
		return ret;

	ret = spinand_erase_op(spinand, pos);
	if (ret)
		return ret;

	ret = spinand_wait(spinand, &status);
	if (!ret && (status & STATUS_ERASE_FAILED))
		ret = -EIO;

	return ret;
}

static int spinand_mtd_erase(struct mtd_info *mtd,
			     struct erase_info *einfo)
{
#ifndef __UBOOT__
	struct spinand_device *spinand = mtd_to_spinand(mtd);
#endif
	int ret;

#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif
	ret = nanddev_mtd_erase(mtd, einfo);
#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif

	return ret;
}

static int spinand_mtd_block_isreserved(struct mtd_info *mtd, loff_t offs)
{
#ifndef __UBOOT__
	struct spinand_device *spinand = mtd_to_spinand(mtd);
#endif
	struct nand_device *nand = mtd_to_nanddev(mtd);
	struct nand_pos pos;
	int ret;

	nanddev_offs_to_pos(nand, offs, &pos);
#ifndef __UBOOT__
	mutex_lock(&spinand->lock);
#endif
	ret = nanddev_isreserved(nand, &pos);
#ifndef __UBOOT__
	mutex_unlock(&spinand->lock);
#endif

	return ret;
}

const struct spi_mem_op *
spinand_find_supported_op(struct spinand_device *spinand,
			  const struct spi_mem_op *ops,
			  unsigned int nops)
{
	unsigned int i;

	for (i = 0; i < nops; i++) {
		if (spi_mem_supports_op(spinand->slave, &ops[i]))
			return &ops[i];
	}

	return NULL;
}

static const struct nand_ops spinand_ops = {
	.erase = spinand_erase,
	.markbad = spinand_markbad,
	.isbad = spinand_isbad,
};

static const struct spinand_manufacturer *spinand_manufacturers[] = {
	&gigadevice_spinand_manufacturer,
	&macronix_spinand_manufacturer,
	//&micron_spinand_manufacturer,//与xtx的XT26G02E的id相同暂时先注掉先,目前此款也不支持
	&winbond_spinand_manufacturer,
#ifdef CONFIG_ANYKA
	&foress_spinand_manufacturer,
	&fm_micro_spinand_manufacturer,
	&xtx_spinand_manufacturer,
	&heyangtek_spinand_manufacturer,
	&dosilicon_spinand_manufacturer,
#endif
};

static int spinand_manufacturer_detect(struct spinand_device *spinand)
{
	unsigned int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(spinand_manufacturers); i++) {
		ret = spinand_manufacturers[i]->ops->detect(spinand);
		if (ret > 0) {
			spinand->manufacturer = spinand_manufacturers[i];
			return 0;
		} else if (ret < 0) {
			return ret;
		}
	}

	return -ENOTSUPP;
}

static int spinand_manufacturer_init(struct spinand_device *spinand)
{
	if (spinand->manufacturer->ops->init)
		return spinand->manufacturer->ops->init(spinand);

	return 0;
}

static void spinand_manufacturer_cleanup(struct spinand_device *spinand)
{
	/* Release manufacturer private data */
	if (spinand->manufacturer->ops->cleanup)
		return spinand->manufacturer->ops->cleanup(spinand);
}

static const struct spi_mem_op *
spinand_select_op_variant(struct spinand_device *spinand,
			  const struct spinand_op_variants *variants)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int i;

	for (i = 0; i < variants->nops; i++) {
		struct spi_mem_op op = variants->ops[i];
		unsigned int nbytes;
		int ret;

		nbytes = nanddev_per_page_oobsize(nand) +
			 nanddev_page_size(nand);

		while (nbytes) {
			op.data.nbytes = nbytes;
			ret = spi_mem_adjust_op_size(spinand->slave, &op);
			if (ret)
				break;

			if (!spi_mem_supports_op(spinand->slave, &op))
				break;

			nbytes -= op.data.nbytes;
		}

		if (!nbytes)
			return &variants->ops[i];
	}

	return NULL;
}

/**
 * spinand_match_and_init() - Try to find a match between a device ID and an
 *			      entry in a spinand_info table
 * @spinand: SPI NAND object
 * @table: SPI NAND device description table
 * @table_size: size of the device description table
 *
 * Should be used by SPI NAND manufacturer drivers when they want to find a
 * match between a device ID retrieved through the READ_ID command and an
 * entry in the SPI NAND description table. If a match is found, the spinand
 * object will be initialized with information provided by the matching
 * spinand_info entry.
 *
 * Return: 0 on success, a negative error code otherwise.
 */
int spinand_match_and_init(struct spinand_device *spinand,
			   const struct spinand_info *table,
			   unsigned int table_size, u8 devid)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	unsigned int i;

	for (i = 0; i < table_size; i++) {
		const struct spinand_info *info = &table[i];
		const struct spi_mem_op *op;

		if (devid != info->devid)
			continue;

		nand->memorg = table[i].memorg;
		nand->eccreq = table[i].eccreq;
		spinand->eccinfo = table[i].eccinfo;
		spinand->flags = table[i].flags;
		spinand->select_target = table[i].select_target;

		op = spinand_select_op_variant(spinand,
					       info->op_variants.read_cache);
		if (!op)
			return -ENOTSUPP;

		spinand->op_templates.read_cache = op;

		op = spinand_select_op_variant(spinand,
					       info->op_variants.write_cache);
		if (!op)
			return -ENOTSUPP;

		spinand->op_templates.write_cache = op;

		op = spinand_select_op_variant(spinand,
					       info->op_variants.update_cache);
		spinand->op_templates.update_cache = op;

		return 0;
	}

	return -ENOTSUPP;
}

static int spinand_detect(struct spinand_device *spinand)
{
	struct nand_device *nand = spinand_to_nand(spinand);
	int ret;
//    printf("[%s %d]\n",__func__,__LINE__);

/*复位*/
    #if 1
	ret = spinand_reset_op(spinand);
	if (ret)
		return ret;
    #else
     spi_nand_reset();
    #endif
  //  printf("[%s %d]\n",__func__,__LINE__);
/*读id*/
       #if 1
	ret = spinand_read_id_op(spinand, spinand->id.data);
       	if (ret)
		return ret;
#else
       spi_nand_rdid(spinand->id.data);
#endif



	spinand->id.len = SPINAND_MAX_ID_LEN;
	printf("ID:%02x, %02x, %02x, %02x\r\n", spinand->id.data[0], spinand->id.data[1], spinand->id.data[2], spinand->id.data[3]);
	ret = spinand_manufacturer_detect(spinand);
	if (ret) {
		dev_err(dev, "unknown raw ID :%02x, %02x, %02x, %02x\n",
			spinand->id.data[0], spinand->id.data[1], spinand->id.data[2], spinand->id.data[3]);
		return ret;
	}
   // printf("[%s %d]\n",__func__,__LINE__);

	if (nand->memorg.ntargets > 1 && !spinand->select_target) {
		dev_err(dev,
			"SPI NANDs with more than one die must implement ->select_target()\n");
		return -EINVAL;
	}

	dev_err(spinand->slave->dev,
		 "%s SPI NAND was found.\n", spinand->manufacturer->name);
/*
     printf("[%s %d]nand->memorg.pagesize :%d nand->memorg.pages_per_eraseblock:%d  nand->memorg.oobsize:%d nand->memorg.luns_per_target:%d nand->memorg.eraseblocks_per_lun:%d nand->memorg.ntargets:%d\n",
     __func__,__LINE__,
     nand->memorg.pagesize , nand->memorg.pages_per_eraseblock,nand->memorg.oobsize,
     nand->memorg.luns_per_target,nand->memorg.eraseblocks_per_lun,nand->memorg.ntargets);

	dev_err(spinand->slave->dev,
		 "%llu MiB, block size: %zu KiB, page size: %zu, OOB size: %u\n",
		 nanddev_size(nand) >> 20, nanddev_eraseblock_size(nand) >> 10,
		 nanddev_page_size(nand), nanddev_per_page_oobsize(nand));
     printf("[%s %d]\n",__func__,__LINE__);
    */

	return 0;
}

static int spinand_noecc_ooblayout_ecc(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *region)
{
	return -ERANGE;
}

static int spinand_noecc_ooblayout_free(struct mtd_info *mtd, int section,
					struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 2 bytes for the BBM. */
	region->offset = 2;
	region->length = 62;

	return 0;
}

static const struct mtd_ooblayout_ops spinand_noecc_ooblayout = {
	.ecc = spinand_noecc_ooblayout_ecc,
	.free = spinand_noecc_ooblayout_free,
};

#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)

//extern void sfcv2_set_parm(unsigned int max_hz);
extern void  sfc_ctrl_master_init(unsigned long	 sfc_clk_freq);
#endif

static int spinand_init(struct spinand_device *spinand)
{
	struct mtd_info *mtd = spinand_to_mtd(spinand);
	struct nand_device *nand = mtd_to_nanddev(mtd);
	int ret, i;

//      printf("[%s %d]\n",__func__,__LINE__);
	/*
	 * We need a scratch buffer because the spi_mem interface requires that
	 * buf passed in spi_mem_op->data.buf be DMA-able.
	 */
	spinand->scratchbuf = kzalloc(SPINAND_MAX_ID_LEN, GFP_KERNEL);
	if (!spinand->scratchbuf)
		return -ENOMEM;
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)


    sfc_ctrl_master_init(50000000);
#endif

	ret = spinand_detect(spinand);

#if 0
//设置/获取功能 ecc & QE
T_U8 feature_protection;
T_U8 features_feature;


//get feature protection
feature_protection = spi_nand_get_features_protection();
printf("feature_protection A0 = 0x%08x\n", feature_protection);


//get feature feature
features_feature = spi_nand_get_features_feature();
printf("feature_feature B0 = 0x%08x\n", features_feature);


//set feature protection
spi_nand_set_features_protection(0);

//set feature feature   enable ecc & QE  ， WINBOND没有QE。
features_feature |= (0x1 | 0x1<<4);
spi_nand_set_features_feature(features_feature);

//get feature protection
printf("feature_protection A0 = 0x%08x\n", spi_nand_get_features_protection());
//get feature feature
printf("feature_feature B0 = 0x%08x\n", spi_nand_get_features_feature());

#endif



	if (ret)
		goto err_free_bufs;

   // printf("[%s %d]\n",__func__,__LINE__);

	/*
	 * Use kzalloc() instead of devm_kzalloc() here, because some drivers
	 * may use this buffer for DMA access.
	 * Memory allocated by devm_ does not guarantee DMA-safe alignment.
	 */
	spinand->databuf = kzalloc(nanddev_page_size(nand) +
			       nanddev_per_page_oobsize(nand),
			       GFP_KERNEL);
	if (!spinand->databuf) {
		ret = -ENOMEM;
		goto err_free_bufs;
	}

	spinand->oobbuf = spinand->databuf + nanddev_page_size(nand);

   // printf("[%s %d]\n",__func__,__LINE__);

	ret = spinand_init_cfg_cache(spinand);
	if (ret)
		goto err_free_bufs;
//printf("[%s %d]\n",__func__,__LINE__);

	ret = spinand_init_quad_enable(spinand);
	if (ret)
		goto err_free_bufs;
   // printf("[%s %d]\n",__func__,__LINE__);
#if 1
	ret = spinand_upd_cfg(spinand, CFG_OTP_ENABLE, 0);
	if (ret)
		goto err_free_bufs;
#endif
  //  printf("[%s %d]\n",__func__,__LINE__);


	ret = spinand_manufacturer_init(spinand);
	if (ret) {
		dev_err(dev,
			"Failed to initialize the SPI NAND chip (err = %d)\n",
			ret);
		goto err_free_bufs;
	}
    //printf("[%s %d]\n",__func__,__LINE__);


	/* After power up, all blocks are locked, so unlock them here. */
	for (i = 0; i < nand->memorg.ntargets; i++) {
        //    printf("[%s %d]\n",__func__,__LINE__);
		ret = spinand_select_target(spinand, i);
		if (ret)
			goto err_free_bufs;
		if (0x01 == spinand->id.data[0])	//GSS0xGSAM0 unprotection
		{
			ret = spinand_lock_block(spinand, 0x10);
			if (ret)
				goto err_free_bufs;
			__udelay(100);

			ret = spinand_lock_block(spinand, 0x02);
			if (ret)
				goto err_free_bufs;
			__udelay(100);

			ret = spinand_lock_block(spinand, 0x00);
			if (ret)
				goto err_free_bufs;
			
		}
		else
		{
		//    printf("[%s %d]\n",__func__,__LINE__);
			ret = spinand_lock_block(spinand, BL_ALL_UNLOCKED);
			if (ret)
				goto err_free_bufs;
		}
	}
  //  printf("[%s %d]\n",__func__,__LINE__);

	ret = nanddev_init(nand, &spinand_ops, THIS_MODULE);
	if (ret)
		goto err_manuf_cleanup;


#if 0
    T_U8 test_buf[2048];

    //if(spi_nand_page_read(op.addr.val, (unsigned char *)op.data.buf.in, 2048 , 0) == AK_FALSE)
    if(spi_nand_page_read(0x100, test_buf, 2048 , 0) == AK_FALSE)
    {
        printf("Read failed.\n");
        while(1);
    }
    printf("[%s %d] success \n",__func__,__LINE__);
    hexdump_spi_data(test_buf,"read nand");

#endif



	/*
	 * Right now, we don't support ECC, so let the whole oob
	 * area is available for user.
	 */
	//      printf("[%s %d]\n",__func__,__LINE__);
	mtd->_read_oob = spinand_mtd_read;
	mtd->_write_oob = spinand_mtd_write;
	mtd->_block_isbad = spinand_mtd_block_isbad;
	mtd->_block_markbad = spinand_mtd_block_markbad;
	mtd->_block_isreserved = spinand_mtd_block_isreserved;
	mtd->_erase = spinand_mtd_erase;
   // printf("[%s %d]\n",__func__,__LINE__);

	if (spinand->eccinfo.ooblayout)
		mtd_set_ooblayout(mtd, spinand->eccinfo.ooblayout);
	else
		mtd_set_ooblayout(mtd, &spinand_noecc_ooblayout);

	ret = mtd_ooblayout_count_freebytes(mtd);
	if (ret < 0)
		goto err_cleanup_nanddev;

	mtd->oobavail = ret;

	return 0;

err_cleanup_nanddev:
	nanddev_cleanup(nand);

err_manuf_cleanup:
	spinand_manufacturer_cleanup(spinand);

err_free_bufs:
	kfree(spinand->databuf);
	kfree(spinand->scratchbuf);
	return ret;
}

static void spinand_cleanup(struct spinand_device *spinand)
{
	struct nand_device *nand = spinand_to_nand(spinand);

	nanddev_cleanup(nand);
	spinand_manufacturer_cleanup(spinand);
	kfree(spinand->databuf);
	kfree(spinand->scratchbuf);
}

static int spinand_probe(struct udevice *dev)
{
	struct spinand_device *spinand = dev_get_priv(dev);
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct mtd_info *mtd = dev_get_uclass_priv(dev);
	struct nand_device *nand = spinand_to_nand(spinand);
	int ret;

#ifndef __UBOOT__
	spinand = devm_kzalloc(&mem->spi->dev, sizeof(*spinand),
			       GFP_KERNEL);
	if (!spinand)
		return -ENOMEM;

	spinand->spimem = mem;
	spi_mem_set_drvdata(mem, spinand);
	spinand_set_of_node(spinand, mem->spi->dev.of_node);
	mutex_init(&spinand->lock);

	mtd = spinand_to_mtd(spinand);
	mtd->dev.parent = &mem->spi->dev;
#else
	nand->mtd = mtd;
	mtd->priv = nand;
	mtd->dev = dev;
	mtd->name = malloc(20);
	if (!mtd->name)
		return -ENOMEM;
	sprintf(mtd->name, "spi-nand%d", spi_nand_idx++);
	spinand->slave = slave;
	spinand_set_of_node(spinand, dev->node.np);
#endif

	ret = spinand_init(spinand);
	if (ret)
		return ret;

#ifndef __UBOOT__
	ret = mtd_device_register(mtd, NULL, 0);
#else
	ret = add_mtd_device(mtd);
#endif
	if (ret)
		goto err_spinand_cleanup;

	return 0;

err_spinand_cleanup:
	spinand_cleanup(spinand);

	return ret;
}

#ifndef __UBOOT__
static int spinand_remove(struct udevice *slave)
{
	struct spinand_device *spinand;
	struct mtd_info *mtd;
	int ret;

	spinand = spi_mem_get_drvdata(slave);
	mtd = spinand_to_mtd(spinand);
	free(mtd->name);

	ret = mtd_device_unregister(mtd);
	if (ret)
		return ret;

	spinand_cleanup(spinand);

	return 0;
}

static const struct spi_device_id spinand_ids[] = {
	{ .name = "spi-nand" },
	{ /* sentinel */ },
};

#ifdef CONFIG_OF
static const struct of_device_id spinand_of_ids[] = {
	{ .compatible = "spi-nand" },
	{ /* sentinel */ },
};
#endif

static struct spi_mem_driver spinand_drv = {
	.spidrv = {
		.id_table = spinand_ids,
		.driver = {
			.name = "spi-nand",
			.of_match_table = of_match_ptr(spinand_of_ids),
		},
	},
	.probe = spinand_probe,
	.remove = spinand_remove,
};
module_spi_mem_driver(spinand_drv);

MODULE_DESCRIPTION("SPI NAND framework");
MODULE_AUTHOR("Peter Pan<peterpandong@micron.com>");
MODULE_LICENSE("GPL v2");
#endif /* __UBOOT__ */

static const struct udevice_id spinand_ids[] = {
	{ .compatible = "spi-nand" },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(spinand) = {
	.name = "spi_nand",
	.id = UCLASS_MTD,
	.of_match = spinand_ids,
	.priv_auto_alloc_size = sizeof(struct spinand_device),
	.probe = spinand_probe,
};

extern int nand_register(int devnum, struct mtd_info *mtd);
void board_nand_init(void)
{
	struct udevice *dev;
	int ret;
	struct mtd_info *mtd;
//    printf("[%s %d]\n",__func__,__LINE__);
	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_GET_DRIVER(spinand), &dev);
	if (ret && ret != -ENODEV){
		pr_err("Failed to initialize %s: %d\n", dev->name, ret);
		return;
	}

	mtd = dev_get_uclass_priv(dev);
	nand_register(0, mtd);
    
}
	

