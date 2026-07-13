#pragma once
#include <engine/engine.h>
#include <mutex>
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

		[[nodiscard]] std::expected<void, std::string> Run() override;

		void Tick();
		void ClearEngine();
		zzz::engine::View* AddView(void* hwnd);
		void RemoveView(void* view);

		void SetProjectPath(std::string_view path);
		void ReloadScripts();
		void UnloadScripts();

		void Play(const char** scriptClasses, int count);
		void Stop();
		void Pause(bool isPaused);

	protected:
		void OnRegisterScripts() override;
		void OnUpdateSystem() override;

	private:
		bool m_IsPaused = false;
		void* m_ScriptsDll = nullptr;
		std::string m_ProjectPath;
		std::string m_LoadedTempDllPath;

		// Защищает m_ScriptsDll/m_LoadedTempDllPath и сам FreeLibrary/LoadLibraryA
		// от параллельных вызовов ReloadScripts()/ClearEngine() - например, когда
		// перезагрузка после attach ещё не завершилась (VS может подолгу обрабатывать
		// отладочные события), а пользователь уже нажал Play, который тоже пытается
		// перезагрузить scripts.dll. Отдельный от stateMutex, чтобы не стопорить
		// Tick() на UI-потоке на время (потенциально долгой) перезагрузки DLL.
		std::mutex m_ScriptsMutex;
	};
}
