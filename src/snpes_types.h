#ifndef __SNPES_TYPES_H__
#define __SNPES_TYPES_H__

#include <stdint.h>
#include "snpes_cfg.h"
#include "CircularQueue.h"
#include "ConnInterface.h"
#include "TimerInterface.h"

typedef enum {
	GATEWAY = 0x00,
	NODE = 0xFF
} DeviceType_t;

typedef enum {
	SEND_INFO = 0x00,
	RESP_SYNC,
	WAIT_ACK,
	IDLE
} GwStates_t;

typedef enum {
	NOT_CONNETED = 0x00,
	CONNECTED,
	CONNECTING
} ConnState_t;

typedef enum {
	SCAN = 0x00,
	INFO,
	SYNC,
	ACK,
	FULL,
	DATA,
	ALIVE
} PacketType_t;

typedef struct {
	/* client info */
	uint8_t unique_id;
	uint8_t network_id;
	/* connection state */
	GwStates_t state;
	/* flags */
	ConnState_t connected;
	uint8_t timeout_cnt;
	/* time references */
	uint32_t timer_ref;
	uint32_t alive_ref;
} ClientCtx_t;

typedef struct {
	LoraItf_t *socket;
	TimerItf_t *timer;
} HwCtx_t;

typedef struct {
	uint8_t unique_id;
	uint8_t network_id;
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

#endif /* __SNPES_TYPES_H__ */
