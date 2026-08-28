#pragma once

#include "core/enums/eGAPIType.h"
#include "core/enums/eGAPIDebugSeverity.h"
#include "core/enums/eGAPIDebugCategory.h"

namespace zzz::engine
{
	using namespace zzz::core;

	// Единая точка форматирования, ФИЛЬТРАЦИИ и маршрутизации подробных сообщений debug-слоя GAPI
	// (Vulkan validation layer / DirectX12 Info Queue / Metal command buffer error) в логгер движка.
	//
	// Каждый бэкенд сам мапит свой нативный severity/category в eGAPIDebugSeverity/eGAPIDebugCategory
	// и разбирает специфичный для своего API формат сообщения (например построчная фильтрация у Vulkan)
	// ДО вызова Report - сюда приходит уже готовая для печати строка.
	//
	// Решение "репортить ли конкретное сообщение" тоже принимается здесь, в одном месте, а не в каждом
	// бэкенде отдельно - единый интерфейс фильтрации для Vulkan/DirectX12/Metal по списку
	// zzz::core::c_GAPIDebugReportFlags (Constants.h), см. ShouldReport в Report.cpp.
	// У Vulkan есть дополнительная оптимизация на стороне самого слоя (messageSeverity/report_flags -
	// см. VulkanAPI::EnableDebugMessenger/BuildVerboseValidationLayerSettings, туда передаётся тот же
	// c_GAPIDebugReportFlags), которая просто не доводит отфильтрованные сообщения до Report вовсе,
	// но это именно оптимизация поверх, а не замена - финальное решение всё равно здесь.
	//
	// Report можно звать безусловно, как DOut/DOutError - в Release-сборке (нет ни Z_DEBUG_BUILD,
	// ни Z_DEVELOPMENT_BUILD) превращается в пустую inline-заглушку, так что бэкендам не нужно
	// оборачивать каждый вызов в свой #if.
	class GAPIDebugLogger
	{
	public:
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
		static void Report(eGAPIType backend, eGAPIDebugSeverity severity, eGAPIDebugCategory category, std::string_view message);
#else
		static void Report(eGAPIType, eGAPIDebugSeverity, eGAPIDebugCategory, std::string_view) {}
#endif
	};
}
