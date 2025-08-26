#include "pch.h"
export module ThreadPool;

import Platforms;

using namespace zzz;

export namespace Zzz::Templates
{
	// Динамический массив с автоматическим изменением размера
	template<typename T>
	class QueueArray
	{
	public:
		QueueArray() = delete;
		QueueArray(const QueueArray&) = delete;
		QueueArray& operator=(const QueueArray&) = delete;

		explicit QueueArray(size_t capacity) :
			size{ 0 },
			capacity{ capacity }
		{
			if (capacity == 0)
				throw_out_of_range(">>>>> [QueueArray::QueueArray]. Constructor parameters cannot be 0.");

			data = static_cast<T*>(_aligned_malloc(capacity * sizeof(T), Platforms::ArrayAligment));
			if (data == nullptr)
				throw_runtime_error(">>>>> [QueueArray::QueueArray]. Failed to allocate memory.");

			// Инициализация элементов по умолчанию
			std::uninitialized_default_construct_n(data, capacity);
		}

		QueueArray(QueueArray&& other) noexcept :
			data{ other.data },
			size{ other.size },
			capacity{ other.capacity }
		{
			other.data = nullptr;
			other.size = 0;
			other.capacity = 0;
		}

		QueueArray& operator=(QueueArray&& other) noexcept
		{
			if (this != &other)
			{
				if (data)
				{
					std::destroy_n(data, capacity);
					_aligned_free(data);
				}

				data = other.data;
				size = other.size;
				capacity = other.capacity;
				other.data = nullptr;
				other.size = 0;
				other.capacity = 0;
			}
			return *this;
		}

		~QueueArray()
		{
			if (data)
			{
				std::destroy_n(data, capacity);
				_aligned_free(data);
			}
		}

		T& operator[](size_t index)
		{
			if (index >= size)
				throw_out_of_range(">>>>> [QueueArray::operator[]]. Index out of range.");

			return data[index];
		}

		const T& operator[](size_t index) const
		{
			if (index >= size)
				throw_out_of_range(">>>>> [QueueArray::operator[]]. Index out of range.");

			return data[index];
		}

		void PushBack(const T& element)
		{
			if (size >= capacity)
				AddSize();

			data[size++] = element;
		}

		void PushBack(T&& element)
		{
			if (size >= capacity)
				AddSize();

			data[size++] = std::move(element);
		}

		inline size_t Capacity() const noexcept { return capacity; }
		inline size_t Size() const noexcept { return size; }

		inline void Reset() noexcept
		{
			std::destroy_n(data, size);
			size = 0;
		}

	private:
		T* data = nullptr;
		size_t size;
		size_t capacity;

		void AddSize()
		{
			size_t newCapacity;
			if (capacity > std::numeric_limits<size_t>::max() / 2)
			{
				// Если capacity слишком большой для роста в 1.5x, пробуем минимальный рост
				if (capacity >= std::numeric_limits<size_t>::max())
					throw_overflow_error(">>>>> #0 [QueueArray::AddSize]. Capacity overflow.");

				newCapacity = std::numeric_limits<size_t>::max();
			}
			else
				newCapacity = capacity + capacity / 2; // Рост в 1.5x

			T* newData = static_cast<T*>(_aligned_malloc(newCapacity * sizeof(T), Platforms::ArrayAligment));
			if (newData == nullptr)
				throw_runtime_error(">>>>> #1 [QueueArray::AddSize]. Failed to allocate memory.");

			std::uninitialized_default_construct_n(newData, newCapacity);
			std::move(data, data + size, newData);

			try
			{
				std::uninitialized_move_n(data, size, newData);
			}
			catch (...)
			{
				_aligned_free(newData);
				throw_runtime_error(">>>>> #2 [QueueArray::AddSize]. Unknown exception.");
			}

			std::destroy_n(data, size);
			_aligned_free(data);
			data = newData;
			capacity = newCapacity;

			DebugOutput(std::format(L">>>>> [QueueArray::AddSize]. size: {}", size));
		}
	};

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
				threadQueue.Reset();
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
	class ThreadPool
	{
	public:
		ThreadPool() = delete;
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		explicit ThreadPool(size_t threadCount) :
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
					threads.emplace_back(&ThreadPool::WorkerThread, this, i);
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

			workQueue.push([f = std::forward<FunctionType>(f), ...args = std::forward<Args>(args)]() mutable {
				f(args...);
				});
			cv.notify_one();
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

		void WorkerThread(size_t id)
		{
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