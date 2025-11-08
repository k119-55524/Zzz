#include "pch.h"
export module IGAPI;

import result;
import size2D;
import IAppWin;
import StrConvert;
import ICheckGapiSupport;
import ICPUtoGPUDataTransfer;
import BaseCPUtoGPUDataTransfer;

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
				L"OpenGL"sv,
				L"Vulkan"sv,
				L"Metal"sv
			};

			return static_cast<size_t>(gapiType) < names.size() ?
				names[static_cast<size_t>(gapiType)] : L"Unknown"sv;
		}
		[[nodiscard]] inline constexpr std::wstring_view GetAPIName() const noexcept { return GetAPIName(gapiType); }

		// Геттеры возможностей
		[[nodiscard]] inline bool SupportsRayTracing() const noexcept { return m_CheckGapiSupport->SupportsRayTracing(); }
		[[nodiscard]] inline bool SupportsVariableRateShading() const noexcept { return m_CheckGapiSupport->SupportsVariableRateShading(); }
		[[nodiscard]] inline bool SupportsMeshShaders() const noexcept { return m_CheckGapiSupport->SupportsMeshShaders(); }
		[[nodiscard]] inline bool SupportsSamplerFeedback() const noexcept { return m_CheckGapiSupport->SupportsSamplerFeedback(); }
		[[nodiscard]] inline bool SupportsCopyQueue() const noexcept { return m_CheckGapiSupport->SupportsCopyQueue(); }
		[[nodiscard]] inline bool SupportsDedicatedDMA() const noexcept { return m_CheckGapiSupport->SupportsDedicatedDMA(); }
		[[nodiscard]] inline std::wstring GetHighestShaderModelAsString(eShaderType eShaderType) const { return m_CheckGapiSupport->GetHighestShaderModelAsString(eShaderType); }

		[[nodiscard]] virtual result<> Initialize();
		virtual void SubmitCommandLists() = 0;
		inline void AddTransferResource(FillCallback fillCallback, PreparedCallback preparedCallback, CompleteCallback completeCallback)
		{
			m_CPUtoGPUDataTransfer->AddTransferResource(fillCallback, preparedCallback, completeCallback);
		};
		inline bool HasResourcesToUpload() { return m_CPUtoGPUDataTransfer->HasResourcesToUpload(); };
		inline void TranferResourceToGPU() { m_CPUtoGPUDataTransfer->TransferResourceToGPU(); };

		virtual void BeginRender() = 0;
		virtual void EndRender() = 0;

#if defined(ZRENDER_API_D3D12)
		virtual const ComPtr<ID3D12Device> GetDevice() const noexcept = 0;
		virtual const ComPtr<ID3D12CommandQueue> GetCommandQueue() const noexcept = 0;
		virtual const ComPtr<IDXGIFactory7> GetFactory() const noexcept = 0;
		virtual const ComPtr<ID3D12GraphicsCommandList>& GetCommandListUpdate() const noexcept = 0;
		virtual const ComPtr<ID3D12GraphicsCommandList>& GetCommandListRender() const noexcept = 0;
		virtual ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept = 0;

		virtual void CommandRenderReset() noexcept = 0;
		virtual [[nodiscard]] result<> CommandRenderReinitialize() = 0;
		virtual void EndPreparedTransfers() = 0;
#elif defined(ZRENDER_API_VULKAN)
#elif defined(ZRENDER_API_METAL)
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	protected:
		[[nodiscard]] virtual result<> Init() = 0;
		virtual void WaitForGpu() = 0;

		eGAPIType gapiType;
		eInitState initState;
		std::unique_ptr<ICPUtoGPUDataTransfer> m_CPUtoGPUDataTransfer;
		std::unique_ptr<ICheckGapiSupport> m_CheckGapiSupport;
		zU32 m_frameIndexRender;
		zU32 m_frameIndexUpdate;

	private:
		std::mutex stateMutex;
	};

	IGAPI::IGAPI(eGAPIType type) :
		gapiType{ type },
		initState{ eInitState::eInitNot },
		m_frameIndexRender{ 0 },
		m_frameIndexUpdate{ 1 }
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
