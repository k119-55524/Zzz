#pragma once

#include <string>

#include "engine/scene/layer/ILayer.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class LayerMVVM
	 * @brief Авторский UI-слой с поддержкой архитектуры MVVM (ViewModels, привязки данных, дерево элементов).
	 */
	class LayerMVVM final : public ILayer
	{
		Z_NO_COPY_MOVE(LayerMVVM);

	public:
		explicit LayerMVVM(std::string name)
			: m_Name(std::move(name))
		{
		}

		~LayerMVVM() override = default;

		[[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
		[[nodiscard]] eLayerType GetType() const noexcept override { return eLayerType::LayerMVVM; }

		[[nodiscard]] bool IsVisible() const noexcept override { return m_IsVisible; }
		void SetVisible(bool visible) noexcept override { m_IsVisible = visible; }

		void Update(float dt) override;
		void Populate(const LayerData& layerData, const ScriptFactory& scriptFactory) override;

	private:
		std::string m_Name;
		bool m_IsVisible{ true };
	};
}

