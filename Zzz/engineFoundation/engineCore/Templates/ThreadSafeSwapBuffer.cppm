#include "pch.h"
export module ThreadSafeSwapBuffer;

import ThreadSafeQueueArray;

export namespace zzz::templates
{
	template<typename T>
	class ThreadSafeSwapBuffer
	{
	public:
		ThreadSafeSwapBuffer() = delete;
		ThreadSafeSwapBuffer(ThreadSafeSwapBuffer&) = delete;
		ThreadSafeSwapBuffer(ThreadSafeSwapBuffer&&) = delete;
		ThreadSafeSwapBuffer& operator=(const ThreadSafeSwapBuffer&) = delete;
		ThreadSafeSwapBuffer& operator=(ThreadSafeSwapBuffer&&) = delete;
		ThreadSafeSwapBuffer(zU32 capacity) :
			data{ ThreadSafeQueueArray<T>(capacity), ThreadSafeQueueArray<T>(capacity) },
			readIndex{ 0 },
			writeIndex{ 1 }
		{
			ensure(capacity > 1);
		}

		~ThreadSafeSwapBuffer() = default;
		inline void Add(const T& _data)
		{
			std::lock_guard<std::mutex> lock(writeMutex);

			data[writeIndex].PushBack(_data);
		};
		inline void ForEach(const std::function<void(const T&)>& callback)
		{
			if (!callback)
				return;

			std::lock_guard<std::mutex> lock(readMutex);

			const auto& currentData = data[readIndex];
			const size_t dataSize = currentData.Size();

			for (size_t i = 0; i < dataSize; i++)
				callback(currentData[i]);
		}
		inline void SwapAndReset() noexcept 
		{
			std::lock(writeMutex, readMutex);
			std::lock_guard<std::mutex> writeLock(writeMutex, std::adopt_lock);
			std::lock_guard<std::mutex> readLock(readMutex, std::adopt_lock);

			std::swap(readIndex, writeIndex);
			data[writeIndex].Clear();
		};

	private:
		mutable std::mutex readMutex;
		mutable std::mutex writeMutex;
		zU32 readIndex;
		zU32 writeIndex;
		ThreadSafeQueueArray<T> data[2];
	};
}