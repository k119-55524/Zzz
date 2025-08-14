#include "pch.h"
export module zViewFactory;

import IGAPI;
import result;
import IAppWin;
import winMSWin;
import strConver;
import IAppWinSurface;
import zViewSettings;
import surfaceAppMSWin_DirectX;

#if defined(_WIN64)
import DXAPI;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

using namespace zzz::platforms;

export namespace zzz
{
	export class zViewFactory final
	{
	public:
		zViewFactory() = default;
		zViewFactory(const zViewFactory&) = delete;
		zViewFactory(zViewFactory&&) = delete;
		zViewFactory& operator=(const zViewFactory&) = delete;
		zViewFactory& operator=(zViewFactory&&) = delete;
		~zViewFactory() = default;

		std::shared_ptr<IAppWin> CreateAppWin(std::shared_ptr<zViewSettings> settings);
		std::shared_ptr<IGAPI> CreateGAPI();
		std::shared_ptr<IAppWinSurface> CreateSurfaceWin(
			std::shared_ptr<zViewSettings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<zzz::platforms::IGAPI> _iGAPI);
	};

	std::shared_ptr<IAppWin> zViewFactory::CreateAppWin(std::shared_ptr<zViewSettings> settings)
	{
		try
		{
#if defined(_WIN64)
			return safe_make_shared<winMSWin>(settings);
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format(">>>>> [zViewFactory::CreateAppWin()]. Failed to create application window: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [zViewFactory::CreateAppWin()]. Unknown exception occurred while creating application window.");
		}
	}

	std::shared_ptr<IGAPI> zViewFactory::CreateGAPI()
	{
		try
		{
#if defined(_WIN64)
			return safe_make_shared<DXAPI>();
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format(">>>>> [zViewFactory::CreateGAPI()]. Failed to create GAPI: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [zViewFactory::CreateGAPI()]. Unknown exception occurred while creating GAPI.");
		}
	}

	export std::shared_ptr<IAppWinSurface> zViewFactory::CreateSurfaceWin(
		std::shared_ptr<zViewSettings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<zzz::platforms::IGAPI> _iGAPI)
	{
		try
		{
#if defined(_WIN64)
			return safe_make_shared<surfaceAppMSWin_DirectX>(_settings, _iAppWin, _iGAPI);
#else
			throw_runtime_error(">>>>> [zViewFactory::CreateSurfaceWin()]. This branch requires implementation for the current platform");
#endif
			}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format(">>>>> [zViewFactory::CreateSurfaceWin()]. Failed to create surface window: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [zViewFactory::CreateSurfaceWin()]. Unknown exception occurred while creating surface window.");
		}
	}
}