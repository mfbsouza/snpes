#ifndef __SNPES_H__
#define __SNPES_H__

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
void snpes_gw_init(uint8_t uid, LoraItf_t *lora, TimerItf_t *timer);

/**
 * @brief calls all the gateway modules
 * should be used in a loop function, like a pooling
 */
void snpes_compute(void);

/**
 * @brief check if there is available data to read
 *
 * @retval 0 for no data, or the amount of data to read
 */
uint16_t snpes_data_available(void);

/**
 * @brief reads available data
 *
 * @param *clt_uid: pointer to the buffer that will store the client unique id
 * @param *dest: pointer to the buffer that will store the data
 * @param *size: pointer to variable that will store the data size
 */
void snpes_read(uint8_t *clt_uid, void *dest, uint16_t *size);

/**
 * @brief send a given data to a client
 *
 * @param clt_uid: the receiver client unique id
 * @param *src: pointer to the buffer that stores the data to send
 * @param size: size of the data located at the src buffer
 */
//void snpes_write(uint8_t clt_uid, const void *src, uint16_t size);

#endif /* __SNPES_H__ */
