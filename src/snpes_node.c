#include "snpes_node.h"
#include "snpes_cfg.h"
#include "snpes_utils.h"
#include "snpes_types.h"
#include <assert.h>
#include <string.h>

/* private variables */

static uint8_t unique_id = 0;
static uint8_t network_id = 0;
static HwCtx_t hw;
static Packet_t buf;
static Packet_t in_pkts[MAX_PKT_CNT];
static uint8_t in_pkts_cnt = 0;

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
			// TODO: 1 << 2: 1 should be how many packets are we sending
			build_signal(&buf, TRANS_START, unique_id, network_id,
				     dest_uid, GATEWAY,
				     ((1 << 2) | (timeout_cnt & 3)), 0);
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
			// TODO: 1 should be the seq number
			build_data(&buf, unique_id, network_id, dest_uid,
				   GATEWAY, 1, src, size);
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

void snpes_node_sync(uint8_t gateway_uid)
{
	uint8_t recv_nid, recv_size;
	uint8_t timeout_cnt = 0;
	uint8_t incoming_pkts = 0;
	uint8_t expected_pkt = 1;

	if (hw.socket->pkt_avail()) {
		hw.socket->pkt_recv(&recv_nid, (uint8_t *)&buf, &recv_size);
		if (get_pkt_type(&buf) == ALIVE) {
			/* send ACK signal to the gateway */
			build_signal(&buf, ACK, unique_id, network_id,
				     gateway_uid, buf.src_nid, 0x0, 0);
			hw.socket->pkt_send(buf.dest_nid, (uint8_t *)&buf,
					    META_SIZE);
		} else if (get_pkt_type(&buf) == TRANS_START) {
			incoming_pkts = (((buf.flgs_seq & 0x0F) >> 2) & 3);
			if (0 == in_pkts_cnt && incoming_pkts <= MAX_PKT_CNT) {
				/* send TRANS_START back */
				build_signal(&buf, TRANS_START, unique_id,
					     network_id, gateway_uid,
					     buf.src_nid, 0x0, 0);
				hw.socket->pkt_send(buf.dest_nid,
						    (uint8_t *)&buf, META_SIZE);
				/* capture the data */
				while (0 != incoming_pkts &&
				       MAX_TIMEOUT_CNT > timeout_cnt) {
					/* wait for data packet */
					if (wait_signal(DATA) == SNPES_OK &&
					    (buf.flgs_seq & 0x0F) ==
						    expected_pkt) {
						memcpy(&in_pkts[(expected_pkt -
								 1)],
						       &buf, sizeof(Packet_t));
						expected_pkt += 1;
						incoming_pkts -= 1;
						in_pkts_cnt += 1;
						timeout_cnt = 0;
						/* send a data ack */
						build_signal(
							&buf, ACK, unique_id,
							network_id, gateway_uid,
							buf.src_nid, 0x0, 0);
						hw.socket->pkt_send(
							buf.dest_nid,
							(uint8_t *)&buf,
							META_SIZE);
					} else {
						timeout_cnt += 1;
					}
				}
				/* check if all the packets were stored */
				if (0 != incoming_pkts)
					in_pkts_cnt = 0;
			} else {
				/* send FULL signal to the gateway */
				build_signal(&buf, FULL, unique_id, network_id,
					     gateway_uid, buf.src_nid, 0x0, 0);
				hw.socket->pkt_send(buf.dest_nid,
						    (uint8_t *)&buf, META_SIZE);
			}
		}
	}
}

uint8_t snpes_node_available_data()
{
	uint8_t bytes = 0;
	uint8_t ii = 0;
	while (ii < in_pkts_cnt) {
		bytes += in_pkts[ii].data_size;
		ii++;
	}
	return bytes;
}

uint8_t snpes_node_read(void *dest, uint8_t amount)
{
	uint8_t copied = 0;
	uint8_t loop = 0;
	uint8_t ii = 0;
	uint8_t total_bytes = snpes_node_available_data();
	if (amount <= total_bytes) {
		loop = amount / MAX_PKT_DATA_SIZE;
		loop += ((amount % MAX_PKT_DATA_SIZE) != 0) ? 1 : 0;
		while (loop != 0) {
			if (loop > 1) {
				memcpy(((uint8_t *)dest + copied),
				       &in_pkts[ii].data, MAX_PKT_DATA_SIZE);
				copied += MAX_PKT_DATA_SIZE;
				amount -= MAX_PKT_DATA_SIZE;
			} else {
				memcpy(((uint8_t *)dest + copied),
				       &in_pkts[ii].data, amount);
				copied += amount;
			}
			ii++;
			loop--;
		}
	}
	return copied;
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
