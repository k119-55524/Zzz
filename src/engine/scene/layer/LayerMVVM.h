#pragma once

#include <string>
#include <memory>

#include "engine/scene/layer/ILayer.h"
#include "engine/scene/domain/IObjectDomain.h"

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
		LayerMVVM(std::string name, std::unique_ptr<IObjectDomain> objectDomain);
		~LayerMVVM() override = default;

		[[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
		[[nodiscard]] eLayerType GetType() const noexcept override { return eLayerType::LayerMVVM; }

		[[nodiscard]] bool IsVisible() const noexcept override { return m_IsVisible; }
		void SetVisible(bool visible) noexcept override { m_IsVisible = visible; }

		void Update(float dt) override;
		void Populate(const LayerData& layerData, const ScriptFactory& scriptFactory) override;

		[[nodiscard]] IObjectDomain& GetObjectDomain() noexcept override { return *m_ObjectDomain; }
		[[nodiscard]] const IObjectDomain& GetObjectDomain() const noexcept override { return *m_ObjectDomain; }

	private:
		std::string m_Name;
		bool m_IsVisible{ true };
		std::unique_ptr<IObjectDomain> m_ObjectDomain;
	};
}

