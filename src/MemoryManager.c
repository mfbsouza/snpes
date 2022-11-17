#include "MemoryManager.h"
#include <string.h>
#include <assert.h>

#define IS_ALLOCATED(x) (*(x) & 1)
#define GET_SIZE(x) ((uint8_t)(*(x) & -2))

int8_t memmgr_init(MemMgr_t *mem_ctx, void *start, uint8_t size)
{
	assert(mem_ctx);
	assert(start);
	assert(size > 0);

	/* check if the size is divisible by 4 */
	if (size % BYTE_ALIGN != 0) return -1;

	/* save the heap start and end to the context */
	mem_ctx->heap_start = start;
	mem_ctx->heap_end = (uint8_t *)start + size;

	/* initialize the memory as a single free block of chunk */
	*(uint8_t *)mem_ctx->heap_start = size;
	*((uint8_t *)mem_ctx->heap_end - HEADER_SIZE) = size;

	return 0;
}

void* memmgr_alloc(MemMgr_t *mem_ctx, uint8_t size)
{
	assert(mem_ctx);
	assert(size > 0);

	void *ret = NULL;
	uint8_t *ptr = (uint8_t *) mem_ctx->heap_start;

	/* compute how many bytes we will need to alloc */
	size += METADATA_SIZE;
	if (size % BYTE_ALIGN != 0) {
		size = (uint8_t)(size + BYTE_ALIGN - (size % BYTE_ALIGN));
	}

	/* look for free chunk that's big enough for what we want */
	while (ptr < (uint8_t *)mem_ctx->heap_end &&
		(IS_ALLOCATED(ptr) || GET_SIZE(ptr) < size))
		ptr += GET_SIZE(ptr);
	
	/* check if we found a valid chunk to allocate */
	if (ptr < (uint8_t *)mem_ctx->heap_end) {
		/* check if we need to break the chunk in two */
		if (size < GET_SIZE(ptr)) {
			/* update the new free chunk header */
			*(ptr + size) = GET_SIZE(ptr) - size;
			/* update the new free chunk footer */
			*(ptr + (GET_SIZE(ptr) - HEADER_SIZE)) = GET_SIZE(ptr) - size;
		}
		/* update the new allocated chunk's foot and header */
		*(ptr + (size - HEADER_SIZE)) = size + 1;
		*ptr = size + 1;
		ret = (void *)(ptr + HEADER_SIZE);
	}

	return ret;
}

void memmgr_free(MemMgr_t *mem_ctx, void *addr)
{
	assert(mem_ctx);
	assert(addr);
	
	uint8_t *ptr = (uint8_t *)addr;

	/* confirm that the address is inside our heap */
	if (ptr >= (uint8_t *)mem_ctx->heap_end ||
		ptr <= (uint8_t *)mem_ctx->heap_start)
		return;

	/* free the chunk */
	ptr -= (uint8_t)HEADER_SIZE;
	*ptr = GET_SIZE(ptr);
	*(ptr + GET_SIZE(ptr) - HEADER_SIZE) = GET_SIZE(ptr);

	/* merge free neighbors chunk into one */
	if (ptr > (uint8_t *)mem_ctx->heap_start && !IS_ALLOCATED((ptr-FOOTER_SIZE))) {
		/* update the new free chunk header */
		*(ptr - GET_SIZE((ptr-FOOTER_SIZE))) = GET_SIZE(ptr) + GET_SIZE(ptr-FOOTER_SIZE);
		/* update the new free chunk footer */
		*(ptr + (GET_SIZE(ptr) - HEADER_SIZE)) = GET_SIZE(ptr) + GET_SIZE(ptr-FOOTER_SIZE);
		/* update pointer the the new free chunk */
		ptr -= GET_SIZE((ptr-FOOTER_SIZE));
	}
	if (ptr < (uint8_t *)mem_ctx->heap_end && !IS_ALLOCATED((ptr+GET_SIZE(ptr)))) {
		/* update the new free chunk footer */
		*(ptr + GET_SIZE(ptr) + GET_SIZE(ptr + GET_SIZE(ptr)) - HEADER_SIZE)
			= GET_SIZE(ptr) + GET_SIZE(ptr + GET_SIZE(ptr));
		/* update the new free chunk header */
		*ptr = GET_SIZE(ptr) + GET_SIZE(ptr + GET_SIZE(ptr));
	}
}

uint8_t memmgr_remaining(MemMgr_t *mem_ctx)
{
	uint8_t ret = 0;
	uint8_t *ptr = (uint8_t *)mem_ctx->heap_start;

	while (ptr < (uint8_t *)mem_ctx->heap_end) {
		if (!IS_ALLOCATED(ptr)) ret += GET_SIZE(ptr);
		ptr += GET_SIZE(ptr);
	}

	return ret;
}
