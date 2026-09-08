#pragma once

#include <string>
#include <memory>

#include "core/enums/eLayerType.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/utils/Ensure.h"
#include "engine/scene/domain/ILayerDomain.h"

namespace zzz::core
{
	class LayerData;
	class ScriptFactory;
}

namespace zzz::engine
{
	class ResourceManager;

	/**
	 * @class ILayer
	 * @brief Базовый абстрактный класс слоя сцены.
	 */
	class ILayer
	{
	public:
		ILayer(std::string name, std::unique_ptr<ILayerDomain> domain)
			: m_Name(std::move(name))
			, m_IsVisible{ true }
			, m_Domain(std::move(domain))
		{
			ensure(m_Domain != nullptr, "Domain не должен быть null в ILayer.");
		}

		virtual ~ILayer() = default;

		Z_NO_COPY_MOVE(ILayer);

		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] virtual eLayerType GetType() const noexcept = 0;

		[[nodiscard]] bool IsVisible() const noexcept { return m_IsVisible; }
		void SetVisible(bool visible) noexcept { m_IsVisible = visible; }

		/// @brief Домен сущностей слоя.
		[[nodiscard]] ILayerDomain& GetDomain() noexcept { return *m_Domain; }
		[[nodiscard]] const ILayerDomain& GetDomain() const noexcept { return *m_Domain; }

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
		virtual void Populate(const LayerData& layerData, const ScriptFactory& scriptFactory) = 0;

	protected:
		std::string                    m_Name;
		bool                           m_IsVisible{ true };
		std::unique_ptr<ILayerDomain>  m_Domain;
	};
}
