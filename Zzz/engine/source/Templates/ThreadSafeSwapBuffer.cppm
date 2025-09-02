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
		inline void ProcessAll(std::function<void(const T&)> callback)
		{
			std::lock_guard<std::mutex> lock(readMutex);
			for (zU64 i = 0; i < data[readIndex].Size(); i++)
				callback(data[readIndex][i]);
		}
		inline void ForEach(const std::function<void(const T&)>& callback)
		{
			std::shared_lock lock(readMutex);

			for (const auto& item : data[readIndex])
				callback(item);
		}
		inline void SwapAndReset() noexcept 
		{
			std::lock(writeMutex, readMutex);
			std::lock_guard<std::mutex> writeLock(writeMutex, std::adopt_lock);
			std::lock_guard<std::mutex> readLock(readMutex, std::adopt_lock);

			std::swap(readIndex, writeIndex);
			data[writeIndex].Reset();
		};

	private:
		mutable std::mutex readMutex;
		mutable std::mutex writeMutex;
		zU32 readIndex;
		zU32 writeIndex;
		ThreadSafeQueueArray<T> data[2];
	};
}