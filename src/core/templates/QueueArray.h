#pragma once

#include "core/CoreIncludes.h"

namespace zzz::templates
{
	template<typename T>
	class QueueArray
	{
	public:
		QueueArray(const QueueArray&) = delete;
		QueueArray& operator=(const QueueArray&) = delete;

		QueueArray(size_t capacity = 1) :
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

		explicit QueueArray(QueueArray&& other) noexcept :
			data{ other.data },
			size{ other.size },
			capacity{ other.capacity }
		{
			other.data = nullptr;
			other.size = 0;
			other.capacity = 0;
		}

		~QueueArray()
		{
			if (data)
			{
				std::destroy_n(data, capacity);
				_aligned_free(data);
			}
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

		T& operator[](size_t index)
		{
			if (index >= size)
				THROW_RUNTIME("Index out of range.");

			return data[index];
		}

		const T& operator[](size_t index) const
		{
			if (index >= size)
				THROW_RUNTIME("Index out of range.");

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
		inline void Clear() noexcept
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
				if (capacity >= std::numeric_limits<size_t>::max())
					THROW_RUNTIME("Capacity overflow.");
				newCapacity = std::numeric_limits<size_t>::max();
			}
			else
				newCapacity = capacity + capacity / 2;

			T* newData = static_cast<T*>(_aligned_malloc(newCapacity * sizeof(T), alignof(T)));
			if (newData == nullptr)
				THROW_RUNTIME("Failed to allocate memory.");

			try
			{
				std::uninitialized_move_n(data, size, newData);
			}
			catch (...)
			{
				_aligned_free(newData);
				THROW_RUNTIME("Unknown exception.");
			}

			std::uninitialized_default_construct_n(newData + size, newCapacity - size);

			std::destroy_n(data, size);
			_aligned_free(data);
			data = newData;
			capacity = newCapacity;
		}
	};
}
