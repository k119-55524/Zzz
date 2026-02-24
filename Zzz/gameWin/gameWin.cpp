
import Engine;
import DebugOutput;

using namespace zzz;
using namespace zzz::math;
using namespace zzz::core;
using namespace zzz::input;

import MemoryLeakDetector;

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
	CRT_LEAK_CHECK_BEGIN();
	{
		Engine engine;
		std::shared_ptr<UserSceneEntity> entity;
		Result<> res = engine.Initialize(L".\\AppData\\ui.zaml")
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

					return {};
				})
			.and_then([&engine]() { return engine.Run(); })
			.or_else([&](const Unexpected& error) { MsgBox::Error(error.getMessage()); });
	}

	return CRT_LEAK_CHECK_END();
}