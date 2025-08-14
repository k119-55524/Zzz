#include "pch.h"
export module zView;

import IGAPI;
import result;
import zEvent;
import IAppWin;
import zSize2D;
import strConver;
import ISurfaceAppWin;
import zViewFactory;
import zViewSettings;

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
		std::shared_ptr<zViewSettings> settings;
		std::shared_ptr<IGAPI> gapi;
		std::shared_ptr<IAppWin> appWin;
		std::shared_ptr<ISurfaceAppWin> winSurface;

		void Initialize();
	};

	zView::zView(std::shared_ptr<zViewSettings> _setting, std::function<void(zSize2D<>, e_TypeWinResize)> _onResizeClbk) :
		settings{ _setting }
	{
		ensure(settings, ">>>>> [zView::zView()]. Settings cannot be null.");
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
			appWin = factory.CreateAppWin(settings);
			appWin->onResize += std::bind(&zView::OnViewResized, this, std::placeholders::_1, std::placeholders::_2);
			auto res = appWin->Initialize();
			if (!res)
				throw_runtime_error(std::format(">>>>> [zView::Initialize()]. Failed to initialize application window: {}.", wstring_to_string(res.error().getMessage())));

			gapi = factory.CreateGAPI();
			res = gapi->Initialize(appWin);
			if (!res)
				throw_runtime_error(std::format(">>>>> [zView::Initialize()]. Failed to initialize GAPI: {}.", wstring_to_string(res.error().getMessage())));

			winSurface = factory.CreateSurfaceWin(settings, appWin, gapi);
			res = winSurface->Initialize();
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
		static int frameCount = 0;
		frameCount++;

		if (winSurface)
			winSurface->OnRender();

		if (frameCount == 30)
			winSurface->SetFullScreen(true);

		if (frameCount == 90)
			winSurface->SetFullScreen(false);

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
		//if(gapi)
		//	gapi->OnResize(size);
		if (winSurface)
			winSurface->OnResize(size);
	}

	void zView::SetFullScreen(bool fs)
	{
	}
}