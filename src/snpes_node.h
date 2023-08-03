#ifndef __SNPES_H__
#define __SNPES_H__

#include <stdint.h>
#include "snpes_cfg.h"
#include "ConnInterface.h"
#include "TimerInterface.h"

/**
 * @brief initializes the snpes protocol as a node device
 *
 * @param uid: Unique Identifier of the device
 * @param *lora: pointer to a LoRa Conn Interface
 * @param *timer: pointer to a Timer Interface
 */
void snpes_node_init(uint8_t uid, LoraItf_t *lora, TimerItf_t *timer);

/**
 * @brief scans for a gateway in the area
 *
 * @param *gateway_uid: pointer to store the found gateway uid
 * @retval: SNPES_OK or SNPES_ERROR
 */
SnpesStatus_t snpes_scan(uint8_t *gateway_uid);

/**
 * @brief connect to a gateway by a given UID
 *
 * @param *gateway_uid: pointer to store the found gateway uid
 * @retval: SNPES_OK or SNPES_ERROR
 */
SnpesStatus_t snpes_connect(uint8_t gateway_uid);

/**
 * @brief connect to a gateway by a given UID
 *
 * @param dest_uid: unique id of the receiver node
 * @param *src: pointer to the buffer holding the data to send
 * @param size: how many bytes inside the src buffer to send
 * @retval: SNPES_OK or SNPES_ERROR
 */
SnpesStatus_t snpes_send(uint8_t dest_uid, const void *src, uint8_t size);

/**
 * @brief calls all the gateway modules
 * should be used in a loop function, like a pooling
 */
//void snpes_compute(void);

#endif /* __SNPES_H__ */
