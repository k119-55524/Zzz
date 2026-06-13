
#include <gtest/gtest.h>
#include "../../src/engine/private/core/templates/Event.h"

using namespace zzz::engine;

TEST(EventTest, VoidEvent)
{
	Event<> evt;
	int callCount = 0;

	evt.SubscribeStatic([&callCount]()
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

	evt.SubscribeStatic([&lastInt, &lastStr](int i, std::string s)
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
	Event<> evt;
	int callCount1 = 0;
	int callCount2 = 0;

	evt.SubscribeStatic([&callCount1]() { callCount1++; });
	evt.SubscribeStatic([&callCount2]() { callCount2++; });

	evt();
	EXPECT_EQ(callCount1, 1);
	EXPECT_EQ(callCount2, 1);
}

TEST(EventTest, Clear)
{
	Event<> evt;
	int callCount = 0;

	evt.SubscribeStatic([&callCount]() { callCount++; });
	evt.Clear();
	evt();

	EXPECT_EQ(callCount, 0);
}

TEST(EventTest, ContextUnsubscribe)
{
	Event<> evt;
	int callCount = 0;

	{
		auto contextObj = std::make_shared<int>(42);
		evt.Subscribe(contextObj, [&callCount]() { callCount++; });

		// Объект жив, вызов должен сработать
		evt();
		EXPECT_EQ(callCount, 1);
	} // contextObj уничтожается здесь

	// Объект мёртв, вызов не должен сработать, а ссылка должна удалиться
	evt();
	EXPECT_EQ(callCount, 1);
}

