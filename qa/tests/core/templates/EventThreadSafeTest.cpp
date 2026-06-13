#include <gtest/gtest.h>
#include "../../src/engine/private/core/templates/EventThreadSafe.h"

using namespace zzz::engine;

TEST(EventThreadSafeTest, VoidEvent)
{
	EventThreadSafe<void> evt;
	int callCount = 0;

	evt.SubscribeStaticSafe([&callCount]()
	{
		callCount++;
	});

	evt();
	EXPECT_EQ(callCount, 1);

	evt();
	EXPECT_EQ(callCount, 2);
}

TEST(EventThreadSafeTest, ArgsEvent)
{
	EventThreadSafe<int, std::string> evt;
	int lastInt = 0;
	std::string lastStr = "";

	evt.SubscribeStaticSafe([&lastInt, &lastStr](int i, std::string s)
	{
		lastInt = i;
		lastStr = s;
	});

	evt(42, "hello");
	EXPECT_EQ(lastInt, 42);
	EXPECT_EQ(lastStr, "hello");
}

TEST(EventThreadSafeTest, MultipleSubscribers)
{
	EventThreadSafe<void> evt;
	int callCount1 = 0;
	int callCount2 = 0;

	evt.SubscribeStaticSafe([&callCount1]() { callCount1++; });
	evt.SubscribeStaticSafe([&callCount2]() { callCount2++; });

	evt();
	EXPECT_EQ(callCount1, 1);
	EXPECT_EQ(callCount2, 1);
}

TEST(EventThreadSafeTest, Clear)
{
	EventThreadSafe<void> evt;
	int callCount = 0;

	evt.SubscribeStaticSafe([&callCount]() { callCount++; });
	evt.clear();
	evt();

	EXPECT_EQ(callCount, 0);
}

TEST(EventThreadSafeTest, ContextUnsubscribe)
{
	EventThreadSafe<void> evt;
	int callCount = 0;

	{
		auto contextObj = std::make_shared<int>(42);
		evt.SubscribeSafe(contextObj, [&callCount]() { callCount++; });

		evt();
		EXPECT_EQ(callCount, 1);
	}

	evt();
	EXPECT_EQ(callCount, 1);
}
