#include "snpes.h"
#include "snpes_types.h"
#include "snpes_utils.h"
#include "CircularQueue.h"
#include <string.h>
#include <assert.h>

/* private variables */

static  DeviceCtx_t dev;
static  uint8_t     buf[BUF_SIZE] = {0};
static  ClientCtx_t clients[CLT_CNT] = {0};

/* private functions */

static void stream_handler(void);

/* functions implementations */

void snpes_init(DeviceType_t type, uint8_t uid, LoraItf_t *lora, TimerItf_t *timer)
{
	/* avoid null address */
	assert(lora);
	assert(timer);

	/* initialize device info */
	dev.unique_id = uid;
	dev.network_id = type;
	dev.type = type;
	dev.hw.socket = lora;
	dev.hw.timer = timer;

	/* protocol data streams config */
	dev.stream_in.start_addr = (void *)buf;
	dev.stream_in.elmt_size = PKT_SIZE;
	dev.stream_in.elmt_cnt = (uint8_t)S_IN_CNT;
	dev.stream_out.start_addr = (void *)(buf+(PKT_SIZE*S_IN_CNT));
	dev.stream_out.elmt_size = PKT_SIZE;
	dev.stream_out.elmt_cnt = (uint8_t)S_OUT_CNT;

	/* initialize data streams */
	queue_init(&dev.stream_in);
	queue_init(&dev.stream_out);
}

static void stream_handler()
{
	uint8_t nid, size;
	void *dest = NULL;
	Packet_t *src = NULL;

	/* if there is some packet availiable, save it */
	if (dev.hw.socket->pkt_avail() && !queue_full(&dev.stream_in)) {
		dest = queue_alloc(&dev.stream_in);
		dev.hw.socket->pkt_recv(&nid, dest, &size);
		queue_push(&dev.stream_in, dest);
	}

	/* if there is some packet to send, send it */
	if (!queue_empty(&dev.stream_out)) {
		src = (Packet_t *) queue_pop(&dev.stream_out);
		dev.hw.socket->pkt_send(src->dest_nid, (uint8_t *)src, (src->data_size + META_SIZE));
	}
}

static void gateway_state_machine()
{
	States_t state;
	Packet_t *pkt = NULL;
	ClientCtx_t *clt = NULL;
	uint8_t nid;

	/* if there isn't packets to process */
	if (queue_empty(&dev.stream_in)) return;

	/* else, get the packet */
	pkt = (Packet_t *) queue_pop(&dev.stream_in);

	/* figure out the protocol state */

	/* if the packet isn't from a client */
	if (pkt->src_nid == 0xFF) {
		if (get_pkt_type(pkt) == SCAN) {
			state = SEND_INFO;
		}
		else if (get_pkt_type(pkt) == SYNC && pkt->dest_uid == dev.unique_id) {
			state = RESP_SYNC;
		}
		else {
			/* corrupted packet, just ignore it */
			return;
		}
	}
	/* it's *probably* a client */
	else {
		clt = get_client_ctx(clients, pkt->src_nid);
		if (clt->connected != NOT_CONNETED) {
			state = clt->state;
		}
		else {
			/* that's not a real client, ignore it */
			return;
		}
	}

	/* */
	switch (state) {
	case SEND_INFO:
		enqueue_signal(&dev, INFO, pkt->src_uid, pkt->src_nid);
		break;
	case RESP_SYNC:
		nid = alloc_nid(clients);
		if (nid == 0x00) {
			enqueue_signal(&dev, FULL, pkt->src_uid, pkt->src_nid);
		}
		else {
			clt = get_client_ctx(clients, nid);
			if (clt->connected != CONNECTING) {
				clt->connected = CONNECTING;
				clt->unique_id = pkt->src_uid;
				clt->timeout = 0;
				clt->state = WAIT_ACK;
			}
			else if (clt->timeout == MAX_TIMEOUT) {
				free_nid(clients, nid);
			}
			else {
				enqueue_data(&dev, pkt->src_uid, pkt->src_nid, 0x00, &nid, sizeof(nid));
			}
		}
		break;
	case WAIT_ACK:
		break;
	}
}
