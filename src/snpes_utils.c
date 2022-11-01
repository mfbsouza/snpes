#include "snpes_types.h"
#include "snpes_utils.h"
#include "snpes_cfg.h"
#include "CircularQueue.h"
#include <string.h>
#include <assert.h>

void build_signal(Packet_t *pkt, PacketType_t signal, uint8_t src_uid, uint8_t src_nid, uint8_t dest_uid, uint8_t dest_nid)
{
	assert(pkt);
	pkt->src_uid = src_uid;
	pkt->src_nid = src_nid;
	pkt->dest_uid = dest_uid;
	pkt->dest_nid = dest_nid;
	pkt->flgs_seq = ((signal<<4)&0xF0);
	pkt->data_size = 0;
}

void build_data(Packet_t *pkt, uint8_t src_uid, uint8_t src_nid, uint8_t dest_uid, uint8_t dest_nid, uint8_t seq, const void *src, uint8_t size)
{
	assert(pkt);
	pkt->src_uid = src_uid;
	pkt->src_nid = src_nid;
	pkt->dest_uid = dest_uid;
	pkt->dest_nid = dest_nid;
	pkt->flgs_seq = ((DATA<<4)&0xF0) | (seq&0x0F);
	pkt->data_size = size;
	memcpy(pkt->data, src, size);
}

void enqueue_signal(DeviceCtx_t *dev, PacketType_t signal, uint8_t dest_uid, uint8_t dest_nid)
{
	assert(dev);
	Packet_t *dest = NULL;

	if (!queue_full(&dev->stream_out)) {
		dest = (Packet_t *) queue_alloc(&dev->stream_out);
		build_signal(dest, signal, dev->unique_id, dev->network_id, dest_uid, dest_nid);
	}
}

void enqueue_data(DeviceCtx_t *dev, uint8_t dest_uid, uint8_t dest_nid, uint8_t seq, const void *src, uint8_t size)
{
	assert(dev);
	assert(src);
	Packet_t *dest = NULL;
	if (!queue_full(&dev->stream_out)) {
		dest = (Packet_t *) queue_alloc(&dev->stream_out);
		build_data(dest, dev->unique_id, dev->network_id, dest_uid, dest_nid, seq, src, size);
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
	arr[(nid-1)].waiting = 0;
}

ClientCtx_t *get_client_ctx(ClientCtx_t *arr, uint8_t nid)
{
	assert(arr);
	assert(nid > 0);
	return &(arr[nid-1]);
}

ClientCtx_t *get_waiting_client(ClientCtx_t *arr)
{
	assert(arr);
	for (int i = 0; i < CLT_CNT; i++) {
		if (arr[i].waiting == 1) {
			return &(arr[i]);
		}
	}
	return NULL;
}

PacketType_t get_pkt_type(Packet_t *pkt)
{
	assert(pkt);
	return ((pkt->flgs_seq>>4)&0x0F);
}
