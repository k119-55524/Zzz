#pragma once

#include "core/CoreIncludes.h"
#include "QueueArray.h"

namespace zzz::templates
{
#if defined(_MSC_VER) && defined(_DEBUG)
	inline void SetThreadName(const char* threadName)
	{
		const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
		struct THREADNAME_INFO
		{
			DWORD dwType;
			LPCSTR szName;
			DWORD dwThreadID;
			DWORD dwFlags;
		} info;
		info.dwType = 0x1000;
		info.szName = threadName;
		info.dwThreadID = static_cast<DWORD>(-1);
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

		inline ThreadSafeArrayQueue& operator=(ThreadSafeArrayQueue&& other) noexcept
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

		inline void push(const T& value)
		{
			std::lock_guard<std::mutex> lk(mut);
			threadQueue.PushBack(value);
			tail = (tail + 1) % threadQueue.Capacity();
			lengthQueue++;
			cv.notify_one();
		}

		inline void push(T&& value)
		{
			std::lock_guard<std::mutex> lk(mut);
			threadQueue.PushBack(std::move(value));
			tail = (tail + 1) % threadQueue.Capacity();
			lengthQueue++;
			cv.notify_one();
		}

		inline bool pop(T& value) noexcept
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

		inline bool IsEmpty() const noexcept
		{
			std::lock_guard<std::mutex> lk(mut);
			return lengthQueue == 0;
		}

		inline size_t Length() const noexcept
		{
			std::lock_guard<std::mutex> lk(mut);
			return lengthQueue;
		}

		inline void clear()
		{
			std::lock_guard<std::mutex> lk(mut);
			head = 0;
			tail = 0;
			lengthQueue = 0;
			threadQueue.Clear();
		}

	private:
		QueueArray<T> threadQueue;
		size_t head;
		size_t tail;
		size_t lengthQueue;
		mutable std::mutex mut;
		std::condition_variable cv;
	};

	class ThreadPool
	{
		Z_NO_COPY_MOVE(ThreadPool);

	public:
		explicit ThreadPool(const std::string& threadName, size_t threadCount) :
			done{ false },
			threadCount{ threadCount },
			activeThreadCount{ 0 },
			workQueue(threadCount)
		{
			if (threadCount == 0)
				THROW_RUNTIME("Parameters cannot be 0.");

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
				THROW_RUNTIME("Failed to create threads.");
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

			threads.clear();
			workQueue.clear();
		}

		template<typename FunctionType, typename... Args>
		void Submit(FunctionType&& f, Args&&... args)
		{
			{
				std::lock_guard<std::mutex> lock(cv_mutex);
				if (done) return;
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

		void WorkerThread(const std::string& _threadName, size_t id)
		{
#if defined(_MSC_VER) && defined(_DEBUG)
			std::string threadName = std::format(">>>>> [zzz::ThreadPool]. Thread-using class: zzz::{}({})", _threadName, id);
			SetThreadName(threadName.c_str());
#endif

			while (true)
			{
				std::function<void()> task;

				{
					std::unique_lock<std::mutex> lk(cv_mutex);

					cv.wait(lk, [this] {
						return !workQueue.IsEmpty() || done;
						});

					if (done && workQueue.IsEmpty())
						return;

					if (!workQueue.pop(task))
						continue;

					activeThreadCount++;
				}

				try
				{
					task();
				}
				catch (...)
				{
					DOutError("ThreadPool task exception caught.");
				}

				{
					std::lock_guard<std::mutex> lk(cv_mutex);
					activeThreadCount--;
				}
				cv.notify_all();
			}
		}
	};
}
