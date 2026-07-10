
#include <gtest/gtest.h>
#include <common/templates/Event.h>
#include <thread>

using namespace zzz::engine;

static std::shared_ptr<int> g_ctx = std::make_shared<int>(0);

TEST(EventTest, VoidEvent)
{
	Event<> evt;
	int callCount = 0;

	evt.Subscribe(g_ctx, [&callCount]()
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

	evt.Subscribe(g_ctx, [&lastInt, &lastStr](int i, std::string s)
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

	evt.Subscribe(g_ctx, [&callCount1]() { callCount1++; });
	evt.Subscribe(g_ctx, [&callCount2]() { callCount2++; });

	evt();
	EXPECT_EQ(callCount1, 1);
	EXPECT_EQ(callCount2, 1);
}

TEST(EventTest, Clear)
{
	Event<> evt;
	int callCount = 0;

	evt.Subscribe(g_ctx, [&callCount]() { callCount++; });
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

TEST(EventTest, MultithreadedSpam)
{
	Event<> evt;
	std::atomic<int> counter = 0;
	auto ctx = std::make_shared<int>(0);
	
	auto threadFunc = [&]() {
		for (int i = 0; i < 1000; ++i)
		{
			evt.Subscribe(ctx, [&counter]() { counter++; });
			evt();
			evt.Unsubscribe(ctx);
		}
	};

	std::vector<std::thread> threads;
	for (int i = 0; i < 10; ++i)
		threads.emplace_back(threadFunc);

	for (auto& t : threads)
		t.join();

	// Мы не проверяем точное значение счетчика, так как это гонка.
	// Главная цель теста — убедиться, что конкурентный доступ не вызывает краш.
	SUCCEED();
}

TEST(EventTest, Reentrancy)
{
	Event<> evt;
	int callCount = 0;

	// Подписываем функцию, которая внутри себя делает новую подписку
	evt.Subscribe(g_ctx, [&]() {
		callCount++;
		if (callCount == 1)
		{
			evt.Subscribe(g_ctx, [&]() { callCount += 10; });
		}
	});

	// Первый вызов: callCount станет 1, добавится новая подписка (в pendingAdditions)
	evt();
	EXPECT_EQ(callCount, 1);

	// Второй вызов: сработает старая подписка (+1) и новая подписка (+10)
	evt();
	EXPECT_EQ(callCount, 12);
}

TEST(EventTest, OrderedVsUnordered)
{
	Event<int> orderedEvt;
	UnorderedEvent<int> unorderedEvt;
	
	std::vector<int> orderedResult;
	std::vector<int> unorderedResult;

	auto ctx1 = std::make_shared<int>(1);
	auto ctx2 = std::make_shared<int>(2);
	auto ctx3 = std::make_shared<int>(3);

	orderedEvt.Subscribe(ctx1, [&](int val) { orderedResult.push_back(1); });
	orderedEvt.Subscribe(ctx2, [&](int val) { orderedResult.push_back(2); });
	orderedEvt.Subscribe(ctx3, [&](int val) { orderedResult.push_back(3); });

	unorderedEvt.Subscribe(ctx1, [&](int val) { unorderedResult.push_back(1); });
	unorderedEvt.Subscribe(ctx2, [&](int val) { unorderedResult.push_back(2); });
	unorderedEvt.Subscribe(ctx3, [&](int val) { unorderedResult.push_back(3); });

	// Удаляем средний элемент
	orderedEvt.Unsubscribe(ctx2);
	unorderedEvt.Unsubscribe(ctx2);

	orderedEvt(0);
	unorderedEvt(0);

	// Ordered должен сохранить порядок оставшихся: 1, 3
	ASSERT_EQ(orderedResult.size(), 2);
	EXPECT_EQ(orderedResult[0], 1);
	EXPECT_EQ(orderedResult[1], 3);

	// Unordered использует Swap-and-Pop, поэтому на место 2 встанет 3.
	// Результат будет: 1, 3. Подождите...
	// Индексы: [0]=1, [1]=2, [2]=3.
	// Удаляем [1]. На место [1] встает [2].
	// Вектор становится: [0]=1, [1]=3.
	// Ого, в данном конкретном случае порядок совпадёт.
	// Чтобы точно сбить порядок, подпишем 4 элемента и удалим первый!
	unorderedResult.clear();
	UnorderedEvent<int> uEvt2;
	auto c1 = std::make_shared<int>(1);
	auto c2 = std::make_shared<int>(2);
	auto c3 = std::make_shared<int>(3);
	auto c4 = std::make_shared<int>(4);

	uEvt2.Subscribe(c1, [&](int val) { unorderedResult.push_back(1); });
	uEvt2.Subscribe(c2, [&](int val) { unorderedResult.push_back(2); });
	uEvt2.Subscribe(c3, [&](int val) { unorderedResult.push_back(3); });
	uEvt2.Subscribe(c4, [&](int val) { unorderedResult.push_back(4); });

	uEvt2.Unsubscribe(c1); 
	
	// Первый вызов: c1 помечен мертвым, просто пропускается.
	// Результат вызова: 2, 3, 4
	uEvt2(0); 

	ASSERT_EQ(unorderedResult.size(), 3);
	EXPECT_EQ(unorderedResult[0], 2);
	EXPECT_EQ(unorderedResult[1], 3);
	EXPECT_EQ(unorderedResult[2], 4);

	// После первого вызова срабатывает очистка (Swap-and-Pop).
	// Последний элемент (4) ставится на место удаленного (1).
	// Порядок в памяти становится: 4, 2, 3.
	
	unorderedResult.clear();
	uEvt2(0);

	ASSERT_EQ(unorderedResult.size(), 3);
	EXPECT_EQ(unorderedResult[0], 4);
	EXPECT_EQ(unorderedResult[1], 2);
	EXPECT_EQ(unorderedResult[2], 3);
}

