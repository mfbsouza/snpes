#ifndef __SNPES_CFG_H__
#define __SNPES_CFG_H__

/* SNPES_MILLI CONFIG */
#if defined(SNPES_MILLI)
/* Packet size in bytes */
#define PKT_SIZE   128
/* Packets' Metadata size in bytes */
#define META_SIZE  6
/* maxium amount of packets in the the buffer */
#define BUF_CNT    8
/* buffer size in bytes */
#define BUF_SIZE   PKT_SIZE*BUF_CNT
/* maxium amount of elements in the stream queues */
#define S_IN_CNT   4
#define S_OUT_CNT  BUF_CNT-S_IN_CNT
/* maxium amount of clients */
#define CLT_CNT    16

/* SNPES_MICRO CONFIG */
#else
/* Packet size in bytes */
#define PKT_SIZE   64
/* Packets' Metadata size in bytes */
#define META_SIZE  6
/* maxium amount of packets in the the buffer */
#define BUF_CNT    4
/* buffer size in bytes */
#define BUF_SIZE   PKT_SIZE*BUF_CNT
/* maxium amount of elements in the stream queues */
#define S_IN_CNT   2
#define S_OUT_CNT  BUF_CNT-S_IN_CNT
/* maxium amount of clients */
#define CLT_CNT    8
#endif /* SNPES_ CONFIG */

#define MAX_TIMEOUT_CNT 2
#define TIMEOUT_THLD 5000 // 5 seconds in milliseconds
#define ALIVE_THLD   5*60 // 5 minutes in seconds

#endif /* __SNPES_CFG_H__ */
