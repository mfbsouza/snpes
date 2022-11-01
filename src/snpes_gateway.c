#include "snpes_gateway.h"
#include "snpes_cfg.h"
#include "snpes_types.h"
#include "snpes_utils.h"
#include "CircularQueue.h"
#include <string.h>
#include <assert.h>

/* private variables */

static DeviceCtx_t dev;
static uint8_t     buf[BUF_SIZE] = {0};
static ClientCtx_t clients[CLT_CNT] = {0};

/* private functions */

static void stream_handler(void);
static void gateway_state_machine(void);

/* functions implementations */

void snpes_compute(void)
{
	/* check if the gateway dev struct was initialized */
	if (dev.network_id == GATEWAY) {
		stream_handler();
		gateway_state_machine();
	}
}

void snpes_init(uint8_t uid, LoraItf_t *lora, TimerItf_t *timer)
{
	/* avoid null address */
	assert(lora);
	assert(timer);

	/* initialize device info */
	dev.unique_id = uid;
	dev.network_id = GATEWAY;
	dev.type = GATEWAY; // TODO: there is probably no need for this data
	dev.hw.socket = lora;
	dev.hw.timer = timer;

	/* protocol data streams config */
	dev.stream_in.start_addr = (void *)buf;
	dev.stream_in.elmt_size = (uint8_t)PKT_SIZE;
	dev.stream_in.elmt_cnt = (uint8_t)S_IN_CNT;
	dev.stream_out.start_addr = (void *)(buf+(PKT_SIZE*S_IN_CNT));
	dev.stream_out.elmt_size = (uint8_t)PKT_SIZE;
	dev.stream_out.elmt_cnt = (uint8_t)S_OUT_CNT;

	/* initialize data streams */
	queue_init(&dev.stream_in);
	queue_init(&dev.stream_out);

	/* clear the clients buffer */
	memset(clients, 0, sizeof(ClientCtx_t)*CLT_CNT);

	/* initialize the LoRa device ID */
	dev.hw.socket->set_id(dev.unique_id);
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
	}

	/* if there is some packet to send, send it */
	if (!queue_empty(&dev.stream_out)) {
		src = (Packet_t *) queue_pop(&dev.stream_out);
		dev.hw.socket->pkt_send(src->dest_nid, (uint8_t *)src, (src->data_size + META_SIZE));
	}
}

static void gateway_state_machine()
{
	GwStates_t state; // TODO: maybe change the type to CltStates
	Packet_t *pkt = NULL;
	ClientCtx_t *clt = NULL;
	uint8_t new_nid;

	/* figure out the protocol state */

	/* Always give priority to packets sitting in the stream in queue */
	if (!queue_empty(&dev.stream_in)) {
		pkt = (Packet_t *) queue_pop(&dev.stream_in);
		/* if the packet isn't from a client */
		if (pkt->src_nid == NODE) {
			/* if it's not yet a client it can only send SCANs and SYNCs */
			if (get_pkt_type(pkt) == SCAN) {
				state = SEND_INFO;
			}
			else if (get_pkt_type(pkt) == SYNC && pkt->dest_uid == dev.unique_id) {
				state = RESP_SYNC;
			}
			else {
				/* probably a corrupted packet, just ignore it */
				return;
			}
		}
		/* it's *probably* a client */
		else {
			clt = get_client_ctx(clients, pkt->src_nid);
			if (clt->connected != NOT_CONNETED) {
				/* it's *actually* a client */
				state = clt->state;
			}
			else {
				/* that's not a real client, ignore it */
				return;
			}
		}
	}
	/* if there isn't packets inputs, check for clients in the waiting state */
	else if ((clt = get_waiting_client(clients)) != NULL) {
		state = clt->state;
	}
	/* if there is nothing to do, just leave */
	else {
		return;
	}

	/* Now that we have a state, let's process */

	switch (state) {
	case SEND_INFO:
		enqueue_signal(&dev, INFO, pkt->src_uid, pkt->src_nid);
		break;
	case RESP_SYNC:
		new_nid = alloc_nid(clients);
		/* if there isn't free network IDs */
		if (new_nid == 0) {
			enqueue_signal(&dev, FULL, pkt->src_uid, pkt->src_nid);
		}
		/* else, setup to waiting ack */
		else {
			clt = get_client_ctx(clients, new_nid);
			clt->unique_id = pkt->src_uid;
			clt->timeout = 0;
			clt->state = WAIT_ACK;
			clt->timer_ref = dev.hw.timer->millis();
			clt->waiting = 1;
			enqueue_data(&dev, clt->unique_id, pkt->src_nid, 0x00, &new_nid, sizeof(uint8_t));
		}
		break;
	case WAIT_ACK:
		/* check if we are here because maybe there is a ACK packet */
		if (pkt != NULL) {
			if (get_pkt_type(pkt) == ACK) {
				/* update client info as connected */
				clt->connected = CONNECTED;
				clt->state = IDLE;
				clt->waiting = 0;
				clt->timeout = 0;
				clt->timer_ref = 0;
			}
		}
		/* *maybe* we're here because we're still waiting an ACK from a client */
		else if (clt != NULL) {
			if ((dev.hw.timer->millis() - clt->timer_ref) >= TIMEOUT_THLD) {
				/* if there is no response till now, and we reached a maximum amount of trys */
				if (++clt->timeout == MAX_TIMEOUT_CNT) {
					/* just deallocate the Network ID */
					free_nid(clients, clt->network_id);
				}
				/* else, try again */
				else {
					clt->timer_ref = dev.hw.timer->millis();
					enqueue_data(&dev, clt->unique_id, NODE, 0x00, &(clt->network_id), sizeof(uint8_t));
				}
			}
		}
		break;
	case IDLE:
		break;
	default:
		break;
	}
}
