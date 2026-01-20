
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
					Result<std::shared_ptr<UserLayer>> resLayer = view->AddLayer_3D();
					if (!resLayer)
						return Result<>(resLayer.error());

					//std::shared_ptr<UserLayer> layer = resLayer.value();
					// Используйте layer если нужно

					return Result<>();
				})
			.and_then([&engine]() { return engine.Run(); });
	}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif // _DEBUG

	return 0;
}
