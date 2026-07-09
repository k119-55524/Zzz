#pragma once

#include <string_view>
#include <memory>

namespace zzz::engine
{
	class SceneEventBus;
}

namespace zzz::script
{
	class Scene : public std::enable_shared_from_this<Scene>
	{
	public:
		Scene() = default;
		virtual ~Scene() = default;

		virtual void Init(zzz::engine::SceneEventBus* /*bus*/) {}
	};
}
