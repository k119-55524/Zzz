#pragma once

#include <memory>
#include "../EngineExport.h"
#include "BaseScript.h"
#include "public/core/events/EventBus.h"

namespace zzz::engine
{
	class Scene;
}

namespace zzz::script
{

#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_ENGINE_API SceneScript : public BaseScript
	{
	public:
		SceneScript();
		virtual ~SceneScript();

	protected:
		virtual void OnUnbindEvents() override
		{
			if (m_Bus) m_Bus->UnsubscribeAll(shared_from_this());
		}

		template<typename F>
		void SubscribeToStart(F&& func) { if (m_Bus) m_Bus->OnStart.Subscribe(shared_from_this(), std::forward<F>(func)); }
		template<typename F>
		void SubscribeToStop(F&& func) { if (m_Bus) m_Bus->OnStop.Subscribe(shared_from_this(), std::forward<F>(func)); }
		template<typename F>
		void SubscribeToUpdate(F&& func) { if (m_Bus) m_Bus->OnUpdate.Subscribe(shared_from_this(), std::forward<F>(func)); }

	private:
		friend class zzz::engine::Scene;
		void Init(std::shared_ptr<zzz::engine::SceneEventBus> bus)
		{
			m_Bus = bus;
			OnBindEvents();
		}

		std::shared_ptr<zzz::engine::SceneEventBus> m_Bus;
	};
#pragma warning(pop)

}
