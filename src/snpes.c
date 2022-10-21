#include "snpes.h"
#include "CircularQueue.h"
#include <string.h>
#include <assert.h>

typedef struct {
	uint8_t unique_id;
	uint8_t state;
	uint8_t connected;
} ClientCtx_t;

typedef struct {
	LoraItf_t *socket;
	TimerItf_t *timer;
} HwCtx_t;

typedef struct {
	uint8_t unique_id;
	uint8_t network_id;
	uint8_t type;
	HwCtx_t hw;
} DeviceCtx_t;

typedef struct {
	uint8_t src_uid;
	uint8_t src_nid;
	uint8_t dest_uid;
	uint8_t dest_nid;
	uint8_t flgs_seq;
	uint8_t data_size;
	uint8_t data[PKT_SIZE-META_SIZE];
} Packet_t;

/* private variables */

static  DeviceCtx_t  dev;
static  uint8_t      buf[BUF_SIZE] = {0};
static  ClientCtx_t  clients[CLT_CNT] = {0};

/* protocol data streams */
static Queue_t stream_in = {
	.start_addr = (void *)buf,
	.elmt_size = PKT_SIZE,
	.elmt_cnt = (uint8_t)S_IN_CNT
};
static Queue_t stream_out = {
	.start_addr = (void *)(buf+(PKT_SIZE*S_IN_CNT)),
	.elmt_size = PKT_SIZE,
	.elmt_cnt = (uint8_t)S_OUT_CNT
};

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

	/* initialize data streams */
	queue_init(&stream_in);
	queue_init(&stream_out);
}

static void stream_handler()
{
	uint8_t nid, size, temp[PKT_SIZE];
	void *src = NULL;

	/* if there is some packet availiable, save it */
	if (dev.hw.socket->pkt_avail()) {
		dev.hw.socket->pkt_recv(&nid, temp, &size);
		queue_push(&stream_in, temp);
	}

	/* if there is some packet to send, send it */
	if (!queue_empty(&stream_out)) {
		src = queue_pop(&stream_out);
		dev.hw.socket->pkt_send(((Packet_t *)src)->dest_nid, (uint8_t *)src, (((Packet_t *)src)->data_size + META_SIZE));
	}
}
