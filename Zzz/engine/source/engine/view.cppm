#include "pch.h"
export module View;

import IGAPI;
import event;
import Scene;
import size2D;
import result;
import IAppWin;
import Settings;
import strConvert;
import ThreadPool;
import viewFactory;
import ISurfaceView;
import ScenesManager;
import PerformanceMeter;
import ResourcesManager;

using namespace Zzz::Templates;
using namespace zzz::platforms;

namespace zzz
{
	class engine;
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

	private:
		viewFactory factory;
		const std::shared_ptr<Settings> m_Settings;
		const std::shared_ptr<IGAPI> m_GAPI;
		const std::shared_ptr<ScenesManager> m_ScenesManager;
		std::shared_ptr<IAppWin> m_Win;
		std::shared_ptr<ISurfaceView> m_SurfaceView;

		eInitState initState;
		void Initialize();
		void PrepareFrame(double deltaTime);

		ThreadPool m_ThreadRenderAnUpdate;
		std::shared_ptr<Scene> m_Scene;

		PerformanceMeter m_PerfRender;
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
		m_ThreadRenderAnUpdate{2}
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

			// Создаём сцену
			auto res1 = m_ScenesManager->GetStartScene();
			if (res1)
				m_Scene = res1.value();

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
			m_ThreadRenderAnUpdate.Submit([&]()
				{
					m_SurfaceView->RenderFrame();
				});
			m_ThreadRenderAnUpdate.Submit([&]()
				{
					PrepareFrame(deltaTime);
				});
			m_ThreadRenderAnUpdate.Join();
		}

		{
			//static int frameCount = 0;
			//frameCount++;

			//if (frameCount == 30)
			//	SetFullScreen(true);

			//if (frameCount == 60)
			//	SetFullScreen(false);
		}
	}

	void View::PrepareFrame(double deltaTime)
	{
		//m_PerfRender.StartPerformance();
		m_SurfaceView->PrepareFrame();
		//double perfTime = m_PerfRender.StopPerformance();

		//{
		//	const int testValue = 60;
		//	static int frameCount = 0;
		//	static double allTime = 0.0;
		//	frameCount++;
		//	allTime += perfTime;
		//	if (frameCount == testValue)
		//	{
		//		double fps = 1.0 / (allTime / testValue);
		//		frameCount = 0;
		//		allTime = 0.0;
		//		DebugOutput(std::format(L">>>>> [View::PrepareFrame()]. FPS: {}.\n", fps));
		//	}
		//}
	}

	void View::OnViewResized(const size2D<>& size, e_TypeWinResize resizeType)
	{
		if (initState != eInitState::eInitOK)
			return;

		switch (resizeType)
		{
		case e_TypeWinResize::eHide:
			DebugOutput(L">>>>> [View::OnViewResized()]. Hide app window.\n");
			break;
		case e_TypeWinResize::eShow:
			DebugOutput(std::format(L">>>>> [View::OnViewResized({}x{})]. Show app window.\n", std::to_wstring(size.width), std::to_wstring(size.height)));
			break;
		case e_TypeWinResize::eResize:
			DebugOutput(std::format(L">>>>> [View::OnViewResized({}x{}))]. Resize app window.\n", std::to_wstring(size.width), std::to_wstring(size.height)));
			break;
		}

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