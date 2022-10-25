#ifndef __SNPES_UTILS_H__
#define __SNPES_UTILS_H__

#include "snpes_types.h"
#include <stdint.h>

void enqueue_signal(DeviceCtx_t *dev, PacketType_t signal, uint8_t dest_uid, uint8_t dest_nid);
void enqueue_data(DeviceCtx_t *dev, uint8_t dest_uid, uint8_t dest_nid, uint8_t seq, const void *src, uint8_t size);
uint8_t alloc_nid(ClientCtx_t *arr);
void free_nid(ClientCtx_t *arr, uint8_t nid);
ClientCtx_t *get_client_ctx(ClientCtx_t *arr, uint8_t nid);
PacketType_t get_pkt_type(Packet_t *pkt);

#endif /* __SNPES_UTILS_H__ */
