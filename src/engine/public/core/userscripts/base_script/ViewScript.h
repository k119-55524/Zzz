#pragma once

#include <memory>
#include "../EngineExport.h"
#include "BaseScript.h"
#include "public/core/events/EventBus.h"

namespace zzz::engine
{
	class View;
}

namespace zzz::script
{
#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_ENGINE_API ViewScript : public BaseScript
	{
	public:
		ViewScript();
		virtual ~ViewScript();


	protected:
		virtual void OnUnbindEvents() override
		{
			if (m_Bus)
				m_Bus->UnsubscribeAll(shared_from_this());
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
		friend class zzz::engine::View;
		void Init(zzz::engine::ViewEventBus* bus)
		{
			m_Bus = bus;
			OnBindEvents();
		}

		zzz::engine::ViewEventBus* m_Bus = nullptr;
	};
#pragma warning(pop)

}
