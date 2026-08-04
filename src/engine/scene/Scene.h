#pragma once

#include <string_view>
#include <memory>
#include <engine/header.h>

namespace zzz::engine
{
	using namespace zzz::core;

	class Scene : public std::enable_shared_from_this<Scene>
	{
	public:
		Scene() = default;
		virtual ~Scene() = default;

		virtual void Init([[maybe_unused]] SceneEventBus* bus) {}
		virtual void Update([[maybe_unused]] const Time& time) {}
	};
}
