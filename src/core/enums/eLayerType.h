#pragma once

#include <cstdint>
#include <string_view>

#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum eLayerType
	 * @brief Тип слоя сцены в многослойном конвейере отрисовки.
	 */
	enum class eLayerType : uint8_t
	{
		Layer3D = 0,   ///< Основной 3D мир
		Layer2D = 1,   ///< Экранный HUD / 2D мир
		LayerUI = 1,   ///< Алиас для обратной совместимости (LayerUI -> Layer2D)
		LayerMVVM = 2  ///< Авторский MVVM-интерфейс
	};

	[[nodiscard]] constexpr std::string_view ToString(eLayerType type)
	{
		switch (type)
		{
		case eLayerType::Layer3D: return "Layer3D";
		case eLayerType::Layer2D: return "Layer2D";
		case eLayerType::LayerMVVM: return "LayerMVVM";
		}
		THROW_RUNTIME("Необработанный eLayerType");
	}
}
