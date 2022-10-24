#ifndef __SNPES_H__
#define __SNPES_H__

#include <stdint.h>
#include "snpes_cfg.h"
#include "ConnInterface.h"
#include "TimerInterface.h"

typedef enum {
	GATEWAY = 0x00,
	NODE = 0xFF
} DeviceType_t;

void snpes_init(DeviceType_t type, uint8_t uid, LoraItf_t *lora, TimerItf_t *timer);

#endif /* __SNPES_H__ */
