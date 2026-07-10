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

	public:
		virtual void InitScript() = 0;

	protected:
		template<typename F>
		void SubscribeToStart(F&& func) { if (m_Bus) m_Bus->OnStart.Subscribe(shared_from_this(), std::forward<F>(func)); }
		template<typename F>
		void SubscribeToStop(F&& func) { if (m_Bus) m_Bus->OnStop.Subscribe(shared_from_this(), std::forward<F>(func)); }
		template<typename F>
		void SubscribeToUpdate(F&& func) { if (m_Bus) m_Bus->OnUpdate.Subscribe(shared_from_this(), std::forward<F>(func)); }

	private:
		friend class zzz::GameObject;
		
		void Init(std::shared_ptr<zzz::engine::GameObjectEventBus> bus)
		{
			m_Bus = bus;
			InitScript();
		}

		GameObject* m_Owner;
		std::shared_ptr<zzz::engine::GameObjectEventBus> m_Bus;
	};
#pragma warning(pop)
}
