
export module View;

import Input;
import IGAPI;
import Event;
import Result;
import Layer3D;
import IAppWin;
import StrConvert;
import ThreadPool;
import UserLayer3D;
import RenderQueue;
import ViewFactory;
import UserLayer3D;
import IRenderLayer;
import AppWinConfig;
import ISurfaceView;
import SceneEntityFactory;

using namespace zzz::core;
using namespace zzz::input;
using namespace zzz::templates;

namespace zzz
{
	class Engine;
}

namespace zzz
{
	export class View final
	{
		Z_NO_CREATE_COPY(View);

	public:
		View(
			const std::shared_ptr<AppWinConfig> winConfig,
			const std::shared_ptr<SceneEntityFactory> entityFactory,
			const std::shared_ptr<IGAPI> GAPI);

		~View() = default;

		Event<Size2D<>, eTypeWinResize> viewResized;
		Event<> viewResizing;

		Result<std::shared_ptr<UserLayer3D>> AddLayer_3D();

		void OnUpdate(double deltaTime);
		void SetFullScreen(bool fs);
		inline void SetVSync(bool vs) { if (m_RenderSurface != nullptr) m_RenderSurface->SetVSync(vs); };
		inline void SetViewCaptionText(std::wstring caption) { if (m_NativeWindow != nullptr) m_NativeWindow->SetCaptionText(caption); };
		inline void AddViewCaptionText(std::wstring caption) { if (m_NativeWindow != nullptr) m_NativeWindow->AddCaptionText(caption); };

	private:
		void OnViewResizing();
		void OnViewResize(const Size2D<>& size, eTypeWinResize resizeType);

		ViewFactory factory;
		const std::shared_ptr<AppWinConfig> m_WinConfig;
		const std::shared_ptr<IGAPI> m_GAPI;
		const std::shared_ptr<SceneEntityFactory> m_EntityFactory;
		std::shared_ptr<IAppWin> m_NativeWindow;
		std::shared_ptr<ISurfaceView> m_RenderSurface;

		eInitState initState;
		void Initialize();

		ThreadPool m_ThreadsUpdate;
		std::shared_ptr<ViewSetup> m_ViewSetup;
		std::vector<std::shared_ptr<IRenderLayer>> m_RenderLayers;
		std::shared_ptr<RenderQueue> m_RenderQueue;

		std::shared_ptr<Input> m_Input;
	};

	View::View(
		const std::shared_ptr<AppWinConfig> winConfig,
		const std::shared_ptr<SceneEntityFactory> entityFactory,
		const std::shared_ptr<IGAPI> GAPI) :
		m_WinConfig{ winConfig },
		m_EntityFactory{ entityFactory },
		m_GAPI{ GAPI },
		initState{ eInitState::InitNot },
		m_ThreadsUpdate{std::string("View"), 2}
	{
		ensure(m_WinConfig, ">>>>> [View::View()]. Window config cannot be null.");
		ensure(m_EntityFactory, ">>>>> [View::View()]. Scene entity factory cannot be null.");
		ensure(m_GAPI, ">>>>> [View::View()]. GAPI cannot be null.");

		Initialize();
	}

	void View::Initialize()
	{
		try
		{
			// Cоздаём обёртку над окном приложения под текущую ОС
			m_NativeWindow = factory.CreateAppWin(m_WinConfig);
			m_NativeWindow->OnResize += std::bind(&View::OnViewResize, this, std::placeholders::_1, std::placeholders::_2);
			m_NativeWindow->OnResizing += std::bind(&View::OnViewResizing, this);
			auto res = m_NativeWindow->Initialize();
			if (!res)
				throw_runtime_error(std::format(">>>>> [View::Initialize()]. Failed to initialize application window: {}.", wstring_to_string(res.error().getMessage())));

			// Cоздаём проверхность рендринга для текущего окна и GAPI
			m_RenderSurface = factory.CreateSurfaceWin(m_NativeWindow, m_GAPI);
			res = m_RenderSurface->Initialize();
			if (!res)
				throw_runtime_error(std::format(">>>>> [View::Initialize()]. Failed to initialize surface window: {}.", wstring_to_string(res.error().getMessage())));

			// TODO: В будущем надо будет учитывать настройки рендеринга из конфигурации
			Size2D<zF32> size;
			size.SetFrom(m_NativeWindow->GetWinSize());
			m_ViewSetup = safe_make_shared<ViewSetup>(true);
			m_ViewSetup->ActivateClearColor(colors::DarkMidnightBlue);
			m_RenderQueue = safe_make_shared<RenderQueue>(m_ViewSetup, m_RenderLayers);

			m_Input = safe_make_shared<Input>(m_NativeWindow);

			initState = eInitState::InitOK;
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format(">>>>> [View::Initialize()]. -> {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [View::Initialize()]. Unknown exception.");
		}
	}

	void View::OnUpdate(double deltaTime)
	{
		if (initState != eInitState::InitOK)
			return;

		if (m_RenderSurface)
		{
			m_ThreadsUpdate.Submit([&]()
				{
					m_RenderSurface->PrepareFrame(m_RenderQueue);
				});
			m_ThreadsUpdate.Submit([&]()
				{
					m_RenderSurface->RenderFrame();
				});
			m_ThreadsUpdate.Join();
		}
	}

	void View::OnViewResizing()
	{
		viewResizing();
	}

	void View::OnViewResize(const Size2D<>& size, eTypeWinResize resizeType)
	{
		switch (resizeType)
		{
		case eTypeWinResize::Hide:
			DebugOutput(L">>>>> [View::OnViewResized()]. Hide app window.");
			break;
		case eTypeWinResize::Show:
			DebugOutput(std::format(L">>>>> [View::OnViewResized({}x{})]. Show app window.", std::to_wstring(size.width), std::to_wstring(size.height)));
			break;
		case eTypeWinResize::Resize:
			DebugOutput(std::format(L">>>>> [View::OnViewResized({}x{}))]. Resize app window.", std::to_wstring(size.width), std::to_wstring(size.height)));

			Size2D<zF32> fsize;
			fsize.SetFrom(size);
			for (auto& layer : m_RenderLayers)
				layer->UpdateSize(fsize);

			break;
		}

		if (initState != eInitState::InitOK)
			return;

		viewResized(size, resizeType);
		if (m_RenderSurface)
			m_RenderSurface->OnResize(size);
	}

	void View::SetFullScreen(bool fs)
	{
		if (initState != eInitState::InitOK)
			return;

		if(m_RenderSurface)
			m_RenderSurface->SetFullScreen(fs);
	}

	Result<std::shared_ptr<UserLayer3D>> View::AddLayer_3D()
	{
		Size2D<zF32> size;
		size.SetFrom(m_NativeWindow->GetWinSize());
		std::shared_ptr<Layer3D> layer = safe_make_shared<Layer3D>(m_EntityFactory, eAspectType::FullWindow, size, 0.0f, 1.0f);

		std::shared_ptr<UserLayer3D> userLayer = safe_make_shared<UserLayer3D>(layer);
		m_RenderLayers.push_back(layer);

		return userLayer;
	}
}