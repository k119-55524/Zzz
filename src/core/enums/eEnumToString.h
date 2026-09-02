#pragma once

#include <string_view>

#include "eResourceType.h"
#include "ePixelFormat.h"
#include "eIndexFormat.h"
#include "eVertexSemantic.h"
#include "eFileLocation.h"
#include "ePackage.h"
#include "eGAPIType.h"
#include "eTargetPlatform.h"
#include "core/hardware/GpuInfo.h"
#include "core/utils/Macroses.h"
#include "platforms/eiOSEnums.h"
#include "core/enums/eWinResize.h"
#include "core/enums/eInitState.h"
#include "platforms/eLinuxEnums.h"
#include "platforms/eMacOSEnums.h"
#include "platforms/eMSWinEnums.h"
#include "platforms/eAndroidEnums.h"
#include "core/utils/ThrowWrappers.h"
#include "core/enums/eLogMessageType.h"
#include "core/enums/eWindowState.h"

namespace zzz::core
{
	class EnumToString
	{
	public:
		static constexpr std::string_view ToString(eResourceType type)
		{
			switch (type)
			{
			case eResourceType::Unknown:    return "Unknown";
			case eResourceType::Texture2D:  return "Texture2D";
			case eResourceType::Mesh:       return "Mesh";
			case eResourceType::Material:   return "Material";
			case eResourceType::Shader:     return "Shader";
			case eResourceType::AudioClip:  return "AudioClip";
			case eResourceType::Font:       return "Font";
			case eResourceType::Scene:      return "Scene";
			case eResourceType::Prefab:     return "Prefab";
			case eResourceType::BinaryData: return "BinaryData";
			}
			THROW_RUNTIME("Необработанный eResourceType");
		}

		static constexpr std::string_view ToString(ePixelFormat format)
		{
			switch (format)
			{
			case ePixelFormat::Unknown:           return "Unknown";
			case ePixelFormat::R8_UNORM:          return "R8_UNORM";
			case ePixelFormat::RGBA8_UNORM:       return "RGBA8_UNORM";
			case ePixelFormat::RGBA8_SRGB:        return "RGBA8_SRGB";
			case ePixelFormat::BGRA8_UNORM:       return "BGRA8_UNORM";
			case ePixelFormat::BGRA8_SRGB:        return "BGRA8_SRGB";
			case ePixelFormat::RGBA16_FLOAT:      return "RGBA16_FLOAT";
			case ePixelFormat::R32_FLOAT:         return "R32_FLOAT";
			case ePixelFormat::D32_FLOAT:         return "D32_FLOAT";
			case ePixelFormat::D24_UNORM_S8_UINT: return "D24_UNORM_S8_UINT";
			case ePixelFormat::D32_FLOAT_S8_UINT: return "D32_FLOAT_S8_UINT";
			case ePixelFormat::BC1_UNORM:         return "BC1_UNORM";
			case ePixelFormat::BC3_UNORM:         return "BC3_UNORM";
			case ePixelFormat::BC7_UNORM:         return "BC7_UNORM";
			case ePixelFormat::ASTC_4x4_UNORM:    return "ASTC_4x4_UNORM";
			case ePixelFormat::ETC2_RGBA8_UNORM:  return "ETC2_RGBA8_UNORM";
			}
			THROW_RUNTIME("Необработанный ePixelFormat");
		}

		static constexpr std::string_view ToString(eIndexFormat format)
		{
			switch (format)
			{
			case eIndexFormat::UInt16: return "UInt16";
			case eIndexFormat::UInt32: return "UInt32";
			}
			THROW_RUNTIME("Необработанный eIndexFormat");
		}

		static constexpr std::string_view ToString(eVertexSemantic semantic)
		{
			switch (semantic)
			{
			case eVertexSemantic::Position:     return "Position";
			case eVertexSemantic::Normal:       return "Normal";
			case eVertexSemantic::TexCoord:     return "TexCoord";
			case eVertexSemantic::Color:        return "Color";
			case eVertexSemantic::Tangent:      return "Tangent";
			case eVertexSemantic::Bitangent:    return "Bitangent";
			case eVertexSemantic::BlendWeight:  return "BlendWeight";
			case eVertexSemantic::BlendIndices: return "BlendIndices";
			case eVertexSemantic::Count:        return "Count";
			}
			THROW_RUNTIME("Необработанный eVertexSemantic");
		}

		static constexpr std::string_view ToString(eFileLocation location)
		{
			switch (location)
			{
			case eFileLocation::App:   return "App";
			case eFileLocation::User:  return "User";
			case eFileLocation::Saves: return "Saves";
			case eFileLocation::Cache: return "Cache";
			case eFileLocation::Logs:  return "Logs";
			}
			THROW_RUNTIME("Необработанный eFileLocation");
		}

		static constexpr std::string_view ToString(eWindowState state)
		{
			switch (state)
			{
			case eWindowState::Normal:               return "Normal";
			case eWindowState::Maximized:            return "Maximized";
			case eWindowState::BorderlessFullscreen: return "BorderlessFullscreen";
			case eWindowState::ExclusiveFullscreen:  return "ExclusiveFullscreen";
			case eWindowState::Minimized:            return "Minimized";
			case eWindowState::Closed:               return "Closed";
			}
			THROW_RUNTIME("Необработанный eWindowState");
		}
		static constexpr std::string_view ToString(eInitState type)
		{
			switch (type)
			{
			case eInitState::NotInitialized: return "NotInitialized";
			case eInitState::Initializing:   return "Initializing";
			case eInitState::Initialized:    return "Initialized";
			case eInitState::Running:        return "Running";
			case eInitState::Destroying:     return "Destroying";
			}
			THROW_RUNTIME("Необработанный eInitState");
		}

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

		static constexpr std::string_view ToString(eGAPIType type)
		{
			switch (type)
			{
			case eGAPIType::DirectX12: return "DirectX12";
			case eGAPIType::Vulkan:    return "Vulkan";
			case eGAPIType::Metal:     return "Metal";
			}
			THROW_RUNTIME("Необработанный eGAPIType");
		}

		static constexpr std::string_view ToString(eGPUType type)
		{
			switch (type)
			{
			case eGPUType::Discrete:    return "Discrete";
			case eGPUType::Integrated:  return "Integrated";
			case eGPUType::CpuSoftware: return "CpuSoftware";
			case eGPUType::Unknown:     return "Unknown";
			}
			THROW_RUNTIME("Необработанный eGPUType");
		}

		static constexpr std::string_view ToString(ePackage type)
		{
			switch (type)
			{
			case ePackage::ProjectManifest: return "ProjectManifest";
			case ePackage::Scene:           return "Scene";
			case ePackage::PrimaryView:     return "PrimaryView";
			case ePackage::ChildView:       return "ChildView";
			case ePackage::IndependentView: return "IndependentView";
			case ePackage::Prefab:          return "Prefab";
			case ePackage::BinaryAsset:     return "BinaryAsset";
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
