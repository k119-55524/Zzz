#pragma once

#include <cstdint>
#include <string_view>

namespace zzz::core
{
	/**
	 * @enum eLayerType
	 * @brief Тип слоя сцены в многослойном конвейере отрисовки.
	 */
	enum class eLayerType : uint8_t
	{
		Layer3D = 0, ///< Основной 3D мир
		LayerUI = 1, ///< Экранный HUD / оверлей
		LayerMVVM = 2  ///< Авторский MVVM-интерфейс
	};

	[[nodiscard]] constexpr std::string_view ToString(eLayerType type) noexcept
	{
		switch (type)
		{
		case eLayerType::Layer3D: return "Layer3D";
		case eLayerType::LayerUI: return "LayerUI";
		case eLayerType::LayerMVVM: return "LayerMVVM";
		}
		return "Unknown";
	}
}
