#ifndef __CIRCULAR_QUEUE_H__
#define __CIRCULAR_QUEUE_H__

#include <stdint.h>

/** 
 * @brief The Queue Context Struct contains information about the Queue
 *
 * @param *start_addr: The start address of a continuous amount of memory
 * to manage as a Queue Data Structure
 * @param elmt_size: The element size in bytes of the Queue
 * @param elmt_cnt: The maximum amount of elements the Queue can hold
 */
typedef struct {
	void *start_addr;
	uint8_t elmt_size;
	uint8_t elmt_cnt;
	int32_t head;
	int32_t tail;
} Queue_t;

/** 
 * @brief Initializes the Queue to manage a continuous amount of memory
 * by a given Queue Context Struct
 *
 * @param *queue_ctx: The Queue Context Struct
 */
void queue_init(Queue_t *queue_ctx);

/** 
 * @brief request allocation for one element in the Queue
 *
 * @param *queue_ctx: The Queue Context Struct
 *
 * @retval the start address allocated for the element
 */
void* queue_alloc(Queue_t *queue_ctx);

/** 
 * @brief Copies a given element into the Queue
 *
 * @param *queue_ctx: The Queue Context Struct
 * @param elmt_addr: The start address of the element to insert in the Queue
 *
 * @retval error code
 */
int8_t queue_push(Queue_t *queue_ctx, void *elmt_addr);

/** 
 * @brief Gets the address of the next element to read from the Queue
 * and marks the element as red in the Queue
 *
 * @retval The element Address or NULL if no element to read
 */
void* queue_pop(Queue_t *queue_ctx);

/** 
 * @brief checks if a given Queue is full
 *
 * @param *queue_ctx: The Queue Context Struct
 *
 * @retval 0 for False, 1 for True
 */
uint8_t queue_full(Queue_t *queue_ctx);

#endif /* __CIRCULAR_QUEUE_H__ */
