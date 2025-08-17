#include "pch.h"
export module viewFactory;

import IGAPI;
import result;
import IAppWin;
import winMSWin;
import settings;
import strConvert;
import ISurfaceView;
import surface_DirectX;

#if defined(_WIN64)
import DXAPI;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

using namespace zzz::platforms;

export namespace zzz
{
	export class viewFactory final
	{
	public:
		viewFactory() = default;
		viewFactory(const viewFactory&) = delete;
		viewFactory(viewFactory&&) = delete;
		viewFactory& operator=(const viewFactory&) = delete;
		viewFactory& operator=(viewFactory&&) = delete;
		~viewFactory() = default;

		std::shared_ptr<IAppWin> CreateAppWin(std::shared_ptr<settings> settings);
		std::shared_ptr<ISurfaceView> CreateSurfaceWin(
			std::shared_ptr<settings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<zzz::platforms::IGAPI> _iGAPI);
	};

	std::shared_ptr<IAppWin> viewFactory::CreateAppWin(std::shared_ptr<settings> settings)
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
			throw_runtime_error(std::format(">>>>> [viewFactory::CreateAppWin()]. Failed to create application window: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [viewFactory::CreateAppWin()]. Unknown exception occurred while creating application window.");
		}
	}

	export std::shared_ptr<ISurfaceView> viewFactory::CreateSurfaceWin(
		std::shared_ptr<settings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<zzz::platforms::IGAPI> _iGAPI)
	{
		ensure(_settings, ">>>>> [viewFactory::CreateSurfaceWin()]. Settings cannot be null.");
		ensure(_iAppWin, ">>>>> [viewFactory::CreateSurfaceWin()]. Application window cannot be null.");
		ensure(_iGAPI, ">>>>> [viewFactory::CreateSurfaceWin()]. GAPI cannot be null.");

		try
		{
			auto gariType = _iGAPI->GetGAPIType();

			switch (gariType)
			{
#if defined(_WIN64)
			case zzz::platforms::eGAPIType::DirectX:
				return safe_make_shared<surface_DirectX>(_settings, _iAppWin, _iGAPI);
				break;
#endif // defined(_WIN64)
			default:
				throw_runtime_error(std::format(">>>>> [viewFactory::CreateSurfaceWin()]. Unsupported GAPI type: {}.", static_cast<uint8_t>(gariType)));
				break;
			}
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format(">>>>> [viewFactory::CreateSurfaceWin()]. Failed to create surface window: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error(">>>>>> [viewFactory::CreateSurfaceWin()]. Unknown exception occurred while creating surface window.");
		}
	}
}