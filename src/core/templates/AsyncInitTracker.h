#pragma once

#include <mutex>
#include <string>
#include <utility>
#include <expected>
#include <functional>

#include "core/templates/CountdownTrigger.h"

namespace zzz::templates
{
	/**
	 * @brief Универсальный неблокирующий трекер параллельной асинхронной инициализации группы элементов.
	 *
	 * Объединяет барьер завершения CountdownTrigger и потокобезопасную фиксацию первой возникшей ошибки.
	 * Идеален для параллельной инициализации слоёв сцены, дочерних GameObject, сабмешей или текстур материала.
	 */
	class AsyncInitTracker final
	{
		Z_NO_COPY_MOVE(AsyncInitTracker);

	public:
		using CallbackType = std::function<void(std::expected<void, std::string>)>;

		AsyncInitTracker(size_t totalCount, CallbackType onComplete)
			: m_OnComplete(ValidateCallback(std::move(onComplete)))
			, m_Trigger(totalCount, [this]()
			{
				std::string err;
				{
					std::lock_guard lock(m_Mutex);
					err = m_FirstError;
				}

				if (!err.empty())
					m_OnComplete(std::unexpected(std::move(err)));
				else
					m_OnComplete({});
			})
		{
		}

		~AsyncInitTracker() = default;

		/// @brief Успешное завершение одного параллельного шага
		void NotifySuccess()
		{
			m_Trigger.CountDown();
		}

		/// @brief Завершение одного параллельного шага с ошибкой
		void NotifyError(std::string errorMsg)
		{
			{
				std::lock_guard lock(m_Mutex);

				if (m_FirstError.empty())
					m_FirstError = std::move(errorMsg);
			}
			m_Trigger.CountDown();
		}

		/// @brief Удобный хелпер для прямой передачи std::expected<void, std::string>
		void Notify(const std::expected<void, std::string>& result)
		{
			if (result)
				NotifySuccess();
			else
				NotifyError(result.error());
		}

	private:
		static CallbackType ValidateCallback(CallbackType callback)
		{
			ensure(callback != nullptr, "onComplete не должен быть null");
			return callback;
		}

		std::mutex m_Mutex;
		std::string m_FirstError;
		CallbackType m_OnComplete;
		CountdownTrigger m_Trigger;
	};
}
