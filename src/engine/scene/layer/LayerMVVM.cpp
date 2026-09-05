#include "engine/scene/layer/LayerMVVM.h"

namespace zzz
{
	void LayerMVVM::Update(float /*dt*/)
	{
		if (!m_IsEnabled)
		{
			return;
		}
		// Заготовка под реактивный движок разметки и Data Binding (Шаги 30, 34)
	}

	void LayerMVVM::PopulateObject(
		const ::zzz::core::GameObjectData& /*objData*/,
		const ::zzz::core::ScriptFactory& /*scriptFactory*/,
		::zzz::core::DataAssetsManager* /*dataAssetsManager*/)
	{
		THROW_RUNTIME("LayerMVVM пока не поддерживает наполнение объектами (Шаги 30, 34)");
	}
}
