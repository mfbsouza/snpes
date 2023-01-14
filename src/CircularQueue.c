#include "CircularQueue.h"
#include <string.h>
#include <assert.h>

void queue_init(Queue_t *queue_ctx, void *start, uint8_t elmt_size,
		uint8_t elmt_cnt)
{
	assert(queue_ctx);
	assert(start);
	assert(elmt_size > 0);
	assert(elmt_cnt > 0);

	queue_ctx->start_addr = start;
	queue_ctx->elmt_size = elmt_size;
	queue_ctx->elmt_cnt = elmt_cnt;
	queue_ctx->head = -1;
	queue_ctx->tail = -1;
}

void *queue_alloc(Queue_t *queue_ctx)
{
	assert(queue_ctx);
	void *dest_addr = NULL;

	/* if queue is empty */
	if (queue_empty(queue_ctx)) {
		queue_ctx->head = queue_ctx->tail = 0;
		dest_addr = queue_ctx->start_addr;
	}
	/* if queue is full */
	else if (queue_full(queue_ctx)) {
		dest_addr = NULL;
	} else {
		queue_ctx->head = (queue_ctx->head + 1) % queue_ctx->elmt_cnt;
		dest_addr = (void *)((uint8_t *)queue_ctx->start_addr +
				     queue_ctx->head * queue_ctx->elmt_size);
	}
	return dest_addr;
}

int8_t queue_push(Queue_t *queue_ctx, void *elmt_addr)
{
	assert(queue_ctx);
	assert(elmt_addr);
	int8_t ret = -1;
	void *dest_addr = NULL;

	dest_addr = queue_alloc(queue_ctx);

	if (dest_addr) {
		memcpy(dest_addr, elmt_addr, queue_ctx->elmt_size);
		ret = 0;
	}
	return ret;
}

void *queue_pop(Queue_t *queue_ctx)
{
	assert(queue_ctx);
	void *ret = NULL;

	/* if queue is empty */
	if (queue_empty(queue_ctx)) {
		ret = NULL;
	}
	/* if is the last element */
	else if (queue_ctx->tail == queue_ctx->head) {
		ret = (void *)((uint8_t *)queue_ctx->start_addr +
			       queue_ctx->tail * queue_ctx->elmt_size);
		queue_ctx->tail = queue_ctx->head = -1;
	} else {
		ret = (void *)((uint8_t *)queue_ctx->start_addr +
			       queue_ctx->tail * queue_ctx->elmt_size);
		queue_ctx->tail = (queue_ctx->tail + 1) % queue_ctx->elmt_cnt;
	}
	return ret;
}

void *queue_peek(Queue_t *queue_ctx)
{
	assert(queue_ctx);
	void *ret = NULL;

	/* if queue is empty */
	if (queue_empty(queue_ctx)) {
		ret = NULL;
	} else {
		ret = (void *)((uint8_t *)queue_ctx->start_addr +
			       queue_ctx->tail * queue_ctx->elmt_size);
	}
	return ret;
}

uint8_t queue_empty(Queue_t *queue_ctx)
{
	assert(queue_ctx);
	uint8_t ret = 0;
	if (queue_ctx->head == -1 && queue_ctx->tail == -1) {
		ret = 1;
	}
	return ret;
}

uint8_t queue_full(Queue_t *queue_ctx)
{
	assert(queue_ctx);
	uint8_t ret = 0;
	if (((queue_ctx->head + 1) % queue_ctx->elmt_cnt) == queue_ctx->tail) {
		ret = 1;
	}
	return ret;
}

uint8_t queue_remaining(Queue_t *queue_ctx)
{
	uint8_t ret = 0;
	int32_t interator = queue_ctx->head;

	if (queue_empty(queue_ctx)) {
		ret = queue_ctx->elmt_cnt;
	} else if (queue_full(queue_ctx)) {
		ret = 0;
	} else {
		while (((interator + 1) % queue_ctx->elmt_cnt) !=
		       queue_ctx->tail) {
			ret++;
			interator++;
		}
	}
	return ret;
}
