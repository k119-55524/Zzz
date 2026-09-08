
#include "core/io/package/LayerData.h"
#include "core/utils/Ensure.h"

#include "LayerMVVM.h"

namespace zzz::engine
{
	LayerMVVM::LayerMVVM(std::string name, std::unique_ptr<IObjectDomain> objectDomain)
		: m_Name(std::move(name))
		, m_IsVisible{ true }
		, m_ObjectDomain(std::move(objectDomain))
	{
		ensure(m_ObjectDomain != nullptr, "ObjectDomain не должен быть null в LayerMVVM.");
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

