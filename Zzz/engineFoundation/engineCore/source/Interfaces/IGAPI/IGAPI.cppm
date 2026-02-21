
export module IGAPI;

import Result;
import Size2D;
import Ensure;
import IAppWin;
import GAPIConfig;
import StrConvert;
import IGPUUpload;
import EngineConstants;
import GPUUploadCallbacks;
import IDeviceCapabilities;

using namespace zzz::core;
using namespace std::literals::string_view_literals;

namespace zzz
{
	export enum class eGAPIType : uint8_t
	{
		DirectX,
		Vulkan,
		Metal
	};

	export class IGAPI
	{
		Z_NO_CREATE_COPY(IGAPI);

	public:
		explicit IGAPI(const std::shared_ptr<GAPIConfig>& config, eGAPIType type);
		virtual ~IGAPI() = default;

		[[nodiscard]] eInitState GetInitState() const noexcept { return m_InitState; }

		[[nodiscard]] constexpr eGAPIType GetGAPIType() const noexcept { return m_GapiType; }
		[[nodiscard]] constexpr std::wstring_view GetAPIName(eGAPIType gapiType) const noexcept {
			using namespace std::literals;
			constexpr std::array names{
				L"DirectX 12"sv,
				L"Vulkan"sv,
				L"Metal"sv
			};

			return static_cast<size_t>(gapiType) < names.size() ?
				names[static_cast<size_t>(gapiType)] : L"Unknown"sv;
		}
		[[nodiscard]] constexpr std::wstring_view GetAPIName() const noexcept { return GetAPIName(m_GapiType); }
		[[nodiscard]] inline bool IsCanDisableVsync() const noexcept { return b_IsCanDisableVsync; }
		inline const std::shared_ptr<GAPIConfig>& GetConfig()  const noexcept { return m_Config; };
		[[nodiscard]] inline const IDeviceCapabilities& GetGapiSupportChecker() const noexcept { return *m_CheckGapiSupport; }

		[[nodiscard]] virtual Result<> Initialize();
		virtual void SubmitCommandLists() = 0;
		inline void AddTransferResource(FillCallback OnFill, PreparedCallback OnPrepared, CompleteCallback OnComplete) { m_CPUtoGPUDataTransfer->AddTransferResource(OnFill, OnPrepared, OnComplete); };
		inline bool HasResourcesToUpload() { return m_CPUtoGPUDataTransfer->HasResourcesToUpload(); };
		inline void TranferResourceToGPU() { m_CPUtoGPUDataTransfer->TransferResourceToGPU(); };

		inline zU32 GetIndexFrameRender() const noexcept { return m_IndexFrameRender; }

		virtual void BeginRender() = 0;
		virtual void EndRender();

		void LogGPUDebugMessage(const std::wstring& message);

	protected:
		[[nodiscard]] virtual Result<> Init() = 0;
		virtual void WaitForGpu() {};

		const std::shared_ptr<GAPIConfig> m_Config;
		eGAPIType m_GapiType;
		eInitState m_InitState;
		bool b_IsCanDisableVsync;

		std::unique_ptr<IGPUUpload> m_CPUtoGPUDataTransfer;
		std::unique_ptr<IDeviceCapabilities> m_CheckGapiSupport;
		zU32 m_IndexFrameRender;
		zU32 m_IndexFrameUpdate;
	};

	IGAPI::IGAPI(const std::shared_ptr<GAPIConfig>& config, eGAPIType type) :
		m_Config(config),
		m_GapiType{ type },
		b_IsCanDisableVsync{ false },
		m_InitState{ eInitState::InitNot },
		m_IndexFrameRender{ 0 },
		m_IndexFrameUpdate{ 1 }
	{
		ensure(config, "GAPIConfig cannot be null.");
	}

	Result<> IGAPI::Initialize()
	{
		if (m_InitState != eInitState::InitNot)
			return Unexpected(eResult::failure, L"GAPI is already initialized or in an invalid state.");

		return Init().and_then([&]() { m_InitState = eInitState::InitOK; });
	}

	inline void IGAPI::LogGPUDebugMessage(const std::wstring& message)
	{
		DebugOutput(message);
	}

	void IGAPI::EndRender()
	{
		m_IndexFrameRender = (m_IndexFrameRender + 1) % BACK_BUFFER_COUNT;
		m_IndexFrameUpdate = (m_IndexFrameRender + 1) % BACK_BUFFER_COUNT;
	}
}
