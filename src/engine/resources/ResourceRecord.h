#pragma once

#include <memory>
#include <string>
#include <expected>
#include "core/events/OneShotEvent.h"

namespace zzz::engine
{
	/**
	 * @brief Унифицированная запись ресурса в кэше менеджера ресурсов.
	 *
	 * Объединяет готовый указатель на ресурс и одноразовое событие OneShotEvent,
	 * позволяющее клиентам ожидать завершения загрузки ресурса без polling и раздельных in-flight таблиц.
	 */
	template<typename T>
	struct ResourceRecord final
	{
		using ResourcePtr = std::shared_ptr<T>;
		using ResultType = std::expected<ResourcePtr, std::string>;
		using EventType = core::OneShotEvent<ResultType>;

		ResourcePtr resource{ nullptr };
		EventType readyEvent;

		ResourceRecord() = default;

		explicit ResourceRecord(typename EventType::DispatcherFunc dispatcher)
			: readyEvent(std::move(dispatcher))
		{
		}
	};
}
