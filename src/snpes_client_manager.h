#ifndef __CLIENT_MANAGER_H__
#define __CLIENT_MANAGER_H__

#include "snpes_types.h"
#include <stdint.h>

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
 * @brief gets the pointer to a Client Context Struct of a given client list
 * array and Netowrk ID
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
 * @brief gets the pointer to the first client that we are waiting for a
 * response
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
 * @brief gets the pointer to the first client that has data availible to send
 *
 * @param *arr: the client list array
 * @retval pointer to the Client Context or Null for None
 */
ClientCtx_t *get_rts_client(ClientCtx_t *arr);

#endif /* __CLIENT_MANAGER_H__ */
