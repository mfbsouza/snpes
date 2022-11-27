#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <chrono>

extern "C" {
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
	*size = recv_buf.data_size+6;
	memcpy(buf, &(recv_buf), *size);
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
	return (uint32_t)mock().actualCall(__func__).returnIntValue();
}

}

TEST_GROUP(SnpesGatewayTests)
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

TEST(SnpesGatewayTests, Scan)
{
	Packet_t *response = NULL;
	snpes_gw_init(0x55, &TestLora, &TestTimer);

	/* build a fake packet */
	build_signal(&(recv_buf), SCAN, 0xAA, 0xFF, 0x00, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
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
	snpes_gw_init(0x55, &TestLora, &TestTimer);
	/* build a fake packet */
	build_signal(&(recv_buf), SYNC, 0xAA, 0xFF, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* pretend like we lost the packet */
	build_signal(&(recv_buf), SYNC, 0xAA, 0xFF, 0x55, 0x00, 0x1);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
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
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();

	/* build a second fake packet */
	build_signal(&(recv_buf), SYNC, 0xBB, 0xFF, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
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
	snpes_gw_init(0x55, &TestLora, &TestTimer);
	/* fake client packet */
	build_signal(&(recv_buf), SYNC, 0xAA, 0x01, 0x55, 0x00, 0x0);
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	snpes_compute();
	/* populate client list */
	snpes_gw_init(0x55, &TestLora, &TestTimer);
	for (int i = 0; i <= CLT_CNT; i++) {
		build_signal(&(recv_buf), SYNC, (uint8_t)i, 0xFF, 0x55, 0x00, 0x0);
		mock().expectOneCall("mock_avail").andReturnValue(1);
		mock().expectOneCall("mock_avail").andReturnValue(0);
		snpes_compute();
	}
	mock().expectOneCall("mock_avail").andReturnValue(0);
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
	snpes_gw_init(0x55, &TestLora, &TestTimerTimeout);
	build_signal(&(recv_buf), SYNC, 0xAA, 0xFF, 0x55, 0x00, 0x0);
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	snpes_compute();
	mock().expectOneCall("mock_avail").andReturnValue(0);
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
	mock().expectOneCall("mock_avail").andReturnValue(0);
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

TEST(SnpesGatewayTests, Alive)
{
	Packet_t *response = NULL;
	snpes_gw_init(0x55, &TestLora, &MockTimer);
	/* build a fake packet */
	build_signal(&(recv_buf), SYNC, 0xAA, 0xFF, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectOneCall("mock_timer").andReturnValue(1);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectOneCall("mock_timer").andReturnValue(2);
	/* let the gateway compute */
	snpes_compute();
	/* build a fake ACK */
	build_signal(&(recv_buf), ACK, 0xAA, 0x01, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectNCalls(2, "mock_timer").andReturnValue(1000);
	/* let the gateway compute */
	snpes_compute();
	/* now that we are connect, lest fake that a long time has passed */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectNCalls(2, "mock_timer").andReturnValue(((ALIVE_THLD*1000)+1000));
	snpes_compute();
	/* compute */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectNCalls(2, "mock_timer").andReturnValue(((ALIVE_THLD*1000)+1000));
	snpes_compute();
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0x55, response->src_uid);
	CHECK_EQUAL(0x00, response->src_nid);
	CHECK_EQUAL(0xAA, response->dest_uid);
	CHECK_EQUAL(0x01, response->dest_nid);
	CHECK_EQUAL(ALIVE, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(0, response->flgs_seq&0x0F);
	CHECK_EQUAL(0, response->data_size);
	/* force a retry */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectNCalls(3, "mock_timer").andReturnValue(((ALIVE_THLD*1000)+1000)+TIMEOUT_THLD);
	snpes_compute();
	/* compute */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectNCalls(2, "mock_timer").andReturnValue(((ALIVE_THLD*1000)+1000)+TIMEOUT_THLD);
	snpes_compute();
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0x55, response->src_uid);
	CHECK_EQUAL(0x00, response->src_nid);
	CHECK_EQUAL(0xAA, response->dest_uid);
	CHECK_EQUAL(0x01, response->dest_nid);
	CHECK_EQUAL(ALIVE, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(1, response->flgs_seq&0x0F);
	CHECK_EQUAL(0, response->data_size);
	/* force a retry, again */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectNCalls(2, "mock_timer").andReturnValue(((ALIVE_THLD*1000)+1000)+TIMEOUT_THLD*2);
	snpes_compute();
}

TEST(SnpesGatewayTests, Transmission)
{
	uint16_t data = 0x1236;
	uint16_t temp = 0;
	uint16_t size = 0;
	uint8_t clt_uid = 0;
	Packet_t *response = NULL;
	snpes_gw_init(0x55, &TestLora, &TestTimer);
	/* build a fake packet */
	build_signal(&(recv_buf), SYNC, 0xAA, 0xFF, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute, it will send a DATA packet */
	snpes_compute();
	/* build a fake ACK */
	build_signal(&(recv_buf), ACK, 0xAA, 0x01, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* client 0xAA is now connected */

	/* build a fake TRANS_START packet */
	build_signal(&(recv_buf), TRANS_START, 0xAA, 0x01, 0x55, 0x00, 0x4);
	// TODO: get rid of this hack
	recv_buf.data_size = 2;
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* pretend that we didn't get the TRANS_START */
	build_signal(&(recv_buf), TRANS_START, 0xAA, 0x01, 0x55, 0x00, 0x5);
	// TODO: get rid of this hack
	recv_buf.data_size = 2;
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* check if the gateway tried to send a TRANS START packet */
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0x55, response->src_uid);
	CHECK_EQUAL(0x00, response->src_nid);
	CHECK_EQUAL(0xAA, response->dest_uid);
	CHECK_EQUAL(0x01, response->dest_nid);
	CHECK_EQUAL(TRANS_START, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(0, response->flgs_seq&0x0F);
	CHECK_EQUAL(0, response->data_size);

	/* build a fake DATA packet */
	build_data(&recv_buf, 0xAA, 0x01, 0x55, 0x00, 0x1, &data, sizeof(uint16_t));
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* check if the gateway tried to send a ACK packet */
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0x55, response->src_uid);
	CHECK_EQUAL(0x00, response->src_nid);
	CHECK_EQUAL(0xAA, response->dest_uid);
	CHECK_EQUAL(0x01, response->dest_nid);
	CHECK_EQUAL(ACK, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(0, response->flgs_seq&0x0F);
	CHECK_EQUAL(0, response->data_size);
	/* force a data avail state */
	build_signal(&(recv_buf), TRANS_START, 0xAA, 0x01, 0x55, 0x00, 0x4);
	recv_buf.data_size = 2;
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	snpes_compute();

	/* try to read the data */
	if (snpes_data_available()) {
		snpes_read(&clt_uid, &temp, &size);
	}
	CHECK_EQUAL(0xAA, clt_uid);
	CHECK_EQUAL(0x1236, temp);
	CHECK_EQUAL(2, size);
}

TEST(SnpesGatewayTests, DataTimeout)
{
	Packet_t *response = NULL;
	snpes_gw_init(0x55, &TestLora, &MockTimer);
	/* build a fake packet */
	build_signal(&(recv_buf), SYNC, 0xAA, 0xFF, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectOneCall("mock_timer").andReturnValue(1);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectOneCall("mock_timer").andReturnValue(1);
	/* let the gateway compute */
	snpes_compute();
	/* build a fake ACK */
	build_signal(&(recv_buf), ACK, 0xAA, 0x01, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectNCalls(2, "mock_timer").andReturnValue(1);
	/* let the gateway compute */
	snpes_compute();
	/* now that we are connect, lest fake that a long time has passed */
	/* build a fake TRANS_START packet */
	build_signal(&(recv_buf), TRANS_START, 0xAA, 0x01, 0x55, 0x00, 0x4);
	// TODO: get rid of this hack
	recv_buf.data_size = 2;
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectNCalls(3, "mock_timer").andReturnValue(1);
	/* let the gateway compute */
	snpes_compute();

	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectNCalls(3, "mock_timer").andReturnValue(1 + TIMEOUT_THLD);
	/* let the gateway compute */
	snpes_compute();
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectNCalls(2, "mock_timer").andReturnValue(1 + TIMEOUT_THLD);
	/* let the gateway compute */
	snpes_compute();
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0x55, response->src_uid);
	CHECK_EQUAL(0x00, response->src_nid);
	CHECK_EQUAL(0xAA, response->dest_uid);
	CHECK_EQUAL(0x01, response->dest_nid);
	CHECK_EQUAL(TRANS_RETRY, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(1, response->flgs_seq&0x0F);
	CHECK_EQUAL(0, response->data_size);

	/* force a retry, again */
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	mock().expectNCalls(2, "mock_timer").andReturnValue(1 + TIMEOUT_THLD*2);
	/* let the gateway compute */
	snpes_compute();
}

TEST(SnpesGatewayTests, TransmissionDataFull)
{
	Packet_t *response = NULL;
	snpes_gw_init(0x55, &TestLora, &TestTimer);
	/* build a fake packet */
	build_signal(&(recv_buf), SYNC, 0xAA, 0xFF, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute, it will send a DATA packet */
	snpes_compute();
	/* build a fake ACK */
	build_signal(&(recv_buf), ACK, 0xAA, 0x01, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* client 0xAA is now connected */

	/* build a fake packet */
	build_signal(&(recv_buf), SYNC, 0xBB, 0xFF, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute, it will send a DATA packet */
	snpes_compute();
	/* build a fake ACK */
	build_signal(&(recv_buf), ACK, 0xBB, 0x02, 0x55, 0x00, 0x0);
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* client 0xBB is now connected */

	/* build a fake TRANS_START packet */
	build_signal(&(recv_buf), TRANS_START, 0xAA, 0x01, 0x55, 0x00, 0x4);
	// TODO: get rid of this hack
	// force a big allocation
	recv_buf.data_size = 249;
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();

	/* build a fake TRANS_START packet */
	build_signal(&(recv_buf), TRANS_START, 0xBB, 0x02, 0x55, 0x00, 0x4);
	// TODO: get rid of this hack
	// force a big allocation, this time must fail
	recv_buf.data_size = 2;
	/* say that there is a packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(1);
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* say that there is no packet availible */
	mock().expectOneCall("mock_avail").andReturnValue(0);
	/* let the gateway compute */
	snpes_compute();
	/* check if the gateway tried to send a TRANS START packet */
	response = (Packet_t *) send_buf.data;
	CHECK_EQUAL(0x55, response->src_uid);
	CHECK_EQUAL(0x00, response->src_nid);
	CHECK_EQUAL(0xBB, response->dest_uid);
	CHECK_EQUAL(0x02, response->dest_nid);
	CHECK_EQUAL(FULL, ((response->flgs_seq)>>4)&0x0F);
	CHECK_EQUAL(0, response->flgs_seq&0x0F);
	CHECK_EQUAL(0, response->data_size);
}
