#pragma once

#include <memory>
#include "core/utils/Export.h"

#include "BaseScript.h"
#include "core/events/EventBus.h"

namespace zzz::engine
{
	class GameObject;
}

namespace zzz
{
	using GameObject = ::zzz::engine::GameObject;
}

namespace zzz::core
{

#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_CORE_API Script : public BaseScript
	{
	public:
		Script() = delete;
		explicit Script(GameObject* owner);
		virtual ~Script();

		GameObject* GetOwner() const { return m_Owner; }

	protected:
		virtual void OnUnbindEvents() override
		{
			if (m_Bus) m_Bus->UnsubscribeAll(shared_from_this());
		}

		template<typename F>
		void SubscribeToStart(F&& func) { if (m_Bus) m_Bus->OnStart.Subscribe(shared_from_this(), std::forward<F>(func)); }
		template<typename F>
		void SubscribeToDestroy(F&& func) { if (m_Bus) m_Bus->OnDestroy.Subscribe(shared_from_this(), std::forward<F>(func)); }
		template<typename F>
		void SubscribeToEnable(F&& func) { if (m_Bus) m_Bus->OnEnable.Subscribe(shared_from_this(), std::forward<F>(func)); }
		template<typename F>
		void SubscribeToDisable(F&& func) { if (m_Bus) m_Bus->OnDisable.Subscribe(shared_from_this(), std::forward<F>(func)); }
		template<typename F>
		void SubscribeToUpdate(F&& func) { if (m_Bus) m_Bus->OnUpdate.Subscribe(shared_from_this(), std::forward<F>(func)); }

	private:
		friend class zzz::GameObject;
		void Init(std::shared_ptr<zzz::core::GameObjectEventBus> bus)
		{
			m_Bus = bus;
			OnBindEvents();
		}

		GameObject* m_Owner;
		std::shared_ptr<zzz::core::GameObjectEventBus> m_Bus;
	};
#pragma warning(pop)
}
