#include "pch.h"
export module IGAPI;

import result;
import size2D;
import IAppWin;
import strConvert;
import ICheckGapiSupport;
import ICPUtoGPUDataTransfer;

using namespace std::literals::string_view_literals;

export namespace zzz::platforms
{
	enum class eGAPIType : uint8_t
	{
		DirectX,
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

		explicit IGAPI(eGAPIType type);

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
		[[nodiscard]] inline bool SupportsRayTracing() const noexcept { return m_CheckGapiSupport->SupportsRayTracing(); }
		[[nodiscard]] inline bool SupportsVariableRateShading() const noexcept { return m_CheckGapiSupport->SupportsVariableRateShading(); }
		[[nodiscard]] inline bool SupportsMeshShaders() const noexcept { return m_CheckGapiSupport->SupportsMeshShaders(); }
		[[nodiscard]] inline bool SupportsSamplerFeedback() const noexcept { return m_CheckGapiSupport->SupportsSamplerFeedback(); }

		[[nodiscard]] virtual result<> Initialize();
		virtual void SubmitCommandLists(zU64 index) = 0;
		inline void AddTransferResource(CommandListFillCallback fillCallback, TransferCompleteCallback completeCallback) { m_CPUtoGPUDataTransfer->AddTransferResource(fillCallback, completeCallback); };
		inline bool HasResourcesToUpload() { return m_CPUtoGPUDataTransfer->HasResourcesToUpload(); };
		inline void TranferResourceToGPU() { m_CPUtoGPUDataTransfer->TransferResourceToGPU(); };
		virtual void WaitForGpu() = 0;

	protected:
		[[nodiscard]] virtual result<> Init() = 0;

		eGAPIType gapiType;
		eInitState initState;
		std::unique_ptr<ICPUtoGPUDataTransfer> m_CPUtoGPUDataTransfer;
		std::unique_ptr<ICheckGapiSupport> m_CheckGapiSupport; // Проверка возможностей GAPI

	private:
		std::mutex stateMutex;
	};

	IGAPI::IGAPI(eGAPIType type)
		: gapiType{ type },
		initState{ eInitState::eInitNot }
	{
	}

	result<> IGAPI::Initialize()
	{
		std::lock_guard<std::mutex> lock(stateMutex);

		if (initState != eInitState::eInitNot)
			return Unexpected(eResult::failure, L">>>>> [IGAPI::Initialize()]. GAPI is already initialized or in an invalid state.");

		auto res = Init()
			.and_then([&]() { initState = eInitState::eInitOK; })
			.or_else([&](const Unexpected& error) { throw_runtime_error(std::format(">>>>> [IGAPI::Initialize()]. {}.", wstring_to_string(error.getMessage()))); });

		return res;
	}
}
