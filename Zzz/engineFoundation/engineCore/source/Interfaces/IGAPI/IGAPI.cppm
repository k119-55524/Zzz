
export module IGAPI;

import Result;
import Size2D;
import IAppWin;
import GAPIConfig;
import StrConvert;
import IGPUUpload;
import GPUUploadCallbacks;
import IDeviceCapabilities;

using namespace zzz::core;
using namespace std::literals::string_view_literals;

export namespace zzz
{
	enum class eGAPIType : uint8_t
	{
		DirectX,
		Vulkan,
		Metal
	};

	export class IGAPI
	{
		Z_NO_CREATE_COPY(IGAPI);

	public:
		explicit IGAPI(const std::shared_ptr<GAPIConfig> config, eGAPIType type);
		virtual ~IGAPI() = default;

		[[nodiscard]] eInitState GetInitState() const noexcept { return initState; }

		[[nodiscard]] inline constexpr eGAPIType GetGAPIType() const noexcept { return gapiType; }
		[[nodiscard]] inline constexpr std::wstring_view GetAPIName(eGAPIType gapiType) const noexcept {
			using namespace std::literals;
			constexpr std::array names{
				L"DirectX 12"sv,
				L"Vulkan"sv,
				L"Metal"sv
			};

			return static_cast<size_t>(gapiType) < names.size() ?
				names[static_cast<size_t>(gapiType)] : L"Unknown"sv;
		}
		[[nodiscard]] inline constexpr std::wstring_view GetAPIName() const noexcept { return GetAPIName(gapiType); }
		inline const IDeviceCapabilities& GetGapiSupportChecker() const noexcept { return *m_CheckGapiSupport; }

		[[nodiscard]] virtual Result<> Initialize();
		virtual void SubmitCommandLists() = 0;
		inline void AddTransferResource(FillCallback OnFill, PreparedCallback OnPrepared, CompleteCallback OnComplete) { m_CPUtoGPUDataTransfer->AddTransferResource(OnFill, OnPrepared, OnComplete); };
		inline bool HasResourcesToUpload() { return m_CPUtoGPUDataTransfer->HasResourcesToUpload(); };
		inline void TranferResourceToGPU() { m_CPUtoGPUDataTransfer->TransferResourceToGPU(); };

		virtual void BeginRender() = 0;
		virtual void EndRender() = 0;

		void LogGPUDebugMessage(const std::wstring& message);

	protected:
		[[nodiscard]] virtual Result<> Init() = 0;
		virtual void WaitForGpu() = 0;

		const std::shared_ptr<GAPIConfig> m_Config;
		eGAPIType gapiType;
		eInitState initState;
		std::unique_ptr<IGPUUpload> m_CPUtoGPUDataTransfer;
		std::unique_ptr<IDeviceCapabilities> m_CheckGapiSupport;
		zU32 m_frameIndexRender;
		zU32 m_frameIndexUpdate;
	};

	IGAPI::IGAPI(const std::shared_ptr<GAPIConfig> config, eGAPIType type) :
		m_Config(config),
		gapiType{ type },
		initState{ eInitState::InitNot },
		m_frameIndexRender{ 0 },
		m_frameIndexUpdate{ 1 }
	{
		ensure(config, "GAPIConfig cannot be null.");
	}

	Result<> IGAPI::Initialize()
	{
		if (initState != eInitState::InitNot)
			return Unexpected(eResult::failure, L"GAPI is already initialized or in an invalid state.");

		return Init().and_then([&]() { initState = eInitState::InitOK; });
	}

	inline void IGAPI::LogGPUDebugMessage(const std::wstring& message)
	{
		DebugOutput(message);
	}
}
