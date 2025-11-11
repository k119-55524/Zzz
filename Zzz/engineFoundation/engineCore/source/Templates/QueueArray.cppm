
#include "pch.h"

export module QueueArray;

import Platforms;

using namespace zzz;

export namespace zzz::templates
{
	// ƒинамический, потоко безопасный, массив с автоматическим изменением размера
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
				throw_out_of_range(">>>>> [QueueArray::QueueArray]. Constructor parameters cannot be 0.");

			data = static_cast<T*>(_aligned_malloc(capacity * sizeof(T), Platforms::ArrayAligment));
			if (data == nullptr)
				throw_runtime_error(">>>>> [QueueArray::QueueArray]. Failed to allocate memory.");

			// »нициализаци€ элементов по умолчанию
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
				throw_out_of_range(">>>>> [ThreadSafeQueueArray::operator[]]. Index out of range.");

			return data[index];
		}
		const T& operator[](size_t index) const
		{
			if (index >= size)
				throw_out_of_range(">>>>> [ThreadSafeQueueArray::operator[]]. Index out of range.");

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
		inline size_t Capacity() const noexcept
		{
			return capacity;
		}
		inline size_t Size() const noexcept
		{
			return size;
		}
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