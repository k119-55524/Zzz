#include "pch.h"
export module ThreadSafeQueueArray;

import Platforms;

export namespace zzz::templates
{
	// ƒинамический, потоко безопасный, массив с автоматическим изменением размера
	template<typename T>
	class ThreadSafeQueueArray
	{
	public:
		ThreadSafeQueueArray() = delete;
		ThreadSafeQueueArray(const ThreadSafeQueueArray&) = delete;
		ThreadSafeQueueArray& operator=(const ThreadSafeQueueArray&) = delete;

		explicit ThreadSafeQueueArray(size_t capacity) :
			size{ 0 },
			capacity{ capacity }
		{
			if (capacity == 0)
				throw_out_of_range(">>>>> [ThreadSafeQueueArray::ThreadSafeQueueArray]. Constructor parameters cannot be 0.");

			data = static_cast<T*>(_aligned_malloc(capacity * sizeof(T), Platforms::ArrayAligment));
			if (data == nullptr)
				throw_runtime_error(">>>>> [ThreadSafeQueueArray::ThreadSafeQueueArray]. Failed to allocate memory.");

			// »нициализаци€ элементов по умолчанию
			std::uninitialized_default_construct_n(data, capacity);
		}
		explicit ThreadSafeQueueArray(ThreadSafeQueueArray&& other) noexcept :
			data{ other.data },
			size{ other.size },
			capacity{ other.capacity }
		{
			other.data = nullptr;
			other.size = 0;
			other.capacity = 0;
		}

		~ThreadSafeQueueArray()
		{
			if (data)
			{
				std::destroy_n(data, capacity);
				_aligned_free(data);
			}
		}

		ThreadSafeQueueArray& operator=(ThreadSafeQueueArray&& other) noexcept
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

		T& operator[](size_t index)
		{
			std::shared_lock<std::shared_mutex> lock(mutex);

			if (index >= size)
				throw_out_of_range(">>>>> [ThreadSafeQueueArray::operator[]]. Index out of range.");

			return data[index];
		}
		const T& operator[](size_t index) const
		{
			std::shared_lock<std::shared_mutex> lock(mutex);

			if (index >= size)
				throw_out_of_range(">>>>> [ThreadSafeQueueArray::operator[]]. Index out of range.");

			return data[index];
		}
		void PushBack(const T& element)
		{
			std::unique_lock<std::shared_mutex> lock(mutex);

			if (size >= capacity)
				AddSize();

			data[size++] = element;
		}
		void PushBack(T&& element)
		{
			std::unique_lock<std::shared_mutex> lock(mutex);

			if (size >= capacity)
				AddSize();

			data[size++] = std::move(element);
		}
		inline size_t Capacity() const noexcept
		{
			std::shared_lock<std::shared_mutex> lock(mutex);
			return capacity;
		}
		inline size_t Size() const noexcept
		{
			std::shared_lock<std::shared_mutex> lock(mutex);
			return size;
		}
		inline void Clear() noexcept
		{
			std::unique_lock<std::shared_mutex> lock(mutex);

			std::destroy_n(data, size);
			size = 0;
		}

	private:
		T* data = nullptr;
		size_t size;
		size_t capacity;
		mutable std::shared_mutex mutex;

		void AddSize()
		{
			std::unique_lock<std::shared_mutex> lock(mutex);

			size_t newCapacity;
			if (capacity > std::numeric_limits<size_t>::max() / 2)
			{
				// ≈сли capacity слишком большой дл€ роста в 1.5x, пробуем минимальный рост
				if (capacity >= std::numeric_limits<size_t>::max())
					throw_overflow_error(">>>>> #0 [ThreadSafeQueueArray::AddSize]. Capacity overflow.");

				newCapacity = std::numeric_limits<size_t>::max();
			}
			else
				newCapacity = capacity + capacity / 2; // –ост в 1.5x

			T* newData = static_cast<T*>(_aligned_malloc(newCapacity * sizeof(T), Platforms::ArrayAligment));
			if (newData == nullptr)
				throw_runtime_error(">>>>> #1 [ThreadSafeQueueArray::AddSize]. Failed to allocate memory.");

			std::uninitialized_default_construct_n(newData, newCapacity);
			//std::move(data, data + size, newData);

			try
			{
				std::uninitialized_move_n(data, size, newData);
			}
			catch (...)
			{
				_aligned_free(newData);
				throw_runtime_error(">>>>> #2 [ThreadSafeQueueArray::AddSize]. Unknown exception.");
			}

			std::destroy_n(data, size);
			_aligned_free(data);
			data = newData;
			capacity = newCapacity;

			DebugOutput(std::format(L">>>>> [ThreadSafeQueueArray::AddSize]. size: {}", size));
		}
	};
}