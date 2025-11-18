
export module ZamlParser_MSWinDirectX;

import Result;
import Size2D;
import ZamlConfig;
import AppWinConfig;

using namespace zzz::core;

#if defined(ZPLATFORM_MSWINDOWS)
namespace zzz
{
	export class ZamlParser_MSWinDirectX
	{
	public:
		Result<std::shared_ptr<AppWinConfig>> GetAppWinConfig(const std::shared_ptr<ZamlConfig> zaml)
		{
			Result<std::wstring> caption = zaml->GetParam<std::wstring>(L"Caption");
			Result<std::wstring> className = zaml->GetParam<std::wstring>(L"MSWin_SpecSettings", L"ClassName");
			Result<int> width = zaml->GetParam<int>(L"Width");
			Result<int> height = zaml->GetParam<int>(L"Height");
			Result<std::wstring> icoPath = zaml->GetParam<std::wstring>(L"IcoFullPath");
			Result<int> icoSize = zaml->GetParam<int>(L"IcoSize");

			std::shared_ptr<AppWinConfig> config;
			if (caption && className && width && height && icoPath && icoSize)
			{
				Size2D<LONG> winSize(width.value(), height.value());
				config = safe_make_shared<AppWinConfig>(
					caption.value(),
					className.value(),
					winSize,
					icoPath.value(),
					icoSize.value());
			}
			else
				config = safe_make_shared<AppWinConfig>();

			return Result<std::shared_ptr<AppWinConfig>>(config);
		}
	};
}
#endif