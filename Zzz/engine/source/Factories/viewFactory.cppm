
export module ViewFactory;

import IGAPI;
import Result;
import Ensure;
import IAppWin;
import StrConvert;
import ISurfView;
import AppWin_MSWin;
import PlatformConfig;

#if defined(ZRENDER_API_D3D12)
import DXAPI;
import SurfView_DX;
using namespace zzz::dx;
#elif defined(ZRENDER_API_VULKAN)
import VKAPI;
import SurfView_VK;
using namespace zzz::vk;
#else
#error ">>>>> [ViewFactory file]. Compile error. This branch requires implementation for the current platform"
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

		std::shared_ptr<IAppWin> CreateAppWin(std::shared_ptr<PlatformConfig> Settings);
		std::shared_ptr<ISurfView> CreateSurfaceWin(std::shared_ptr<IAppWin> _iAppWin, std::shared_ptr<IGAPI> _iGAPI);
	};

	std::shared_ptr<IAppWin> ViewFactory::CreateAppWin(std::shared_ptr<PlatformConfig> Settings)
	{
		try
		{
#if defined(ZPLATFORM_MSWINDOWS)
			return safe_make_shared<AppWin_MSWin>(Settings);
#elif
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format("Failed to create application window: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error("Unknown exception occurred while creating application window.");
		}
	}

	export std::shared_ptr<ISurfView> ViewFactory::CreateSurfaceWin(std::shared_ptr<IAppWin> _iAppWin, std::shared_ptr<IGAPI> _iGAPI)
	{
		ensure(_iAppWin, "Application window cannot be null.");
		ensure(_iGAPI, "GAPI cannot be null.");

		try
		{
			auto gariType = _iGAPI->GetGAPIType();

#if defined(ZRENDER_API_D3D12)
			return safe_make_shared<SurfView_DX>(_iAppWin, _iGAPI);
#elif defined(ZRENDER_API_VULKAN)
			return safe_make_shared<SurfView_VK>(_iAppWin, _iGAPI);
#else
#error ">>>>> [ViewFactory.CreateSurfaceWin()]. Compile error. This branch requires implementation for the current platform"
#endif // defined(ZPLATFORM_MSWINDOWS)
		}
		catch (const std::exception& e)
		{
			throw_runtime_error(std::format("Failed to create surface window: {}.", std::string(e.what())));
		}
		catch (...)
		{
			throw_runtime_error("Unknown exception occurred while creating surface window.");
		}
	}
}