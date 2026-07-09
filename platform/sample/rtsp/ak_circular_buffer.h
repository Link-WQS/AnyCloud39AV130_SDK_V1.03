


#ifndef _CIRBUF_H_
#define _CIRBUF_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef void* CIRBUF_HANDLE;

//#define CirularHeadInfoMax 	(48)		//数据头信息最大大小

/**
 * 调用流程,流程分两部分:1)把数据放进环形缓冲;2)注册一个读用户,从环形缓冲中读数据
 * 1:cirbuf_new()---->cirbuf_get_write_buf()---->cirbuf_put_write_buf()
 * 2:cirbuf_reader_new()----> cirbuf_get_buff_data()
 * 注意：
 * cirbuf_reader_new()会返回unReaderId,调CirBuf_GetBuffData()时需要传unReaderId;
 * cirbuf_reader_new()后不再读后可以删除一个读用户,即cirbuf_reader_new()与cirbuf_reader_delete()是一对的
 * 删除一个环形缓冲CirBuf_Delete，CirBuf_New()与CirBuf_Delete()是一对的
 */

//新建一个环形缓冲
CIRBUF_HANDLE cirbuf_new( unsigned int unMemSize);

//删除一个环形缓冲
int cirbuf_delete(CIRBUF_HANDLE pCirBufHandle);

//从一个环形缓冲中获取写地址
unsigned char * cirbuf_get_write_buf(CIRBUF_HANDLE pCirBufHandle,unsigned int unWriteLen);

//把数据写入环形缓冲后更并更新
int cirbuf_put_write_buf(CIRBUF_HANDLE pCirBufHandle ,unsigned int unWriteLen);

//注册一个读用户
int cirbuf_reader_new(CIRBUF_HANDLE pCirBufHandle,char *pName);

//删除一个读用户
int cirbuf_reader_delete(CIRBUF_HANDLE pCirBufHandle,unsigned int unReaderId);

//从环形缓冲中获取数据
int cirbuf_get_buff_data(CIRBUF_HANDLE pCirBufHandle,unsigned int unReaderId, 
    unsigned char **ppDataBuf,unsigned int *pDataSize,unsigned int unTimeOutms);

#ifdef __cplusplus
}
#endif

#endif

