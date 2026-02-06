
import Engine;

using namespace zzz;
using namespace zzz::math;
using namespace zzz::core;

class MyScript : public IBehavior
{
public:
	MyScript() :
		rotAngle{0}
	{
	}
	~MyScript() noexcept
	{
	}

	void OnUpdate(float deltaTime) override
	{
		rotAngle -= 0.5f * deltaTime;
		Quaternion m_QRor = Quaternion::rotateY(rotAngle);
		Transform& transform = GetTransform();
		transform.SetRotation(m_QRor);
		//m_Transform->Move(0.1f * deltaTime, 0.0f, 0.0f);
	}

protected:
	float rotAngle;
};

int APIENTRY wWinMain(
	_In_		HINSTANCE	hInstance,
	_In_opt_	HINSTANCE	hPrevInstance,
	_In_		LPWSTR		lpCmdLine,
	_In_		int			nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(263);
	//_CrtSetBreakAlloc(160);
	//_CrtSetBreakAlloc(202);
#endif // _DEBUG
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
					auto resScript = entity->SetScript<MyScript>();
					//entity->GetTransform().SetPosition(-1.5f, 0.0f, 0.0f);

					return Result<>();
				})
			.and_then([&engine]() { return engine.Run(); });
	}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif // _DEBUG

	return 0;
}
