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
		0,
		{nullptr, nullptr},
		{(void *)(stream_buf+(PKT_SIZE*S_IN_CNT)), (uint8_t)PKT_SIZE, (uint8_t)S_OUT_CNT, 0, 0},
		{(void *)stream_buf, (uint8_t)PKT_SIZE, (uint8_t)S_IN_CNT, 0, 0},
		{nullptr, 0, 0, 0 ,0}
	};
};

TEST(SnpesUtilsTests, EnqueueSignal)
{
	queue_init(&test_dev.stream_out);
	enqueue_signal(&test_dev, SYNC, 0x14, 0x16);
	CHECK_EQUAL(0x13, ((Packet_t *)stream_buf)->src_uid);
	CHECK_EQUAL(0x45, ((Packet_t *)stream_buf)->src_nid);
	CHECK_EQUAL(0x14, ((Packet_t *)stream_buf)->dest_uid);
	CHECK_EQUAL(0x16, ((Packet_t *)stream_buf)->dest_nid);
	CHECK_EQUAL(SYNC, (((Packet_t *)stream_buf)->flgs_seq>>4)&0x0F);
	CHECK_EQUAL(META_SIZE, ((Packet_t *)stream_buf)->data_size);
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
	CHECK_EQUAL(META_SIZE+sizeof(int), ((Packet_t *)stream_buf)->data_size);
	CHECK_EQUAL(13, *(int *)(((Packet_t *)stream_buf)->data));
}
