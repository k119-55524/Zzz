
#include "core/io/package/LayerData.h"
#include "core/utils/Ensure.h"

#include "LayerMVVM.h"

namespace zzz::engine
{
	LayerMVVM::LayerMVVM(std::string name, std::unique_ptr<IMVVMDomain> mvvmDomain)
		: ILayer(std::move(name), std::move(mvvmDomain))
	{
		ensure(m_Domain->GetDomainType() == ::zzz::core::eObjectDomain::MVVM,
			"LayerMVVM требует домен типа eObjectDomain::MVVM.");
	}

	void LayerMVVM::Update(float /*dt*/)
	{
		if (!m_IsVisible)
			return;

		// Декларативный UI слой: обновление элементов интерфейса (будет реализовано в MVP ZzzGUI)
	}

	void LayerMVVM::Populate(const LayerData& /*layerData*/, const ScriptFactory& /*scriptFactory*/)
	{
		// Декларативный UI слой: парсинг и биндинг XAML/MVVM разметки (будет реализовано в MVP ZzzGUI)
	}
}

