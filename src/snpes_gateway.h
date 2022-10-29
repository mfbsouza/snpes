#ifndef __SNPES_GATEWAY_H__
#define __SNPES_GATEWAY_H__

#include <stdint.h>
#include "snpes_cfg.h"
#include "ConnInterface.h"
#include "TimerInterface.h"

/**
 * @brief initializes the snpes protocol as a gateway device
 *
 * @param uid: Unique Identifier of the device
 * @param *lora: pointer to a LoRa Conn Interface
 * @param *timer: pointer to a Timer Interface
 */
void snpes_init(uint8_t uid, LoraItf_t *lora, TimerItf_t *timer);

/**
 * @brief calls all the gateway modules
 * should be used in a loop function, like a pooling
 */
void snpes_compute(void);

#endif /* __SNPES_GATEWAY_H__ */
