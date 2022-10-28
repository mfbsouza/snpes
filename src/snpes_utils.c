#include "snpes_types.h"
#include "snpes_utils.h"
#include "snpes_cfg.h"
#include "CircularQueue.h"
#include <string.h>
#include <assert.h>

void enqueue_signal(DeviceCtx_t *dev, PacketType_t signal, uint8_t dest_uid, uint8_t dest_nid)
{
	assert(dev);
	Packet_t *dest = NULL;

	if (!queue_full(&dev->stream_out)) {
		dest = (Packet_t *) queue_alloc(&dev->stream_out);
		dest->src_uid = dev->unique_id;
		dest->src_nid = dev->network_id;
		dest->dest_uid = dest_uid;
		dest->dest_nid = dest_nid;
		dest->flgs_seq = ((signal<<4)&0xF0);
		dest->data_size = META_SIZE;
	}
}

void enqueue_data(DeviceCtx_t *dev, uint8_t dest_uid, uint8_t dest_nid, uint8_t seq, const void *src, uint8_t size)
{
	assert(dev);
	assert(src);
	Packet_t *dest = NULL;
	if (!queue_full(&dev->stream_out)) {
		dest = (Packet_t *) queue_alloc(&dev->stream_out);
		dest->src_uid = dev->unique_id;
		dest->src_nid = dev->network_id;
		dest->dest_uid = dest_uid;
		dest->dest_nid = dest_nid;
		dest->flgs_seq = ((DATA<<4)&0xF0) | (seq&0x0F);
		dest->data_size = size + META_SIZE;
		memcpy(dest->data, src, size);
	}
}

uint8_t alloc_nid(ClientCtx_t *arr)
{
	assert(arr);
	int nid;
	for (nid = 0; nid < CLT_CNT; nid++) {
		if (arr[nid].connected == NOT_CONNETED) {
			arr[nid].connected = CONNECTING;
			arr[nid].network_id = (uint8_t)(nid+1);
			return (uint8_t)(nid+1);
		}
	}
	return 0;
}

void free_nid(ClientCtx_t *arr, uint8_t nid)
{
	assert(arr);
	assert(nid > 0);
	arr[(nid-1)].connected = 0;
	arr[(nid-1)].unique_id = 0;
	arr[(nid-1)].network_id = 0;
	arr[(nid-1)].timer_ref = 0;
	arr[(nid-1)].timeout = 0;
	arr[(nid-1)].state = 0;
}

ClientCtx_t *get_client_ctx(ClientCtx_t *arr, uint8_t nid)
{
	assert(arr);
	assert(nid > 0);
	return &(arr[nid-1]);
}

PacketType_t get_pkt_type(Packet_t *pkt)
{
	assert(pkt);
	return ((pkt->flgs_seq>>4)&0x0F);
}
