#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_CORE_BIT_TREE_TRACKER

#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include "core/containers/BitTreeTracker.h"

using namespace zzz::core;
using zzz::zU32;

TEST(BitTreeTrackerTest, InitialStateAndPrepare)
{
	BitTreeTracker tracker;
	EXPECT_TRUE(tracker.GetDirtyIndices().empty());

	EXPECT_NO_THROW(tracker.Prepare(0));
	tracker.Set(0);
	ASSERT_EQ(tracker.GetDirtyIndices().size(), 1u);

	BitTreeTracker emptySceneTracker(0);
	emptySceneTracker.Set(0);
	ASSERT_EQ(emptySceneTracker.GetDirtyIndices().size(), 1u);

	tracker.Prepare(100);
	EXPECT_TRUE(tracker.GetDirtyIndices().empty());

	// Выставляем бит и проверяем возврат через Prepare
	tracker.Set(10);
	auto dirty = tracker.GetDirtyIndices();
	ASSERT_EQ(dirty.size(), 1u);
	EXPECT_EQ(dirty[0], 10u);

	// Новый Prepare сбрасывает трекер
	tracker.Prepare(50);
	EXPECT_TRUE(tracker.GetDirtyIndices().empty());
}

TEST(BitTreeTrackerTest, BoundaryIndices)
{
	constexpr uint32_t capacity = 10000;
	BitTreeTracker tracker(capacity);

	const std::vector<uint32_t> boundaries = { 0, 63, 64, 65, 127, 128, 4095, 4096, 4097, 8191, 8192, 9999 };

	for (uint32_t idx : boundaries)
	{
		tracker.Set(idx);
		auto dirty = tracker.GetDirtyIndices();
		ASSERT_EQ(dirty.size(), 1u);
		EXPECT_EQ(dirty[0], idx);

		tracker.Prepare(capacity);
		EXPECT_TRUE(tracker.GetDirtyIndices().empty());
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

	auto dirty = tracker.GetDirtyIndices();
	ASSERT_EQ(dirty.size(), expectedIndices.size());
	for (size_t i = 0; i < dirty.size(); ++i)
	{
		EXPECT_EQ(dirty[i], expectedIndices[i]);
	}
}

TEST(BitTreeTrackerTest, DenseGetDirtyIndices)
{
	BitTreeTracker tracker(200);
	for (uint32_t i = 10; i < 30; ++i)
	{
		tracker.Set(i);
	}

	auto dirty = tracker.GetDirtyIndices();
	ASSERT_EQ(dirty.size(), 20u);
	for (uint32_t i = 0; i < 20; ++i)
	{
		EXPECT_EQ(dirty[i], 10u + i);
	}
}

TEST(BitTreeTrackerTest, PrepareShrinksLogicalCapacity)
{
	BitTreeTracker tracker(100);
	tracker.Set(99);

	tracker.Prepare(50);
	EXPECT_TRUE(tracker.GetDirtyIndices().empty());

	tracker.Set(49);
	const auto dirty = tracker.GetDirtyIndices();
	ASSERT_EQ(dirty.size(), 1u);
	EXPECT_EQ(dirty[0], 49u);
}

#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
TEST(BitTreeTrackerTest, SetOutOfRangeTerminates)
{
	EXPECT_DEATH(
		{
			BitTreeTracker tracker(1);
			tracker.Set(1);
		},
		"BitTreeTracker::Set: index must be less than capacity");
}
#endif

#endif // Z_TEST_CORE_BIT_TREE_TRACKER
