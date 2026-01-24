
import Engine;

using namespace zzz;

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
		Result<> res = engine.Initialize(L".\\appdata\\ui.zaml")
			.and_then([&engine](void) -> Result<>
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

					//auto entity = resEntity.value();
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
