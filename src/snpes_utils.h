#ifndef __SNPES_UTILS_H__
#define __SNPES_UTILS_H__

#include "snpes_types.h"
#include <stdint.h>

void send_signal(DeviceCtx_t *dev, PacketType_t signal, uint8_t dest_uid);
void send_data(DeviceCtx_t *dev, const void *data, uint8_t size);
PacketType_t get_pkt_type(void *pkt);

#endif /* __SNPES_UTILS_H__ */
