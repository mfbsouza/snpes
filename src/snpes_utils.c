#include "snpes_types.h"
#include "snpes_utils.h"
#include "snpes_cfg.h"
#include "CircularQueue.h"
#include <string.h>
#include <assert.h>

void build_signal(Packet_t *pkt, PacketType_t signal, uint8_t src_uid,
		  uint8_t src_nid, uint8_t dest_uid, uint8_t dest_nid,
		  uint8_t seq, uint8_t size)
{
	assert(pkt);
	pkt->src_uid = src_uid;
	pkt->src_nid = src_nid;
	pkt->dest_uid = dest_uid;
	pkt->dest_nid = dest_nid;
	pkt->flgs_seq = ((signal << 4) & 0xF0) | (seq & 0x0F);
	pkt->data_size = size;
}

void build_data(Packet_t *pkt, uint8_t src_uid, uint8_t src_nid,
		uint8_t dest_uid, uint8_t dest_nid, uint8_t seq,
		const void *src, uint8_t size)
{
	assert(pkt);
	pkt->src_uid = src_uid;
	pkt->src_nid = src_nid;
	pkt->dest_uid = dest_uid;
	pkt->dest_nid = dest_nid;
	pkt->flgs_seq = ((DATA << 4) & 0xF0) | (seq & 0x0F);
	pkt->data_size = size;
	memcpy(pkt->data, src, size);
}

void enqueue_signal(DeviceCtx_t *dev, PacketType_t signal, uint8_t dest_uid,
		    uint8_t dest_nid, uint8_t seq, uint8_t size)
{
	assert(dev);
	Packet_t *dest = NULL;

	if (!queue_full(&dev->stream_out)) {
		dest = (Packet_t *)queue_alloc(&dev->stream_out);
		build_signal(dest, signal, dev->unique_id, dev->network_id,
			     dest_uid, dest_nid, seq, size);
	}
}

void enqueue_data(DeviceCtx_t *dev, uint8_t dest_uid, uint8_t dest_nid,
		  uint8_t seq, const void *src, uint8_t size)
{
	assert(dev);
	assert(src);
	Packet_t *dest = NULL;
	if (!queue_full(&dev->stream_out)) {
		dest = (Packet_t *)queue_alloc(&dev->stream_out);
		build_data(dest, dev->unique_id, dev->network_id, dest_uid,
			   dest_nid, seq, src, size);
	}
}

uint8_t alloc_nid(ClientCtx_t *arr)
{
	assert(arr);
	for (int nid = 0; nid < CLT_CNT; nid++) {
		if (arr[nid].connected == NOT_CONNETED) {
			arr[nid].connected = CONNECTING;
			arr[nid].network_id = (uint8_t)(nid + 1);
			return (uint8_t)(nid + 1);
		}
	}
	return 0;
}

void free_nid(ClientCtx_t *arr, uint8_t nid)
{
	assert(arr);
	assert(nid > 0);
	memset(&(arr[(nid - 1)]), 0, sizeof(ClientCtx_t));
}

ClientCtx_t *get_client_ctx(ClientCtx_t *arr, uint8_t nid)
{
	assert(arr);
	assert(nid > 0);
	return &(arr[nid - 1]);
}

ClientCtx_t *find_client_ctx(ClientCtx_t *arr, uint8_t uid)
{
	assert(arr);
	ClientCtx_t *ret = NULL;

	for (int ii = 0; ii < CLT_CNT; ii++) {
		if (arr[ii].unique_id == uid &&
		    arr[ii].connected != NOT_CONNETED) {
			ret = &(arr[ii]);
			break;
		}
	}
	return ret;
}

// TODO: i don't like that "waiting" word
ClientCtx_t *get_waiting_client(ClientCtx_t *arr)
{
	assert(arr);
	for (int ii = 0; ii < CLT_CNT; ii++) {
		if (arr[ii].state == WAIT_ACK || arr[ii].state == WAIT_DATA || arr[ii].state == WAIT_TRANS_START || arr[ii].state == WAIT_DATA_ACK) {
			return &(arr[ii]);
		}
	}
	return NULL;
}

// TODO: i don't like that function name
ClientCtx_t *get_data_avail_client(ClientCtx_t *arr)
{
	assert(arr);
	for (int ii = 0; ii < CLT_CNT; ii++) {
		if (arr[ii].state == DATA_AVAIL) {
			return &(arr[ii]);
		}
	}
	return NULL;
}

ClientCtx_t *get_rts_client(ClientCtx_t *arr)
{
	assert(arr);
	for (int ii = 0; ii < CLT_CNT; ii++) {
		if (arr[ii].out_expt_pkt != 0) {
			return &(arr[ii]);
		}
	}
	return NULL;
}

PacketType_t get_pkt_type(Packet_t *pkt)
{
	assert(pkt);
	return (PacketType_t)((pkt->flgs_seq >> 4) & 0x0F);
}

uint8_t get_pkt_seq_number(Packet_t *pkt)
{
	assert(pkt);
	return (uint8_t)(pkt->flgs_seq & 0x0F);
}
