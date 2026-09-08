#include "engine/scene/layer/LayerMVVM.h"
#include "core/io/package/LayerData.h"

namespace zzz
{
	void LayerMVVM::Update(float /*dt*/)
	{
		if (!m_IsVisible)
		{
			return;
		}

		// Декларативный UI слой: обновление элементов интерфейса (будет реализовано в MVP ZzzGUI)
	}

	void LayerMVVM::Populate(
		const ::zzz::core::LayerData& /*layerData*/,
		const ::zzz::core::ScriptFactory& /*scriptFactory*/)
	{
		// Декларативный UI слой: парсинг и биндинг XAML/MVVM разметки (будет реализовано в MVP ZzzGUI)
	}
}

