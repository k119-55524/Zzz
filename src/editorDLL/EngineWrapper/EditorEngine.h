#pragma once
#include <engine/engine.h>

namespace zzz::editor
{
	class EditorEngine : public zzz::engine::Engine
	{
	public:
		using Engine::Engine; // Наследуем конструктор

		// Перекрытие (shadowing) метода базового класса, без virtual
		[[nodiscard]] std::expected<void, std::string> Run();

		void Tick();
	};
}
