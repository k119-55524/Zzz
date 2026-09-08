#pragma once

#include <string>
#include <memory>

#include "engine/scene/layer/ILayer.h"
#include "engine/scene/domain/IMVVMDomain.h"

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
		LayerMVVM(std::string name, std::unique_ptr<IMVVMDomain> mvvmDomain);
		~LayerMVVM() override = default;

		[[nodiscard]] eLayerType GetType() const noexcept override { return eLayerType::LayerMVVM; }

		[[nodiscard]] IMVVMDomain& GetMVVMDomain() noexcept { return static_cast<IMVVMDomain&>(*m_Domain); }
		[[nodiscard]] const IMVVMDomain& GetMVVMDomain() const noexcept { return static_cast<const IMVVMDomain&>(*m_Domain); }

		void Update(float dt) override;
		void Populate(const LayerData& layerData, const ScriptFactory& scriptFactory) override;
	};
}

