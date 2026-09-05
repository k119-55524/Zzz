#pragma once

#include "engine/scene/layer/ILayer.h"

namespace zzz
{
	/**
	 * @class LayerMVVM
	 * @brief Авторский UI-слой с поддержкой архитектуры MVVM (ViewModels, привязки данных, дерево элементов).
	 */
	class LayerMVVM final : public ILayer
	{
	public:
		explicit LayerMVVM(std::string name = "LayerMVVM")
			: m_Name(std::move(name))
		{
		}

		~LayerMVVM() override = default;

		Z_NO_COPY_MOVE(LayerMVVM);

		[[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
		[[nodiscard]] eLayerType GetType() const noexcept override { return eLayerType::LayerMVVM; }

		[[nodiscard]] bool IsVisible() const noexcept override { return m_IsVisible; }
		void SetVisible(bool visible) noexcept override { m_IsVisible = visible; }

		[[nodiscard]] bool IsEnabled() const noexcept override { return m_IsEnabled; }
		void SetEnabled(bool enabled) noexcept override { m_IsEnabled = enabled; }

		void Update(float dt) override;

		void PopulateObject(
			const ::zzz::core::GameObjectData& objData,
			const ::zzz::core::ScriptFactory& scriptFactory,
			::zzz::core::DataAssetsManager* dataAssetsManager) override;

	private:
		std::string m_Name;
		bool m_IsVisible{ true };
		bool m_IsEnabled{ true };
	};
}
