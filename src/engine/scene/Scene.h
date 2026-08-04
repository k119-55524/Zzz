#pragma once

#include <string_view>
#include <memory>
#include <core/utils/Types.h>

namespace zzz::engine
{
	class SceneEventBus;
	class Time;
}

namespace zzz::script
{
	class Scene : public std::enable_shared_from_this<Scene>
	{
	public:
		Scene() = default;
		virtual ~Scene() = default;

		virtual void Init(zzz::engine::SceneEventBus* /*bus*/) {}
		virtual void Update(const zzz::engine::Time& /*time*/) {}
	};
}
