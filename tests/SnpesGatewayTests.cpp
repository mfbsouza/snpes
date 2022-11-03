#include <CppUTest/TestHarness.h>
#include <chrono>

extern "C"{
#include <snpes_gateway.h>
#include <snpes_types.h>
#include <snpes_utils.h>
#include <snpes_cfg.h>
#include <string.h>

/* test struct */
typedef struct {
	uint8_t id;
	uint8_t *data;
	uint8_t size;
} SendBuffer; 

/* test variables */
static Packet_t recv_buf;
static uint8_t avail = 0;

static SendBuffer send_buf;

/* test interface functions */
static void test_set_id(uint8_t)
{
	return;
}

static void test_pkt_send(uint8_t id, uint8_t *data, uint8_t size)
{
	send_buf.id = id;
	send_buf.data = data;
	send_buf.size = size;
}

static void test_pkt_recv(uint8_t *id, uint8_t *buf, uint8_t *size)
{
	*id = 0;
	*size = recv_buf.data_size+6;
	memcpy(buf, &(recv_buf), *size);
}

static uint8_t test_avail()
{
	return avail;
}

static uint32_t test_millis()
{
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto value = now_ms.time_since_epoch();
	long duration = value.count();
	return (uint32_t) duration;
}

static uint32_t timeout_millis()
{
	static int32_t time_ref = -TIMEOUT_THLD;
	time_ref += (TIMEOUT_THLD+1);
	return (uint32_t)time_ref;
}

}

TEST_GROUP(SnpesGatewayTests)
{
	LoraItf_t TestLora = {
		test_set_id,
		test_pkt_send,
		test_pkt_recv,
		test_avail
	};

	TimerItf_t TestTimer = {
		nullptr,
		test_millis
	};

	TimerItf_t TestTimerTimeout = {
		nullptr,
		timeout_millis
	};
};

TEST(SnpesGatewayTests, Scan)
{
	Packet_t *response = NULL;
	snpes_init(0x55, &TestLora, &TestTimer);

	/* build a fake packet */
	build_signal(&(recv_buf), SCAN, 0xAA, 0xFF, 0x00, 0x00, 0x0);
	/* say that there is a packet availible */
	avail = 1;
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	avail = 0;
	/* let the gateway compute */
	snpes_compute();
	/* check if the gateway tried to send a INFO packet */
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0x55, response->src_uid);
	CHECK_EQUAL(0x00, response->src_nid);
	CHECK_EQUAL(0xAA, response->dest_uid);
	CHECK_EQUAL(0xFF, response->dest_nid);
	CHECK_EQUAL(INFO, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(0, response->data_size);
}

TEST(SnpesGatewayTests, SyncAndAck)
{
	Packet_t *response = NULL;
	snpes_init(0x55, &TestLora, &TestTimer);
	/* build a fake packet */
	build_signal(&(recv_buf), SYNC, 0xAA, 0xFF, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	avail = 1;
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	avail = 0;
	/* let the gateway compute */
	snpes_compute();
	/* pretend like we lost the packet */
	build_signal(&(recv_buf), SYNC, 0xAA, 0xFF, 0x55, 0x00, 0x1);
	/* say that there is a packet availible */
	avail = 1;
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	avail = 0;
	/* let the gateway compute */
	snpes_compute();
	/* check if the gateway tried to send a DATA packet */
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0x55, response->src_uid);
	CHECK_EQUAL(0x00, response->src_nid);
	CHECK_EQUAL(0xAA, response->dest_uid);
	CHECK_EQUAL(0xFF, response->dest_nid);
	CHECK_EQUAL(DATA, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(0, response->flgs_seq&0x0F);
	CHECK_EQUAL(1, response->data_size);
	CHECK_EQUAL(1, response->data[0]);

	/* build a fake ACK */
	build_signal(&(recv_buf), ACK, 0xAA, 0x01, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	avail = 1;
	/* let the gateway compute */
	snpes_compute();

	/* build a second fake packet */
	build_signal(&(recv_buf), SYNC, 0xBB, 0xFF, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	avail = 1;
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	avail = 0;
	/* let the gateway compute */
	snpes_compute();
	/* check if the gateway tried to send a DATA packet */
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0x55, response->src_uid);
	CHECK_EQUAL(0x00, response->src_nid);
	CHECK_EQUAL(0xBB, response->dest_uid);
	CHECK_EQUAL(0xFF, response->dest_nid);
	CHECK_EQUAL(DATA, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(0, response->flgs_seq&0x0F);
	CHECK_EQUAL(1, response->data_size);
	CHECK_EQUAL(2, response->data[0]);
}

TEST(SnpesGatewayTests, FakeClientAndSyncFull)
{
	Packet_t *response = NULL;
	snpes_init(0x55, &TestLora, &TestTimer);
	/* fake client packet */
	build_signal(&(recv_buf), SYNC, 0xAA, 0x01, 0x55, 0x00, 0x0);
	avail = 1;
	snpes_compute();
	/* populate client list */
	snpes_init(0x55, &TestLora, &TestTimer);
	for (int i = 0; i <= CLT_CNT; i++) {
		build_signal(&(recv_buf), SYNC, (uint8_t)i, 0xFF, 0x55, 0x00, 0x0);
		avail = 1;
		snpes_compute();
	}
	avail = 0;
	snpes_compute();
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0x55, response->src_uid);
	CHECK_EQUAL(0x00, response->src_nid);
	CHECK_EQUAL(CLT_CNT, response->dest_uid);
	CHECK_EQUAL(0xFF, response->dest_nid);
	CHECK_EQUAL(FULL, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(0, response->flgs_seq&0x0F);
	CHECK_EQUAL(0, response->data_size);
}

TEST(SnpesGatewayTests, Timeout)
{
	Packet_t *response = NULL;
	snpes_init(0x55, &TestLora, &TestTimerTimeout);
	build_signal(&(recv_buf), SYNC, 0xAA, 0xFF, 0x55, 0x00, 0x0);
	avail = 1;
	snpes_compute();
	avail = 0;
	snpes_compute();
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0x55, response->src_uid);
	CHECK_EQUAL(0x00, response->src_nid);
	CHECK_EQUAL(0xAA, response->dest_uid);
	CHECK_EQUAL(0xFF, response->dest_nid);
	CHECK_EQUAL(DATA, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(0, response->flgs_seq&0x0F);
	CHECK_EQUAL(1, response->data_size);
	CHECK_EQUAL(1, response->data[0]);
	memset(response, 0, sizeof(Packet_t));
	snpes_compute();
	CHECK_EQUAL(0, response->src_uid);
	CHECK_EQUAL(0, response->src_nid);
	CHECK_EQUAL(0, response->dest_uid);
	CHECK_EQUAL(0, response->dest_nid);
	CHECK_EQUAL(0, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(0, response->flgs_seq&0x0F);
	CHECK_EQUAL(0, response->data_size);
	CHECK_EQUAL(0, response->data[0]);
}
