#pragma once

#include "core/CoreIncludes.h"
#include <shared_mutex>

namespace zzz::templates
{
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
				THROW_RUNTIME("Constructor parameters cannot be 0.");

			data = static_cast<T*>(_aligned_malloc(capacity * sizeof(T), alignof(T)));
			if (data == nullptr)
				THROW_RUNTIME("Failed to allocate memory.");

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
				THROW_RUNTIME("Index out of range.");

			return data[index];
		}

		const T& operator[](size_t index) const
		{
			std::shared_lock<std::shared_mutex> lock(mutex);

			if (index >= size)
				THROW_RUNTIME("Index out of range.");

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
				if (capacity >= std::numeric_limits<size_t>::max())
					THROW_RUNTIME("Capacity overflow.");

				newCapacity = std::numeric_limits<size_t>::max();
			}
			else
				newCapacity = capacity + capacity / 2;

			T* newData = static_cast<T*>(_aligned_malloc(newCapacity * sizeof(T), alignof(T)));
			if (newData == nullptr)
				THROW_RUNTIME("Failed to allocate memory.");

			std::uninitialized_default_construct_n(newData, newCapacity);

			try
			{
				std::uninitialized_move_n(data, size, newData);
			}
			catch (...)
			{
				_aligned_free(newData);
				THROW_RUNTIME("Unknown exception.");
			}

			std::destroy_n(data, size);
			_aligned_free(data);
			data = newData;
			capacity = newCapacity;
		}
	};
}
