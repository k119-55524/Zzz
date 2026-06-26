#pragma once
#include <engine/engine.h>

namespace zzz::engine
{
	class View;
}

namespace zzz::editor
{
	class EditorEngine : public zzz::engine::Engine
	{
	public:
		using Engine::Engine;
		[[nodiscard]] std::expected<void, std::string> Run();

		void Tick();
		void ClearEngine();
		zzz::engine::View* AddView(void* hwnd);
		void RemoveView(void* view);
	};
}
