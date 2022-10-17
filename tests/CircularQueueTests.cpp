#include <CppUTest/TestHarness.h>

extern "C"{
#include <CircularQueue.h>
}

TEST_GROUP(CircularQueueTests)
{
	uint8_t buffer[sizeof(int)*4];
	Queue_t test_queue {
		(void *)buffer,
		sizeof(int),
		4,
		0,
		0
	};
};

TEST(CircularQueueTests, Init)
{
	queue_init(&test_queue);
	CHECK_EQUAL(-1, test_queue.head);
	CHECK_EQUAL(-1, test_queue.tail);
}

TEST(CircularQueueTests, QueueFirstPush)
{
	int a = 12;
	queue_init(&test_queue);
	queue_push(&test_queue, &a);
	CHECK_EQUAL(12, *(int *)buffer);
}

TEST(CircularQueueTests, QueueTwoPushs)
{
	int a = 12;
	int b = 14;
	queue_init(&test_queue);
	queue_push(&test_queue, &a);
	queue_push(&test_queue, &b);
	CHECK_EQUAL(12, *(int *)buffer);
	CHECK_EQUAL(14, *(int *)(buffer+test_queue.elmt_size));
}

TEST(CircularQueueTests, QueueIsFull)
{
	int a = 12;
	int8_t retval = 0;
	queue_init(&test_queue);
	queue_push(&test_queue, &a);
	queue_push(&test_queue, &a);
	queue_push(&test_queue, &a);
	queue_push(&test_queue, &a);
	retval = queue_push(&test_queue, &a);
	CHECK_EQUAL(-1, retval);
}

TEST(CircularQueueTests, EmptyQueue)
{
	void *retval = buffer;
	queue_init(&test_queue);
	retval = queue_pop(&test_queue);
	POINTERS_EQUAL(NULL, retval);
}

TEST(CircularQueueTests, PopLastElement)
{
	int a = 13;
	void *retval;
	queue_init(&test_queue);
	queue_push(&test_queue, &a);
	retval = queue_pop(&test_queue);
	CHECK_EQUAL(13, *(int *)retval);
	CHECK_EQUAL(-1, test_queue.head);
	CHECK_EQUAL(-1, test_queue.tail);
}

TEST(CircularQueueTests, WrapAround)
{
	int a = 13;
	int b = 15;
	void *retval;
	queue_init(&test_queue);
	queue_push(&test_queue, &a);
	queue_push(&test_queue, &b);
	queue_push(&test_queue, &a);
	queue_push(&test_queue, &a);
	retval = queue_pop(&test_queue);
	CHECK_EQUAL(1, test_queue.tail);
	CHECK_EQUAL(13, *(int *)retval);
	queue_push(&test_queue, &a);
	CHECK_EQUAL(0, test_queue.head);
	retval = queue_pop(&test_queue);
	CHECK_EQUAL(15, *(int *)retval);
	CHECK_EQUAL(2, test_queue.tail);
}
