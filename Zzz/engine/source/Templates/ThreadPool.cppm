#include "pch.h"
export module ThreadPool;

import Platforms;

using namespace zzz;

namespace Zzz::Templates
{
	// TODO Не тестировал
	template<typename T>
	class zQueueArray
	{
	public:
		zQueueArray() = delete;
		zQueueArray(zQueueArray&) = delete;
		zQueueArray(zQueueArray&&) = delete;

		explicit zQueueArray(size_t _capacity, size_t _resizeStep) :
			size{ 0 },
			capacity{ _capacity },
			resizeStep{ _resizeStep }
		{
			if (capacity == 0 || resizeStep == 0)
				throw_out_of_range(">>>>> [ZArray.ZArray()]. Constructor parameters cannot be 0.");

			data = static_cast<T*>(_aligned_malloc(capacity * sizeof(T), Platforms::ArrayAligment));
			if (data == nullptr)
				throw_runtime_error(">>>>> [ZQueueArray.ZQueueArray()]. An exception occurred while allocating memory.");

			memset(data, 0, capacity * sizeof(T));
		}

		~zQueueArray()
		{
			_aligned_free(data);
		}

		T& operator[](size_t index)
		{
			if (index >= size)
				throw_out_of_range(">>>>> [ZArray.operator[]]. Index out of range.");

			return data[index];
		}

		void PushBack(const T& element)
		{
			if (size >= capacity)
				AddSize();

			data[size++] = element;
		}

		inline size_t Capacity() const noexcept
		{
			return capacity;
		}

		inline void Reset() noexcept
		{
			size = 0;
		}

	private:
		T* data;
		size_t size;
		size_t capacity;
		size_t resizeStep;

		void AddSize()
		{
			capacity += resizeStep;

			T* newData = static_cast<T*>(_aligned_malloc(capacity * sizeof(T), Platforms::ArrayAligment));
			if (newData == nullptr)
				throw_runtime_error(">>>>> [ZQueueArray.resizeArray()]. An exception occurred while allocating memory.");

			memset(newData, 0, capacity * sizeof(T));
			copy(data, data + size, newData);
			_aligned_free(data);
			data = newData;
		}
	};

	template<typename T>
	class zThreadSafeArrayQueue
	{
	public:
		zThreadSafeArrayQueue() = delete;
		zThreadSafeArrayQueue(zThreadSafeArrayQueue&) = delete;
		zThreadSafeArrayQueue(zThreadSafeArrayQueue&&) = delete;

		explicit zThreadSafeArrayQueue(size_t _startArraySize, size_t _arrayResizeStep) :
			threadQueue(_startArraySize, _arrayResizeStep),
			head{ 0 },
			tail{ 0 },
			lenghtQueue{ 0 }
		{
		}

		void push(const T value)
		{
			std::lock_guard<std::mutex> lk(mut);

			threadQueue.PushBack(value);
			tail = (tail + 1) % threadQueue.Capacity();
			lenghtQueue++;
		}

		inline bool pop(T& value) noexcept
		{
			std::lock_guard<std::mutex> lk(mut);

			if (lenghtQueue == 0)
				return false;

			value = threadQueue[head];
			head = (head + 1) % threadQueue.Capacity();
			lenghtQueue--;

			if (lenghtQueue == 0)
			{
				head = tail = 0;
				threadQueue.Reset();
			}

			return true;
		}

		inline bool IsEmpty() const noexcept
		{
			std::lock_guard<std::mutex> lk(mut);

			return lenghtQueue == 0;
		}

	private:
		zQueueArray<T> threadQueue;
		size_t head;
		size_t tail;
		size_t lenghtQueue;
		mutable std::mutex mut;
	};

	class zThreadPool
	{
	public:
		zThreadPool() = delete;
		zThreadPool(zThreadPool&) = delete;
		zThreadPool(zThreadPool&&) = delete;

		explicit zThreadPool(size_t _threadCount, size_t _queueResizeStep) :
			done{ false },
			pause{ true },
			threadCount{ _threadCount },
			activTreadCount{ 0 },
			workQueue(_threadCount, _queueResizeStep)
		{
			ensure(_threadCount == 0);
			ensure(_queueResizeStep == 0);

			threads.reserve(_threadCount);

			try
			{
				for (unsigned i = 0; i < threadCount; ++i)
				{
					threads.push_back(std::thread(&zThreadPool::WorkerThread, this, i));
				}
			}
			catch (...)
			{
				done = true;
				throw_runtime_error(">>>>> [ZArray.ZArray()]. An exception occurred while creating threads.");
			}
		}

		~zThreadPool()
		{
			done = true;

			for (unsigned long i = 0; i < threads.size(); ++i)
			{
				if (threads[i].joinable())
					threads[i].join();
			}
		}

		template<typename FunctionType, typename... Args>
		inline void Submit(FunctionType f, Args... args)
		{
			workQueue.push(bind(f, args...));
		}

		inline void Join()
		{
			pause = false;
			while (!workQueue.IsEmpty()) {};
			while (activTreadCount != 0) {};
			pause = true;
		};

	private:
		size_t threadCount;
		std::atomic_bool done;
		std::atomic_bool pause;
		std::atomic_uint activTreadCount;

		zThreadSafeArrayQueue<std::function<void()>> workQueue;
		std::vector<std::thread> threads;

		void WorkerThread(int id)
		{
			while (!done)
			{
				if (!pause)
				{
					std::function<void()> task;
					if (workQueue.pop(task))
					{
						activTreadCount++;
						task();
						activTreadCount--;
					}
					else
					{
						std::this_thread::yield();
					}
				}
			}
		}
	};
}