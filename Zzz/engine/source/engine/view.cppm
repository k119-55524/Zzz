#include "pch.h"
export module view;

import IGAPI;
import result;
import event;
import IAppWin;
import size2D;
import strConvert;
import viewFactory;
import settings;
import IAppWinSurface;

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

		view(std::shared_ptr<settings> _setting, std::shared_ptr<IGAPI> _GAPI, std::function<void(size2D<>, e_TypeWinResize)> _onResizeClbk);

		~view() = default;

		event<size2D<>, e_TypeWinResize> viewResized;
		void OnUpdate();
		void OnViewResized(const size2D<>& size, e_TypeWinResize resizeType);
		void SetFullScreen(bool fs);

	private:
		viewFactory factory;
		std::shared_ptr<settings> m_Settings;
		std::shared_ptr<IGAPI> m_GAPI;
		std::shared_ptr<IAppWin> m_Win;
		std::shared_ptr<IAppWinSurface> m_WinSurface;

		void Initialize();
	};

	view::view(std::shared_ptr<settings> _setting, std::shared_ptr<IGAPI> _GAPI, std::function<void(size2D<>, e_TypeWinResize)> _onResizeClbk) :
		m_Settings{ _setting },
		m_GAPI{ _GAPI }
	{
		ensure(m_Settings, ">>>>> [view::view()]. Settings cannot be null.");
		ensure(m_GAPI, ">>>>> [view::view()]. GAPI cannot be null.");

		if(_onResizeClbk)
			viewResized += _onResizeClbk;

		Initialize();
	}

	void view::Initialize()
	{
		try
		{
			m_Win = factory.CreateAppWin(m_Settings);
			m_Win->onResize += std::bind(&view::OnViewResized, this, std::placeholders::_1, std::placeholders::_2);
			auto res = m_Win->Initialize();
			if (!res)
				throw_runtime_error(std::format(">>>>> [view::Initialize()]. Failed to initialize application window: {}.", wstring_to_string(res.error().getMessage())));

			m_WinSurface = factory.CreateSurfaceWin(m_Settings, m_Win, m_GAPI);
			res = m_WinSurface->Initialize();
			if (!res)
				throw_runtime_error(std::format(">>>>> [view::Initialize()]. Failed to initialize surface window: {}.", wstring_to_string(res.error().getMessage())));

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

		if (m_WinSurface)
			m_WinSurface->OnRender();

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
		if (m_WinSurface)
			m_WinSurface->OnResize(size);
	}

	void view::SetFullScreen(bool fs)
	{
		if(m_WinSurface)
			m_WinSurface->SetFullScreen(fs);
	}
}