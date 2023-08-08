#include <CppUTest/TestHarness.h>

extern "C" {
#include <MemoryManager.h>
#include <string.h>
}

TEST_GROUP(MemoryManagerTests)
{
	uint8_t buffer[24];
	MemMgr_t heap;
};

TEST(MemoryManagerTests, InitOk)
{
	memmgr_init(&heap, (void *)buffer, 24);
	POINTERS_EQUAL((void *)buffer, heap.heap_start);
	POINTERS_EQUAL((void *)&buffer[24], heap.heap_end);
	CHECK_EQUAL(24, ((uint16_t *)buffer)[0]);
	CHECK_EQUAL(24, ((uint16_t *)buffer)[11]);
}

TEST(MemoryManagerTests, InitFail)
{
	int8_t ret = 0;
	ret = memmgr_init(&heap, (void *)buffer, 23);
	CHECK_EQUAL(-1, ret);
}

TEST(MemoryManagerTests, Alloc)
{
	void *ret = nullptr;
	memmgr_init(&heap, (void *)buffer, 24);
	ret = memmgr_alloc(&heap, 2);
	POINTERS_EQUAL((void *)&((uint16_t *)buffer)[1], ret);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[0]);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[3]);
	CHECK_EQUAL(16, ((uint16_t *)buffer)[4]);
	CHECK_EQUAL(16, ((uint16_t *)buffer)[11]);
	ret = memmgr_alloc(&heap, 1);
	POINTERS_EQUAL((void *)&((uint16_t *)buffer)[5], ret);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[0]);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[3]);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[4]);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[7]);
	CHECK_EQUAL(8, ((uint16_t *)buffer)[8]);
	CHECK_EQUAL(8, ((uint16_t *)buffer)[11]);
	ret = memmgr_alloc(&heap, 2);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[0]);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[3]);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[4]);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[7]);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[8]);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[11]);
	// full codition
	ret = memmgr_alloc(&heap, 1);
	POINTERS_EQUAL(nullptr, ret);
}

TEST(MemoryManagerTests, Free)
{
	void *ret1 = nullptr;
	void *ret2 = nullptr;
	void *ret3 = nullptr;
	memmgr_init(&heap, (void *)buffer, 24);
	ret1 = memmgr_alloc(&heap, 2);
	memset(ret1, 'A', 4);
	ret2 = memmgr_alloc(&heap, 1);
	memset(ret2, 'B', 4);
	ret3 = memmgr_alloc(&heap, 2);
	memset(ret3, 'C', 4);
	CHECK_EQUAL('A', buffer[2]);
	CHECK_EQUAL('A', buffer[3]);
	CHECK_EQUAL('A', buffer[4]);
	CHECK_EQUAL('A', buffer[5]);
	CHECK_EQUAL('B', buffer[10]);
	CHECK_EQUAL('B', buffer[11]);
	CHECK_EQUAL('B', buffer[12]);
	CHECK_EQUAL('B', buffer[13]);
	CHECK_EQUAL('C', buffer[18]);
	CHECK_EQUAL('C', buffer[19]);
	CHECK_EQUAL('C', buffer[20]);
	CHECK_EQUAL('C', buffer[21]);
	memmgr_free(&heap, ret1);
	CHECK_EQUAL(8, ((uint16_t *)buffer)[0]);
	CHECK_EQUAL(8, ((uint16_t *)buffer)[3]);
	memmgr_free(&heap, ret3);
	CHECK_EQUAL(8, ((uint16_t *)buffer)[8]);
	CHECK_EQUAL(8, ((uint16_t *)buffer)[11]);
	// test a memory invasion
	memmgr_free(&heap, &buffer[24]);
	memmgr_free(&heap, ret2);
	CHECK_EQUAL(24, ((uint16_t *)buffer)[0]);
	CHECK_EQUAL(24, ((uint16_t *)buffer)[11]);
}

TEST(MemoryManagerTests, DifferentBlockSizes)
{
	memmgr_init(&heap, (void *)buffer, 24);
	memmgr_alloc(&heap, 5);
	memmgr_alloc(&heap, 2);
	CHECK_EQUAL(17, ((uint16_t *)buffer)[0]);
	CHECK_EQUAL(17, ((uint16_t *)buffer)[7]);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[8]);
	CHECK_EQUAL(9, ((uint16_t *)buffer)[11]);
}

TEST(MemoryManagerTests, RemainingBytes)
{
	void *ptr = NULL;
	uint16_t ret = 0;
	memmgr_init(&heap, (void *)buffer, 24);
	ret = memmgr_remaining(&heap);
	CHECK_EQUAL(24, ret);
	ptr = memmgr_alloc(&heap, 5);
	ret = memmgr_remaining(&heap);
	CHECK_EQUAL(8, ret);
	memmgr_alloc(&heap, 2);
	ret = memmgr_remaining(&heap);
	CHECK_EQUAL(0, ret);
	memmgr_free(&heap, ptr);
	ret = memmgr_remaining(&heap);
	CHECK_EQUAL(16, ret);
}
