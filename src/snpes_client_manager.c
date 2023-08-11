#include "snpes_client_manager.h"
#include "snpes_types.h"
#include "snpes_cfg.h"
#include "CircularQueue.h"
#include <string.h>
#include <assert.h>

uint8_t alloc_nid(ClientCtx_t *arr)
{
	assert(arr);
	for (int nid = 0; nid < CLT_CNT; nid++) {
		if (arr[nid].connected == NOT_CONNETED) {
			arr[nid].connected = CONNECTING;
			arr[nid].network_id = (uint8_t)(nid + 1);
			return (uint8_t)(nid + 1);
		}
	}
	return 0;
}

void free_nid(ClientCtx_t *arr, uint8_t nid)
{
	assert(arr);
	assert(nid > 0);
	memset(&(arr[(nid - 1)]), 0, sizeof(ClientCtx_t));
}

ClientCtx_t *get_client_ctx(ClientCtx_t *arr, uint8_t nid)
{
	assert(arr);
	assert(nid > 0);
	return &(arr[nid - 1]);
}

ClientCtx_t *find_client_ctx(ClientCtx_t *arr, uint8_t uid)
{
	assert(arr);
	ClientCtx_t *ret = NULL;

	for (int ii = 0; ii < CLT_CNT; ii++) {
		if (arr[ii].unique_id == uid &&
		    arr[ii].connected != NOT_CONNETED) {
			ret = &(arr[ii]);
			break;
		}
	}
	return ret;
}

// TODO: i don't like that "waiting" word
ClientCtx_t *get_waiting_client(ClientCtx_t *arr)
{
	assert(arr);
	for (int ii = 0; ii < CLT_CNT; ii++) {
		if (arr[ii].state == WAIT_ACK || arr[ii].state == WAIT_DATA ||
		    arr[ii].state == WAIT_TRANS_START ||
		    arr[ii].state == WAIT_DATA_ACK) {
			return &(arr[ii]);
		}
	}
	return NULL;
}

// TODO: i don't like that function name
ClientCtx_t *get_data_avail_client(ClientCtx_t *arr)
{
	assert(arr);
	for (int ii = 0; ii < CLT_CNT; ii++) {
		if (arr[ii].state == DATA_AVAIL) {
			return &(arr[ii]);
		}
	}
	return NULL;
}

ClientCtx_t *get_rts_client(ClientCtx_t *arr)
{
	assert(arr);
	for (int ii = 0; ii < CLT_CNT; ii++) {
		if (arr[ii].out_expt_pkt != 0) {
			return &(arr[ii]);
		}
	}
	return NULL;
}
