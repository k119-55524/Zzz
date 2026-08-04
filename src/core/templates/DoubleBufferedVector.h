#pragma once

#include <vector>
#include <mutex>
#include <utility>

namespace zzz::core
{
	/**
	 * @brief Потокобезопасный вектор с двойной буферизацией (пинг-понг буфер).
	 *
	 * Позволяет нескольким потокам безопасно записывать данные, 
	 * пока один главный поток может читать данные без блокировок (lock-free) после смены буферов.
	 */
	template <typename T>
	class DoubleBufferedVector
	{
	public:
		DoubleBufferedVector() = default;

		explicit DoubleBufferedVector(size_t reserveSize)
		{
			m_ReadBuffer.reserve(reserveSize);
			m_WriteBuffer.reserve(reserveSize);
		}

		// Добавляет элемент в буфер записи. (Потокобезопасно)
		void Push(const T& item)
		{
			std::lock_guard lock(m_Mutex);
			m_WriteBuffer.push_back(item);
		}

		// Добавляет элемент в буфер записи через перемещение. (Потокобезопасно)
		void Push(T&& item)
		{
			std::lock_guard lock(m_Mutex);
			m_WriteBuffer.push_back(std::move(item));
		}

		// Создаёт элемент напрямую в буфере записи. (Потокобезопасно)
		template <typename... Args>
		void Emplace(Args&&... args)
		{
			std::lock_guard lock(m_Mutex);
			m_WriteBuffer.emplace_back(std::forward<Args>(args)...);
		}

		// Очищает буфер чтения, затем атомарно меняет его местами с буфером записи.
		// Возвращает ссылку на заполненный буфер чтения.
		// (НЕ потокобезопасно вызывать параллельно с другими вызовами Swap или чтением)
		std::vector<T>& SwapAndGetReadBuffer()
		{
			// Очищаем старые данные из буфера чтения перед сменой.
			// Это предотвращает вызов деструкторов элементов во время удержания мьютекса.
			m_ReadBuffer.clear();

			{
				std::lock_guard lock(m_Mutex);
				m_ReadBuffer.swap(m_WriteBuffer);
			}

			return m_ReadBuffer;
		}

		void Clear()
		{
			std::lock_guard lock(m_Mutex);
			m_WriteBuffer.clear();
			m_ReadBuffer.clear();
		}

		bool IsEmpty()
		{
			std::lock_guard lock(m_Mutex);
			return m_WriteBuffer.empty() && m_ReadBuffer.empty();
		}

	private:
		std::vector<T> m_ReadBuffer;
		std::vector<T> m_WriteBuffer;
		std::mutex m_Mutex;
	};
}
