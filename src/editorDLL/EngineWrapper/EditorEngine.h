#pragma once
#include <engine/engine.h>
#include <string>

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
		~EditorEngine();

		[[nodiscard]] std::expected<void, std::string> Run();

		void Tick();
		void ClearEngine();
		zzz::engine::View* AddView(void* hwnd);
		void RemoveView(void* view);

		void SetProjectPath(std::string_view path);
		void ReloadScripts();
		void UnloadScripts();

	protected:
		void OnRegisterScripts() override;

	private:
		void* m_ScriptsDll = nullptr;
		std::string m_ProjectPath;
	};
}
