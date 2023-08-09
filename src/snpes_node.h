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
 * @param gateway_uid: unique id of the gateway
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
 * @brief check if the gateway is sending some request to the client
 * should be used in a loop function, like a pooling
 *
 * @param gateway_uid: unique id of the gateway
 */
void snpes_node_sync(uint8_t gateway_uid);

/**
 * @brief checks how many bytes are available to read from the gateway
 *
 * @retval: returns the amount of bytes available to read
 */
uint8_t snpes_node_available_data(void);

/**
 * @brief copies the available data to the user destination
 *
 * @param *dest: pointer to the destination where the data will be copied
 * @param amount: amount of bytes to be copied
 *
 * @retval: returns the amount of bytes copied
 */
uint8_t snpes_node_read(void *dest, uint8_t amount);

#endif /* __SNPES_H__ */
