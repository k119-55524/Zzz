#include "pch.h"
export module ViewFactory;

import IGAPI;
import Result;
import IAppWin;
import Settings;
import StrConvert;
import SurfDirectX;
import ISurfaceView;
import AppWin_MSWin;

#if defined(ZRENDER_API_D3D12)
import DXAPI;
using namespace zzz::directx;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

using namespace zzz::core;

export namespace zzz
{
	export class ViewFactory final
	{
	public:
		ViewFactory() = default;
		ViewFactory(const ViewFactory&) = delete;
		ViewFactory(ViewFactory&&) = delete;
		ViewFactory& operator=(const ViewFactory&) = delete;
		ViewFactory& operator=(ViewFactory&&) = delete;
		~ViewFactory() = default;

		std::shared_ptr<IAppWin> CreateAppWin(std::shared_ptr<Settings> Settings);
		std::shared_ptr<ISurfaceView> CreateSurfaceWin(
			std::shared_ptr<Settings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);
	};

	std::shared_ptr<IAppWin> ViewFactory::CreateAppWin(std::shared_ptr<Settings> Settings)
	{
		try
		{
#if defined(ZRENDER_API_D3D12)
			return safe_make_shared<AppWin_MSWin>(Settings);
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format(">>>>> [ViewFactory::CreateAppWin()]. Failed to create application window: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [ViewFactory::CreateAppWin()]. Unknown exception occurred while creating application window.");
		}
	}

	export std::shared_ptr<ISurfaceView> ViewFactory::CreateSurfaceWin(
		std::shared_ptr<Settings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI)
	{
		ensure(_settings, ">>>>> [ViewFactory::CreateSurfaceWin()]. Settings cannot be null.");
		ensure(_iAppWin, ">>>>> [ViewFactory::CreateSurfaceWin()]. Application window cannot be null.");
		ensure(_iGAPI, ">>>>> [ViewFactory::CreateSurfaceWin()]. GAPI cannot be null.");

		try
		{
			auto gariType = _iGAPI->GetGAPIType();

			switch (gariType)
			{
#if defined(_WIN64)
			case eGAPIType::DirectX:
				return safe_make_shared<SurfDirectX>(_settings, _iAppWin, _iGAPI);
				break;
#endif // defined(_WIN64)
			default:
				throw_runtime_error(std::format(">>>>> [ViewFactory::CreateSurfaceWin()]. Unsupported GAPI type: {}.", static_cast<uint8_t>(gariType)));
				break;
			}
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format(">>>>> [ViewFactory::CreateSurfaceWin()]. Failed to create surface window: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [ViewFactory::CreateSurfaceWin()]. Unknown exception occurred while creating surface window.");
		}
	}
}