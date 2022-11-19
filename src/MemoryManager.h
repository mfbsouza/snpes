#ifndef __MEMORY_MANAGER_H__
#define __MEMORY_MANAGER_H__

#include <stdint.h>

#define BYTE_ALIGN 8
#define HEADER_SIZE 2
#define FOOTER_SIZE HEADER_SIZE
#define METADATA_SIZE HEADER_SIZE*2

/**
 * @brief The Memory Manager Context Struct contains information about the Fake Heap
 *
 * @param *heap_start: The start address of a continuous amount of memory
 * @param *heap_end: The last address of a continuous amount of memory
 */
typedef struct {
	void *heap_start;
	void *heap_end;
} MemMgr_t;

/**
 * @brief initializes a given memory region as a heap
 *
 * @param *mem_ctx: The MemMgr Context Struct
 * @param *start: The start address of a continuous amount of memory 
 * @param size: how many bytes this memory region has
 *
 * @retval 0 for no errors, -1 for not a byte aligned region
 */
int8_t memmgr_init(MemMgr_t *mem_ctx, void *start, uint16_t size);

/**
 * @brief allocates a continuous memory region
 *
 * @param *mem_ctx: The MemMgr Context Struct
 * @param size: how many bytes to allocate
 *
 * @retval the start address of the allocated memory region
 */
void* memmgr_alloc(MemMgr_t *mem_ctx, uint16_t size);

/**
 * @brief allocates a continuous memory region
 *
 * @param *mem_ctx: The MemMgr Context Struct
 * @param *addr: the address of a previous allocated region to free
 */
void memmgr_free(MemMgr_t *mem_ctx, void *addr);

/**
 * @brief computes the remaining free bytes of a given MemMgr heap
 *
 * @param *mem_ctx: The MemMgr Context Struct
 *
 * @retval the amount of free bytes in the MemMgr Heap
 */
uint16_t memmgr_remaining(MemMgr_t *mem_ctx);

#endif /* __MEMORY_MANAGER_H__ */
