#include "snpes_gateway.h"
#include "snpes_cfg.h"
#include "snpes_types.h"
#include "snpes_utils.h"
#include "CircularQueue.h"
#include "MemoryManager.h"
#include <stdint.h>
#include <string.h>
#include <assert.h>

/* private variables */

static DeviceCtx_t dev;
static uint8_t buf[BUF_SIZE] = { 0 }; // stream handler memory
static uint8_t heap[HEAP_SIZE] = { 0 }; // memory manager memory
static ClientCtx_t clients[CLT_CNT] = { 0 }; // client manager memory

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

	/* initialize data streams */
	queue_init(&dev.stream_in, buf, PKT_SIZE, S_IN_CNT);
	queue_init(&dev.stream_out, (buf + (PKT_SIZE * S_IN_CNT)), PKT_SIZE,
		   S_OUT_CNT);

	/* initialize Memory Manager */
	memmgr_init(&dev.mem, (void *)heap, (uint16_t)HEAP_SIZE);

	/* clear the clients buffer */
	memset(clients, 0, sizeof(ClientCtx_t) * CLT_CNT);

	/* initialize the LoRa device ID */
	dev.hw.socket->set_id(dev.network_id);
}

uint16_t snpes_data_available(void)
{
	uint16_t ret = 0;
	ClientCtx_t *clt = NULL;
	if ((clt = get_data_avail_client(clients)) != NULL) {
		ret = clt->recv_data_size;
	}
	return ret;
}

void snpes_read(uint8_t *clt_uid, void *dest, uint16_t *size)
{
	/* avoid null pointers */
	assert(clt_uid);
	assert(dest);
	assert(size);

	ClientCtx_t *clt = NULL;

	if ((clt = get_data_avail_client(clients)) != NULL) {
		/* copy the data to the user */
		memcpy(dest, clt->recv_data, clt->recv_data_size);
		*size = clt->recv_data_size;
		*clt_uid = clt->unique_id;
		/* free the memory */
		memmgr_free(&dev.mem, clt->recv_data);
		/* send the client back to the idle state */
		clt->state = IDLE;
	}
}

SnpesStatus_t snpes_write(uint8_t clt_uid, const void *src, uint16_t size)
{
	/* avoid null pointers */
	assert(src);

	ClientCtx_t *clt = NULL;
	SnpesStatus_t ret = SNPES_ERROR;

	if (MAX_USER_DATA_SIZE >= size &&
	    (clt = find_client_ctx(clients, clt_uid)) != NULL &&
	    (clt->send_data = memmgr_alloc(&dev.mem, size)) != NULL) {
		memcpy(clt->send_data, src, size);
		clt->send_data_size = size;
		clt->outgoing_pkt_cnt = (uint8_t)(size / MAX_PKT_DATA_SIZE);
		clt->outgoing_pkt_cnt += ((size % MAX_PKT_DATA_SIZE) != 0) ? 1 :
									     0;
		clt->out_expt_pkt = 1;
		ret = SNPES_OK;
	}
	return ret;
}

static void stream_handler()
{
	uint8_t nid, size;
	void *dest = NULL;
	Packet_t *src = NULL;

	/* if there is some packet availiable, save it */
	while (dev.hw.socket->pkt_avail() && !queue_full(&dev.stream_in)) {
		dest = queue_alloc(&dev.stream_in);
		dev.hw.socket->pkt_recv(&nid, (uint8_t *)dest, &size);
	}

	/* if there is some packet to send, send it */
	while (!queue_empty(&dev.stream_out)) {
		src = (Packet_t *)queue_pop(&dev.stream_out);
		dev.hw.socket->pkt_send(src->dest_nid, (uint8_t *)src,
					(src->data_size + META_SIZE));
	}
}

static void alive_checker()
{
	ClientCtx_t *clt = NULL;

	/* look for a idle client */
	for (int ii = 0; ii < CLT_CNT; ii++) {
		/* if it's a connect client check how many seconds has passed since
		 * the last communication */
		if (clients[ii].connected == CONNECTED &&
		    ((uint32_t)(dev.hw.timer->millis() / 1000) -
		     clients[ii].alive_ref) >= ALIVE_THLD) {
			/* is this the first time im testing this client? */
			if (clients[ii].timer_ref != 0) {
				/* well, it's not, but should i send another ALIVE signal? */
				if ((dev.hw.timer->millis() -
				     clients[ii].timer_ref) >= TIMEOUT_THLD) {
					if (++clients[ii].timeout_cnt ==
					    MAX_TIMEOUT_CNT) {
						/* to many tries already, the client must be dead */
						free_nid(
							clients,
							clients[ii].network_id);
					} else {
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
	if (clt != NULL) {
		enqueue_signal(&dev, ALIVE, clt->unique_id, clt->network_id,
			       clt->timeout_cnt, 0);
		clt->timer_ref = dev.hw.timer->millis();
	}
}

static void gateway_state_machine()
{
	States_t state;
	Packet_t *pkt = NULL;
	ClientCtx_t *clt = NULL;
	uint8_t new_nid;

	/* figure out the protocol state */

	/* Always give priority to packets sitting in the stream input queue */
	if (!queue_empty(&dev.stream_in)) {
		pkt = (Packet_t *)queue_pop(&dev.stream_in);
		/* if the packet isn't from a client */
		if (pkt->src_nid == NODE) {
			/* if it's not yet a client it can only send SCANs and SYNCs */
			if (get_pkt_type(pkt) == SCAN) {
				state = SEND_INFO;
			} else if (get_pkt_type(pkt) == SYNC &&
				   pkt->dest_uid == dev.unique_id) {
				state = RECV_SYNC;
			} else {
				/* probably a corrupted packet, just ignore it */
				return;
			}
		}
		/* it's *probably* a client */
		else {
			clt = get_client_ctx(clients, pkt->src_nid);
			if (clt->connected != NOT_CONNETED) {
				/* it's *actually* a client */
				if (get_pkt_type(pkt) == ACK &&
				    clt->state != WAIT_DATA_ACK)
					state = RECV_ACK;
				else if (get_pkt_type(pkt) == DATA &&
					 clt->state == WAIT_DATA)
					state = RECV_DATA;
				else if (get_pkt_type(pkt) == TRANS_START &&
					 clt->state == WAIT_TRANS_START)
					state = SEND_DATA;
				else if (get_pkt_type(pkt) == FULL &&
					 clt->state == WAIT_TRANS_START)
					state = RECV_FULL;
				else if (get_pkt_type(pkt) == ACK &&
					 clt->state == WAIT_DATA_ACK)
					state = SEND_DATA;
				else
					state = clt->state;
				/**
				 * TODO: This feels like a fault: if the gateway gets a ACK
				 * signal and if the node that send this signal is in the
				 * client list, it just send this client to the RECV_ACK state
				 * without checking the previous state of the connection.
				 * it's doing this because the ACK can influence the WAIT_ACK
				 * state and also the Alive Checker block, so by doing this
				 * way it can handle both cases.
				 * */
			} else {
				/* that's not a real client, ignore it */
				return;
			}
		}
	}
	/* if there isn't packets inputs, check if there is clients to send data */
	else if ((clt = get_rts_client(clients)) != NULL &&
		 clt->state == IDLE) {
		state = clt->state = SEND_TRANS_START;
	}
	/* if none of the above, check for clients in the waiting state */
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
		enqueue_signal(&dev, INFO, pkt->src_uid, pkt->src_nid, 0x0, 0);
		break;

	case RECV_SYNC:
		/* check if the SYNC request is the first or a retry */
		if (get_pkt_seq_number(pkt) != 0 &&
		    (clt = find_client_ctx(clients, pkt->src_uid)) != NULL) {
			new_nid = clt->network_id;
		} else {
			new_nid = alloc_nid(clients);
		}
		/* if there isn't free network IDs */
		if (new_nid == 0) {
			enqueue_signal(&dev, FULL, pkt->src_uid, pkt->src_nid,
				       0x0, 0);
		}
		/* else, setup to waiting ack */
		else {
			clt = get_client_ctx(clients, new_nid);
			clt->unique_id = pkt->src_uid;
			clt->timeout_cnt = 0;
			clt->state = WAIT_ACK;
			clt->recv_data = NULL;
			clt->recv_data_size = 0;
			clt->timer_ref = dev.hw.timer->millis();
			enqueue_data(&dev, clt->unique_id, pkt->src_nid, 0x00,
				     &new_nid, sizeof(uint8_t));
		}
		break;

	case WAIT_ACK:
		if ((dev.hw.timer->millis() - clt->timer_ref) >= TIMEOUT_THLD) {
			/* if there is no response till now, and we reached a maximum
			 * amount of trys */
			if (++clt->timeout_cnt == MAX_TIMEOUT_CNT) {
				/* just deallocate the Network ID */
				free_nid(clients, clt->network_id);
			}
			/* else, try again */
			else {
				clt->timer_ref = dev.hw.timer->millis();
				enqueue_data(&dev, clt->unique_id, NODE, 0x00,
					     &(clt->network_id),
					     sizeof(uint8_t));
			}
		}
		break;

	case RECV_ACK:
		/* update client info as connected */
		if (clt->state == WAIT_ACK) {
			clt->connected = CONNECTED;
			clt->state = IDLE;
		}

		/* reset the timer references */
		clt->timeout_cnt = 0;
		clt->timer_ref = 0;

		/* save the last time the gateway talked to this client in seconds */
		clt->alive_ref = (uint32_t)(dev.hw.timer->millis() / 1000);
		break;

	case IDLE:
		/* check if it's a valid transmission start signal */
		/* TODO: should do a "+1" before comparing to MAX_PKT_CNT */
		if (get_pkt_type(pkt) == TRANS_START &&
		    ((get_pkt_seq_number(pkt) >> 2) & 0b11) <= MAX_PKT_CNT) {
			/* make sure we only allocate memory for this transmission once */
			/* TODO: this ain't probably safe, we are trusting the client to 
			 * send us a valid retry number */
			if ((get_pkt_seq_number(pkt) & 3) == 0) {
				clt->recv_data =
					memmgr_alloc(&dev.mem, pkt->data_size);
			}
			/* check if there was free memory space to receive the
			 * client data */
			if (clt->recv_data == NULL) {
				/* there wasn't */
				enqueue_signal(&dev, FULL, clt->unique_id,
					       clt->network_id, 0x0, 0);
			}
			/* it's all good, tell the client it can start sending data */
			else {
				clt->state = WAIT_DATA;
				clt->timeout_cnt = 0;
				clt->recv_data_size = 0;
				clt->income_pkt_cnt =
					((get_pkt_seq_number(pkt) >> 2) & 3);
				clt->in_expt_pkt = 1;
				clt->timer_ref = dev.hw.timer->millis();
				enqueue_signal(&dev, TRANS_START,
					       clt->unique_id, clt->network_id,
					       0x0, 0);
			}
		}

		/* save the last time the gateway talked to this client in seconds */
		clt->alive_ref = (uint32_t)(dev.hw.timer->millis() / 1000);
		break;

	case WAIT_DATA:
		if ((dev.hw.timer->millis() - clt->timer_ref) >= TIMEOUT_THLD) {
			/* if there is no response till now, and we reached a maximum
			 * amount of trys */
			if (++clt->timeout_cnt == MAX_TIMEOUT_CNT) {
				/* send the client back to the idle state */
				// TODO: should i deallocate that client?
				memmgr_free(&dev.mem, clt->recv_data);
				clt->timeout_cnt = 0;
				clt->timer_ref = 0;
				clt->recv_data_size = 0;
				clt->income_pkt_cnt = 0;
				clt->in_expt_pkt = 0;
				clt->state = IDLE;
			}
			/* else, try again */
			else {
				clt->timer_ref = dev.hw.timer->millis();
				enqueue_signal(&dev, TRANS_RETRY,
					       clt->unique_id, clt->network_id,
					       clt->in_expt_pkt, 0);
			}
		}
		break;

	case RECV_DATA:
		/* check if the packet we received was the expected */
		if (get_pkt_seq_number(pkt) == clt->in_expt_pkt) {
			/* save the data */
			memcpy(((uint8_t *)clt->recv_data +
				clt->recv_data_size),
			       pkt->data, pkt->data_size);
			clt->recv_data_size += pkt->data_size;
			/* tell the client we received the data */
			enqueue_signal(&dev, ACK, clt->unique_id,
				       clt->network_id, 0x0, 0);

			/* check if that was the last packet */
			if (++clt->in_expt_pkt > clt->income_pkt_cnt) {
				clt->state = DATA_AVAIL;
				clt->timeout_cnt = 0;
				clt->timer_ref = 0;
			}
		}

		/* save the last time the gateway talked to this client in seconds */
		clt->alive_ref = (uint32_t)(dev.hw.timer->millis() / 1000);
		break;

	case SEND_TRANS_START:
		enqueue_signal(
			&dev, TRANS_START, clt->unique_id, clt->network_id,
			((clt->outgoing_pkt_cnt << 2) | (clt->timeout_cnt & 3)),
			clt->send_data_size);
		/* setup to waiting trans start ack */
		clt->timeout_cnt = 0;
		clt->timer_ref = dev.hw.timer->millis();
		clt->state = WAIT_TRANS_START;
		break;

	case WAIT_TRANS_START:
		if ((dev.hw.timer->millis() - clt->timer_ref) >= TIMEOUT_THLD) {
			/* if there is no response till now, and we reached a maximum
			 * amount of trys */
			if (++clt->timeout_cnt == MAX_TIMEOUT_CNT) {
				/* clear the data that we failed to send */
				/* send the client back to the idle state */
				// TODO: should i deallocate that client?
				memmgr_free(&dev.mem, clt->send_data);
				clt->timeout_cnt = 0;
				clt->timer_ref = 0;
				clt->send_data_size = 0;
				clt->outgoing_pkt_cnt = 0;
				clt->out_expt_pkt = 0;
				clt->state = IDLE;
			}
			/* else, try again */
			else {
				clt->timer_ref = dev.hw.timer->millis();
				enqueue_signal(&dev, TRANS_START,
					       clt->unique_id, clt->network_id,
					       ((clt->outgoing_pkt_cnt << 2) |
						(clt->timeout_cnt & 3)),
					       clt->send_data_size);
			}
		}
		break;

	case RECV_FULL:
		/* clear the data that we failed to send */
		/* send the client back to the idle state */
		// TODO: inform that we were unable to send the data
		memmgr_free(&dev.mem, clt->send_data);
		clt->timeout_cnt = 0;
		clt->timer_ref = 0;
		clt->send_data_size = 0;
		clt->outgoing_pkt_cnt = 0;
		clt->out_expt_pkt = 0;
		clt->state = IDLE;

		/* save the last time the gateway talked to this client in seconds */
		clt->alive_ref = (uint32_t)(dev.hw.timer->millis() / 1000);
		break;

	case SEND_DATA:
		/* check if we are here because of a data ack */
		if (pkt != NULL && get_pkt_type(pkt) == ACK) {
			clt->out_expt_pkt++;
			clt->outgoing_pkt_cnt--;
			clt->send_data_size -= MAX_PKT_DATA_SIZE;
			clt->timeout_cnt = 0;
			/* check if there is any more data to send */
			if (0 == clt->outgoing_pkt_cnt) {
				/* there isn't. free the memory and send it back to idle */
				memmgr_free(&dev.mem, clt->send_data);
				clt->timeout_cnt = 0;
				clt->timer_ref = 0;
				clt->send_data_size = 0;
				clt->outgoing_pkt_cnt = 0;
				clt->out_expt_pkt = 0;
				clt->state = IDLE;
				// TODO: don't like this in here
				clt->alive_ref =
					(uint32_t)(dev.hw.timer->millis() /
						   1000);
				break;
			}
		}
		/* create data packet to send */
		if (1 < clt->outgoing_pkt_cnt) {
			enqueue_data(
				&dev, clt->unique_id, clt->network_id,
				clt->out_expt_pkt,
				((uint8_t *)clt->send_data +
				 ((clt->out_expt_pkt - 1) * MAX_PKT_DATA_SIZE)),
				MAX_PKT_DATA_SIZE);
		} else {
			enqueue_data(
				&dev, clt->unique_id, clt->network_id,
				clt->out_expt_pkt,
				((uint8_t *)clt->send_data +
				 ((clt->out_expt_pkt - 1) * MAX_PKT_DATA_SIZE)),
				clt->send_data_size);
		}
		clt->state = WAIT_DATA_ACK;

		/* save the last time the gateway talked to this client in seconds */
		clt->alive_ref = (uint32_t)(dev.hw.timer->millis() / 1000);
		break;

	case WAIT_DATA_ACK:
		if ((dev.hw.timer->millis() - clt->timer_ref) >= TIMEOUT_THLD) {
			/* if there is no response till now, and we reached a maximum
			 * amount of trys */
			if (++clt->timeout_cnt == MAX_TIMEOUT_CNT) {
				/* clear the data that we failed to send */
				/* send the client back to the idle state */
				// TODO: should i deallocate that client?
				memmgr_free(&dev.mem, clt->send_data);
				clt->timeout_cnt = 0;
				clt->timer_ref = 0;
				clt->send_data_size = 0;
				clt->outgoing_pkt_cnt = 0;
				clt->out_expt_pkt = 0;
				clt->state = IDLE;
			}
			/* else, try again */
			else {
				clt->timer_ref = dev.hw.timer->millis();
				/* create data packet to send */
				if (1 < clt->outgoing_pkt_cnt) {
					enqueue_data(
						&dev, clt->unique_id,
						clt->network_id,
						clt->out_expt_pkt,
						((uint8_t *)clt->send_data +
						 ((clt->out_expt_pkt - 1) *
						  MAX_PKT_DATA_SIZE)),
						MAX_PKT_DATA_SIZE);
				} else {
					enqueue_data(
						&dev, clt->unique_id,
						clt->network_id,
						clt->out_expt_pkt,
						((uint8_t *)clt->send_data +
						 ((clt->out_expt_pkt - 1) *
						  MAX_PKT_DATA_SIZE)),
						clt->send_data_size);
				}
			}
		}
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
