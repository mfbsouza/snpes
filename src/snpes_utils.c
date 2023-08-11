#include "snpes_utils.h"
#include "snpes_types.h"
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
