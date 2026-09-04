#include "engine/scene/layer/Layer3D.h"
#include "core/utils/MemoryUtils.h"

namespace zzz
{
	Layer3D::Layer3D(std::string name)
		: m_Name(std::move(name))
		, m_Storage(::zzz::core::safe_make_unique<DefaultSceneStorage>())
	{
		m_ObjectWorld.SetStorage(m_Storage.get());
	}

	void Layer3D::Update(float dt)
	{
		if (!m_IsEnabled)
		{
			return;
		}

		m_ObjectWorld.Update(dt);
	}
}
