#include <CppUTest/TestHarness.h>
#include <cstdint>

extern "C" {
#include <snpes_client_manager.h>
#include <snpes_cfg.h>
#include <snpes_types.h>
}

TEST_GROUP(SnpesClientManagerTests)
{
	ClientCtx_t test_clients[CLT_CNT];
};

TEST(SnpesClientManagerTests, FindClient)
{
	ClientCtx_t *ret = NULL;
	test_clients[2].unique_id = 0x13;
	test_clients[2].connected = CONNECTING;
	ret = find_client_ctx(test_clients, 0x13);
	POINTERS_EQUAL(&(test_clients[2]), ret);
}

TEST(SnpesClientManagerTests, GetWaitingClient)
{
	ClientCtx_t *ret = NULL;
	test_clients[2].state = WAIT_ACK;
	ret = get_waiting_client(test_clients);
	POINTERS_EQUAL(&(test_clients[2]), ret);
	test_clients[2].state = IDLE;
	ret = get_waiting_client(test_clients);
	POINTERS_EQUAL(NULL, ret);
	test_clients[1].state = WAIT_DATA;
	ret = get_waiting_client(test_clients);
	POINTERS_EQUAL(&(test_clients[1]), ret);
}

TEST(SnpesClientManagerTests, GetDataAvailClient)
{
	ClientCtx_t *ret = NULL;
	test_clients[2].state = DATA_AVAIL;
	ret = get_data_avail_client(test_clients);
	POINTERS_EQUAL(&(test_clients[2]), ret);
	test_clients[2].state = IDLE;
	ret = get_data_avail_client(test_clients);
	POINTERS_EQUAL(NULL, ret);
}

TEST(SnpesClientManagerTests, AllocNID)
{
	uint8_t retval = 0;
	retval = alloc_nid(test_clients);
	CHECK_EQUAL(CONNECTING, test_clients[0].connected);
	CHECK_EQUAL(1, test_clients[0].network_id);
	CHECK_EQUAL(1, retval);
	retval = alloc_nid(test_clients);
	CHECK_EQUAL(CONNECTING, test_clients[1].connected);
	CHECK_EQUAL(2, test_clients[1].network_id);
	CHECK_EQUAL(2, retval);
}

TEST(SnpesClientManagerTests, AllocNIDFull)
{
	uint8_t retval = 0;
	int i;
	for (i = 0; i < CLT_CNT; i++) {
		retval = alloc_nid(test_clients);
	}
	CHECK_EQUAL(CLT_CNT, retval);
	retval = alloc_nid(test_clients);
	CHECK_EQUAL(0, retval);
}

TEST(SnpesClientManagerTests, FreeNID)
{
	uint8_t retval = 0;
	retval = alloc_nid(test_clients);
	alloc_nid(test_clients);
	free_nid(test_clients, retval);
	CHECK_EQUAL(NOT_CONNETED, test_clients[0].connected);
	CHECK_EQUAL(0, test_clients[0].network_id);
	CHECK_EQUAL(0, test_clients[0].unique_id);
	CHECK_EQUAL(0, test_clients[0].timer_ref);
	CHECK_EQUAL(0, test_clients[0].timeout_cnt);
	CHECK_EQUAL(0, test_clients[0].state);
}

TEST(SnpesClientManagerTests, GetClientContext)
{
	uint8_t retval = 0;
	ClientCtx_t *ptr = NULL;
	retval = alloc_nid(test_clients);
	ptr = get_client_ctx(test_clients, retval);
	POINTERS_EQUAL(&(test_clients[0]), ptr);
	retval = alloc_nid(test_clients);
	ptr = get_client_ctx(test_clients, retval);
	POINTERS_EQUAL(&(test_clients[1]), ptr);
}

TEST(SnpesClientManagerTests, GetRTSClient)
{
	ClientCtx_t *ret = NULL;
	test_clients[2].out_expt_pkt = 1;
	ret = get_rts_client(test_clients);
	POINTERS_EQUAL(&(test_clients[2]), ret);
	test_clients[2].out_expt_pkt = 0;
	ret = get_rts_client(test_clients);
	POINTERS_EQUAL(NULL, ret);
}
