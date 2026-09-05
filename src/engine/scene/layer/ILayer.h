#pragma once

#include <string>
#include <string_view>
#include "core/utils/Defines.h"

#include "core/enums/eLayerType.h"

namespace zzz
{
	class ISceneStorage;

	using ::zzz::core::eLayerType;
	using ::zzz::core::ToString;

	namespace core
	{
		class GameObjectData;
		class DataAssetsManager;
		class ScriptFactory;
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

		/**
		 * @brief Наполнить слой объектом из сериализованных данных сцены.
		 * Каждый тип слоя инкапсулирует разбор своей специфики.
		 */
		virtual void PopulateObject(
			const ::zzz::core::GameObjectData& objData,
			const ::zzz::core::ScriptFactory& scriptFactory,
			::zzz::core::DataAssetsManager* dataAssetsManager) = 0;
	};
}
