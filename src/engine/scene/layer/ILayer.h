#pragma once

#include <string>
#include <string_view>
#include "core/utils/Defines.h"

#include "core/enums/eLayerType.h"

namespace zzz
{
	namespace engine
	{
		class ResourceManager;
	}

	using ::zzz::core::eLayerType;
	using ::zzz::core::ToString;

	namespace core
	{
		class LayerData;
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

		/// @brief Начало кадра логики: подготовка сброса dirty-трекеров слоя перед выполнением скриптов.
		virtual void BeginFrame() {}

		virtual void Update(float dt) = 0;

		/// @brief Барьер сдачи кадра: передача измененных данных кадра в буфер рендера.
		virtual void ApplyHandoverBarrier() {}

		/**
		 * @brief Наполнить слой целиком из его LayerData (имя, тип уже разрешены снаружи).
		 * Слой сам обходит layerData.GetObjects() и разбирает каждый объект - Scene ему просто
		 * отдаёт данные слоя целиком, не занимаясь построчным разбором.
		 */
		virtual void Populate(
			const ::zzz::core::LayerData& layerData,
			const ::zzz::core::ScriptFactory& scriptFactory) = 0;
	};
}
