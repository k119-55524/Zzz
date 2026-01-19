
#include "pch.h"

export module ThreadPool;

import Platforms;
import QueueArray;

using namespace zzz;

namespace zzz::templates
{
	// Установка имени потока для отладки в Visual Studio
#if defined(_MSC_VER) && defined(_DEBUG)
	inline void SetThreadName(const char* threadName)
	{
		const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
		struct THREADNAME_INFO
		{
			DWORD dwType;       // must be 0x1000
			LPCSTR szName;      // имя потока
			DWORD dwThreadID;   // Thread ID (-1 = текущий поток)
			DWORD dwFlags;      // reserved, must be zero
		} info;
		info.dwType = 0x1000;
		info.szName = threadName;
		info.dwThreadID = -1;  // текущий поток
		info.dwFlags = 0;
#pragma pack(pop)

		__try
		{
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
#else
	inline void SetThreadName(const char*) {}
#endif

	// Потокобезопасная очередь на основе QueueArray
	template<typename T>
	class ThreadSafeArrayQueue
	{
	public:
		ThreadSafeArrayQueue() = delete;
		ThreadSafeArrayQueue(const ThreadSafeArrayQueue&) = delete;
		ThreadSafeArrayQueue& operator=(const ThreadSafeArrayQueue&) = delete;

		explicit ThreadSafeArrayQueue(size_t startArraySize) :
			threadQueue(startArraySize),
			head{ 0 },
			tail{ 0 },
			lengthQueue{ 0 }
		{
		}

		ThreadSafeArrayQueue(ThreadSafeArrayQueue&& other) noexcept :
			threadQueue(std::move(other.threadQueue)),
			head{ other.head },
			tail{ other.tail },
			lengthQueue{ other.lengthQueue }
		{
			other.head = 0;
			other.tail = 0;
			other.lengthQueue = 0;
		}

		ThreadSafeArrayQueue& operator=(ThreadSafeArrayQueue&& other) noexcept
		{
			if (this != &other)
			{
				threadQueue = std::move(other.threadQueue);
				head = other.head;
				tail = other.tail;
				lengthQueue = other.lengthQueue;
				other.head = 0;
				other.tail = 0;
				other.lengthQueue = 0;
			}
			return *this;
		}

		void push(const T& value)
		{
			std::lock_guard<std::mutex> lk(mut);
			threadQueue.PushBack(value);
			tail = (tail + 1) % threadQueue.Capacity();
			lengthQueue++;
			cv.notify_one();
		}

		void push(T&& value)
		{
			std::lock_guard<std::mutex> lk(mut);
			threadQueue.PushBack(std::move(value));
			tail = (tail + 1) % threadQueue.Capacity();
			lengthQueue++;
			cv.notify_one();
		}

		bool pop(T& value) noexcept
		{
			std::lock_guard<std::mutex> lk(mut);
			if (lengthQueue == 0)
				return false;

			value = std::move(threadQueue[head]);
			head = (head + 1) % threadQueue.Capacity();
			lengthQueue--;

			if (lengthQueue == 0)
			{
				head = tail = 0;
				threadQueue.Clear();
			}
			return true;
		}

		bool IsEmpty() const noexcept
		{
			std::lock_guard<std::mutex> lk(mut);
			return lengthQueue == 0;
		}

		size_t Length() const noexcept
		{
			std::lock_guard<std::mutex> lk(mut);
			return lengthQueue;
		}

	private:
		QueueArray<T> threadQueue;
		size_t head;
		size_t tail;
		size_t lengthQueue;
		mutable std::mutex mut;
		std::condition_variable cv;
	};

	// Пул потоков для выполнения задач
	export class ThreadPool
	{
		Z_NO_CREATE_COPY(ThreadPool);

	public:
		explicit ThreadPool(std::string threadName, size_t threadCount) :
			done{ false },
			threadCount{ threadCount },
			activeThreadCount{ 0 },
			workQueue(threadCount)
		{
			if (threadCount == 0)
				throw std::invalid_argument(">>>>> [ThreadPool::ThreadPool]. Parameters cannot be 0.");

			threads.reserve(threadCount);
			try
			{
				for (size_t i = 0; i < threadCount; ++i)
				{
					threads.emplace_back(&ThreadPool::WorkerThread, this, threadName, i);
				}
			}
			catch (...)
			{
				done = true;
				cv.notify_all();
				for (auto& thread : threads)
				{
					if (thread.joinable())
						thread.join();
				}
				throw std::runtime_error(">>>>> [ThreadPool::ThreadPool]. Failed to create threads.");
			}
		}

		~ThreadPool()
		{
			{
				std::lock_guard<std::mutex> lock(cv_mutex);
				done = true;
			}
			cv.notify_all();

			for (auto& thread : threads)
			{
				if (thread.joinable())
					thread.join();
			}
		}

		template<typename FunctionType, typename... Args>
		void Submit(FunctionType&& f, Args&&... args)
		{
			{
				std::lock_guard<std::mutex> lock(cv_mutex);
				if (done) return; // Не добавляем задачи если пул закрывается
			}

			workQueue.push([f = std::forward<FunctionType>(f), ...args = std::forward<Args>(args)]() mutable { f(args...); });
			cv.notify_one();
		}

		bool IsCompleted()
		{
			std::lock_guard<std::mutex> lk(cv_mutex);
			bool queueEmpty = workQueue.IsEmpty();
			uint32_t activeCount = activeThreadCount.load();

			return queueEmpty && activeCount == 0;
		}

		void Join()
		{
			std::unique_lock<std::mutex> lk(cv_mutex);

			// Ждем пока все задачи не будут выполнены
			cv.wait(lk, [this]
				{
					bool queueEmpty = workQueue.IsEmpty();
					uint32_t activeCount = activeThreadCount.load();
					return queueEmpty && activeCount == 0;
				});
		}

	private:
		size_t threadCount;
		std::atomic_bool done;
		std::atomic_uint activeThreadCount;
		ThreadSafeArrayQueue<std::function<void()>> workQueue;
		std::vector<std::thread> threads;
		std::mutex cv_mutex;
		std::condition_variable cv;

		void WorkerThread(std::string _threadName, size_t id)
		{
#if defined(_MSC_VER) && defined(_DEBUG)
			// Устанавливаем имя потока для отладки только в Visual Studio
			std::string threadName = std::format(">>>>> [zzz::ThreadPool]. Thread-using class: zzz::{}({})", _threadName, id);
			SetThreadName(threadName.c_str());
#endif

			while (true)
			{
				std::function<void()> task;

				{
					std::unique_lock<std::mutex> lk(cv_mutex);

					// Ждем задачу или сигнал завершения
					cv.wait(lk, [this] {
						return !workQueue.IsEmpty() || done;
						});

					// Если пул закрывается и очередь пуста - выходим
					if (done && workQueue.IsEmpty())
						return;

					// Пытаемся взять задачу
					if (!workQueue.pop(task))
						continue; // Нет задач, продолжаем ожидание

					// Увеличиваем счетчик активных потоков
					activeThreadCount++;
				} // Освобождаем мьютекс перед выполнением задачи

				// Выполняем задачу вне критической секции
				try
				{
					task();
				}
				catch (...)
				{
					throw_runtime_error("Unknown exceptions.");
				}

				// Уменьшаем счетчик и уведомляем
				{
					std::lock_guard<std::mutex> lk(cv_mutex);
					activeThreadCount--;
				}
				cv.notify_all(); // Используем notify_all для надежности
			}
		}
	};
}