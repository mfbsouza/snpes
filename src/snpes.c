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
