
#include "pch.h"

export module View;

import IGAPI;
import Event;
import Scene;
import size2D;
import result;
import IAppWin;
import Settings;
import StrConvert;
import ThreadPool;
import RenderQueue;
import ViewFactory;
import ISurfaceView;
import ScenesManager;

using namespace zzz::templates;
using namespace zzz::engineCore;

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
			const std::shared_ptr<Settings> _setting,
			const std::shared_ptr<ScenesManager> _scenesManager,
			const std::shared_ptr<IGAPI> _GAPI);

		~View() = default;

		Event<size2D<>, eTypeWinResize> viewResized;
		Event<> viewResizing;

		void OnUpdate(double deltaTime);
		void SetFullScreen(bool fs);
		inline void SetVSync(bool vs) { if (m_SurfaceView != nullptr) m_SurfaceView->SetVSync(vs); };
		inline void SetViewCaptionText(std::wstring caption) { if (m_Win != nullptr) m_Win->SetCaptionText(caption); };
		inline void AddViewCaptionText(std::wstring caption) { if (m_Win != nullptr) m_Win->AddCaptionText(caption); };

	private:
		void OnViewResizing();
		void OnViewResize(const size2D<>& size, eTypeWinResize resizeType);

		ViewFactory factory;
		const std::shared_ptr<Settings> m_Settings;
		const std::shared_ptr<IGAPI> m_GAPI;
		const std::shared_ptr<ScenesManager> m_ScenesManager;
		std::shared_ptr<IAppWin> m_Win;
		std::shared_ptr<ISurfaceView> m_SurfaceView;

		eInitState initState;
		void Initialize();
		void PrepareFrame(double deltaTime);

		ThreadPool m_ThreadsUpdate;
		std::shared_ptr<Scene> m_Scene;
	};

	View::View(
		const std::shared_ptr<Settings> _setting,
		const std::shared_ptr<ScenesManager> _scenesManager,
		const std::shared_ptr<IGAPI> _GAPI) :
		m_Settings{ _setting },
		m_ScenesManager{ _scenesManager },
		m_GAPI{ _GAPI },
		initState{ eInitState::InitNot },
		m_ThreadsUpdate{2}
	{
		ensure(m_Settings, ">>>>> [View::View()]. Settings cannot be null.");
		ensure(m_ScenesManager, ">>>>> [View::View()]. Scenes manager cannot be null.");
		ensure(m_GAPI, ">>>>> [View::View()]. GAPI cannot be null.");

		Initialize();
	}

	void View::Initialize()
	{
		try
		{
			// Cоздаём обёртку над окном приложения под текущую ОС
			m_Win = factory.CreateAppWin(m_Settings);
			m_Win->OnResize += std::bind(&View::OnViewResize, this, std::placeholders::_1, std::placeholders::_2);
			m_Win->OnResizing += std::bind(&View::OnViewResizing, this);
			auto res = m_Win->Initialize();
			if (!res)
				throw_runtime_error(std::format(">>>>> [View::Initialize()]. Failed to initialize application window: {}.", wstring_to_string(res.error().getMessage())));

			// Cоздаём проверхность рендринга для текущего окна и GAPI
			m_SurfaceView = factory.CreateSurfaceWin(m_Settings, m_Win, m_GAPI);
			res = m_SurfaceView->Initialize();
			if (!res)
				throw_runtime_error(std::format(">>>>> [View::Initialize()]. Failed to initialize surface window: {}.", wstring_to_string(res.error().getMessage())));

			// Кусок кода для теста
			{
				try
				{
					// Создаём сцену для теста
					auto res1 = m_ScenesManager->GetDefaultScene();
					if (!res1)
						throw_runtime_error(">>>>> #0 [View::Initialize()}. ERROR!!! Failed to create test scene.");

					m_Scene = res1.value();
				}
				catch (...)
				{
					DebugOutput(L">>>>> #1 [View::Initialize()}. ERROR!!! Failed to create test scene.");
				}
			}

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

		if (m_SurfaceView)
		{
			m_ThreadsUpdate.Submit([&]()
				{
					PrepareFrame(deltaTime);
				});
			m_ThreadsUpdate.Submit([&]()
				{
					m_SurfaceView->RenderFrame();
				});
			m_ThreadsUpdate.Join();
		}
	}

	void View::OnViewResizing()
	{
		viewResizing();
	}

	void View::PrepareFrame(double deltaTime)
	{
		std::shared_ptr<RenderQueue> renderQueue = m_Scene->GetCalcRenderQueue();
		m_SurfaceView->PrepareFrame(m_Scene, renderQueue);
	}

	void View::OnViewResize(const size2D<>& size, eTypeWinResize resizeType)
	{
#if defined(_DEBUG)
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
			break;
		}
#endif	// _DEBUG

		if (initState != eInitState::InitOK)
			return;

		viewResized(size, resizeType);
		if (m_SurfaceView)
			m_SurfaceView->OnResize(size);
	}

	void View::SetFullScreen(bool fs)
	{
		if (initState != eInitState::InitOK)
			return;

		if(m_SurfaceView)
			m_SurfaceView->SetFullScreen(fs);
	}
}