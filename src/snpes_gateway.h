#ifndef __SNPES_GATEWAY_H__
#define __SNPES_GATEWAY_H__

#include <stdint.h>
#include "snpes_cfg.h"
#include "ConnInterface.h"
#include "TimerInterface.h"

void snpes_init(uint8_t uid, LoraItf_t *lora, TimerItf_t *timer);

#endif /* __SNPES_GATEWAY_H__ */
