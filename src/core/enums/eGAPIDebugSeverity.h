#pragma once

#include "core/utils/Types.h"

namespace zzz::core
{
	// Общий для всех GAPI-бэкендов уровень важности сообщения debug-слоя (Vulkan validation layer / DX12 Info Queue / Metal command buffer error).
	// Каждый бэкенд мапит свой нативный severity-тип в этот enum перед передачей в GAPIDebugLogger.
	enum class eGAPIDebugSeverity : zU8
	{
		Verbose, // Детальная информация (только при Z_GAPI_VERBOSE_DEBUG_LAYER)
		Info,    // Информационные сообщения (например, баннер со списком слоёв при старте)
		Warning, // Предупреждение, не критично
		Error    // Ошибка/нарушение спецификации
	};
}
