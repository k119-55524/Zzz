
#include <gtest/gtest.h>
#include "../../src/engine/private/core/templates/Event.h"

using namespace zzz::engine;

TEST(EventTest, VoidEvent)
{
	Event<void> evt;
	int callCount = 0;

	evt.SubscribeStaticUnsafe([&callCount]()
	{
		callCount++;
	});

	evt();
	EXPECT_EQ(callCount, 1);

	evt();
	EXPECT_EQ(callCount, 2);
}

TEST(EventTest, ArgsEvent)
{
	Event<int, std::string> evt;
	int lastInt = 0;
	std::string lastStr = "";

	evt.SubscribeStaticUnsafe([&lastInt, &lastStr](int i, std::string s)
	{
		lastInt = i;
		lastStr = s;
	});

	evt(42, "hello");
	EXPECT_EQ(lastInt, 42);
	EXPECT_EQ(lastStr, "hello");
}

TEST(EventTest, MultipleSubscribers)
{
	Event<void> evt;
	int callCount1 = 0;
	int callCount2 = 0;

	evt.SubscribeStaticUnsafe([&callCount1]() { callCount1++; });
	evt.SubscribeStaticUnsafe([&callCount2]() { callCount2++; });

	evt();
	EXPECT_EQ(callCount1, 1);
	EXPECT_EQ(callCount2, 1);
}

TEST(EventTest, Clear)
{
	Event<void> evt;
	int callCount = 0;

	evt.SubscribeStaticUnsafe([&callCount]() { callCount++; });
	evt.clear();
	evt();

	EXPECT_EQ(callCount, 0);
}

TEST(EventTest, ContextUnsubscribe)
{
	Event<void> evt;
	int callCount = 0;

	{
		auto contextObj = std::make_shared<int>(42);
		evt.SubscribeUnsafe(contextObj, [&callCount]() { callCount++; });

		// Объект жив, вызов должен сработать
		evt();
		EXPECT_EQ(callCount, 1);
	} // contextObj уничтожается здесь

	// Объект мёртв, вызов не должен сработать, а ссылка должна удалиться
	evt();
	EXPECT_EQ(callCount, 1);
}

