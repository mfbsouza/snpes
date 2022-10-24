#include "snpes_utils.h"
#include "snpes_types.h"
#include <string.h>

/* private functions */
static void set_pkt_metadata(void *, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
static void set_pkt_data(void *, const void *, uint8_t );

void send_signal(DeviceCtx_t *dev, PacketType_t signal, uint8_t dest_nid)
{
	// TODO: implement
}

PacketType_t get_pkt_type(void *pkt)
{
	Packet_t *src = (Packet_t *)pkt;
	return ((src->flgs_seq>>4)&0x0F);
}

static void set_pkt_metadata(void *pkt, uint8_t s_uid, uint8_t s_nid, uint8_t d_uid, uint8_t d_nid, uint8_t flag, uint8_t seq)
{
	Packet_t *dest = (Packet_t *)pkt;
	dest->src_uid = s_uid;
	dest->src_nid = s_nid;
	dest->dest_uid = d_uid;
	dest->dest_nid = d_nid;
	dest->flgs_seq = ((flag<<4)&0xF0) | (seq&0x0F);
}

static void set_pkt_data(void *pkt, const void *src, uint8_t size)
{
	Packet_t *dest = (Packet_t *)pkt;
	memcpy(dest->data, src, size);
	dest->data_size = size;
}
