#pragma once

#include <common/templates/Event.h>
#include "public/core/EngineTime.h"

namespace zzz::script
{
	class GameScript;
	class SceneScript;
	class Script;
}

namespace zzz::engine
{
	class ProjectEventBus
	{
	public:
		void InvokeStart() { OnStart(); }
		void InvokeStop() { OnStop(); }
		void InvokeUpdate(const zzz::engine::Time& t) { OnUpdate(t); }

	private:
		friend class zzz::script::GameScript;
		zzz::engine::Event<> OnStart;
		zzz::engine::Event<> OnStop;
		zzz::engine::Event<const zzz::engine::Time&> OnUpdate;
	};

	class SceneEventBus
	{
	public:
		void InvokeStart() { OnStart(); }
		void InvokeStop() { OnStop(); }
		void InvokeUpdate(const zzz::engine::Time& t) { OnUpdate(t); }

	private:
		friend class zzz::script::SceneScript;
		zzz::engine::Event<> OnStart;
		zzz::engine::Event<> OnStop;
		zzz::engine::Event<const zzz::engine::Time&> OnUpdate;
	};

	class GameObjectEventBus
	{
	public:
		void InvokeStart() { OnStart(); }
		void InvokeStop() { OnStop(); }
		void InvokeUpdate(float t) { OnUpdate(t); }

	private:
		friend class zzz::script::Script;
		zzz::engine::UnorderedEvent<> OnStart;
		zzz::engine::UnorderedEvent<> OnStop;
		zzz::engine::UnorderedEvent<float> OnUpdate;
	};
}
