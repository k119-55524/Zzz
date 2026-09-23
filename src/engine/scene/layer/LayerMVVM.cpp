
#include "core/io/package/scene/LayerData.h"
#include "core/utils/Ensure.h"

#include "LayerMVVM.h"

namespace zzz::engine
{
	LayerMVVM::LayerMVVM(Guid guid, std::string name, std::unique_ptr<MVVMDomain> mvvmDomain)
		: ILayer(guid, std::move(name), eLayerType::LayerMVVM)
		, m_MVVMDomain(std::move(mvvmDomain))
	{
		ensure(m_MVVMDomain != nullptr, "MVVMDomain не должен быть null в LayerMVVM.");
	}

	void LayerMVVM::Update(float /*dt*/)
	{
		if (!m_IsVisible)
			return;

		// Декларативный UI слой: обновление элементов интерфейса (будет реализовано в MVP ZzzGUI)
	}

	void LayerMVVM::Populate(
		const LayerData& /*layerData*/,
		const ScriptFactory& /*scriptFactory*/,
		std::function<void(std::expected<void, std::string>)> onReady,
		eTaskPriority /*priority*/)
	{
		// Декларативный UI слой: парсинг и биндинг XAML/MVVM разметки (будет реализовано в MVP ZzzGUI)
		if (onReady)
		{
			onReady({});
		}
	}
}

