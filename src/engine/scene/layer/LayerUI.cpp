#include "engine/scene/layer/LayerUI.h"

namespace zzz
{
	void LayerUI::Update(float /*dt*/)
	{
		if (!m_IsEnabled)
		{
			return;
		}
		// Заготовка под классический 2D GUI Batcher (Шаг 32)
	}

	void LayerUI::PopulateObject(
		const ::zzz::core::GameObjectData& /*objData*/,
		const ::zzz::core::ScriptFactory& /*scriptFactory*/,
		::zzz::core::DataAssetsManager* /*dataAssetsManager*/)
	{
		THROW_RUNTIME("LayerUI пока не поддерживает наполнение объектами (Шаг 32)");
	}
}
