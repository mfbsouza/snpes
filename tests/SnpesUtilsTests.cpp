#include <CppUTest/TestHarness.h>
#include <cstdint>

extern "C"{
#include <snpes_utils.h>
#include <snpes_cfg.h>
#include <snpes_types.h>
}

TEST_GROUP(SnpesUtilsTests)
{
	ClientCtx_t test_clients[CLT_CNT];
	uint8_t     stream_buf[BUF_SIZE];
	DeviceCtx_t test_dev {
		0x13,
		0x45,
		{nullptr, nullptr},
		{(void *)(stream_buf+(PKT_SIZE*S_IN_CNT)), (uint8_t)PKT_SIZE, (uint8_t)S_OUT_CNT, 0, 0},
		{(void *)stream_buf, (uint8_t)PKT_SIZE, (uint8_t)S_IN_CNT, 0, 0},
	};
};

TEST(SnpesUtilsTests, GetSeqNumber)
{
	Packet_t test;
	uint8_t dummy = 0x13;
	uint8_t ret = 0;
	build_data(&test, 0x13, 0x13, 0x13, 0x13, 0xF, &dummy, sizeof(uint8_t));
	ret = get_pkt_seq_number(&test);
	CHECK_EQUAL(0xF, ret);
}

TEST(SnpesUtilsTests, FindClient)
{
	ClientCtx_t *ret = NULL;
	test_clients[2].unique_id = 0x13;
	test_clients[2].connected = CONNECTING;
	ret = find_client_ctx(test_clients, 0x13);
	POINTERS_EQUAL(&(test_clients[2]), ret);
}

TEST(SnpesUtilsTests, GetWaitingClient)
{
	ClientCtx_t *ret = NULL;
	test_clients[2].waiting = 1;
	ret = get_waiting_client(test_clients);
	POINTERS_EQUAL(&(test_clients[2]), ret);
	test_clients[2].waiting = 0;
	ret = get_waiting_client(test_clients);
	POINTERS_EQUAL(NULL, ret);
}

TEST(SnpesUtilsTests, EnqueueSignal)
{
	queue_init(&test_dev.stream_out);
	enqueue_signal(&test_dev, SYNC, 0x14, 0x16);
	CHECK_EQUAL(0x13, ((Packet_t *)stream_buf)->src_uid);
	CHECK_EQUAL(0x45, ((Packet_t *)stream_buf)->src_nid);
	CHECK_EQUAL(0x14, ((Packet_t *)stream_buf)->dest_uid);
	CHECK_EQUAL(0x16, ((Packet_t *)stream_buf)->dest_nid);
	CHECK_EQUAL(SYNC, (((Packet_t *)stream_buf)->flgs_seq>>4)&0x0F);
	CHECK_EQUAL(0, ((Packet_t *)stream_buf)->data_size);
}

TEST(SnpesUtilsTests, EnqueueData)
{
	queue_init(&test_dev.stream_out);
	int a = 13;
	enqueue_data(&test_dev, 0x14, 0x16, 0x1, &a, sizeof(int));
	CHECK_EQUAL(0x13, ((Packet_t *)stream_buf)->src_uid);
	CHECK_EQUAL(0x45, ((Packet_t *)stream_buf)->src_nid);
	CHECK_EQUAL(0x14, ((Packet_t *)stream_buf)->dest_uid);
	CHECK_EQUAL(0x16, ((Packet_t *)stream_buf)->dest_nid);
	CHECK_EQUAL(DATA, (((Packet_t *)stream_buf)->flgs_seq>>4)&0x0F);
	CHECK_EQUAL(sizeof(int), ((Packet_t *)stream_buf)->data_size);
	CHECK_EQUAL(13, *(int *)(((Packet_t *)stream_buf)->data));
}

TEST(SnpesUtilsTests, AllocNID)
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

TEST(SnpesUtilsTests, AllocNIDFull)
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

TEST(SnpesUtilsTests, FreeNID)
{
	uint8_t retval = 0;
	retval = alloc_nid(test_clients);
	alloc_nid(test_clients);
	free_nid(test_clients, retval);
	CHECK_EQUAL(NOT_CONNETED, test_clients[0].connected);
	CHECK_EQUAL(0, test_clients[0].network_id);
	CHECK_EQUAL(0, test_clients[0].unique_id);
	CHECK_EQUAL(0, test_clients[0].timer_ref);
	CHECK_EQUAL(0, test_clients[0].timeout);
	CHECK_EQUAL(0, test_clients[0].state);
}

TEST(SnpesUtilsTests, GetClientContext)
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

TEST(SnpesUtilsTests, GetPacketType)
{
	queue_init(&test_dev.stream_out);
	enqueue_signal(&test_dev, SYNC, 0x14, 0x16);
	CHECK_EQUAL(SYNC, get_pkt_type((Packet_t *)stream_buf));
}
