#pragma once

#include <string>
#include <expected>
#include <functional>
#include <memory>

#include "core/enums/eLayerType.h"
#include "core/utils/Guid.h"
#include "core/utils/macros/MiscMacros.h"

namespace zzz::core
{
	class LayerData;
	class ScriptFactory;
}

namespace zzz::engine
{
	/**
	 * @class ILayer
	 * @brief Базовый абстрактный класс слоя сцены.
	 */
	class ILayer
	{
	public:
		ILayer(zzz::core::Guid guid, std::string name, eLayerType type)
			: m_Guid(guid)
			, m_Name(std::move(name))
			, m_Type(type)
			, m_IsVisible(true)
		{
		}

		virtual ~ILayer() = default;

		Z_NO_COPY_MOVE(ILayer);

		[[nodiscard]] const zzz::core::Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] eLayerType GetType() const noexcept { return m_Type; }

		[[nodiscard]] bool IsVisible() const noexcept { return m_IsVisible; }
		void SetVisible(bool visible) noexcept { m_IsVisible = visible; }

		/// @brief Начало кадра логики: подготовка сброса dirty-трекеров слоя перед выполнением скриптов.
		virtual void BeginFrame() {}

		virtual void Update(float dt) = 0;

		/**
		 * @brief Наполнить слой целиком из его LayerData (имя, тип уже разрешены снаружи).
		 * Слой сам обходит layerData.GetObjects() и разбирает каждый объект - Scene ему просто
		 * отдаёт данные слоя целиком, не занимаясь построчным разбором.
		 */
		virtual void Populate(
			const LayerData& layerData,
			const ScriptFactory& scriptFactory,
			std::function<void(std::expected<void, std::string>)> onReady = {},
			std::weak_ptr<const void> ownerToken = {}) = 0;

	protected:
		zzz::core::Guid m_Guid;
		std::string  m_Name;
		eLayerType   m_Type;
		bool         m_IsVisible;
	};
}
