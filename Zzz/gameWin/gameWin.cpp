
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

import Engine;
import DebugOutput;

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif

using namespace zzz;
using namespace zzz::math;
using namespace zzz::core;
using namespace zzz::input;

#ifdef _DEBUG
#ifdef _MSC_VER
_CrtMemState g_memStateVeryEarly;
bool g_memoryTrackingInit = false;

void __cdecl InitMemoryTracking()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);

	_CrtMemCheckpoint(&g_memStateVeryEarly);
	g_memoryTrackingInit = true;
}

void __cdecl TerminateMemoryTracking()
{
	if (!g_memoryTrackingInit)
		return;

	_CrtMemState memStateLate;
	_CrtMemCheckpoint(&memStateLate);

	_CrtMemState diff;
	if (_CrtMemDifference(&diff, &g_memStateVeryEarly, &memStateLate))
	{
		OutputDebugStringA("=== LEAKS (entire program lifetime) ===\n");
		_CrtMemDumpStatistics(&diff);
		_CrtMemDumpAllObjectsSince(&g_memStateVeryEarly);
	}
	else
		OutputDebugStringA("=== NO LEAKS (full program check) ===\n");

	OutputDebugStringA("=== All remaining objects ===\n");
	_CrtDumpMemoryLeaks();
}

// MSVC-specific: Регистрация в CRT секциях
#pragma section(".CRT$XCU", read)
__declspec(allocate(".CRT$XCU"))
void(__cdecl* _Pep_Init)() = InitMemoryTracking;
#pragma section(".CRT$XPA", read)
__declspec(allocate(".CRT$XPA"))
void(__cdecl* _Pep_Term)() = TerminateMemoryTracking;
#else
class MemoryLeakDetector
{
public:
	_CrtMemState memStateBeforeMain;

	MemoryLeakDetector()
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);

		_CrtMemCheckpoint(&memStateBeforeMain);
	}

	~MemoryLeakDetector()
	{
		_CrtMemState memStateAfter;
		_CrtMemCheckpoint(&memStateAfter);

		_CrtMemState diff;
		if (_CrtMemDifference(&diff, &memStateBeforeMain, &memStateAfter))
		{
			OutputDebugStringA("=== !!! LEAKS DETECTED !!! ===\n");
			_CrtMemDumpStatistics(&diff);
			_CrtMemDumpAllObjectsSince(&memStateBeforeMain);
		}
		else
			OutputDebugStringA("=== NO LEAKS ===\n");

		OutputDebugStringA("=== All remaining objects ===\n");
		_CrtDumpMemoryLeaks();
	}
};

// Глобальный экземпляр - создастся до main, уничтожится после
static MemoryLeakDetector g_MemoryLeakDetector;

#endif // _MSC_VER
#endif // _DEBUG

class MyScript : public IBehavior
{
public:
	MyScript(const std::shared_ptr<SceneEntity> entity, Engine& engine) :
		IBehavior{ entity },
		m_Engine{ engine },
		rotSpeed{ 0.5f },
		moveSpeed{ 1.0f }
	{
		m_Input = GetInput();
		m_Input->Keyboard()->GetButtonWatcher(KeyCode::F1).OnDown += std::bind(&MyScript::OnKeyDown_F1, this);
		m_Input->Keyboard()->GetButtonWatcher(KeyCode::F2).OnDown += std::bind(&MyScript::OnKeyDown_F2, this);
	}
	~MyScript() = default;

	void OnKeyDown_F1()
	{
		m_Engine.SetVSyncState(!m_Engine.GetVSyncState());
	}

	void OnKeyDown_F2()
	{
		m_Engine.SetFullScreenState(!m_Engine.GetFullScreenState());
	}

	void OnUpdate(float deltaTime) override
	{
		Transform& transform = GetTransform();
		transform.AddRotation(0.0f, -rotSpeed * deltaTime, 0.0f);

		if (m_Input->Keyboard()->GetKeyState(KeyCode::ArrowLeft) == KeyState::Down)
			transform.Move(-moveSpeed * deltaTime, 0.0f, 0.0f);

		if (m_Input->Keyboard()->GetKeyState(KeyCode::ArrowRight) == KeyState::Down)
			transform.Move(moveSpeed * deltaTime, 0.0f, 0.0f);

		if (m_Input->Keyboard()->GetKeyState(KeyCode::ArrowUp) == KeyState::Down)
			transform.Move(0.0f, 0.0f, moveSpeed * deltaTime);

		if (m_Input->Keyboard()->GetKeyState(KeyCode::ArrowDown) == KeyState::Down)
			transform.Move(0.0f, 0.0f, -moveSpeed * deltaTime);
	}

protected:
	float rotSpeed;
	float moveSpeed;
	std::shared_ptr<Input> m_Input;
	Engine& m_Engine;
};

int APIENTRY wWinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPWSTR		lpCmdLine,
	_In_		int			nCmdShow)
{
#ifdef _DEBUG
	// Breakpoints (можно оставить закомментированными)
	//_CrtSetBreakAlloc(279);
	//_CrtSetBreakAlloc(278);
	//_CrtSetBreakAlloc(277);
	//_CrtSetBreakAlloc(276);

	OutputDebugStringA("\n>>> Entered wWinMain\n");

	// Снимок в начале wWinMain
	_CrtMemState memStateBegin;
	_CrtMemCheckpoint(&memStateBegin);
#endif

	{
		Engine engine;
		std::shared_ptr<UserSceneEntity> entity;
		Result<> res = engine.Initialize(L".\\appdata\\ui.zaml")
			.and_then([&](void) -> Result<>
				{
					auto view = engine.GetMainView();
					auto resLayer = view->AddLayer_3D();
					if (!resLayer)
						return Result<>(resLayer.error());

					auto layer = resLayer.value();
					auto resScene = layer->AddScene();
					if (!resScene)
						return Result<>(resScene.error());

					auto scene = resScene.value();
					Transform transform;
					auto resEntity = scene->AddColorBox(transform);
					if (!resEntity)
						return Result<>(resEntity.error());

					entity = resEntity.value();
					auto resScript = entity->SetScript<MyScript>(engine);

					return Result<>();
				})
			.and_then([&engine]() { return engine.Run(); })
			.or_else([&](const Unexpected& error) { MsgBox::Error(error.getMessage()); });
	}

#ifdef _DEBUG
	_CrtMemState memStateEnd;
	_CrtMemCheckpoint(&memStateEnd);

	_CrtMemState memStateDiff;
	if (_CrtMemDifference(&memStateDiff, &memStateBegin, &memStateEnd))
	{
		OutputDebugStringA("=== LEAKS in wWinMain scope ===\n");
		_CrtMemDumpStatistics(&memStateDiff);
		_CrtMemDumpAllObjectsSince(&memStateBegin);
	}
	else
		OutputDebugStringA("=== NO LEAKS in wWinMain scope ===\n");
#endif

	return 0;
}