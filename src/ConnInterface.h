#ifndef __CONN_INTERFACE_H__
#define __CONN_INTERFACE_H__

#include <stdint.h>

typedef struct {
	void (*set_id)(uint8_t id);
	void (*pkt_send)(uint8_t id, uint8_t *data, uint8_t size);
	void (*pkt_recv)(uint8_t *id, uint8_t *buf, uint8_t *size);
	uint8_t (*pkt_avail)(void);
} LoraItf_t;

#endif /* __CONN_INTERFACE_H__ */
