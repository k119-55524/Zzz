
export module IGAPI;

import Result;
import Size2D;
import IAppWin;
import StrConvert;
import ICheckGapiSupport;
import CPUtoGPUDataTransfer;
import ICPUtoGPUDataTransfer;

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
		explicit IGAPI(eGAPIType type);
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
		inline const ICheckGapiSupport& GetGapiSupportChecker() const noexcept { return *m_CheckGapiSupport; }

		[[nodiscard]] virtual Result<> Initialize();
		virtual void SubmitCommandLists() = 0;
		inline void AddTransferResource(FillCallback fillCallback, PreparedCallback preparedCallback, CompleteCallback completeCallback)
		{
			m_CPUtoGPUDataTransfer->AddTransferResource(fillCallback, preparedCallback, completeCallback);
		};
		inline bool HasResourcesToUpload() { return m_CPUtoGPUDataTransfer->HasResourcesToUpload(); };
		inline void TranferResourceToGPU() { m_CPUtoGPUDataTransfer->TransferResourceToGPU(); };

		virtual void BeginRender() = 0;
		virtual void EndRender() = 0;

	protected:
		[[nodiscard]] virtual Result<> Init() = 0;
		virtual void WaitForGpu() = 0;

		eGAPIType gapiType;
		eInitState initState;
		std::unique_ptr<ICPUtoGPUDataTransfer> m_CPUtoGPUDataTransfer;
		std::unique_ptr<ICheckGapiSupport> m_CheckGapiSupport;
		zU32 m_frameIndexRender;
		zU32 m_frameIndexUpdate;
	};

	IGAPI::IGAPI(eGAPIType type) :
		gapiType{ type },
		initState{ eInitState::InitNot },
		m_frameIndexRender{ 0 },
		m_frameIndexUpdate{ 1 }
	{
	}

	Result<> IGAPI::Initialize()
	{
		if (initState != eInitState::InitNot)
			return Unexpected(eResult::failure, L"GAPI is already initialized or in an invalid state.");

		return Init().and_then([&]() { initState = eInitState::InitOK; });
	}
}
