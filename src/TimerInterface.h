#ifndef __TIMER_INTERFACE_H__
#define __TIMER_INTERFACE_H__

#include <stdint.h>

typedef struct {
	void (*delay)(uint32_t ms);
	uint32_t (*millis)(void);
} TimerItf_t;

#endif /* __TIMER_INTERFACE_H__ */
