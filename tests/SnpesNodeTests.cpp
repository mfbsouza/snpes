#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <chrono>

extern "C" {
#include <snpes_node.h>
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
static Packet_t recv_buf2;
static uint8_t cnt = 0;
uint8_t buf_select = 0;

static SendBuffer send_buf;

/* test interface functions */
static void fake_set_id(uint8_t)
{
	return;
}

static void fake_pkt_send(uint8_t id, uint8_t *data, uint8_t size)
{
	send_buf.id = id;
	send_buf.data = data;
	send_buf.size = size;
}

static void fake_pkt_recv(uint8_t *id, uint8_t *buf, uint8_t *size)
{
	*id = 0;
	if (buf_select == 0) {
		*size = recv_buf.data_size+6;
		memcpy(buf, &(recv_buf), *size);
	}
	else if (buf_select == 1) {
		*size = recv_buf2.data_size+6;
		memcpy(buf, &(recv_buf2), *size);
	}
}

static uint8_t mock_avail()
{
	return (uint8_t)mock().actualCall(__func__).returnIntValue();
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

static uint32_t mock_timer()
{
	if (++cnt == 18 || cnt == 22) buf_select = 1;
	else buf_select = 0;
	return (uint32_t)mock().actualCall(__func__).returnIntValue();
}

}

TEST_GROUP(SnpesNodeTests)
{
	LoraItf_t TestLora = {
		fake_set_id,
		fake_pkt_send,
		fake_pkt_recv,
		mock_avail
	};

	TimerItf_t TestTimer = {
		nullptr,
		test_millis
	};

	TimerItf_t TestTimerTimeout = {
		nullptr,
		timeout_millis
	};

	TimerItf_t MockTimer = {
		nullptr,
		mock_timer
	};

	void teardown() {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(SnpesNodeTests, Scan)
{
	SnpesStatus_t ret = SNPES_ERROR;
	uint8_t gw_uid = 0;
	snpes_node_init(0xAA, &TestLora, &TestTimer);
	/* build fake INFO packet */
	build_signal(&recv_buf, INFO, 0x55, 0x00, 0xAA, 0xFF, 0x0);
	mock().expectOneCall("mock_avail").andReturnValue(1);
	ret = snpes_scan(&gw_uid);
	CHECK_EQUAL(SNPES_OK, ret);
	CHECK_EQUAL(0x55, gw_uid);
}

TEST(SnpesNodeTests, ScanFail)
{
	SnpesStatus_t ret = SNPES_OK;
	uint8_t gw_uid = 0;
	snpes_node_init(0xAA, &TestLora, &TestTimerTimeout);
	ret = snpes_scan(&gw_uid);
	CHECK_EQUAL(SNPES_ERROR, ret);
	CHECK_EQUAL(0, gw_uid);
}

TEST(SnpesNodeTests, Connect)
{
	Packet_t *response = NULL;
	SnpesStatus_t ret = SNPES_ERROR;
	uint8_t data = 1;
	snpes_node_init(0xAA, &TestLora, &TestTimer);
	/* build fake DATA packet */
	build_data(&recv_buf, 0x55, 0x00, 0xAA, 0xFF, 0x0, &data, sizeof(uint8_t));
	mock().expectOneCall("mock_avail").andReturnValue(1);
	ret = snpes_connect(0x55);
	CHECK_EQUAL(SNPES_OK, ret);
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0xAA, response->src_uid);
	CHECK_EQUAL(0x01, response->src_nid);
	CHECK_EQUAL(0x55, response->dest_uid);
	CHECK_EQUAL(0x00, response->dest_nid);
	CHECK_EQUAL(ACK, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(0, response->data_size);

	snpes_node_init(0xAA, &TestLora, &TestTimer);
	/* build fake INFO packet */
	build_signal(&recv_buf, FULL, 0x55, 0x00, 0xAA, 0xFF, 0x0);
	mock().expectOneCall("mock_avail").andReturnValue(1);
	ret = snpes_connect(0x55);
	CHECK_EQUAL(SNPES_ERROR, ret);
}

TEST(SnpesNodeTests, ConnectTimout)
{
	SnpesStatus_t ret = SNPES_OK;
	snpes_node_init(0xAA, &TestLora, &TestTimerTimeout);
	ret = snpes_connect(0x55);
	CHECK_EQUAL(SNPES_ERROR, ret);
}

TEST(SnpesNodeTests, Send)
{
	Packet_t *response = NULL;
	SnpesStatus_t ret = SNPES_ERROR;
	uint8_t uid = 1;
	uint8_t data = 11;
	snpes_node_init(0xAA, &TestLora, &MockTimer);
	/* build fake DATA packet */
	build_data(&recv_buf, 0x55, 0x00, 0xAA, 0xFF, 0x0, &uid, sizeof(uint8_t));
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectNCalls(2, "mock_timer").andReturnValue(1);
	ret = snpes_connect(0x55);
	CHECK_EQUAL(SNPES_OK, ret);

	mock().expectOneCall("mock_timer").andReturnValue(1);
	mock().expectNCalls(2, "mock_timer").andReturnValue(2 + TIMEOUT_THLD);
	mock().expectOneCall("mock_timer").andReturnValue(4 + TIMEOUT_THLD*2);
	ret = snpes_send(0x55, &data, sizeof(uint8_t));
	CHECK_EQUAL(SNPES_ERROR, ret);
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0xAA, response->src_uid);
	CHECK_EQUAL(0x01, response->src_nid);
	CHECK_EQUAL(0x55, response->dest_uid);
	CHECK_EQUAL(0x00, response->dest_nid);
	CHECK_EQUAL(TRANS_START, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(1, response->data_size);
	CHECK_EQUAL(5, response->flgs_seq&0x0F);

	/* build fake TRANS_START packet */
	build_signal(&recv_buf, TRANS_START, 0x55, 0x00, 0xAA, 0x01, 0x0);
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectNCalls(2, "mock_timer").andReturnValue(1);
	mock().expectOneCall("mock_timer").andReturnValue(1);
	mock().expectNCalls(2, "mock_timer").andReturnValue(2 + TIMEOUT_THLD);
	mock().expectOneCall("mock_timer").andReturnValue(4 + TIMEOUT_THLD*2);
	ret = snpes_send(0x55, &data, sizeof(uint8_t));
	CHECK_EQUAL(SNPES_ERROR, ret);
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0xAA, response->src_uid);
	CHECK_EQUAL(0x01, response->src_nid);
	CHECK_EQUAL(0x55, response->dest_uid);
	CHECK_EQUAL(0x00, response->dest_nid);
	CHECK_EQUAL(DATA, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(1, response->data_size);
	CHECK_EQUAL(1, response->flgs_seq&0x0F);
	CHECK_EQUAL(data, response->data[0]);

	/* build fake FULL packet */
	build_signal(&recv_buf, FULL, 0x55, 0x00, 0xAA, 0x01, 0x0);
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectNCalls(2, "mock_timer").andReturnValue(1);
	ret = snpes_send(0x55, &data, sizeof(uint8_t));
	CHECK_EQUAL(SNPES_ERROR, ret);

	build_signal(&recv_buf, TRANS_START, 0x55, 0x00, 0xAA, 0x01, 0x0);
	build_signal(&recv_buf2, ACK, 0x55, 0x00, 0xAA, 0x01, 0x0);
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectNCalls(2, "mock_timer").andReturnValue(1);

	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectNCalls(2, "mock_timer").andReturnValue(1);
	ret = snpes_send(0x55, &data, sizeof(uint8_t));
	CHECK_EQUAL(SNPES_OK, ret);

	build_signal(&recv_buf2, TRANS_RETRY, 0x55, 0x00, 0xAA, 0x01, 0x0);
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectNCalls(2, "mock_timer").andReturnValue(1);

	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectNCalls(2, "mock_timer").andReturnValue(1);

	mock().expectOneCall("mock_timer").andReturnValue(1);
	mock().expectOneCall("mock_timer").andReturnValue(2 + TIMEOUT_THLD);

	ret = snpes_send(0x55, &data, sizeof(uint8_t));
	CHECK_EQUAL(SNPES_ERROR, ret);
	buf_select = 0;
}
