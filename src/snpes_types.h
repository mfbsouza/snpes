#ifndef __SNPES_TYPES_H__
#define __SNPES_TYPES_H__

#include <stdint.h>
#include "snpes_cfg.h"
#include "CircularQueue.h"
#include "ConnInterface.h"
#include "TimerInterface.h"

typedef enum {
	IDLE = 0x00,
} States_t;

typedef struct {
	uint8_t unique_id;
	States_t state;
	uint8_t connected;
} ClientCtx_t;

typedef struct {
	LoraItf_t *socket;
	TimerItf_t *timer;
} HwCtx_t;

typedef struct {
	uint8_t unique_id;
	uint8_t network_id;
	uint8_t type;
	HwCtx_t hw;
	Queue_t stream_in;
	Queue_t stream_out;
} DeviceCtx_t;

typedef struct {
	uint8_t src_uid;
	uint8_t src_nid;
	uint8_t dest_uid;
	uint8_t dest_nid;
	uint8_t flgs_seq;
	uint8_t data_size;
	uint8_t data[PKT_SIZE-META_SIZE];
} Packet_t;

typedef enum {
	SCAN = 0x00,
	INFO,
	SET,
	SYNC,
	ACK,
	FULL,
	DATA
} PacketType_t;

#endif /* __SNPES_TYPES_H__ */
