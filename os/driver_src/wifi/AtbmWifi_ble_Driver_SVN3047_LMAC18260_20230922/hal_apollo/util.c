
#include <net/atbm_mac80211.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>
#include <linux/skbuff.h>

#include "apollo.h"
#include "bh.h"
#include "hwio.h"
#include "wsm.h"
#include "sbus.h"
#include "debug.h"
#include "apollo_plat.h"
#include "sta.h"
#include "ap.h"
#include "scan.h"
/* Must be called from BH thraed. */
void atbm_enable_powersave(struct atbm_vif *priv,
			     bool enable)
{
	atbm_dbg(ATBM_APOLLO_DBG_MSG, "[BH] Powerave is %s.\n",
			enable ? "enabled" : "disabled");
	priv->powersave_enabled = enable;
}

void atbm_bh_wakeup(struct atbm_common *hw_priv)
{
	atbm_dbg(ATBM_APOLLO_DBG_MSG, "[BH] wakeup.\n");
	if(hw_priv->sbus_ops->sbus_xmit_schedule)
		hw_priv->sbus_ops->sbus_xmit_schedule(hw_priv->sbus_priv);
	else if (atomic_add_return(1, &hw_priv->bh_tx) == 1){
		wake_up(&hw_priv->bh_wq);
	}
}

int wsm_release_vif_tx_buffer(struct atbm_common *hw_priv, int if_id,
				int count)
{
	int ret = 0;

	spin_lock_bh(&hw_priv->tx_com_lock);
	hw_priv->hw_bufs_used_vif[if_id] -= count;

	if (WARN_ON(hw_priv->hw_bufs_used_vif[if_id] < 0)){
		atbm_printk_err( "%s:[%d][%d]\n",__func__,if_id,hw_priv->hw_bufs_used_vif[if_id]);
		hw_priv->hw_bufs_used_vif[if_id] = 0;
		//BUG_ON(1);
		//ret = -1;
	}

	spin_unlock_bh(&hw_priv->tx_com_lock);

	if (!hw_priv->hw_bufs_used_vif[if_id])
		wake_up(&hw_priv->bh_evt_wq);

	return ret;
}


int wsm_release_vif_tx_buffer_Nolock(struct atbm_common *hw_priv, int if_id,
				int count)
{
	int ret = 0;

	hw_priv->hw_bufs_used_vif[if_id] -= count;

	if (!hw_priv->hw_bufs_used_vif[if_id])
		wake_up(&hw_priv->bh_evt_wq);

	if (WARN_ON(hw_priv->hw_bufs_used_vif[if_id] < 0)){
		atbm_printk_err( "%s:[%d][%d]\n",__func__,if_id,hw_priv->hw_bufs_used_vif[if_id]);
		hw_priv->hw_bufs_used_vif[if_id] =0;
		//BUG_ON(1);
		//ret = -1;
	}

	return ret;
}
void atbm_monitor_pc(struct atbm_common *hw_priv)
{
	u32 testreg1[10] = {0};
	u32 testreg_pc = 0;
	u32 testreg_ipc = 0;
	u32 val28;
	u32 val20;
	int i = 0;

	atbm_direct_write_reg_32(hw_priv,0x16100050,1);
	//atbm_direct_read_reg_32(hw_priv,0x18e00014,&testreg1);
	atbm_direct_read_reg_32(hw_priv,0x16100054,&testreg_pc);
	atbm_direct_read_reg_32(hw_priv,0x16100058,&testreg_ipc);
	atbm_direct_read_reg_32(hw_priv,0x16101028,&val28);
	atbm_direct_read_reg_32(hw_priv,0x16101020,&val20);
	//atbm_direct_read_reg_32(hw_priv,0x16400000,&testreg_uart);

	for(i=0;i<10;i++){
		atbm_direct_read_reg_32(hw_priv,hw_priv->wsm_caps.exceptionaddr+4*i+4,&testreg1[i]);
	}
	atbm_direct_write_reg_32(hw_priv,0x16100050,0);

	atbm_printk_err("ERROR !! pc:[%x],ipc[%x] \n",testreg_pc,testreg_ipc);
	atbm_printk_err("ERROR !! reg0:[%x],reg1[%x],reg2[%x],reg3[%x],reg4[%x],reg5[%x],reg6[%x] \n",
														testreg1[0],
														testreg1[1],
														testreg1[2],
														testreg1[3],
														testreg1[4],
														testreg1[5],
														testreg1[6]);

	atbm_printk_err( "[PC]:0x16101028(%x)\n",val28);	
	atbm_printk_err( "[PC]:0x16101020(%x)\n",val20);
	
}


#ifdef	ATBM_WIFI_QUEUE_LOCK_BUG

void atbm_set_priv_queue_cap(struct atbm_vif *priv)
{
	struct atbm_common	*hw_priv = priv->hw_priv;
	struct atbm_vif *other_priv;
	u8 i = 0;

	priv->queue_cap = ATBM_QUEUE_SINGLE_CAP;
	atbm_for_each_vif(hw_priv, other_priv, i) {
		if(other_priv == NULL)
			continue;
		if(other_priv == priv)
			continue;
		if(other_priv->join_status <=  ATBM_APOLLO_JOIN_STATUS_MONITOR)
			continue;
		other_priv->queue_cap = ATBM_QUEUE_COMB_CAP;
		priv->queue_cap = ATBM_QUEUE_COMB_CAP;
	}

}

void atbm_clear_priv_queue_cap(struct atbm_vif *priv)
{
	struct atbm_common	*hw_priv = priv->hw_priv;
	struct atbm_vif *other_priv;
	struct atbm_vif *prev_priv = NULL;
	u8 i = 0;

	priv->queue_cap = ATBM_QUEUE_DEFAULT_CAP;

	atbm_for_each_vif(hw_priv, other_priv, i) {

		if(other_priv == NULL)
			continue;
		if(other_priv == priv)
			continue;
		if(other_priv->join_status <=  ATBM_APOLLO_JOIN_STATUS_MONITOR)
			continue;
		
		other_priv->queue_cap = ATBM_QUEUE_SINGLE_CAP;

		if(prev_priv == NULL){
			prev_priv = other_priv;
			continue;
		}

		prev_priv->queue_cap = ATBM_QUEUE_COMB_CAP;
		other_priv->queue_cap = ATBM_QUEUE_COMB_CAP;
		prev_priv = other_priv;
		
	}
}
#endif
int atbm_save_efuse(struct atbm_common *hw_priv,struct efuse_headr *efuse_save)
{
	int ret = 0;
	int iResult=0;
	//struct atbm_vif *vif;
	struct efuse_headr efuse_bak;
	
	/*
	*LMC_STATUS_CODE__EFUSE_VERSION_CHANGE	failed because efuse version change  
	*LMC_STATUS_CODE__EFUSE_FIRST_WRITE, 		failed because efuse by first write   
	*LMC_STATUS_CODE__EFUSE_PARSE_FAILED,		failed because efuse data wrong, cannot be parase
	*LMC_STATUS_CODE__EFUSE_FULL,				failed because efuse have be writen full
	*/
	ret = wsm_efuse_change_data_cmd(hw_priv, efuse_save,0);
	if (ret == LMC_STATUS_CODE__EFUSE_FIRST_WRITE)
	{
		atbm_printk_err("first write\n");
		iResult = -3;
	}else if (ret == LMC_STATUS_CODE__EFUSE_PARSE_FAILED)
	{
		atbm_printk_err("parse failed\n");
		iResult = -4;
	}else if (ret == LMC_STATUS_CODE__EFUSE_FULL)
	{
		atbm_printk_err("efuse full\n");
		iResult = -5;
	}else if (ret == LMC_STATUS_CODE__EFUSE_VERSION_CHANGE)
	{
		atbm_printk_err("efuse version change\n");
		iResult = -6;
	}else
	{
		iResult = 0;
	}
	if (iResult == 0)
	{
		//frame_hexdump("efuse_d", efuse_save, sizeof(struct efuse_headr));
		memset(&efuse_bak,0,sizeof(struct efuse_headr));
		wsm_get_efuse_data(hw_priv,(void *)&efuse_bak, sizeof(struct efuse_headr));

		if(efuse_bak.specific != 0)
		{
			//sigmastar oid
			efuse_save->specific = efuse_bak.specific;
		}
		
		if(memcmp((void *)&efuse_bak, efuse_save, sizeof(struct efuse_headr)) !=0)
		{
			frame_hexdump("efuse_bak", (u8 *)&efuse_bak, sizeof(struct efuse_headr));
			frame_hexdump("efuse_save", (u8 *)&efuse_save, sizeof(struct efuse_headr));
			iResult = -2;
		}else
		{
			iResult = 0;
		}
	}
	return iResult;
}
void atbm_destroy_wsm_cmd(struct atbm_common *hw_priv)
{
	/*
	*cancle cmd,return err;
	*/
	spin_lock_bh(&hw_priv->wsm_cmd.lock);
	if(hw_priv->wsm_cmd.cmd != 0xFFFF){
		hw_priv->wsm_cmd.ret = -1;
		hw_priv->wsm_cmd.done = 1;
		hw_priv->wsm_cmd.cmd = 0xFFFF;
		if(hw_priv->wsm_cmd.ptr){
			hw_priv->wsm_cmd.ptr = NULL;
		}else {
		}
		wake_up(&hw_priv->wsm_cmd_wq);
	}
	spin_unlock_bh(&hw_priv->wsm_cmd.lock);
	/*
	*flush work
	*/
	atbm_flush_workqueue(hw_priv->workqueue);
	atbm_printk_exit("Flush hw_priv->workqueue\n");
	/*
	*release scan 
	*/
	if(atomic_read(&hw_priv->scan.in_progress)&&(hw_priv->scan.status != -ETIMEDOUT) &&
	   (atbm_hw_cancel_delayed_work(&hw_priv->scan.timeout,true) > 0)){
	    
	    wsm_oper_unlock(hw_priv);
	    /*
		*scan is running,wait timeout.....
		*/
		hw_priv->scan.status = 1;
		/*
		*force scan stop
		*/
		hw_priv->scan.req = NULL;
		
	    atbm_printk_exit("scan running,try to cancle\n");
		atbm_scan_timeout(&hw_priv->scan.timeout.work);
	}
	atbm_printk_exit("Flush pm and scan\n");
	/*
	*try to release wsm_oper_unlock
	*/
	atbm_del_timer_sync(&hw_priv->wsm_pm_timer);
	spin_lock_bh(&hw_priv->wsm_pm_spin_lock);
	if(atomic_read(&hw_priv->wsm_pm_running) == 1){
		atomic_set(&hw_priv->wsm_pm_running, 0);
		wsm_oper_unlock(hw_priv);
		atbm_release_suspend(hw_priv);
		atbm_printk_exit("%s,up pm lock\n",__func__);
	}
	spin_unlock_bh(&hw_priv->wsm_pm_spin_lock);

	/*
	*scan cmd may be need comp event
	*/
	if(down_trylock(&hw_priv->scan.lock)){
		WARN_ON(1);
	}else {
		up(&hw_priv->scan.lock);
	}
	synchronize_net();
}
int atbm_reinit_firmware(struct atbm_common *hw_priv)
{
	struct wsm_operational_mode mode = {
		.power_mode = wsm_power_mode_quiescent,
		.disableMoreFlagUsage = true,
	};
	int ret = 0;
	u8 if_id = 0;

	/*
	*load firmware
	*/
	hw_priv->wsm_caps.firmwareReady = 0;
	ret = atbm_load_firmware(hw_priv);
	if (ret){
		atbm_printk_err( "atbm_load_firmware ERROR!\n");
		goto error_reload;
	}
	atbm_printk_init("mdelay wait wsm_startup_done  !!\n");
	if (wait_event_interruptible_timeout(hw_priv->wsm_startup_done,
			hw_priv->wsm_caps.firmwareReady,3*HZ)<=0){
		atbm_printk_err("%s: reload fw err\n",__func__);
		ret = -1;
		goto error_reload;
	}
	atomic_xchg(&hw_priv->bh_halt,0);
	atbm_firmware_init_check(hw_priv);
	for (if_id = 0; if_id < ABwifi_get_nr_hw_ifaces(hw_priv); if_id++) {
		/* Set low-power mode. */
		ret = wsm_set_operational_mode(hw_priv, &mode, if_id);
		if (ret) {
			WARN_ON(1);
			goto error_reload;
		}
		/* Enable multi-TX confirmation */
		ret = wsm_use_multi_tx_conf(hw_priv, true, if_id);
		if (ret) {
			ret = 0;
		}
	}
	
error_reload:
	return ret;
}
int atbm_rx_bh_flush(struct atbm_common *hw_priv)
{
	struct sk_buff *skb ;

	while ((skb = atbm_skb_dequeue(&hw_priv->rx_frame_queue)) != NULL) {
		//printk("test=====>atbm_kfree skb %p \n",skb);
		atbm_dev_kfree_skb(skb);
	}
	return 0;
}
void atbm_bh_halt(struct atbm_common *hw_priv)
{

//	return;
	if (atomic_add_return(1, &hw_priv->bh_halt) == 1){
		atomic_set(&hw_priv->atbm_pluged,0);
		wake_up(&hw_priv->bh_wq);
	}
#if 0	
	spin_lock_bh(&hw_priv->wsm_cmd.lock);
	if(hw_priv->wsm_cmd.ptr == NULL){
		if(hw_priv->wsm_cmd.cmd != 0xFFFF){
			atbm_printk_err("%s:release cmd [%x]\n",__func__,hw_priv->wsm_cmd.cmd);
			hw_priv->wsm_cmd.ret = -1;
			hw_priv->wsm_cmd.done = 1;
			wake_up(&hw_priv->wsm_cmd_wq);
		}else {
			
		}
	}else {
		atbm_printk_err("%s:cmd[%x] not send\n",__func__,hw_priv->wsm_cmd.cmd);
	}
	spin_unlock_bh(&hw_priv->wsm_cmd.lock);
#endif
}
void  atbm_bh_multrx_trace(struct atbm_common *hw_priv,u8 n_rx)
{
	hw_priv->multrx_trace[hw_priv->multrx_index++] = n_rx;
	hw_priv->multrx_index &= (64-1);
}static u8 atbm_rx_need_alloc_skb(int wsm_id)
{
	int id = (wsm_id  & ~WSM_TX_LINK_ID(WSM_TX_LINK_ID_MAX));

	return (id==WSM_RECEIVE_INDICATION_ID)||(id == WSM_SMARTCONFIG_INDICATION_ID);
}
bool atbm_rx_serial(struct wsm_rx_encap *encap)
{
	bool serial_rx = true;
	int wsm_seq;
	
	if(encap->check_seq == false){
		goto ret;
	}
	
	if(encap->wsm_id == 0x0800){
		goto ret;
	}

	wsm_seq = (__le32_to_cpu(encap->wsm->id) >> 13) & 7;
	//
	//add because usb not reset when rmmod driver, just drop error frame  
	//
	if((encap->hw_priv->wsm_caps.firmwareReady==0)
		&&((wsm_seq != encap->hw_priv->wsm_rx_seq)
		||((encap->wsm_id & ~WSM_TX_LINK_ID(WSM_TX_LINK_ID_MAX)) != WSM_STARTUP_IND_ID))){
		atbm_printk_err("rx wsm_seq err1 (rxseq=%d need_rxseq=%d)  wsm_id %x firmwareReady %d\n",wsm_seq,
						encap->hw_priv->wsm_rx_seq,encap->wsm_id,encap->hw_priv->wsm_caps.firmwareReady);
		if((encap->wsm_id & ~WSM_TX_LINK_ID(WSM_TX_LINK_ID_MAX)) != WSM_STARTUP_IND_ID){
			serial_rx = false;
			goto ret;
		}
		atbm_printk_err("startup seq(%d)\n",wsm_seq);
	}
	else if (WARN_ON(wsm_seq != encap->hw_priv->wsm_rx_seq)) {
		atbm_printk_err("rx wsm_seq error wsm_seq[%d] wsm_rx_seq[%d] wsm_id[%x]\n",
			wsm_seq,encap->hw_priv->wsm_rx_seq,encap->wsm_id);
		//frame_hexdump("rxdata",wsm,64);
		//BUG_ON(1);
		atbm_hif_status_set(1);
		while(1){
			msleep(200);
		}
		atbm_bh_halt(encap->hw_priv);
		serial_rx = false;
		goto ret;
	}
	
	encap->hw_priv->wsm_rx_seq = (wsm_seq + 1) & 7;
ret:
	return serial_rx;
}

void atbm_rx_multi_rx(struct wsm_rx_encap *encap)
{
	
#define RX_ALLOC_BUFF_OFFLOAD (  (56+16)/*RX_DESC_OVERHEAD*/+4/*FCS_LEN*/ -16 /*WSM_HI_RX_IND*/)
	struct wsm_multi_rx *  multi_rx = (struct wsm_multi_rx *)encap->skb->data;			
	int RxFrameNum = multi_rx->RxFrameNum;	
	struct sk_buff *atbm_skb_copy;
	struct wsm_hdr *wsm;
	u32 wsm_len;
	int wsm_id;
	int data_len;
	
	data_len = __le32_to_cpu(multi_rx->MsgLen);
	
	data_len -= sizeof(struct wsm_multi_rx);
	wsm = (struct wsm_hdr *)(multi_rx+1);
	wsm_len = __le32_to_cpu(wsm->len);
	wsm_id	= __le32_to_cpu(wsm->id) & 0xFFF;
	
	encap->wsm_id = WSM_RECEIVE_INDICATION_ID;
	do {
				
		if(data_len < wsm_len){
			atbm_printk_err("skb->len %x,wsm_len %x\n",encap->skb->len,wsm_len);
			break;
		}
		
		BUG_ON((wsm_id  & ~WSM_TX_LINK_ID(WSM_TX_LINK_ID_MAX)) !=  WSM_RECEIVE_INDICATION_ID);
		
		encap->wsm = wsm;
		
		if(unlikely(WARN_ON(atbm_rx_serial(encap) == false))){
		    goto next_skb;
		}
		
		atbm_skb_copy = atbm_rx_alloc_frame(encap);
		
		if (unlikely(!atbm_skb_copy)){
			atbm_printk_err("alloc--skb Error(%d)1,move to next skb\n",wsm_len + 16);
			WARN_ON(1);	
			goto next_skb;
		}
		
		encap->rx_func(encap,atbm_skb_copy);
		
next_skb:
		data_len -= ALIGN(wsm_len + RX_ALLOC_BUFF_OFFLOAD,4);
		RxFrameNum--;
		wsm = (struct wsm_hdr *)((u8 *)wsm +ALIGN(( wsm_len + RX_ALLOC_BUFF_OFFLOAD),4));
		wsm_len = __le32_to_cpu(wsm->len);
		wsm_id	= __le32_to_cpu(wsm->id) & 0xFFF;
		
	}while((RxFrameNum>0) && (data_len > 32));
	BUG_ON(RxFrameNum != 0);
}
static struct sk_buff *atbm_alloc_bh_skb(unsigned int length)
{
	#ifdef USB_USE_TASTLET_TXRX
	#define __atbm_allo_bh_skb(_length) atbm_dev_alloc_skb(_length)
	#else
	#define __atbm_allo_bh_skb(_length) __atbm_dev_alloc_skb(_length,GFP_KERNEL);
	#endif
	#ifdef ALLOC_SKB_NULL_TEST
	static u32 skb_alloc_cnt = 0;

	skb_alloc_cnt++;
	if((skb_alloc_cnt%4096) == 0)
		return NULL;
	#endif
	return __atbm_allo_bh_skb(length);
}
struct sk_buff * atbm_rx_alloc_frame(struct wsm_rx_encap *encap)
{
	struct sk_buff *skb;
	/*
	*alloc 8023 frame
	*/
	if(wsm_8023_format(encap->wsm)){
		
		skb = wsm_rx_alloc_8023_frame(encap->hw_priv,encap->wsm);
		if(skb){
			skb->pkt_type = ATBM_RX_WSM_8023_DATA_FRAME;
		}else {
			atbm_printk_err("alloc 8023 skb err\n");
		}

		return skb;
	}
	skb = atbm_alloc_bh_skb(encap->wsm->len + 16);
	
	if(skb){
		memmove(skb->data, encap->wsm, encap->wsm->len);
		atbm_skb_put(skb,encap->wsm->len);
		skb->pkt_type = ATBM_RX_WSM_DATA_FRAME;
	}
	
	return skb;
}
static bool atbm_rx_wsm_cmd_bh_cb(struct wsm_rx_encap *encap,struct sk_buff* skb)
{
	struct wsm_hdr *wsm;
	u32 wsm_len;
	int wsm_id;
	
	wsm = (struct wsm_hdr *)skb->data;

	wsm_len = __le16_to_cpu(wsm->len);

	wsm_id	= __le16_to_cpu(wsm->id) & 0xFFF;

	//BUG_ON(wsm_len > 4096);
	atbm_skb_trim(skb,0);
	atbm_skb_put(skb,wsm_len);

	if (unlikely(wsm_id == 0x0800)) {
		wsm_handle_exception(encap->hw_priv, &skb->data[sizeof(*wsm)],wsm_len - sizeof(*wsm));
		while(1){
			msleep(200);
		}
		atbm_bh_halt(encap->hw_priv);
		goto __free;
	}
	//frame_hexdump("dump  wsm rx ->",wsm,32);
	if (wsm_id & 0x0400) {
		int rc = wsm_release_tx_buffer(encap->hw_priv, 1);
		if (WARN_ON(rc < 0))
			goto __free;
	}

	/* atbm_wsm_rx takes care on SKB livetime */
	if (WARN_ON(wsm_handle_rx(encap->hw_priv, wsm_id, wsm,&skb))){
		atbm_printk_err("%s %d \n",__func__,__LINE__);
		goto __free;
	}

__free:
	if(skb != NULL){
		atbm_dev_kfree_skb(skb);
	}

	return true;
}
bool atbm_process_wsm_cmd_frame(struct wsm_rx_encap *encap)
{
	struct wsm_hdr *wsm;
	u32 wsm_len;
	int wsm_id;
	struct sk_buff *atbm_skb_copy;
	
	wsm = (struct wsm_hdr *)encap->skb->data;
	wsm_len = __le32_to_cpu(wsm->len);
	wsm_id	= __le32_to_cpu(wsm->id) & 0xFFF;

	WARN_ON(atbm_rx_need_alloc_skb(wsm_id));

	atbm_skb_copy = atbm_skb_get(encap->skb);
	atbm_skb_put(atbm_skb_copy,wsm_len);
	atbm_skb_copy->pkt_type = encap->skb->pkt_type;
	atbm_rx_wsm_cmd_bh_cb(encap,atbm_skb_copy);
	atbm_skb_tx_debug(atbm_skb_copy);

	return true;
}
static int atbm_rx_80211_mpdu_bh_cb(struct wsm_rx_encap *encap)
{
	struct wsm_hdr *wsm;
	u32 wsm_len;
	int wsm_id;
	struct sk_buff* skb = encap->skb;
	
	wsm = (struct wsm_hdr *)skb->data;

	wsm_len = __le16_to_cpu(wsm->len);

	wsm_id	= __le16_to_cpu(wsm->id) & 0xFFF;

	//BUG_ON(wsm_len > 4096);
	atbm_skb_trim(encap->skb,0);
	atbm_skb_put(encap->skb,wsm_len);

	/* atbm_wsm_rx takes care on SKB livetime */
	if (WARN_ON(wsm_handle_rx(encap->hw_priv, wsm_id, wsm,&skb))){
		atbm_printk_err("%s %d \n",__func__,__LINE__);
		goto __free;
	}

__free:
	if(skb != NULL){
		atbm_dev_kfree_skb(skb);
	}

	return 0;
}
static int  atbm_rx_8023_framme_bh_cb(struct wsm_rx_encap *encap)
{
	return wsm_receive_indication_8023_frame(encap->hw_priv,encap->skb);
}

bool atbm_rx_directly(struct wsm_rx_encap *encap)
{
	struct sk_buff *atbm_skb_copy;
	encap->wsm_id = (__le32_to_cpu(encap->wsm->id) & 0xFFF)  & (~WSM_TX_LINK_ID(WSM_TX_LINK_ID_MAX));
	
	switch(encap->wsm_id){
	case WSM_SINGLE_CHANNEL_MULTI_RECEIVE_INDICATION_ID:
		/*
		*rx single channel but mult data package
		*/		
		if (unlikely(WARN_ON(atbm_rx_serial(encap) == false))) {			
			atbm_hif_status_set(1);
			atbm_bh_halt(encap->hw_priv);
			goto processed;
		}
		encap->check_seq = false;
		/*no break here*/
		atbm_fallthrough;
	case WSM_MULTI_RECEIVE_INDICATION_ID:
		/*
		*rx muilt data package
		*/		
		atbm_rx_multi_rx(encap);
		goto processed;
	case WSM_RECEIVE_INDICATION_ID:
	case WSM_SMARTCONFIG_INDICATION_ID:
		/*
		*rx single data package
		*/
		if (unlikely(WARN_ON(atbm_rx_serial(encap) == false))) {			
			atbm_hif_status_set(1);
			atbm_bh_halt(encap->hw_priv);
			goto processed;
		}
		
		encap->check_seq = false;
		
		atbm_skb_copy = atbm_rx_alloc_frame(encap);
		if (unlikely(!atbm_skb_copy)){
			atbm_printk_err("alloc--skb Error\n");
			WARN_ON(1);
			goto processed;
		}
		encap->rx_func(encap,atbm_skb_copy);
		goto processed;
#ifdef CONFIG_WIFI_BT_COMB
	case HI_MSG_ID_BLE_EVENT:
	case HI_MSG_ID_BLE_ACK:
		/*
		*rx ble
		*/
		if (unlikely(WARN_ON(atbm_rx_serial(encap) == false))) {			
			atbm_hif_status_set(1);
			atbm_bh_halt(encap->hw_priv);
			goto processed;
		}

		atbm_skb_copy = atbm_alloc_bh_skb(encap->wsm->len + 16);

		if (unlikely(!atbm_skb_copy)){
			atbm_printk_err("alloc--skb Error\n");
			WARN_ON(1);
			goto processed;
		}
		
		memmove(atbm_skb_copy->data, encap->wsm, encap->wsm->len);
		atbm_skb_put(atbm_skb_copy,encap->wsm->len);
		atbm_skb_copy->pkt_type = ATBM_RX_BLE_FRAME;		
		encap->check_seq = false;

		encap->rx_func(encap,atbm_skb_copy);
		break;
#endif
	default:
		
		encap->skb->pkt_type = ATBM_RX_WSM_CMD_FRAME;
		
		if (unlikely(WARN_ON(atbm_rx_serial(encap) == false))) {			
			atbm_hif_status_set(1);
			atbm_bh_halt(encap->hw_priv);
			
		}else{
			encap->check_seq = false;
			encap->rx_func(encap,encap->skb);
		}
		break;
	}
	return false;
processed:
	return true;
}
#ifdef CONFIG_WIFI_BT_COMB
static int  atbm_rx_ble_framme_bh_cb(struct wsm_rx_encap *encap)
{
	struct wsm_hdr *wsm = encap->wsm;
	u16 wsm_len;
	u16 wsm_id;

	wsm_len = __le16_to_cpu(wsm->len);
	wsm_id	= (__le32_to_cpu(wsm->id) & 0xFFF) & (~WSM_TX_LINK_ID(WSM_TX_LINK_ID_MAX));
	
	atbm_printk_debug("%s:ble(%x)(%x)\n",__func__,wsm_id,wsm_len);
	
	if(wsm_id == HI_MSG_ID_BLE_EVENT){
		wsm->id = BLE_MSG_TYPE_EVT;
	}
	else if (wsm_id == HI_MSG_ID_BLE_ACK) {
		wsm->id = BLE_MSG_TYPE_ACK;
	}else {
		atbm_printk_err("unkown type(%x)\n",wsm_id);
		//BUG_ON(1);
		return 0;
	}

	ieee80211_ble_dev_recv(encap->hw_priv->hw,(u8 *)wsm,wsm_len);
	
	atbm_dev_kfree_skb(encap->skb);
	return 0;
}

#endif

bool atbm_rx_tasklet_process_encap(struct wsm_rx_encap *encap,struct sk_buff *skb)
{
	enum atbm_rx_frame_type frame_type = skb->pkt_type;
	
	encap->skb = skb;
	encap->wsm = (struct wsm_hdr *)skb->data;
//	atbm_printk_err("frame_type=%d \n",frame_type);
	switch (frame_type) {
	case ATBM_RX_RAW_FRAME:
		atbm_printk_bh("%s:RX_RAW_FRAME\n",__func__);
		atbm_rx_directly(encap);
		break;
	case ATBM_RX_WSM_CMD_FRAME:
		atbm_printk_bh("%s:WSM_CMD_FRAME\n",__func__);
		atbm_process_wsm_cmd_frame(encap);
		break;
#ifdef CONFIG_WIFI_BT_COMB
	case ATBM_RX_BLE_FRAME:
		atbm_printk_debug("%s:ble\n",__func__);
		atbm_rx_ble_framme_bh_cb(encap);
		goto consume;
#endif
	case ATBM_RX_WSM_DATA_FRAME:
		atbm_printk_bh("%s:RX_SLOW_MGMT_FRAME\n",__func__);
		atbm_rx_80211_mpdu_bh_cb(encap);
		goto consume;
	case ATBM_RX_WSM_8023_DATA_FRAME:
		atbm_rx_8023_framme_bh_cb(encap);
		goto consume;
	case ATBM_RX_DERICTLY_DATA_FRAME:
		WARN_ON(1);
		atbm_dev_kfree_skb(encap->skb);
		goto consume;
	case ATBM_RX_WSM_GRO_FLUSH:
		atbm_printk_bh("ATBM_RX_GRO_FLUSH\n");
		ieee80211_napi_sched(encap->hw_priv->hw);
		break;
	default:
		BUG_ON(1);
		break;
	}

	return false;
consume:
	return true;
}
#ifdef CONFIG_ATBM_BLE
int atbm_ble_do_ble_xmit(struct ieee80211_hw *hw,u8* xmit,size_t len)
{
	struct atbm_common *hw_priv = hw->priv;

	return wsm_ble_xmit(hw_priv,xmit,len);
}
#endif
