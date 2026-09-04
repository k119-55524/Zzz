#pragma once

#include <string>
#include <string_view>
#include "core/utils/Defines.h"

namespace zzz
{
	class ISceneStorage;

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

	/**
	 * @class ILayer
	 * @brief Базовый абстрактный интерфейс слоя сцены.
	 */
	class ILayer
	{
	public:
		virtual ~ILayer() = default;

		[[nodiscard]] virtual const std::string& GetName() const noexcept = 0;
		[[nodiscard]] virtual eLayerType GetType() const noexcept = 0;

		[[nodiscard]] virtual bool IsVisible() const noexcept = 0;
		virtual void SetVisible(bool visible) noexcept = 0;

		[[nodiscard]] virtual bool IsEnabled() const noexcept = 0;
		virtual void SetEnabled(bool enabled) noexcept = 0;

		virtual void Update(float dt) = 0;

		[[nodiscard]] virtual ISceneStorage* GetStorage() noexcept { return nullptr; }
	};
}
