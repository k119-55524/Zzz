#pragma once

#include <memory>
#include "../EngineExport.h"
#include "public/core/events/EventBus.h"

namespace zzz::script
{

#pragma warning(push)
#pragma warning(disable: 4251)
	class Z_ENGINE_API SceneScript : public std::enable_shared_from_this<SceneScript>
	{
	public:
		SceneScript() = default;
		virtual ~SceneScript() = default;

	private:
		virtual void Init(std::shared_ptr<zzz::engine::SceneEventBus> bus) = 0;
	};
#pragma warning(pop)
}
