#pragma once

#include <memory>
#include "../EngineExport.h"
#include "public/core/events/EventBus.h"

namespace zzz::engine
{
	class Engine;
}

namespace zzz::script
{

#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_ENGINE_API GameScript : public std::enable_shared_from_this<GameScript>
	{
	public:
		GameScript() = default;
		virtual ~GameScript() = default;

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
		friend class zzz::engine::Engine;
		
		void Init(std::shared_ptr<zzz::engine::ProjectEventBus> bus)
		{
			m_Bus = bus;
			InitScript();
		}

		std::shared_ptr<zzz::engine::ProjectEventBus> m_Bus;
	};
#pragma warning(pop)
}
