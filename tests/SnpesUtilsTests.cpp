#include <CppUTest/TestHarness.h>
#include <cstdint>

extern "C" {
#include <snpes_utils.h>
#include <snpes_types.h>
}

TEST_GROUP(SnpesUtilsTests)
{
	uint8_t stream_buf[BUF_SIZE];
	DeviceCtx_t test_dev{ 0x13, 0x45 };
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

TEST(SnpesUtilsTests, EnqueueSignal)
{
	queue_init(&test_dev.stream_out, stream_buf, PKT_SIZE, S_IN_CNT);
	enqueue_signal(&test_dev, SYNC, 0x14, 0x16, 0x0, 0);
	CHECK_EQUAL(0x13, ((Packet_t *)stream_buf)->src_uid);
	CHECK_EQUAL(0x45, ((Packet_t *)stream_buf)->src_nid);
	CHECK_EQUAL(0x14, ((Packet_t *)stream_buf)->dest_uid);
	CHECK_EQUAL(0x16, ((Packet_t *)stream_buf)->dest_nid);
	CHECK_EQUAL(SYNC, (((Packet_t *)stream_buf)->flgs_seq >> 4) & 0x0F);
	CHECK_EQUAL(0, ((Packet_t *)stream_buf)->data_size);
}

TEST(SnpesUtilsTests, EnqueueData)
{
	queue_init(&test_dev.stream_out, stream_buf, PKT_SIZE, S_IN_CNT);
	int a = 13;
	enqueue_data(&test_dev, 0x14, 0x16, 0x1, &a, sizeof(int));
	CHECK_EQUAL(0x13, ((Packet_t *)stream_buf)->src_uid);
	CHECK_EQUAL(0x45, ((Packet_t *)stream_buf)->src_nid);
	CHECK_EQUAL(0x14, ((Packet_t *)stream_buf)->dest_uid);
	CHECK_EQUAL(0x16, ((Packet_t *)stream_buf)->dest_nid);
	CHECK_EQUAL(DATA, (((Packet_t *)stream_buf)->flgs_seq >> 4) & 0x0F);
	CHECK_EQUAL(sizeof(int), ((Packet_t *)stream_buf)->data_size);
	CHECK_EQUAL(13, *(int *)(((Packet_t *)stream_buf)->data));
}

TEST(SnpesUtilsTests, GetPacketType)
{
	queue_init(&test_dev.stream_out, stream_buf, PKT_SIZE, S_IN_CNT);
	enqueue_signal(&test_dev, SYNC, 0x14, 0x16, 0x0, 0);
	CHECK_EQUAL(SYNC, get_pkt_type((Packet_t *)stream_buf));
}
