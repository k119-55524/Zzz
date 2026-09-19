#pragma once

#include <utility>

#include "core/events/OneShotEvent.h"

namespace zzz::core
{
	/**
	 * @brief Унифицированная запись асинхронного ресурса/объекта в кэше.
	 *
	 * Объединяет готовый целевой объект (или умный указатель на него) и одноразовое событие OneShotEvent,
	 * позволяющее клиентам ожидать завершения загрузки без активного ожидания (polling)
	 * и без раздельных in-flight таблиц.
	 *
	 * @tparam T Тип хранимого объекта/указателя (например, std::shared_ptr<Scene> или std::shared_ptr<Mesh>).
	 * @tparam ResultType Тип результата, передаваемого через OneShotEvent (по умолчанию совпадает с T).
	 */
	template<typename T, typename ResultType = T>
	struct AsyncRecord final
	{
		using ValueType = T;
		using EventResultType = ResultType;
		using EventType = OneShotEvent<EventResultType>;
		using DispatcherFunc = typename EventType::DispatcherFunc;

		ValueType resource{};
		EventType readyEvent;

		AsyncRecord() = delete;

		explicit AsyncRecord(DispatcherFunc dispatcher)
			: readyEvent(std::move(dispatcher))
		{
		}

		AsyncRecord& operator=(ValueType val)
		{
			resource = std::move(val);
			return *this;
		}

		[[nodiscard]] explicit operator bool() const noexcept
		{
			if constexpr (requires { resource != nullptr; })
				return resource != nullptr;
			else
				return readyEvent.IsResolved();
		}
	};
}
