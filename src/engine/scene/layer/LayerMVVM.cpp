
#include "core/io/package/LayerData.h"

#include "LayerMVVM.h"

namespace zzz::engine
{
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

