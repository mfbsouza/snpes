#include "CircularQueue.h"
#include <string.h>

void queue_init(Queue_t *queue_ctx)
{
	queue_ctx->head = -1;
	queue_ctx->tail = -1;
}

int8_t queue_push(Queue_t *queue_ctx, void *elmt_addr)
{
	int8_t ret = 0;
	void *dest_addr = NULL;

	/* if queue is empty */
	if (queue_ctx->head == -1 && queue_ctx->tail == -1) {
		queue_ctx->head = queue_ctx->tail = 0;
		memcpy(queue_ctx->start_addr, elmt_addr, queue_ctx->elmt_size);
	}
	/* if queue is full */
	else if (((queue_ctx->head+1) % queue_ctx->elmt_cnt) == queue_ctx->tail) {
		ret = -1;
	}
	else {
		queue_ctx->head = (queue_ctx->head+1) % queue_ctx->elmt_cnt;
		dest_addr = (void *)((uint8_t *)queue_ctx->start_addr + queue_ctx->head*queue_ctx->elmt_size);
		memcpy(dest_addr, elmt_addr, queue_ctx->elmt_size);
	}
	return ret;
}

void* queue_pop(Queue_t *queue_ctx)
{
	void *ret = NULL;

	/* if queue is empty */
	if (queue_ctx->head == -1 && queue_ctx->tail == -1) {
		ret = NULL;
	}
	/* if is the last element */
	else if (queue_ctx->tail == queue_ctx->head) {
		ret = (void *)((uint8_t *)queue_ctx->start_addr + queue_ctx->tail*queue_ctx->elmt_size);
		queue_ctx->tail = queue_ctx->head = -1;
	}
	else {
		ret = (void *)((uint8_t *)queue_ctx->start_addr + queue_ctx->tail*queue_ctx->elmt_size);
		queue_ctx->tail = (queue_ctx->tail+1) % queue_ctx->elmt_cnt;
	}
	return ret;
}

uint8_t queue_empty(Queue_t *queue_ctx)
{
	uint8_t ret = 0;
	if (queue_ctx->head == -1 && queue_ctx->tail == -1) {
		ret = 1;
	}
	return ret;
}
