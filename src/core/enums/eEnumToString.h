#pragma once

#include <string_view>
#include <core/utils/Macroses.h>
#include <core/utils/ThrowWrappers.h>
#include <core/enums/eWinResize.h>
#include <core/enums/eLogMessageType.h>

#include "ePackage.h"
#include "eTargetPlatform.h"
#include "platforms/eiOSEnums.h"
#include "platforms/eLinuxEnums.h"
#include "platforms/eMacOSEnums.h"
#include "platforms/eMSWinEnums.h"
#include "platforms/eAndroidEnums.h"

namespace zzz::common
{
	class EnumToString
	{
	public:
		static constexpr std::string_view ToString(eLogMessageType type)
		{
			switch (type)
			{
			case eLogMessageType::Message:   return "MESSAGE";
			case eLogMessageType::Warning:   return "WARNING";
			case eLogMessageType::Error:     return "ERROR";
			case eLogMessageType::Exception: return "EXCEPTION";
			case eLogMessageType::Critical:  return "CRITICAL";
			case eLogMessageType::Fatal:     return "FATAL";
			case eLogMessageType::All:       return "ALL";
			case eLogMessageType::None:      return "NONE";
			}
			THROW_RUNTIME("Необработанный eLogMessageType");
		}

		static constexpr std::string_view ToString(eWinResize type)
		{
			switch (type)
			{
			case eWinResize::Show:   return "SHOW";
			case eWinResize::Hide:   return "HIDE";
			case eWinResize::Resize: return "RESIZE";
			}
			THROW_RUNTIME("Необработанный eWinResize");
		}

		static constexpr std::string_view ToString(ePackage type)
		{
			switch (type)
			{
			case ePackage::ProjectManifest: return "ProjectManifest";
			case ePackage::Scene:           return "Scene";
			case ePackage::View:            return "View";
			case ePackage::Prefab:          return "Prefab";
			case ePackage::BinaryAsset:     return "BinaryAsset";
			case ePackage::AppView:         return "AppView";
			}
			THROW_RUNTIME("Необработанный ePackage");
		}

		static constexpr std::string_view ToString(eTargetPlatform type)
		{
			switch (type)
			{
			case eTargetPlatform::Windows: return "Windows";
			case eTargetPlatform::Linux:   return "Linux";
			case eTargetPlatform::Android: return "Android";
			case eTargetPlatform::MacOS:   return "MacOS";
			case eTargetPlatform::iOS:     return "iOS";
			}
			THROW_RUNTIME("Необработанный eTargetPlatform");
		}

		static constexpr std::string_view ToString(eMSWinWindowMode type)
		{
			switch (type)
			{
			case eMSWinWindowMode::Windowed:            return "Windowed";
			case eMSWinWindowMode::BorderlessFullscreen:return "BorderlessFullscreen";
			case eMSWinWindowMode::ExclusiveFullscreen: return "ExclusiveFullscreen";
			}
			THROW_RUNTIME("Необработанный eMSWinWindowMode");
		}

		static constexpr std::string_view ToString(eMSWinWindowStyle type)
		{
			switch (type)
			{
			case eMSWinWindowStyle::OverlappedWindow: return "OverlappedWindow";
			case eMSWinWindowStyle::PopUp:            return "PopUp";
			case eMSWinWindowStyle::ToolWindow:       return "ToolWindow";
			}
			THROW_RUNTIME("Необработанный eMSWinWindowStyle");
		}

		static constexpr std::string_view ToString(eAndroidScreenOrientation type)
		{
			switch (type)
			{
			case eAndroidScreenOrientation::Sensor:        return "Sensor";
			case eAndroidScreenOrientation::Portrait:      return "Portrait";
			case eAndroidScreenOrientation::LandscapeLeft: return "LandscapeLeft";
			case eAndroidScreenOrientation::LandscapeRight:return "LandscapeRight";
			}
			THROW_RUNTIME("Необработанный eAndroidScreenOrientation");
		}

		static constexpr std::string_view ToString(eAndroidCutoutMode type)
		{
			switch (type)
			{
			case eAndroidCutoutMode::Default:   return "Default";
			case eAndroidCutoutMode::ShortEdges:return "ShortEdges";
			case eAndroidCutoutMode::Never:     return "Never";
			}
			THROW_RUNTIME("Необработанный eAndroidCutoutMode");
		}

		static constexpr std::string_view ToString(eiOSScreenOrientation type)
		{
			switch (type)
			{
			case eiOSScreenOrientation::AutoRotate:    return "AutoRotate";
			case eiOSScreenOrientation::Portrait:      return "Portrait";
			case eiOSScreenOrientation::LandscapeLeft: return "LandscapeLeft";
			case eiOSScreenOrientation::LandscapeRight:return "LandscapeRight";
			}
			THROW_RUNTIME("Необработанный eiOSScreenOrientation");
		}

		static constexpr std::string_view ToString(eiOSSafeAreaMode type)
		{
			switch (type)
			{
			case eiOSSafeAreaMode::UseSafeArea:       return "UseSafeArea";
			case eiOSSafeAreaMode::ExtendIntoSafeArea: return "ExtendIntoSafeArea";
			}
			THROW_RUNTIME("Необработанный eiOSSafeAreaMode");
		}

		static constexpr std::string_view ToString(eiOSHomeIndicatorMode type)
		{
			switch (type)
			{
			case eiOSHomeIndicatorMode::Visible:   return "Visible";
			case eiOSHomeIndicatorMode::AutoHidden:return "AutoHidden";
			}
			THROW_RUNTIME("Необработанный eiOSHomeIndicatorMode");
		}

		static constexpr std::string_view ToString(eLinuxDisplayServer type)
		{
			switch (type)
			{
			case eLinuxDisplayServer::Auto:   return "Auto";
			case eLinuxDisplayServer::Wayland:return "Wayland";
			case eLinuxDisplayServer::X11:    return "X11";
			}
			THROW_RUNTIME("Необработанный eLinuxDisplayServer");
		}

		static constexpr std::string_view ToString(eLinuxWindowMode type)
		{
			switch (type)
			{
			case eLinuxWindowMode::Windowed:  return "Windowed";
			case eLinuxWindowMode::Fullscreen:return "Fullscreen";
			}
			THROW_RUNTIME("Необработанный eLinuxWindowMode");
		}

		static constexpr std::string_view ToString(eMacOSWindowMode type)
		{
			switch (type)
			{
			case eMacOSWindowMode::Windowed:  return "Windowed";
			case eMacOSWindowMode::Fullscreen:return "Fullscreen";
			}
			THROW_RUNTIME("Необработанный eMacOSWindowMode");
		}
	};
}
