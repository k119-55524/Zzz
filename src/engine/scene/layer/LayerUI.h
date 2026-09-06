#pragma once

#include "engine/scene/layer/ILayer.h"
#include "engine/scene/ObjectWorld.h"
#include "engine/scene/EntityWorld.h"
#include <memory>

namespace zzz
{
	/**
	 * @class LayerUI
	 * @brief Слой классического игрового HUD / UI (спрайты, панели, экранный текст).
	 */
	class LayerUI final : public ILayer
	{
	public:
		explicit LayerUI(std::string name)
			: m_Name(std::move(name))
		{
		}

		~LayerUI() override = default;

		Z_NO_COPY_MOVE(LayerUI);

		[[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
		[[nodiscard]] eLayerType GetType() const noexcept override { return eLayerType::LayerUI; }

		[[nodiscard]] bool IsVisible() const noexcept override { return m_IsVisible; }
		void SetVisible(bool visible) noexcept override { m_IsVisible = visible; }

		void Update(float dt) override;

		void Populate(
			const ::zzz::core::LayerData& layerData,
			const ::zzz::core::ScriptFactory& scriptFactory) override;

	private:
		std::string m_Name;
		bool m_IsVisible{ true };

		ObjectWorld m_ObjectWorld;
		::zzz::engine::EntityWorld m_EntityWorld;
	};
}
