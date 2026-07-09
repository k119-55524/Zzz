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

	private:
		friend class zzz::engine::Engine;
		virtual void Init(std::shared_ptr<zzz::engine::ProjectEventBus> bus) = 0;
	};
#pragma warning(pop)
}
