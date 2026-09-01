
#include "qa/tests/TestsConfig.h"

#ifdef Z_TEST_CORE_TEMPLATES

#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include <core/Core.h>

using namespace zzz::core;

TEST(DoubleBufferedVectorTest, BasicPushAndSwap)
{
	DoubleBufferedVector<int> vec;
	vec.Push(1);
	vec.Push(2);
	vec.Emplace(3);

	auto& readBuf = vec.SwapAndGetReadBuffer();
	ASSERT_EQ(readBuf.size(), 3);
	EXPECT_EQ(readBuf[0], 1);
	EXPECT_EQ(readBuf[1], 2);
	EXPECT_EQ(readBuf[2], 3);

	// After swap, the next write buffer should be initially empty
	vec.Push(4);
	auto& readBuf2 = vec.SwapAndGetReadBuffer();
	ASSERT_EQ(readBuf2.size(), 1);
	EXPECT_EQ(readBuf2[0], 4);
}

TEST(DoubleBufferedVectorTest, ReserveConstructor)
{
	DoubleBufferedVector<int> vec(100);
	vec.Push(10);

	auto& readBuf = vec.SwapAndGetReadBuffer();
	ASSERT_EQ(readBuf.size(), 1);
	EXPECT_GE(readBuf.capacity(), 100);
}

TEST(DoubleBufferedVectorTest, MultiThreadedWriting)
{
	DoubleBufferedVector<int> vec;

	auto writer = [&vec]() {
		for (int i = 0; i < 1000; ++i) {
			vec.Push(i);
		}
		};

	std::thread t1(writer);
	std::thread t2(writer);
	std::thread t3(writer);

	t1.join();
	t2.join();
	t3.join();

	auto& readBuf = vec.SwapAndGetReadBuffer();
	// 3 threads each wrote 1000 items
	EXPECT_EQ(readBuf.size(), 3000);
}

TEST(DoubleBufferedVectorTest, StressTest)
{
	DoubleBufferedVector<int> vec(50000);
	std::atomic<bool> running{ true };
	std::atomic<size_t> totalPushed{ 0 };

	// Потоки-писатели постоянно спамят в вектор
	auto writer = [&]() {
		size_t count = 0;
		while (running.load(std::memory_order_relaxed)) {
			vec.Push(1);
			count++;
		}
		totalPushed.fetch_add(count, std::memory_order_relaxed);
		};

	std::vector<std::thread> writers;
	for (int i = 0; i < 8; ++i) {
		writers.emplace_back(writer);
	}

	// Главный поток (читатель) постоянно делает Swap и "обрабатывает" данные в течение 500мс
	size_t totalRead = 0;
	auto startTime = std::chrono::steady_clock::now();
	while (std::chrono::steady_clock::now() - startTime < std::chrono::milliseconds(500)) {
		auto& readBuf = vec.SwapAndGetReadBuffer();
		totalRead += readBuf.size();
		std::this_thread::sleep_for(std::chrono::milliseconds(2)); // Имитация работы
	}

	// Останавливаем писателей
	running.store(false, std::memory_order_relaxed);
	for (auto& t : writers) {
		t.join();
	}

	// Делаем финальный Swap, чтобы забрать последние остатки данных, 
	// которые писатели успели закинуть после нашего последнего Swap.
	auto& readBuf = vec.SwapAndGetReadBuffer();
	totalRead += readBuf.size();

	// Проверяем, что ни один элемент не потерялся под огромной нагрузкой
	EXPECT_EQ(totalRead, totalPushed.load());
}

#endif // Z_TEST_CORE_TEMPLATES
