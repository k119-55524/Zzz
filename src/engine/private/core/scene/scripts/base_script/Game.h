#pragma once

#include <string_view>

namespace zzz::script
{
	class Game
	{
	public:
		Game() = default;
		virtual ~Game() = default;
		virtual std::string_view GetScriptTypeName() const = 0;
	};
}
