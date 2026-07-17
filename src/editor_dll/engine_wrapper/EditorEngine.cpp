#include <engine/headers/enums.h>
#include <engine/private/core/view/ViewManager.h>
#include <engine/public/core/userscripts/ScriptRegistry.h>
#include <engine/public/core/events/EventBus.h>
#include <engine/public/core/scene/GameObject.h>
#include <engine/public/core/userscripts/base_script/Script.h>
#include <engine/public/core/userscripts/base_script/GameScript.h>
#include <algorithm>
#include <cctype>
#include <utility>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

#include "EditorEngine.h"

using namespace zzz;
using namespace zzz::engine;
using namespace zzz::logger;

namespace zzz::editor
{
	namespace
	{
		std::string NormalizeModulePath(std::string path)
		{
			std::replace(path.begin(), path.end(), '\\', '/');
			std::transform(path.begin(), path.end(), path.begin(), [](unsigned char ch) {
				return static_cast<char>(std::tolower(ch));
			});
			return path;
		}

		bool IsScriptTempModulePath(const char* rawPath)
		{
			if (!rawPath || !*rawPath)
				return false;

			std::string path = NormalizeModulePath(rawPath);
			return path.find("/.editor/bin/scripts_temp_") != std::string::npos &&
				path.ends_with(".dll");
		}

		std::string WideToUtf8(const wchar_t* value)
		{
			if (!value || !*value)
				return {};

			int size = WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
			if (size <= 1)
				return {};

			std::string result(static_cast<size_t>(size - 1), '\0');
			WideCharToMultiByte(CP_UTF8, 0, value, -1, result.data(), size, nullptr, nullptr);
			return result;
		}

		// Каждый загруженный модуль scripts.dll/scripts_temp_*.dll держит свой собственный
		// g_Logger (logger_lib линкуется статически) с фоновым потоком рассылки логов,
		// запущенным через InitScriptLogger(). Если не остановить этот поток явно ДО
		// FreeLibrary(), деструктор g_Logger попытается join() его уже внутри
		// DllMain(DLL_PROCESS_DETACH) - а это гарантированный deadlock на loader lock
		// (поток, чтобы завершиться, тоже претендует на loader lock, который уже держит
		// поток, вызвавший FreeLibrary). ShutdownScriptLogger может отсутствовать в старых,
		// собранных до этого фикса DLL - тогда просто ничего не делаем.
		void ShutdownScriptModuleLogger(HMODULE module)
		{
			using ShutdownLogFunc = void(*)();
			if (ShutdownLogFunc shutdownLogger = (ShutdownLogFunc)GetProcAddress(module, "ShutdownScriptLogger"))
			{
				shutdownLogger();
			}
		}

		void UnloadScriptModuleUntilGone(HMODULE module, const char* path)
		{
			ShutdownScriptModuleLogger(module);

			for (int i = 0; i < 16; ++i)
			{
				if (!FreeLibrary(module))
				{
					DOutWarning("[Scripts Debug] FreeLibrary failed for stale script module '{}'. Error: {}", path, GetLastError());
					return;
				}

				if (!GetModuleHandleA(path))
				{
					DOut("[Scripts Debug] Unloaded stale script module '{}'.", path);
					return;
				}
			}

			DOutWarning("[Scripts Debug] Stale script module '{}' is still loaded after repeated FreeLibrary calls.", path);
		}

		void UnloadStaleScriptTempModules()
		{
			HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
			if (snapshot == INVALID_HANDLE_VALUE)
			{
				DOutWarning("[Scripts Debug] Failed to enumerate loaded modules. Error: {}", GetLastError());
				return;
			}

			MODULEENTRY32W entry{};
			entry.dwSize = sizeof(entry);

			if (!Module32FirstW(snapshot, &entry))
			{
				CloseHandle(snapshot);
				return;
			}

			std::vector<std::pair<HMODULE, std::string>> staleModules;
			do
			{
				std::string path = WideToUtf8(entry.szExePath);
				if (IsScriptTempModulePath(path.c_str()))
				{
					staleModules.emplace_back(entry.hModule, std::move(path));
				}
			}
			while (Module32NextW(snapshot, &entry));

			CloseHandle(snapshot);

			for (const auto& [module, path] : staleModules)
			{
				UnloadScriptModuleUntilGone(module, path.c_str());
				std::string pdbPath = path.substr(0, path.find_last_of('.')) + ".pdb";
				DeleteFileA(path.c_str());
				DeleteFileA(pdbPath.c_str());
			}
		}
	}

	EditorEngine::~EditorEngine()
	{
		UnloadScripts();
	}

	[[nodiscard]] std::expected<void, std::string> EditorEngine::Run()
	{
		std::lock_guard lock(stateMutex);

		if (engineState.load() != eInitState::Initialized)
			return UNEXPECTED("Движок не инициализирован.");

		engineState.store(eInitState::Running);
		return {};
	}

	void EditorEngine::Tick()
	{
		if (engineState.load() == eInitState::Running)
		{
			std::lock_guard lock(stateMutex);
			OnUpdateSystem();
		}
	}

	void EditorEngine::ClearEngine()
	{
		std::lock_guard lock(m_ScriptsMutex);
		UnloadScripts();
	}

	zzz::engine::View* EditorEngine::AddView(void* hwnd)
	{
		if (engineState.load() != eInitState::Running)
		{
			DOutError("EditorEngine не запущен. Невозможно выполнить AddView.");
			return nullptr;
		}

		return m_ViewManager->CreateView(hwnd);
	}

	void EditorEngine::RemoveView(void* view)
	{
		if (engineState.load() == eInitState::Running && view)
			m_ViewManager->RemoveView(static_cast<zzz::engine::View*>(view));
	}

	void EditorEngine::SetProjectPath(std::string_view path)
	{
		m_ProjectPath = path;
	}

	void EditorEngine::ReloadScripts()
	{
		// Захватываем на всю перезагрузку (а не только на мутацию m_ScriptsDll): FreeLibrary/
		// LoadLibraryA сами по себе могут надолго заблокироваться, если только что подключенная
		// VS ещё не разгребла очередь отладочных событий - и если в этот момент параллельно
		// прилетит ещё один ReloadScripts() (например, из Play, пока висит перезагрузка после
		// attach), два потока начнут одновременно дёргать FreeLibrary/LoadLibraryA на одном и
		// том же модуле. Это уже само по себе гонка данных и надёжный способ подвесить процесс.
		std::lock_guard lock(m_ScriptsMutex);

		UnloadScripts();

		if (m_ProjectPath.empty())
		{
			DOutError("Путь проекта пуст. Невозможно загрузить scripts DLL.");
			return;
		}

		std::string originDllPath = m_ProjectPath + "/.editor/bin/scripts.dll";
		std::string tempDllPath = m_ProjectPath + "/.editor/bin/scripts_temp_" + std::to_string(GetTickCount()) + ".dll";
		std::string originPdbPath = m_ProjectPath + "/.editor/bin/scripts.pdb";
		std::string tempPdbPath = tempDllPath.substr(0, tempDllPath.find_last_of('.')) + ".pdb";
		bool debuggerAttached = IsDebuggerPresent() != FALSE;
		std::string loadDllPath = originDllPath;

		DOut("[Scripts Debug] Reload requested. dll='{}', pdb='{}', debuggerAttached={}", originDllPath, originPdbPath, debuggerAttached);

		if (!debuggerAttached)
		{
			bool copied = false;
			for (int i = 0; i < 50; ++i)
			{
				if (CopyFileA(originDllPath.c_str(), tempDllPath.c_str(), FALSE))
				{
					copied = true;
					break;
				}

				DOutWarning("Retrying scripts DLL copy for hot reload.");
				Sleep(100);
			}

			if (!copied)
			{
				DOutWarning("Failed to copy scripts.dll to temporary DLL. Error: {}", GetLastError());
				return;
			}

			DOut("[Scripts Debug] Copied DLL to '{}'.", tempDllPath);

			if (GetFileAttributesA(originPdbPath.c_str()) != INVALID_FILE_ATTRIBUTES)
			{
				if (!CopyFileA(originPdbPath.c_str(), tempPdbPath.c_str(), FALSE))
					DOutWarning("Failed to copy scripts PDB for temporary DLL. Error: {}", GetLastError());
				else
					DOut("[Scripts Debug] Copied PDB to '{}'.", tempPdbPath);
			}
			else
			{
				DOutWarning("[Scripts Debug] PDB not found near scripts.dll: '{}'. Breakpoints may not bind.", originPdbPath);
			}

			loadDllPath = tempDllPath;
		}
		else
		{
			UnloadStaleScriptTempModules();
			DOut("[Scripts Debug] Debugger attached; loading original scripts.dll so Visual Studio can bind breakpoints to the project output module.");
		}

		HMODULE handle = LoadLibraryA(loadDllPath.c_str());
		if (!handle)
		{
			DOutError("Failed to load scripts module '{}'. Error: {}", loadDllPath, GetLastError());
			return;
		}

		m_ScriptsDll = handle;
		m_LoadedTempDllPath = debuggerAttached ? std::string{} : tempDllPath;
		DOut("[Scripts Debug] Loaded module '{}', handle={}.", loadDllPath, static_cast<void*>(handle));
		extern zzz::logger::LogCallback g_EditorLogCallback;
		using InitLogFunc = void(*)(void*);
		InitLogFunc initLogger = (InitLogFunc)GetProcAddress(handle, "InitScriptLogger");
		if (initLogger && g_EditorLogCallback)
		{
			initLogger((void*)g_EditorLogCallback);
		}

		using RegisterFunc = void(*)();
		RegisterFunc registerAll = (RegisterFunc)GetProcAddress(handle, "RegisterAllScripts");
		if (registerAll)
		{
			registerAll();
			DOut("Scripts DLL загружена и зарегистрирована успешно.");
		}
		else
		{
			DOutError("Не удалось найти экспорт RegisterAllScripts в scripts DLL.");
			UnloadScripts();
		}
	}

	void EditorEngine::UnloadScripts()
	{
		if (m_ScriptsDll)
		{
			DOut("Выгрузка scripts DLL...");

#if Z_EDITOR
			// 1. Удаляем все активные GameObject-скрипты
			// SetActive(false) вызывает OnUnbindEvents() -> EventBus::UnsubscribeAll() ДО того,
			// как объект скрипта будет разрушен - иначе подписка на OnUpdate/OnStart/OnDestroy
			// остаётся висеть в EventBus и указывает на код внутри scripts.dll, которую мы
			// вот-вот выгрузим FreeLibrary(). Разрушение скрипта без предварительной отписки
			// не роняло ничего, пока сама DLL оставалась в памяти (адрес ещё валиден), но как
			// только модуль реально выгружается - это чтение по невалидному адресу.
			auto activeScripts = zzz::script::ScriptRegistry::GetActiveInstances();
			for (auto* scriptRaw : activeScripts)
			{
				scriptRaw->SetActive(false);
				if (auto owner = scriptRaw->GetOwner())
				{
					owner->RemoveScript(std::static_pointer_cast<zzz::script::Script>(scriptRaw->shared_from_this()));
				}
			}

			// 2. Удаляем все активные GameScript-ы (та же логика: сначала отписка, потом очистка)
			auto activeGameScripts = zzz::script::ScriptRegistry::GetActiveGameScripts();
			for (auto* gameScriptRaw : activeGameScripts)
			{
				gameScriptRaw->SetActive(false);
			}
			m_Scripts.clear();

			// 3. (TODO Phase 3: Сцен пока нет, но тут будет удаление SceneScripts)
#endif

			zzz::script::ScriptRegistry::Clear();
			HMODULE module = (HMODULE)m_ScriptsDll;
			ShutdownScriptModuleLogger(module);
			FreeLibrary(module);
			m_ScriptsDll = nullptr;

			if (!m_LoadedTempDllPath.empty())
			{
				if (HMODULE staleModule = GetModuleHandleA(m_LoadedTempDllPath.c_str()))
				{
					UnloadScriptModuleUntilGone(staleModule, m_LoadedTempDllPath.c_str());
				}

				std::string tempPdbPath = m_LoadedTempDllPath.substr(0, m_LoadedTempDllPath.find_last_of('.')) + ".pdb";
				DeleteFileA(m_LoadedTempDllPath.c_str());
				DeleteFileA(tempPdbPath.c_str());
				m_LoadedTempDllPath.clear();
			}
		}
	}

	void EditorEngine::OnUpdateSystem()
	{
		m_Time->Update();

		if (!m_IsPaused)
			m_EventBus->InvokeUpdate(*m_Time);

		if (m_ViewManager)
			m_ViewManager->Update(*m_Time);
	}

	void EditorEngine::OnRegisterScripts()
	{
		if (!m_ProjectPath.empty())
		{
			// m_LoadedTempDllPath.empty() значит, что уже загружен оригинальный scripts.dll (а не
			// временная копия) - именно его и нужно, если подключен отладчик. Если сейчас всё ещё
			// загружена временная копия (attach произошёл, но своп ещё не делался), перезагрузка
			// всё равно нужна - но делаем её тут, при Play, а не сразу после attach: сразу после
			// attach VS может быть занята обработкой только что подключенного процесса, и
			// FreeLibrary/LoadLibraryA в этот момент рискуют подвиснуть надолго (см. AttachDebugger).
			if (IsDebuggerPresent() && m_ScriptsDll && m_LoadedTempDllPath.empty())
			{
				DOut("[Scripts Debug] Debugger attached and original scripts.dll is already loaded; keeping module loaded so Visual Studio breakpoints stay bound.");
				return;
			}

			ReloadScripts();
		}
	}

	void EditorEngine::StartGame(const std::vector<std::string>& globalScripts)
	{
		OnRegisterScripts();

#ifdef _WIN32
		if (IsDebuggerPresent())
		{
			// Даем время Visual Studio загрузить .pdb символы и расставить брейкпоинты
			// после перезагрузки scripts.dll. Делаем это до захвата stateMutex, иначе
			// Tick() (крутится на UI-потоке редактора каждый кадр и тоже берет stateMutex)
			// стопорится на все 2 секунды и редактор выглядит зависшим.
			DOut("[Debugger] Visual Studio attached. Waiting for script symbols to bind before Start...");
			Sleep(2000);
		}
#endif

		LoadGlobalScripts(globalScripts);
		m_EventBus->InvokeStart();
	}

	void EditorEngine::Play(const char** scriptClasses, int count)
	{
		std::vector<std::string> classes;
		for (int i = 0; i < count; ++i)
		{
			if (scriptClasses[i])
			{
				classes.push_back(scriptClasses[i]);
			}
		}

		// stateMutex захватывается уже внутри StartGame() - только вокруг мутации
		// m_Scripts/EventBus, а не вокруг ожидания отладчика (см. Engine::StartGame).
		StartGame(classes);
		m_Time->ResetFrameTimer();
	}

	void EditorEngine::Stop()
	{
		std::lock_guard lock(stateMutex);
		StopGame();
	}

	void EditorEngine::Pause(bool isPaused)
	{
		std::lock_guard lock(stateMutex);
		m_IsPaused = isPaused;
		DOut("EditorEngine::Pause called with: {}", isPaused);
	}
}
