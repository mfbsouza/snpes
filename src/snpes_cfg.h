#ifndef __SNPES_CFG_H__
#define __SNPES_CFG_H__

/* SNPES_MILLI CONFIG */
#if defined(SNPES_MILLI)
/* Packet size in bytes */
#define PKT_SIZE 128
/* Fake heap size in bytes */
#define HEAP_SIZE 512
/* Packets' Metadata size in bytes */
#define META_SIZE 6
/* maxium amount of packets in the the buffer */
#define BUF_CNT 8
/* buffer size in bytes */
#define BUF_SIZE PKT_SIZE *BUF_CNT
/* maxium amount of elements in the stream queues */
#define S_IN_CNT 4
#define S_OUT_CNT BUF_CNT - S_IN_CNT
/* maxium amount of clients */
#define CLT_CNT 16
/* maxium amount of packets per data transmission */
#define MAX_PKT_CNT 2

/* SNPES_MICRO CONFIG */
#else
/* Packet size in bytes */
#define PKT_SIZE 64
/* Fake heap size in bytes */
#define HEAP_SIZE 256
/* Packets' Metadata size in bytes */
#define META_SIZE 6
/* maxium amount of packets in the the buffer */
#define BUF_CNT 4
/* buffer size in bytes */
#define BUF_SIZE PKT_SIZE *BUF_CNT
/* maxium amount of elements in the stream queues */
#define S_IN_CNT 2
#define S_OUT_CNT BUF_CNT - S_IN_CNT
/* maxium amount of clients */
#define CLT_CNT 8
/* maxium amount of packets per data transmission */
#define MAX_PKT_CNT 1
#endif /* SNPES_ CONFIG */

#define MAX_TIMEOUT_CNT 2
#define TIMEOUT_THLD 10000 // 10 seconds in milliseconds
#define ALIVE_THLD 5 * 60 // 5 minutes in seconds

#endif /* __SNPES_CFG_H__ */
