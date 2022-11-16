#include <CppUTest/TestHarness.h>

extern "C"{
#include <MemoryManager.h>
}

TEST_GROUP(MemoryManagerTests)
{
	uint8_t buffer[12];
	MemMgr_t heap;
};

TEST(MemoryManagerTests, Init)
{
	memmgr_init(&heap, (void *)buffer, 12);
	POINTERS_EQUAL((void *)buffer, heap.heap_start);
	POINTERS_EQUAL((void *)&buffer[12], heap.heap_end);
	CHECK_EQUAL(12, buffer[0]);
	CHECK_EQUAL(12, buffer[11]);
}

TEST(MemoryManagerTests, Alloc)
{
	void *ret = nullptr;
	memmgr_init(&heap, (void *)buffer, 12);
	ret = memmgr_alloc(&heap, 2);
	POINTERS_EQUAL((void *)&buffer[1], ret);
	CHECK_EQUAL(5, buffer[0]);
	CHECK_EQUAL(5, buffer[3]);
	CHECK_EQUAL(8, buffer[4]);
	CHECK_EQUAL(8, buffer[11]);
	ret = memmgr_alloc(&heap, 1);
	POINTERS_EQUAL((void *)&buffer[5], ret);
	CHECK_EQUAL(5, buffer[0]);
	CHECK_EQUAL(5, buffer[3]);
	CHECK_EQUAL(5, buffer[4]);
	CHECK_EQUAL(5, buffer[7]);
	CHECK_EQUAL(4, buffer[8]);
	CHECK_EQUAL(4, buffer[11]);
	ret = memmgr_alloc(&heap, 2);
	CHECK_EQUAL(5, buffer[0]);
	CHECK_EQUAL(5, buffer[3]);
	CHECK_EQUAL(5, buffer[4]);
	CHECK_EQUAL(5, buffer[7]);
	CHECK_EQUAL(5, buffer[8]);
	CHECK_EQUAL(5, buffer[11]);
	// full codition
	ret = memmgr_alloc(&heap, 1);
	POINTERS_EQUAL(nullptr, ret);
}

TEST(MemoryManagerTests, Free)
{
	void *ret1 = nullptr;
	void *ret2 = nullptr;
	void *ret3 = nullptr;
	memmgr_init(&heap, (void *)buffer, 12);
	ret1 = memmgr_alloc(&heap, 2);
	ret2 = memmgr_alloc(&heap, 1);
	ret3 = memmgr_alloc(&heap, 2);
	memmgr_free(&heap, ret1);
	CHECK_EQUAL(4, buffer[0]);
	CHECK_EQUAL(4, buffer[3]);
	memmgr_free(&heap, ret3);
	CHECK_EQUAL(4, buffer[8]);
	CHECK_EQUAL(4, buffer[11]);
	// test a memory invasion
	memmgr_free(&heap, &buffer[12]);
	memmgr_free(&heap, ret2);
	CHECK_EQUAL(12, buffer[0]);
	CHECK_EQUAL(12, buffer[11]);
}

TEST(MemoryManagerTests, DifferentBlockSizes)
{
	memmgr_init(&heap, (void *)buffer, 12);
	memmgr_alloc(&heap, 5);
	memmgr_alloc(&heap, 2);
	CHECK_EQUAL(9, buffer[0]);
	CHECK_EQUAL(9, buffer[7]);
	CHECK_EQUAL(5, buffer[8]);
	CHECK_EQUAL(5, buffer[11]);
}
