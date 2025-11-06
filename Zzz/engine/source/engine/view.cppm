#include "pch.h"
export module View;

import IGAPI;
import event;
import Scene;
import size2D;
import result;
import IAppWin;
import Settings;
import StrConvert;
import ThreadPool;
import ViewFactory;
import ISurfaceView;
import ScenesManager;
import CPUResourcesManager;

using namespace zzz::templates;
using namespace zzz::platforms;

namespace zzz
{
	class Engine;
}

namespace zzz
{
	export class View final
	{
	public:
		View() = delete;
		View(const View&) = delete;
		View(View&&) = delete;
		View& operator=(const View&) = delete;
		View& operator=(View&&) = delete;

		View(
			const std::shared_ptr<Settings> _setting,
			const std::shared_ptr<ScenesManager> _scenesManager,
			const std::shared_ptr<IGAPI> _GAPI,
			std::function<void(size2D<>, e_TypeWinResize)> _onResizeClbk);

		~View() = default;

		event<size2D<>, e_TypeWinResize> viewResized;
		void OnUpdate(double deltaTime);
		void OnViewResized(const size2D<>& size, e_TypeWinResize resizeType);
		void SetFullScreen(bool fs);
		inline void SetVSync(bool vs) { if (m_SurfaceView != nullptr) m_SurfaceView->SetVSync(vs); };
		inline void SetViewCaptionText(std::wstring caption) { if (m_Win != nullptr) m_Win->SetCaptionText(caption); };
		inline void AddViewCaptionText(std::wstring caption) { if (m_Win != nullptr) m_Win->AddCaptionText(caption); };

	private:
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
		const std::shared_ptr<IGAPI> _GAPI,
		std::function<void(size2D<>, e_TypeWinResize)> _onResizeClbk) :
		m_Settings{ _setting },
		m_ScenesManager{ _scenesManager },
		m_GAPI{ _GAPI },
		initState{ eInitState::eInitNot },
		m_ThreadsUpdate{2}
	{
		ensure(m_Settings, ">>>>> [View::View()]. Settings cannot be null.");
		ensure(m_ScenesManager, ">>>>> [View::View()]. Scenes manager cannot be null.");
		ensure(m_GAPI, ">>>>> [View::View()]. GAPI cannot be null.");

		if(_onResizeClbk)
			viewResized += _onResizeClbk;

		Initialize();
	}

	void View::Initialize()
	{
		try
		{
			// Cоздаём обёртку над окном приложения под текущую ОС
			m_Win = factory.CreateAppWin(m_Settings);
			m_Win->onResize += std::bind(&View::OnViewResized, this, std::placeholders::_1, std::placeholders::_2);
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

			initState = eInitState::eInitOK;
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
		if (initState != eInitState::eInitOK)
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

	void View::PrepareFrame(double deltaTime)
	{
		m_SurfaceView->PrepareFrame(m_Scene);
	}

	void View::OnViewResized(const size2D<>& size, e_TypeWinResize resizeType)
	{
#if defined(_DEBUG)
		switch (resizeType)
		{
		case e_TypeWinResize::eHide:
			DebugOutput(L">>>>> [View::OnViewResized()]. Hide app window.");
			break;
		case e_TypeWinResize::eShow:
			DebugOutput(std::format(L">>>>> [View::OnViewResized({}x{})]. Show app window.", std::to_wstring(size.width), std::to_wstring(size.height)));
			break;
		case e_TypeWinResize::eResize:
			DebugOutput(std::format(L">>>>> [View::OnViewResized({}x{}))]. Resize app window.", std::to_wstring(size.width), std::to_wstring(size.height)));
			break;
		}
#endif	// _DEBUG

		if (initState != eInitState::eInitOK)
			return;

		viewResized(size, resizeType);
		if (m_SurfaceView)
			m_SurfaceView->OnResize(size);
	}

	void View::SetFullScreen(bool fs)
	{
		if (initState != eInitState::eInitOK)
			return;

		if(m_SurfaceView)
			m_SurfaceView->SetFullScreen(fs);
	}
}