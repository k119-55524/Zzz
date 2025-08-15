#include "pch.h"
export module IGAPI;

import result;
import size2D;
import IAppWin;
import ICheckGapiSupport;

using namespace std::literals::string_view_literals;

namespace zzz
{
	class engine;
	class IAppWinSurface;
}

export namespace zzz::platforms
{
	enum class eGAPIType : uint8_t
	{
		DirectX,
		OpenGL,
		Vulkan,
		Metal
	};

	export class IGAPI
	{
		friend class zzz::engine;
		friend class zzz::IAppWinSurface;

	public:
		IGAPI() =							delete;
		IGAPI(const IGAPI&) =				delete;
		IGAPI& operator=(const IGAPI&) =	delete;
		IGAPI(IGAPI&&) =					delete;
		IGAPI& operator=(IGAPI&&) =			delete;

		explicit IGAPI(eGAPIType type) noexcept
			: gapiType{ type },
			initState{ eInitState::eInitNot }
		{}

		virtual ~IGAPI() = default;

		[[nodiscard]] eInitState GetInitState() const noexcept { return initState; }

		[[nodiscard]] inline constexpr eGAPIType GetGAPIType() const noexcept { return gapiType; }
		[[nodiscard]] inline constexpr std::wstring_view GetAPIName(eGAPIType gapiType) const noexcept {
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
		[[nodiscard]] inline constexpr std::wstring_view GetAPIName() const noexcept {
			return GetAPIName(gapiType);
		}

		// Геттеры возможностей
		[[nodiscard]] inline bool SupportsRayTracing() const noexcept { return checkGapiSupport->SupportsRayTracing(); }
		[[nodiscard]] inline bool SupportsVariableRateShading() const noexcept { return checkGapiSupport->SupportsVariableRateShading(); }
		[[nodiscard]] inline bool SupportsMeshShaders() const noexcept { return checkGapiSupport->SupportsMeshShaders(); }
		[[nodiscard]] inline bool SupportsSamplerFeedback() const noexcept { return checkGapiSupport->SupportsSamplerFeedback(); }

	protected:
		[[nodiscard]] virtual result<> Initialize() = 0;
		virtual void WaitForPreviousFrame() {};

		eGAPIType gapiType;
		eInitState initState;
		std::unique_ptr<ICheckGapiSupport> checkGapiSupport; // Проверка возможностей GAPI
	};
}
