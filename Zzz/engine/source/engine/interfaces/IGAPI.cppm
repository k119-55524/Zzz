#include "pch.h"
export module IGAPI;

import result;
import zSize2D;
import IAppWin;

using namespace std::literals::string_view_literals;

namespace zzz
{
	class zView;
	class engine;
	class surfaceAppMSWin_DirectX;
}

export namespace zzz::platforms
{
	enum class eGAPIType : uint8_t
	{
		DirectX12,
		OpenGL,
		Vulkan,
		Metal
	};

	export class IGAPI
	{
	public:
		IGAPI() =							delete;
		IGAPI(const IGAPI&) =				delete;
		IGAPI& operator=(const IGAPI&) =	delete;
		IGAPI(IGAPI&&) =					delete;
		IGAPI& operator=(IGAPI&&) =			delete;

		explicit IGAPI(eGAPIType type) noexcept
			: gapiType{ type },
			initState{ eInitState::eInitNot },
			m_supportsRayTracing{ false },
			m_supportsVariableRateShading{ false },
			m_supportsMeshShaders{ false },
			m_supportsSamplerFeedback{ false }
		{}

		virtual ~IGAPI() = default;

		[[nodiscard]] eInitState GetInitState() const noexcept { return initState; }


		// Геттеры возможностей
		[[nodiscard]] constexpr bool SupportsRayTracing() const noexcept { return m_supportsRayTracing; }
		[[nodiscard]] constexpr bool SupportsVariableRateShading() const noexcept { return m_supportsVariableRateShading; }
		[[nodiscard]] constexpr bool SupportsMeshShaders() const noexcept { return m_supportsMeshShaders; }
		[[nodiscard]] constexpr bool SupportsSamplerFeedback() const noexcept { return m_supportsSamplerFeedback; }
		constexpr std::wstring_view GetAPIName(eGAPIType gapiType) noexcept {
			using namespace std::literals;
			constexpr std::array names{
				L"DirectX 12"sv,
				L"OpenGL"sv,
				L"Vulkan"sv,
				L"Metal"sv
			};

			return static_cast<size_t>(gapiType) < names.size() ?
				names[static_cast<size_t>(gapiType)] : L"Unknown"sv;
		}

	protected:
		[[nodiscard]] virtual result<> Initialize(std::shared_ptr<IAppWin> appWin) = 0;
		friend class zzz::engine;
		friend class zzz::zView;

		eGAPIType gapiType;
		eInitState initState;

		//virtual void OnUpdate() = 0;
		virtual void WaitForPreviousFrame() {};
		friend class zzz::surfaceAppMSWin_DirectX;

		//virtual void OnRender() = 0;
		//virtual void OnResize(const zSize2D<>& size) = 0;

		// Возможности GAPI
		bool m_supportsRayTracing;			// DXR
		bool m_supportsVariableRateShading;	// VRS
		bool m_supportsMeshShaders;			// Mesh Shaders  
		bool m_supportsSamplerFeedback;		// Sampler Feedback
	};
}
