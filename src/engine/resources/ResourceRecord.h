#pragma once

#include <memory>
#include <string>
#include <expected>
#include "core/templates/AsyncRecord.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @brief Унифицированная запись ресурса в кэше менеджера ресурсов.
	 *
	 * Базируется на общем шаблоне ядра zzz::core::AsyncRecord,
	 * объединяя готовый указатель на ресурс и OneShotEvent с результатом std::expected<std::shared_ptr<T>, std::string>.
	 */
	template<typename T>
	using ResourceRecord = AsyncRecord<std::shared_ptr<T>, std::expected<std::shared_ptr<T>, std::string>>;
}
