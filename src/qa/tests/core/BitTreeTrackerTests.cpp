#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_CORE_BIT_TREE_TRACKER

#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include "core/containers/BitTreeTracker.h"

using namespace zzz::core;

TEST(BitTreeTrackerTest, InitialStateAndPrepare)
{
	BitTreeTracker tracker;
	std::vector<uint32_t> dirty;
	EXPECT_EQ(tracker.GetDirtyIndices(dirty), 0u);
	EXPECT_TRUE(dirty.empty());

	tracker.Prepare(100);
	EXPECT_EQ(tracker.GetDirtyIndices(dirty), 0u);
	EXPECT_TRUE(dirty.empty());

	// Выставляем бит и проверяем возврат через Prepare
	tracker.Set(10);
	EXPECT_EQ(tracker.GetDirtyIndices(dirty), 1u);
	ASSERT_EQ(dirty.size(), 1u);
	EXPECT_EQ(dirty[0], 10u);

	// Новый Prepare сбрасывает трекер
	tracker.Prepare(50);
	EXPECT_EQ(tracker.GetDirtyIndices(dirty), 0u);
	EXPECT_TRUE(dirty.empty());
}

TEST(BitTreeTrackerTest, BoundaryIndices)
{
	constexpr uint32_t capacity = 10000;
	BitTreeTracker tracker(capacity);

	const std::vector<uint32_t> boundaries = { 0, 63, 64, 65, 127, 128, 4095, 4096, 4097, 8191, 8192, 9999 };

	std::vector<uint32_t> dirty;
	for (uint32_t idx : boundaries)
	{
		tracker.Set(idx);
		EXPECT_EQ(tracker.GetDirtyIndices(dirty), 1u);
		ASSERT_EQ(dirty.size(), 1u);
		EXPECT_EQ(dirty[0], idx);

		tracker.Prepare(capacity);
		EXPECT_EQ(tracker.GetDirtyIndices(dirty), 0u);
	}
}

TEST(BitTreeTrackerTest, SparseGetDirtyIndices)
{
	constexpr uint32_t capacity = 20000;
	BitTreeTracker tracker(capacity);

	const std::vector<uint32_t> expectedIndices = { 0, 1, 63, 64, 128, 500, 4095, 4096, 15000, 19999 };
	for (uint32_t idx : expectedIndices)
	{
		tracker.Set(idx);
	}

	std::vector<uint32_t> dirty;
	const size_t count = tracker.GetDirtyIndices(dirty);
	EXPECT_EQ(count, expectedIndices.size());
	EXPECT_EQ(dirty, expectedIndices);
}

TEST(BitTreeTrackerTest, DenseGetDirtyIndices)
{
	BitTreeTracker tracker(200);
	for (uint32_t i = 10; i < 30; ++i)
	{
		tracker.Set(i);
	}

	std::vector<uint32_t> dirty;
	const size_t count = tracker.GetDirtyIndices(dirty);
	ASSERT_EQ(count, 20u);
	ASSERT_EQ(dirty.size(), 20u);
	for (uint32_t i = 0; i < 20; ++i)
	{
		EXPECT_EQ(dirty[i], 10u + i);
	}
}

#endif // Z_TEST_CORE_BIT_TREE_TRACKER
