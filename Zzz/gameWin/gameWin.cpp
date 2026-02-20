
import Engine;

using namespace zzz;
using namespace zzz::math;
using namespace zzz::core;
using namespace zzz::input;

class MyScript : public IBehavior
{
public:
	MyScript(const std::shared_ptr<SceneEntity> entity,
		const std::shared_ptr<UserView>& view) :
		IBehavior{ entity },
		m_View{view},
		rotSpeed{ 0.5f },
		moveSpeed{ 1.0f }
	{
		ensure(m_View);
		m_Input = GetInput();
		m_Input->Keyboard()->GetButtonWatcher(KeyCode::F1).OnDown += std::bind(&MyScript::OnKeyDown_F1, this);
	}
	~MyScript() = default;

	void OnKeyDown_F1()
	{
		m_View->SetVsyncState(!m_View->GetVsyncState());
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
	const std::shared_ptr<UserView>& m_View;
};

int APIENTRY wWinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPWSTR		lpCmdLine,
	_In_		int			nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(1689);
	//_CrtSetBreakAlloc(160);
	//_CrtSetBreakAlloc(202);
#endif // _DEBUG
	{
		std::shared_ptr<UserView> view;

		Engine engine;
		std::shared_ptr<UserSceneEntity> entity;
		Result<> res = engine.Initialize(L".\\appdata\\ui.zaml")
			.and_then([&](void) -> Result<>
				{
					view = engine.GetMainView();
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
					auto resScript = entity->SetScript<MyScript>(view);

					return Result<>();
				})
			.and_then([&engine]() { return engine.Run(); })
			.or_else([&](const Unexpected& error) { MsgBox::Error(error.getMessage()); });
	}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif // _DEBUG

	return 0;
}
