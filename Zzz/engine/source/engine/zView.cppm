#include "pch.h"
export module zView;

import IGAPI;
import result;
import zEvent;
import IAppWin;
import zSize2D;
import strConver;
import zViewFactory;
import zViewSettings;
import IAppWinSurface;

using namespace zzz::platforms;

namespace zzz
{
	class engine;
}

namespace zzz
{
	export class zView final
	{
	public:
		zView() = delete;
		zView(const zView&) = delete;
		zView(zView&&) = delete;
		zView& operator=(const zView&) = delete;
		zView& operator=(zView&&) = delete;

		zView(std::shared_ptr<zViewSettings> _setting, std::function<void(zSize2D<>, e_TypeWinResize)> _onResizeClbk);

		~zView();

		zEvent<zSize2D<>, e_TypeWinResize> viewResized;
		void OnUpdate();
		void OnViewResized(const zSize2D<>& size, e_TypeWinResize resizeType);
		void SetFullScreen(bool fs);

	private:
		zViewFactory factory;
		std::shared_ptr<zViewSettings> m_Settings;
		std::shared_ptr<IGAPI> m_GAPI;
		std::shared_ptr<IAppWin> m_Win;
		std::shared_ptr<IAppWinSurface> m_WinSurface;

		void Initialize();
	};

	zView::zView(std::shared_ptr<zViewSettings> _setting, std::function<void(zSize2D<>, e_TypeWinResize)> _onResizeClbk) :
		m_Settings{ _setting }
	{
		ensure(m_Settings, ">>>>> [zView::zView()]. Settings cannot be null.");
		if(_onResizeClbk)
			viewResized += _onResizeClbk;

		Initialize();
	}

	zView::~zView()
	{
		//for (auto& gapi : gapiList)
		//{
		//	if (gapi)
		//	{
		//		gapi->~IGAPI();
		//	}
		//}

		//gapiList.clear();
	}

	void zView::Initialize()
	{
		try
		{
			m_Win = factory.CreateAppWin(m_Settings);
			m_Win->onResize += std::bind(&zView::OnViewResized, this, std::placeholders::_1, std::placeholders::_2);
			auto res = m_Win->Initialize();
			if (!res)
				throw_runtime_error(std::format(">>>>> [zView::Initialize()]. Failed to initialize application window: {}.", wstring_to_string(res.error().getMessage())));

			m_GAPI = factory.CreateGAPI();
			res = m_GAPI->Initialize(m_Win);
			if (!res)
				throw_runtime_error(std::format(">>>>> [zView::Initialize()]. Failed to initialize GAPI: {}.", wstring_to_string(res.error().getMessage())));

			m_WinSurface = factory.CreateSurfaceWin(m_Settings, m_Win, m_GAPI);
			res = m_WinSurface->Initialize();
			if (!res)
				throw_runtime_error(std::format(">>>>> [zView::Initialize()]. Failed to initialize surface window: {}.", wstring_to_string(res.error().getMessage())));

		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format(">>>>> [zView::Initialize()]. -> {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [zView::Initialize()]. Unknown exception.");
		}
	}

	void zView::OnUpdate()
	{

		if (m_WinSurface)
			m_WinSurface->OnRender();

		{
			//static int frameCount = 0;
			//frameCount++;

			//if (frameCount == 30)
			//	winSurface->SetFullScreen(true);

			//if (frameCount == 90)
			//	winSurface->SetFullScreen(false);
		}
	}

	void zView::OnViewResized(const zSize2D<>& size, e_TypeWinResize resizeType)
	{
		switch (resizeType)
		{
		case e_TypeWinResize::eHide:
			DebugOutput(L">>>>> [zView::OnViewResized()]. Hide app window.\n");
			break;
		case e_TypeWinResize::eShow:
			DebugOutput(std::format(L">>>>> [zView::OnViewResized({}x{})]. Show app window.\n", std::to_wstring(size.width), std::to_wstring(size.height)));
			break;
		case e_TypeWinResize::eResize:
			DebugOutput(std::format(L">>>>> [zView::OnViewResized({}x{}))]. Resize app window.\n", std::to_wstring(size.width), std::to_wstring(size.height)));
			break;
		}

		viewResized(size, resizeType);
		if (m_WinSurface)
			m_WinSurface->OnResize(size);
	}

	void zView::SetFullScreen(bool fs)
	{
	}
}