#ifndef __SNPES_UTILS_H__
#define __SNPES_UTILS_H__

#include "snpes_types.h"
#include <stdint.h>

/**
 * @brief loads given data to a Packet in signal format
 *
 * @param *pkt: pointer to the Packet
 * @param signal: signal flag define at snpes_types
 * @param src_uid: UID of the source
 * @param src_nid: NID of the source
 * @param dest_uid: UID of the destination
 * @param dest_nid: NID of the destination
 * @param seq: Sequence Number of the Packet
 */
void build_signal(Packet_t *pkt, PacketType_t signal, uint8_t src_uid, uint8_t src_nid, uint8_t dest_uid, uint8_t dest_nid, uint8_t seq);

/**
 * @brief loads given data to a Packet in data format
 *
 * @param *pkt: pointer to the Packet
 * @param src_uid: UID of the source node
 * @param src_nid: NID of the source node
 * @param dest_uid: UID of the destination node
 * @param dest_nid: NID of the destination node
 * @param seq: Sequence Number of the data Packet
 * @param src: pointer to data source to send in the Packet
 * @param size: the amount of bytes from data source to send
 */
void build_data(Packet_t *pkt, uint8_t src_uid, uint8_t src_nid, uint8_t dest_uid, uint8_t dest_nid, uint8_t seq, const void *src, uint8_t size);

/**
 * @brief enqueue a Signal Packet to the stream out of a given protocol device
 *
 * @param *dev: pointer to the protocol device
 * @param signal: signal flag define at snpes_types
 * @param dest_uid: UID of the destination node
 * @param dest_nid: NID of the destination node
 */
void enqueue_signal(DeviceCtx_t *dev, PacketType_t signal, uint8_t dest_uid, uint8_t dest_nid);

/**
 * @brief enqueue a Data Packet to the stream out of a given protocol device
 *
 * @param *dev: pointer to the protocol device
 * @param dest_uid: UID of the destination node
 * @param dest_nid: NID of the destination node
 * @param seq: Sequence Number of the data Packet
 * @param src: pointer to data source to send in the Packet
 * @param size: the amount of bytes from data source to send
 */
void enqueue_data(DeviceCtx_t *dev, uint8_t dest_uid, uint8_t dest_nid, uint8_t seq, const void *src, uint8_t size);

/**
 * @brief allocates a Network ID of a given client list array
 *
 * @param *arr: the client list array
 * @retval a new Network ID or O for None
 */
uint8_t alloc_nid(ClientCtx_t *arr);

/**
 * @brief free a given Network ID of a given cliente list array
 *
 * @param *arr: the client list array
 * @param nid: the Network ID to free from the array
 */
void free_nid(ClientCtx_t *arr, uint8_t nid);

/**
 * @brief gets the pointer to a Client Context Struct of a given client list array and Netowrk ID
 *
 * @param *arr: the client list array
 * @param nid: the Network ID of the client
 * @retval pointer to the Client Context or Null for None
 */
ClientCtx_t *get_client_ctx(ClientCtx_t *arr, uint8_t nid);

/**
 * @brief searchs for a client in the client list array and returns it address
 *
 * @param *arr: the client list array
 * @param uid: the unique ID of the client
 * @retval pointer to the Client Context or Null for None
 */
ClientCtx_t *find_client_ctx(ClientCtx_t *arr, uint8_t uid);

/**
 * @brief gets the pointer to the first client that we are waiting for a response
 *
 * @param *arr: the client list array
 * @retval pointer to the Client Context or Null for None
 */
ClientCtx_t *get_waiting_client(ClientCtx_t *arr);

/**
 * @brief gets the pointer to the first client that has data availible to read
 *
 * @param *arr: the client list array
 * @retval pointer to the Client Context or Null for None
 */
ClientCtx_t *get_data_avail_client(ClientCtx_t *arr);

/**
 * @brief gets the Packet Flag Type of a given Packet Struct
 *
 * @param *pkt: pointer to a Packet Struct in the memory
 * @retval The Packet Type
 */
PacketType_t get_pkt_type(Packet_t *pkt);

/**
 * @brief gets the Packet sequence number of a given Packet Struct
 *
 * @param *pkt: pointer to a Packet Struct in the memory
 * @retval The Packet Sequence Number
 */
uint8_t get_pkt_seq_number(Packet_t *pkt);

#endif /* __SNPES_UTILS_H__ */
