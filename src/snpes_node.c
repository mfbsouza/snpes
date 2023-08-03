#include "snpes_node.h"
#include "snpes_utils.h"
#include "snpes_types.h"
#include <assert.h>
#include <string.h>

/* private variables */

static uint8_t unique_id = 0;
static uint8_t network_id = 0;
static HwCtx_t hw;
static Packet_t buf;

/* private functions */

static SnpesStatus_t wait_signal(PacketType_t signal);

/* functions implementations */

void snpes_node_init(uint8_t uid, LoraItf_t *lora, TimerItf_t *timer)
{
	/* avoid null pointers */
	assert(lora);
	assert(timer);

	/* initialize device info */
	unique_id = uid;
	network_id = NODE;
	hw.socket = lora;
	hw.timer = timer;

	/* initialize the LoRa device ID */
	hw.socket->set_id(network_id);
}

SnpesStatus_t snpes_scan(uint8_t *gateway_uid)
{
	uint8_t timeout_cnt = 0;

	/* check if the protocol was initialized */
	if (hw.socket == NULL)
		return SNPES_ERROR;

	while (timeout_cnt < MAX_TIMEOUT_CNT) {
		/* build a SCAN packet and send it */
		build_signal(&buf, SCAN, unique_id, network_id, 0x00, GATEWAY,
			     timeout_cnt, 0);
		hw.socket->pkt_send(buf.dest_nid, (uint8_t *)&buf, META_SIZE);
		/* wait for a gateway response */
		if (wait_signal(INFO) == SNPES_OK) {
			*gateway_uid = buf.src_uid;
			return SNPES_OK;
		}
		/* else, no packet was recived */
		timeout_cnt++;
	}
	return SNPES_ERROR;
}

SnpesStatus_t snpes_connect(uint8_t gateway_uid)
{
	uint8_t recv_nid, recv_size;
	uint8_t timeout_cnt = 0;
	uint32_t timer_ref;

	/* check if the protocol was initialized */
	if (hw.socket == NULL)
		return SNPES_ERROR;

	while (timeout_cnt < MAX_TIMEOUT_CNT) {
		/* build a SYNC packet and send it */
		build_signal(&buf, SYNC, unique_id, network_id, gateway_uid,
			     GATEWAY, timeout_cnt, 0);
		hw.socket->pkt_send(buf.dest_nid, (uint8_t *)&buf, META_SIZE);
		timer_ref = hw.timer->millis();

		while ((hw.timer->millis() - timer_ref) <= TIMEOUT_THLD) {
			/* wait until there is some packet availible */
			if (hw.socket->pkt_avail()) {
				/* read the availible packet */
				hw.socket->pkt_recv(&recv_nid, (uint8_t *)&buf,
						    &recv_size);
				/* if it's the signal we're looking for */
				if (get_pkt_type(&buf) == DATA) {
					/* save the new network id */
					network_id = buf.data[0];
					/* update the LoRa ID */
					hw.socket->set_id(network_id);
					/* send ACK to the gateway */
					build_signal(&buf, ACK, unique_id,
						     network_id, gateway_uid,
						     buf.src_nid, 0x0, 0);
					hw.socket->pkt_send(buf.dest_nid,
							    (uint8_t *)&buf,
							    META_SIZE);
					/* now we can safely return */
					return SNPES_OK;
				} else if (get_pkt_type(&buf) == FULL) {
					return SNPES_ERROR;
				}
				/* else, just ignore it */
			}
		}
		timeout_cnt++;
	}
	return SNPES_ERROR;
}

SnpesStatus_t snpes_send(uint8_t dest_uid, const void *src, uint8_t size)
{
	assert(src);
	assert(size > 0);

	SnpesStatus_t ret = SNPES_ERROR;
	uint8_t recv_nid, recv_size;
	uint8_t timeout_cnt = 0;
	uint32_t timer_ref = 0;
	/* check if connected */
	if (network_id != NODE && network_id != 0x00) {
		/* ask for permission to send data */
		while (timeout_cnt < MAX_TIMEOUT_CNT) {
			/* build a TRANS_START packet and send it */
			build_signal(&buf, TRANS_START, unique_id, network_id,
				     dest_uid, GATEWAY,
				     ((MAX_PKT_CNT << 2) | (timeout_cnt & 3)),
				     0);
			buf.data_size = size;
			hw.socket->pkt_send(buf.dest_nid, (uint8_t *)&buf,
					    META_SIZE);
			timer_ref = hw.timer->millis();
			while ((hw.timer->millis() - timer_ref) <=
			       TIMEOUT_THLD) {
				/* wait until there is some packet availible */
				if (hw.socket->pkt_avail()) {
					/* read the availible packet */
					hw.socket->pkt_recv(&recv_nid,
							    (uint8_t *)&buf,
							    &recv_size);
					/* if it's the signal we're looking for */
					if (get_pkt_type(&buf) == TRANS_START) {
						/* get out of this nested loops */
						goto SEND_DATA;
					} else if (get_pkt_type(&buf) == FULL) {
						return SNPES_ERROR;
					}
					/* else, just ignore it */
				}
			}
			timeout_cnt++;
		}
	}
	return ret;

SEND_DATA:
	if (get_pkt_type(&buf) == TRANS_START) {
		timeout_cnt = 0;
		while (timeout_cnt < MAX_TIMEOUT_CNT) {
			/* build the data packet */
			build_data(&buf, unique_id, network_id, dest_uid,
				   GATEWAY, MAX_PKT_CNT, src, size);
			hw.socket->pkt_send(buf.dest_nid, (uint8_t *)&buf,
					    META_SIZE + size);
			timer_ref = hw.timer->millis();
			while ((hw.timer->millis() - timer_ref) <=
			       TIMEOUT_THLD) {
				if (hw.socket->pkt_avail()) {
					hw.socket->pkt_recv(&recv_nid,
							    (uint8_t *)&buf,
							    &recv_size);
					if (get_pkt_type(&buf) == ACK) {
						return SNPES_OK;
					} else if (get_pkt_type(&buf) ==
						   TRANS_RETRY) {
						break;
					}
					/* else, just ignore it */
				}
			}
			timeout_cnt++;
		}
	}
	return ret;
}

static SnpesStatus_t wait_signal(PacketType_t signal)
{
	SnpesStatus_t ret = SNPES_ERROR;
	uint8_t recv_nid, recv_size;
	uint32_t timer_ref = hw.timer->millis();

	while ((hw.timer->millis() - timer_ref) <= TIMEOUT_THLD) {
		/* wait util there is some packet availible */
		if (hw.socket->pkt_avail()) {
			/* read the availible packet */
			hw.socket->pkt_recv(&recv_nid, (uint8_t *)&buf,
					    &recv_size);
			/* if it's the signal we're looking for */
			if (get_pkt_type(&buf) == signal) {
				ret = SNPES_OK;
				break;
			}
			/* else, just ignore it */
		}
	}
	return ret;
}
