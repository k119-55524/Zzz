#pragma once

#include <memory>
#include "../EngineExport.h"

#include "public/core/events/EventBus.h"

namespace zzz
{
	class GameObject;
}

namespace zzz::script
{

#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_ENGINE_API Script : public std::enable_shared_from_this<Script>
	{
	public:
		Script() = delete;
		explicit Script(GameObject* owner);
		virtual ~Script();

		GameObject* GetOwner() const { return m_Owner; }

	private:
		virtual void Init(std::shared_ptr<zzz::engine::GameObjectEventBus> bus) = 0;

		GameObject* m_Owner;
	};
#pragma warning(pop)
}
