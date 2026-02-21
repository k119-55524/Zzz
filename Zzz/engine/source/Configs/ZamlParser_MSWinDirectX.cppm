
export module ZamlParser_MSWinDirectX;

import Result;
import Size2D;
import ZamlConfig;
import PlatformConfig;

using namespace zzz::core;

#if defined(ZPLATFORM_MSWINDOWS)
namespace zzz
{
	export class ZamlParser_MSWinDirectX
	{
	public:
		Result<std::shared_ptr<PlatformConfig>> GetAppWinConfig(const std::shared_ptr<ZamlConfig> zaml)
		{
			Result<std::wstring> caption = zaml->GetParam<std::wstring>(L"Caption");
			Result<std::wstring> className = zaml->GetParam<std::wstring>(L"MSWin_SpecSettings", L"ClassName");
			Result<int> width = zaml->GetParam<int>(L"Width");
			Result<int> height = zaml->GetParam<int>(L"Height");
			Result<std::wstring> icoPath = zaml->GetParam<std::wstring>(L"IcoFullPath");
			Result<int> icoSize = zaml->GetParam<int>(L"IcoSize");

			std::shared_ptr<PlatformConfig> config;
			if (caption && className && width && height && icoPath && icoSize)
			{
				Size2D<LONG> winSize(width.value(), height.value());
				config = safe_make_shared<PlatformConfig>(
					caption.value(),
					className.value(),
					winSize,
					icoPath.value(),
					icoSize.value());
			}
			else
				config = safe_make_shared<PlatformConfig>();

			return Result<std::shared_ptr<PlatformConfig>>(config);
		}
	};
}
#endif