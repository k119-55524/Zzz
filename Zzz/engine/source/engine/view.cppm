#include "pch.h"
export module view;

import IGAPI;
import event;
import Scene;
import size2D;
import result;
import IAppWin;
import settings;
import strConvert;
import viewFactory;
import ISurfaceView;
import ScenesManager;
import resourcesManager;

using namespace zzz::platforms;

namespace zzz
{
	class engine;
}

namespace zzz
{
	export class view final
	{
	public:
		view() = delete;
		view(const view&) = delete;
		view(view&&) = delete;
		view& operator=(const view&) = delete;
		view& operator=(view&&) = delete;

		view(
			const std::shared_ptr<settings> _setting,
			const std::shared_ptr<ScenesManager> _scenesManager,
			const std::shared_ptr<IGAPI> _GAPI,
			std::function<void(size2D<>, e_TypeWinResize)> _onResizeClbk);

		~view() = default;

		event<size2D<>, e_TypeWinResize> viewResized;
		void OnUpdate();
		void OnViewResized(const size2D<>& size, e_TypeWinResize resizeType);
		void SetFullScreen(bool fs);

	private:
		viewFactory factory;
		const std::shared_ptr<settings> m_Settings;
		const std::shared_ptr<IGAPI> m_GAPI;
		const std::shared_ptr<ScenesManager> m_ScenesManager;
		std::shared_ptr<IAppWin> m_Win;
		std::shared_ptr<ISurfaceView> m_SurfaceView;

		eInitState initState;
		void Initialize();

		std::shared_ptr<Scene> m_Scene;
	};

	view::view(
		const std::shared_ptr<settings> _setting,
		const std::shared_ptr<ScenesManager> _scenesManager,
		const std::shared_ptr<IGAPI> _GAPI,
		std::function<void(size2D<>, e_TypeWinResize)> _onResizeClbk) :
		m_Settings{ _setting },
		m_ScenesManager{ _scenesManager },
		m_GAPI{ _GAPI },
		initState{ eInitState::eInitNot }
	{
		ensure(m_Settings, ">>>>> [view::view()]. Settings cannot be null.");
		ensure(m_ScenesManager, ">>>>> [view::view()]. Scenes manager cannot be null.");
		ensure(m_GAPI, ">>>>> [view::view()]. GAPI cannot be null.");

		if(_onResizeClbk)
			viewResized += _onResizeClbk;

		Initialize();
	}

	void view::Initialize()
	{
		try
		{
			// Cоздаём обёртку над окном приложения под текущую ОС
			m_Win = factory.CreateAppWin(m_Settings);
			m_Win->onResize += std::bind(&view::OnViewResized, this, std::placeholders::_1, std::placeholders::_2);
			auto res = m_Win->Initialize();
			if (!res)
				throw_runtime_error(std::format(">>>>> [view::Initialize()]. Failed to initialize application window: {}.", wstring_to_string(res.error().getMessage())));

			// Cоздаём проверхность рендринга для текущего окна и GAPI
			m_SurfaceView = factory.CreateSurfaceWin(m_Settings, m_Win, m_GAPI);
			res = m_SurfaceView->Initialize();
			if (!res)
				throw_runtime_error(std::format(">>>>> [view::Initialize()]. Failed to initialize surface window: {}.", wstring_to_string(res.error().getMessage())));

			// Создаём сцену
			auto res1 = m_ScenesManager->GetStartScene();
			if (res1)
				m_Scene = res1.value();

			initState = eInitState::eInitOK;
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format(">>>>> [view::Initialize()]. -> {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [view::Initialize()]. Unknown exception.");
		}
	}

	void view::OnUpdate()
	{
		if (initState != eInitState::eInitOK)
			return;

		if (m_SurfaceView)
		{
			// Если есть сцена
			if (m_Scene)
			{
				// рендрим её
			}

			m_SurfaceView->BeginRender();
			m_SurfaceView->Render();
			m_SurfaceView->EndRender();
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

	void view::OnViewResized(const size2D<>& size, e_TypeWinResize resizeType)
	{
		if (initState != eInitState::eInitOK)
			return;

		switch (resizeType)
		{
		case e_TypeWinResize::eHide:
			DebugOutput(L">>>>> [view::OnViewResized()]. Hide app window.\n");
			break;
		case e_TypeWinResize::eShow:
			DebugOutput(std::format(L">>>>> [view::OnViewResized({}x{})]. Show app window.\n", std::to_wstring(size.width), std::to_wstring(size.height)));
			break;
		case e_TypeWinResize::eResize:
			DebugOutput(std::format(L">>>>> [view::OnViewResized({}x{}))]. Resize app window.\n", std::to_wstring(size.width), std::to_wstring(size.height)));
			break;
		}

		viewResized(size, resizeType);
		if (m_SurfaceView)
			m_SurfaceView->OnResize(size);
	}

	void view::SetFullScreen(bool fs)
	{
		if (initState != eInitState::eInitOK)
			return;

		if(m_SurfaceView)
			m_SurfaceView->SetFullScreen(fs);
	}
}