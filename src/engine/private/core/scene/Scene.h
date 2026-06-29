#pragma once

#include <string_view>

namespace zzz::script
{
	class Scene
	{
	public:
		Scene() = default;
		virtual ~Scene() = default;
		virtual std::string_view GetScriptTypeName() const = 0;
	};
}
