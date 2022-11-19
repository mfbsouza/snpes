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
	uint8_t retval = 13;
	queue_init(&test_queue);
	CHECK_EQUAL(-1, test_queue.head);
	CHECK_EQUAL(-1, test_queue.tail);
	retval = queue_remaining(&test_queue);
	CHECK_EQUAL(4, retval);
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
	a = queue_remaining(&test_queue);
	CHECK_EQUAL(2, a);
}

TEST(CircularQueueTests, QueueIsFull)
{
	int a = 12;
	int8_t retval = 0;
	uint8_t remaining = 13;
	queue_init(&test_queue);
	queue_push(&test_queue, &a);
	queue_push(&test_queue, &a);
	queue_push(&test_queue, &a);
	queue_push(&test_queue, &a);
	retval = queue_push(&test_queue, &a);
	CHECK_EQUAL(-1, retval);
	remaining = queue_remaining(&test_queue);
	CHECK_EQUAL(0, remaining);
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

TEST(CircularQueueTests, QueueIsEmpty)
{
	int a = 14;
	int ret = 0;
	queue_init(&test_queue);
	ret = queue_empty(&test_queue);
	CHECK_EQUAL(1, ret);
	queue_push(&test_queue, &a);
	ret = queue_empty(&test_queue);
	CHECK_EQUAL(0, ret);
}

TEST(CircularQueueTests, CheckFullState)
{
	int c = 18;
	uint8_t retval = 10;
	queue_init(&test_queue);
	retval = queue_full(&test_queue);
	CHECK_EQUAL(0, retval);
	retval = 10;
	queue_push(&test_queue, &c);
	retval = queue_full(&test_queue);
	CHECK_EQUAL(0, retval);
	queue_push(&test_queue, &c);
	queue_push(&test_queue, &c);
	queue_push(&test_queue, &c);
	retval = queue_full(&test_queue);
	CHECK_EQUAL(1, retval);
}

TEST(CircularQueueTests, QueuePeek)
{
	int a = 13;
	void *addr = NULL;
	queue_init(&test_queue);
	addr = queue_peek(&test_queue);
	POINTERS_EQUAL(NULL, addr);
	queue_push(&test_queue, &a);
	addr = queue_peek(&test_queue);
	CHECK_EQUAL(13, *(int *)addr);
}

TEST(CircularQueueTests, QueueOfPointers)
{
	int* pointer_buf[3];
	Queue_t pointer_queue {
		(void *)pointer_buf,
		sizeof(int *),
		3,
		0,
		0
	};
	int a = 13;
	int *ptr = &a;
	void *retval = NULL;
	queue_init(&pointer_queue);
	queue_push(&pointer_queue, &ptr);
	retval = queue_pop(&pointer_queue);
	POINTERS_EQUAL(ptr, *(int **)retval);
	CHECK_EQUAL(13, *(*(int **)retval));
}
