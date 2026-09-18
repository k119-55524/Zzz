#pragma once

#include <string>
#include <memory>

#include "engine/scene/layer/ILayer.h"
#include "engine/scene/domain/MVVMDomain.h"

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
		LayerMVVM(zzz::core::Guid guid, std::string name, std::unique_ptr<MVVMDomain> mvvmDomain);
		~LayerMVVM() override = default;

		void Update(float dt) override;
		void Populate(
			const LayerData& layerData,
			const ScriptFactory& scriptFactory,
			std::function<void(std::expected<void, std::string>)> onReady = {}) override;

	private:
		std::unique_ptr<MVVMDomain> m_MVVMDomain;
	};
}

