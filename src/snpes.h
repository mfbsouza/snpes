#ifndef __SNPES_H__
#define __SNPES_H__

#include <stdint.h>
#include "ConnInterface.h"
#include "TimerInterface.h"

#if !defined(SNPES_MICRO)
#define SNPES_MICRO
#define PKT_SIZE   64
#define META_SIZE  6
#define BUF_CNT    8
#define BUF_SIZE   PKT_SIZE*BUF_CNT
#define S_IN_CNT   7
#define S_OUT_CNT  BUF_CNT-S_IN_CNT
#define CLT_CNT    8
#endif /* SNPES_MICRO */

typedef enum {
	GATEWAY = 0x00,
	NODE = 0xFF
} DeviceType_t;

void snpes_init(DeviceType_t type, uint8_t uid, LoraItf_t *lora, TimerItf_t *timer);

#endif /* __SNPES_H__ */
