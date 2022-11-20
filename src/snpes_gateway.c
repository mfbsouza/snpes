#include "snpes_gateway.h"
#include "snpes_cfg.h"
#include "snpes_types.h"
#include "snpes_utils.h"
#include "CircularQueue.h"
#include "MemoryManager.h"
#include <string.h>
#include <assert.h>

/* private variables */

static DeviceCtx_t dev;
static uint8_t     buf[BUF_SIZE] = {0};
static uint8_t     heap[HEAP_SIZE] = {0};
static ClientCtx_t clients[CLT_CNT] = {0};

/* private functions */

static void stream_handler(void);
static void alive_checker(void);
static void gateway_state_machine(void);

/* functions implementations */

void snpes_compute(void)
{
	/* check if the gateway dev struct was initialized */
	if (dev.network_id == GATEWAY) {
		stream_handler();
		gateway_state_machine();
		alive_checker();
	}
}

void snpes_gw_init(uint8_t uid, LoraItf_t *lora, TimerItf_t *timer)
{
	/* avoid null address */
	assert(lora);
	assert(timer);

	/* initialize device info */
	dev.unique_id = uid;
	dev.network_id = GATEWAY;
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

	/* initialize Memory Manager */
	memmgr_init(&dev.mem, (void *)heap, (uint16_t)HEAP_SIZE);

	/* clear the clients buffer */
	memset(clients, 0, sizeof(ClientCtx_t)*CLT_CNT);

	/* initialize the LoRa device ID */
	dev.hw.socket->set_id(dev.network_id);
}

uint16_t snpes_data_available(void)
{
	uint16_t ret = 0;
	ClientCtx_t *clt = NULL;
	if ((clt = get_data_avail_client(clients)) != NULL) {
		ret = clt->data_size;
	}
	return ret;
}

void snpes_read(uint8_t *clt_uid, void *dest, uint16_t *size)
{
	assert(clt_uid);
	assert(dest);
	assert(size);
	ClientCtx_t *clt = NULL;

	if ((clt = get_data_avail_client(clients)) != NULL) {
		/* copy the data to the user */
		memcpy(dest, clt->data, clt->data_size);
		*size = clt->data_size;
		*clt_uid = clt->unique_id;
		/* free the memory */
		memmgr_free(&dev.mem, clt->data);
		/* send the client back to the idle state */
		clt->state = IDLE;
	}
}

static void stream_handler()
{
	uint8_t nid, size;
	void *dest = NULL;
	Packet_t *src = NULL;

	/* if there is some packet availiable, save it */
	// TODO: this maybe should be a "while" instead of a "if"
	if (dev.hw.socket->pkt_avail() && !queue_full(&dev.stream_in)) {
		dest = queue_alloc(&dev.stream_in);
		dev.hw.socket->pkt_recv(&nid, (uint8_t *)dest, &size);
	}

	/* if there is some packet to send, send it */
	if (!queue_empty(&dev.stream_out)) {
		src = (Packet_t *) queue_pop(&dev.stream_out);
		dev.hw.socket->pkt_send(src->dest_nid, (uint8_t *)src, (src->data_size + META_SIZE));
	}
}

static void alive_checker()
{
	ClientCtx_t *clt = NULL;
	static uint8_t signal_pkt[META_SIZE] = {0};

	/* look for a idle client */
	for (int ii = 0; ii < CLT_CNT; ii++) {
		/* if it's a connect client check how many seconds has passed since the last communication */
		if (clients[ii].connected == CONNECTED && ((uint32_t)(dev.hw.timer->millis()/1000) - clients[ii].alive_ref) >= ALIVE_THLD) {
			/* is this the first time im testing this client? */
			if (clients[ii].timer_ref != 0) {
				/* well, it's not, but should i send another ALIVE signal? */
				if ((dev.hw.timer->millis() - clients[ii].timer_ref) >= TIMEOUT_THLD) {
					if(++clients[ii].timeout_cnt == MAX_TIMEOUT_CNT) {
						/* to many tries already, the client must be dead */
						free_nid(clients, clients[ii].network_id);
					}
					else {
						/* yeh, i should send another one */
						clt = &(clients[ii]);
						break;
					}
				}
				/* no, i shouldn't, just keep looking for another client */
			}
			/* it is */
			else {
				clt = &(clients[ii]);
				break;
			}
		}
	}

	/* send a ALIVE signal */
	// TODO: maybe this module shouldn't send the Packet directly using the hw, instead it should enqueue the signal in the stream output
	if (clt != NULL) {
		build_signal((Packet_t *)signal_pkt, ALIVE, dev.unique_id, dev.network_id, clt->unique_id, clt->network_id, clt->timeout_cnt);
		clt->timer_ref = dev.hw.timer->millis();
		dev.hw.socket->pkt_send(((Packet_t *)signal_pkt)->dest_nid, signal_pkt, META_SIZE);
	}
}

static void gateway_state_machine()
{
	GwStates_t state; // TODO: maybe change the type to CltStates
	Packet_t *pkt = NULL;
	ClientCtx_t *clt = NULL;
	uint8_t new_nid;

	/* figure out the protocol state */

	/* Always give priority to packets sitting in the stream input queue */
	if (!queue_empty(&dev.stream_in)) {
		pkt = (Packet_t *) queue_pop(&dev.stream_in);
		/* if the packet isn't from a client */
		if (pkt->src_nid == NODE) {
			/* if it's not yet a client it can only send SCANs and SYNCs */
			if (get_pkt_type(pkt) == SCAN) {
				state = SEND_INFO;
			}
			else if (get_pkt_type(pkt) == SYNC && pkt->dest_uid == dev.unique_id) {
				state = RECV_SYNC;
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
				/**
				 * TODO: This feels like a fault: if the gateway gets a ACK signal
				 * and if the node that send this signal is in the client list, it
				 * just send this client to the RECV_ACK state without checking the
				 * previous state of the connection.
				 * it's doing this because the ACK can influence the WAIT_ACK state
				 * and also the Alive Checker block, so by doing this way it can
				 * handle both cases.
				 * */
				if (get_pkt_type(pkt) == ACK) state = RECV_ACK;
				else if (get_pkt_type(pkt) == DATA && clt->state == WAIT_DATA) state = RECV_DATA;
				else state = clt->state;
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
	case RECV_SYNC:
		/* check if the SYNC request is the first or a retry */
		if (get_pkt_seq_number(pkt) != 0 && (clt = find_client_ctx(clients, pkt->src_uid)) != NULL) {
			new_nid = clt->network_id;
		}
		else {
			new_nid = alloc_nid(clients);
		}
		/* if there isn't free network IDs */
		if (new_nid == 0) {
			enqueue_signal(&dev, FULL, pkt->src_uid, pkt->src_nid);
		}
		/* else, setup to waiting ack */
		else {
			clt = get_client_ctx(clients, new_nid);
			clt->unique_id = pkt->src_uid;
			clt->timeout_cnt = 0;
			clt->state = WAIT_ACK;
			clt->data = NULL;
			clt->data_size = 0;
			clt->timer_ref = dev.hw.timer->millis();
			enqueue_data(&dev, clt->unique_id, pkt->src_nid, 0x00, &new_nid, sizeof(uint8_t));
		}
		break;
	case WAIT_ACK:
		if ((dev.hw.timer->millis() - clt->timer_ref) >= TIMEOUT_THLD) {
			/* if there is no response till now, and we reached a maximum amount of trys */
			if (++clt->timeout_cnt == MAX_TIMEOUT_CNT) {
				/* just deallocate the Network ID */
				free_nid(clients, clt->network_id);
			}
			/* else, try again */
			else {
				clt->timer_ref = dev.hw.timer->millis();
				enqueue_data(&dev, clt->unique_id, NODE, 0x00, &(clt->network_id), sizeof(uint8_t));
			}
		}
		break;
	case RECV_ACK:
		/* update client info as connected */
		clt->connected = CONNECTED;
		clt->state = IDLE;
		clt->timeout_cnt = 0;
		clt->timer_ref = 0;

		/* save the last time the gateway talked to this client in seconds */
		clt->alive_ref = (uint32_t)(dev.hw.timer->millis()/1000);
		break;
	case IDLE:
		/* check if it's a valid transmission start signal */
		if (get_pkt_type(pkt) == TRANS_START && ((get_pkt_seq_number(pkt)>>2)&3) <= MAX_PKT_CNT) {
			/* make sure we only allocate memory for this transmission once */
			// TODO: this ain't probably safe, we are trusting the client to send us a valid retry number
			if ((get_pkt_seq_number(pkt)&3) == 0) {
				clt->data = memmgr_alloc(&dev.mem, pkt->data_size);
			}
			/* check if there was free memory space to receive the client data */
			if (clt->data == NULL) {
				/* there wasn't */
				enqueue_signal(&dev, FULL, clt->unique_id, clt->network_id);
			}
			/* it's all good, tell the client it can start sending data */
			else {
				clt->state = WAIT_DATA;
				clt->timeout_cnt = 0;
				clt->data_size = 0;
				clt->pkt_cnt = ((get_pkt_seq_number(pkt)>>2)&3);
				clt->expected_pkt = 1;
				clt->timer_ref = dev.hw.timer->millis();
				enqueue_signal(&dev, TRANS_START, clt->unique_id, clt->network_id);
			}
		}

		/* save the last time the gateway talked to this client in seconds */
		clt->alive_ref = (uint32_t)(dev.hw.timer->millis()/1000);
		break;
	case WAIT_DATA:
		if ((dev.hw.timer->millis() - clt->timer_ref) >= TIMEOUT_THLD) {
			/* if there is no response till now, and we reached a maximum amount of trys */
			if (++clt->timeout_cnt == MAX_TIMEOUT_CNT) {
				/* send the client back to the idle state */
				// TODO: should i deallocate that client?
				memmgr_free(&dev.mem, clt->data);
				clt->timeout_cnt = 0;
				clt->timer_ref = 0;
				clt->data_size = 0;
				clt->pkt_cnt = 0;
				clt->expected_pkt = 0;
				clt->state = IDLE;
			}
			/* else, try again */
			else {
				clt->timer_ref = dev.hw.timer->millis();
				// TODO: send a retry signal with the missing packet
				enqueue_signal(&dev, TRANS_RETRY, clt->unique_id, clt->network_id);
			}
		}
		break;
	case RECV_DATA:
		/* check if the packet we received was the expected */
		if (get_pkt_seq_number(pkt) == clt->expected_pkt) {
			/* save the data */
			memcpy(((uint8_t *)clt->data + clt->data_size), pkt->data, pkt->data_size);
			clt->data_size += pkt->data_size;
			/* tell the client we received the data */
			enqueue_signal(&dev, ACK, clt->unique_id, clt->network_id);

			/* check if that was the last packet */
			if (++clt->expected_pkt > clt->pkt_cnt) {
				clt->state = DATA_AVAIL;
				clt->timeout_cnt = 0;
				clt->timer_ref = 0;
			}
		}

		/* save the last time the gateway talked to this client in seconds */
		clt->alive_ref = (uint32_t)(dev.hw.timer->millis()/1000);
		break;
	case DATA_AVAIL:
		/** 
		 * this is a locked state,
		 * technically the client shouldn't be able to do anything
		 * util the gateway user reads the availiable data
		 * */
		break;
	}
}
