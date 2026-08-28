#pragma once

#include "core/utils/Types.h"

namespace zzz::core
{
	// Общая для всех GAPI-бэкендов категория сообщения debug-слоя (Vulkan validation layer / DX12 Info Queue / Metal).
	// Каждый бэкенд мапит свой нативный message type/category в этот enum перед передачей в GAPIDebugLogger.
	enum class eGAPIDebugCategory : zU8
	{
		General,     // Общее сообщение (создание/уничтожение объектов, инициализация)
		Validation,  // Нарушение спецификации API
		Performance  // Предупреждение о неоптимальном использовании API
	};
}
