#include "pch.h"
export module DXAPI;

using namespace zzz::platforms::directx;

#if defined(_WIN64)
import IGAPI;
import result;
import StrConvert;
import RootSignature;
import AppWindowMsWin;
import CommandWrapperDX;
import CheckDirectXSupport;
import ThreadSafeSwapBuffer;
import CPUtoGPUDataTransferDX;

using namespace zzz;
using namespace zzz::templates;

export namespace zzz::platforms::directx
{
	// Создана только для обхода ограничений VisualStudio при передаче параметров
	template <class T>
	constexpr inline auto& keep(T&& x) noexcept
	{
		return x;
	}

	export class DXAPI final : public IGAPI
	{
	public:
		explicit DXAPI();
		DXAPI(DXAPI&) = delete;
		DXAPI(DXAPI&&) = delete;

		DXAPI& operator=(const DXAPI&) = delete;
		DXAPI& operator=(DXAPI&&) = delete;

		virtual ~DXAPI() override;

		inline const ComPtr<ID3D12Device> GetDevice() const noexcept { return m_device; };
		inline const ComPtr<ID3D12CommandQueue> GetCommandQueue() const noexcept { return m_commandQueue; };
		inline const ComPtr<IDXGIFactory7> GetFactory() const noexcept { return m_factory; };
		inline const ComPtr<ID3D12GraphicsCommandList>& GetCommandListUpdate() const noexcept { return m_commandWrapper[m_frameIndexUpdate]->GetCommandList(); };
		inline const ComPtr<ID3D12GraphicsCommandList>& GetCommandListRender() const noexcept { return m_commandWrapper[m_frameIndexRender]->GetCommandList(); };
		inline ComPtr<ID3D12RootSignature> GetRootSignature() const noexcept { return m_rootSignature.Get(); }

		void CommandRenderReset() noexcept;
		[[nodiscard]] result<> CommandRenderReinitialize();
		void EndPreparedTransfers();
		void SubmitCommandLists() override;

		void BeginRender() override;
		void EndRender() override;

	protected:
		[[nodiscard]] result<> Init() override;
		void WaitForGpu() override;

	private:
		static constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT;

		ThreadSafeSwapBuffer<std::shared_ptr<sInTransfersCallbacks>> m_PreparedTransfers;

		UINT64 m_fenceValue;
		unique_handle m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;
		UINT m_swapChainFlags;

		RootSignature m_rootSignature;
		D3D_FEATURE_LEVEL m_featureLevel;

		ComPtr<IDXGIFactory7> m_factory;
		ComPtr<IDXGIAdapter3> m_adapter3;
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		std::shared_ptr<CommandWrapperDX> m_commandWrapper[BACK_BUFFER_COUNT];

		result<> InitializeDevice();
		result<> InitializeFence();
		void EnableDebugLayer(UINT& dxgiFactoryFlags);
		result<> CreateFactory(UINT dxgiFactoryFlags, ComPtr<IDXGIFactory7>& outFactory);
		result<> CreateDevice(ComPtr<IDXGIAdapter1> adapter, ComPtr<ID3D12Device>& outDevice, D3D_FEATURE_LEVEL& outFeatureLevel);
		result<> CreateCommandQueue(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue>& outQueue);
		result<> GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);

		void BeginPreparedTransfers();
	};

	DXAPI::DXAPI() :
		IGAPI(eGAPIType::DirectX),
		m_fenceValue{ 0 },
		m_swapChainFlags{ 0 },
		m_featureLevel{ D3D_FEATURE_LEVEL_12_0 },
		m_PreparedTransfers{ 100 }
	{
	}

	DXAPI::~DXAPI()
	{
		WaitForGpu();
	}

	void DXAPI::CommandRenderReset() noexcept
	{
		for (int i = 0; i < BACK_BUFFER_COUNT; i++)
			m_commandWrapper[i]->Reset();
	}

	result<> DXAPI::CommandRenderReinitialize()
	{
		result<> res;
		for (int i = 0; i < BACK_BUFFER_COUNT; i++)
		{
			res = m_commandWrapper[i]->Reinitialize(m_device);
			if (!res)
				break;
		}

		return res;
	}

#pragma region Initialize
	result<> DXAPI::Init()
	{
		result<> res = InitializeDevice()
			.and_then([&]() { m_CPUtoGPUDataTransfer = safe_make_unique<CPUtoGPUDataTransferDX>(m_device, m_PreparedTransfers); })
			.and_then([&]() { return  InitializeFence(); });

		return res;
	}

	result<> DXAPI::InitializeDevice()
	{
		UINT dxgiFactoryFlags = 0;
		EnableDebugLayer(dxgiFactoryFlags);

		ComPtr<IDXGIFactory7> factory;
		ComPtr<IDXGIAdapter1> adapter;
		auto res = CreateFactory(dxgiFactoryFlags, factory)
			.and_then([&]() { m_factory = factory; })
			.and_then([&]() { return GetAdapter(factory.Get(), &adapter); });
		if (!res)
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeDevice()]. -> {}", res.error().getMessage()));

		HRESULT hr = adapter.As(&m_adapter3);
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeDevice()]. Failed to query IDXGIAdapter3. HRESULT = 0x{:08X}", hr));

		res = CreateDevice(adapter, m_device, m_featureLevel)
			.and_then([&]() { return CreateCommandQueue(m_device, m_commandQueue); });
		if (!res)
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeDevice()]. -> {}", res.error().getMessage()));

		for (int index = 0; index < BACK_BUFFER_COUNT; index++)
			m_commandWrapper[index] = safe_make_shared<CommandWrapperDX>(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

		return {};
	}

	void DXAPI::EnableDebugLayer(UINT& dxgiFactoryFlags)
	{
#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debugController;
		if (S_OK == D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			DebugOutput(L">>>>> [DXAPI::EnableDebugLayer()]. DirectX debug layer enabled.");
		}
#endif
	}

	result<> DXAPI::CreateFactory(UINT dxgiFactoryFlags, ComPtr<IDXGIFactory7>& outFactory)
	{
		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&outFactory));
		if (FAILED(hr))
		{
			ComPtr<IDXGIFactory4> factory4;
			hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory4));
			if (FAILED(hr))
				return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeDevice()]. Failed to create DXGI Factory");

			hr = factory4.As(&outFactory);
			if (FAILED(hr))
				return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeDevice()]. Failed to query IDXGIFactory7");
		}

		return {};
	}

	result<> DXAPI::CreateDevice(ComPtr<IDXGIAdapter1> adapter, ComPtr<ID3D12Device>& outDevice, D3D_FEATURE_LEVEL& outFeatureLevel)
	{
		static constexpr D3D_FEATURE_LEVEL levels[] =
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0
		};

		HRESULT hr = E_FAIL;
		for (auto level : levels)
		{
			hr = D3D12CreateDevice(adapter.Get(), level, IID_PPV_ARGS(&outDevice));
			if (SUCCEEDED(hr))
			{
				outFeatureLevel = level;
				break;
			}
		}

		if (FAILED(hr))
			return Unexpected(eResult::failure,
				std::format(L">>>>> [DXAPI::InitializeDevice()]. Failed to create D3D12 device. HRESULT = 0x{:08X}", hr));

		SET_RESOURCE_DEBUG_NAME(outDevice, L"Main ID3D12Device");

#ifdef _DEBUG
		std::wstring levelName = (outFeatureLevel == D3D_FEATURE_LEVEL_12_2) ? L"12.2 (DirectX 12 Ultimate)" :
			(outFeatureLevel == D3D_FEATURE_LEVEL_12_1) ? L"12.1" :
			(outFeatureLevel == D3D_FEATURE_LEVEL_12_0) ? L"12.0" : L"Unknown";
		DebugOutput(std::format(
			L">>>>> [DXAPI::InitializePipeline()]. Created D3D12 device with feature level: {}",
			levelName).c_str());
#endif

		m_CheckGapiSupport = safe_make_unique<CheckDirectXSupport>(outDevice);

		return {};
	}

	result<> DXAPI::CreateCommandQueue(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue>& outQueue)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc{};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&outQueue));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeDevice()]. Failed to create Command Queue. HRESULT = 0x{:08X}", hr));

		return {};
	}

	result<> DXAPI::GetAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
	{
		if (!pFactory || !ppAdapter)
			return Unexpected(eResult::invalid_argument);

		*ppAdapter = nullptr;

		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;

		auto trySelectAdapter = [&](IDXGIAdapter1* candidate) -> bool
			{
				DXGI_ADAPTER_DESC1 desc{};
				candidate->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					return false;

				if (SUCCEEDED(D3D12CreateDevice(candidate,
					D3D_FEATURE_LEVEL_12_0,
					__uuidof(ID3D12Device),
					nullptr)))
				{
					DebugOutput(std::format( L">>>>> [DXAPI::GetAdapter()] Selected adapter: {} VRAM: {} MB", desc.Description, desc.DedicatedVideoMemory / (1024 * 1024)).c_str());

					* ppAdapter = candidate;
					(*ppAdapter)->AddRef(); // так как мы не Detach'им
					return true;
				}
				return false;
			};

		// Через IDXGIFactory6 с приоритетом дискретных GPU
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (UINT adapterIndex = 0;
				SUCCEEDED(factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
					IID_PPV_ARGS(&adapter)));
					++adapterIndex)
			{
				if (trySelectAdapter(adapter.Get()))
					return {};
			}
		}

		// Обычное перечисление (в случае если Factory6 нет или не найдено подходящих)
		for (UINT adapterIndex = 0;
			SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter));
			++adapterIndex)
		{
			if (trySelectAdapter(adapter.Get()))
				return {};
		}

		return Unexpected(eResult::failure);
	}

	result<> DXAPI::InitializeFence()
	{
		// Защита от повторной инициализации
		if (m_fence || m_fenceEvent)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeAssets()]. Already initialized.");

		// Инициализация root signature
		//if (auto res = m_rootSignature.Initialize(m_device); !res)
		//	return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializeAssets()]. -> " + res.error().getMessage());

		// Создание объектов синхронизации
		{
			ComPtr<ID3D12Fence> fence;
			HRESULT hr = m_device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence) );
			if (FAILED(hr))
				return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeAssets()]. CreateFence failed. HRESULT = 0x{:08X}", hr));

			m_fence = fence;

			// RAII для события
			unique_handle fenceEvent{ CreateEvent(nullptr, FALSE, FALSE, nullptr) };
			if (!fenceEvent)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
				return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializeAssets()]. CreateEvent failed. HRESULT = 0x{:08X}", hr));
			}

			m_fenceEvent = fenceEvent.release();
		}

		return {};
	}
#pragma endregion Initialize

#pragma region Rendring
	void DXAPI::BeginRender()
	{
		auto commandAllocator = m_commandWrapper[m_frameIndexUpdate]->GetCommandAllocator();
		auto commandList = m_commandWrapper[m_frameIndexUpdate]->GetCommandList();

		// Сбрасываем аллокатор и командный список
		ensure(S_OK == commandAllocator->Reset());
		ensure(S_OK == commandList->Reset(commandAllocator.Get(), nullptr));

		// Устанавливаем состояние ресурсов как готовых к рендрингу после копирования в память GPU
		BeginPreparedTransfers();
	}

	void DXAPI::EndRender()
	{
		// Закрываем командный список
		auto commandListUpdate = m_commandWrapper[m_frameIndexUpdate]->GetCommandList();
		ensure(S_OK == commandListUpdate->Close());

		m_frameIndexRender = (m_frameIndexRender + 1) % BACK_BUFFER_COUNT;
		m_frameIndexUpdate = (m_frameIndexRender + 1) % BACK_BUFFER_COUNT;

		// Устанавливаем состояние ресурсов как готовых к рендрингу после копирования в память GPU
		EndPreparedTransfers();

		WaitForGpu();
	}

	// Устанавливаем состояние ресурса готовое к рендрингу после копирования в память GPU
	void DXAPI::BeginPreparedTransfers()
	{
		auto commandList = m_commandWrapper[m_frameIndexUpdate]->GetCommandList();

		m_PreparedTransfers.ForEach([&](std::shared_ptr<sInTransfersCallbacks> callback)
			{
				if (callback->isCorrect)
					callback->preparedCallback(commandList);
			});
	}

	void DXAPI::EndPreparedTransfers()
	{
		m_PreparedTransfers.ForEach([&](std::shared_ptr<sInTransfersCallbacks> callback)
			{
				if (callback->isCorrect)
					callback->completeCallback(callback->isCorrect);
			});

		m_PreparedTransfers.SwapAndReset();
	}

	void DXAPI::SubmitCommandLists()
	{
		//DebugOutput(std::format(L">>>>> [DXAPI::SubmitCommandLists()]. m_frameIndexRender: {}.", m_frameIndexRender));

		ID3D12CommandList* ppCommandLists[] = { m_commandWrapper[m_frameIndexRender]->GetCommandList().Get()};
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	void DXAPI::WaitForGpu()
	{
		const UINT64 fence = m_fenceValue;
		ensure(S_OK == m_commandQueue->Signal(m_fence.Get(), fence));

		m_fenceValue++;
		if (m_fence->GetCompletedValue() < fence)
		{
			ensure(S_OK == m_fence->SetEventOnCompletion(fence, m_fenceEvent.handle));

			WaitForSingleObject(m_fenceEvent.handle, INFINITE);
		}
	}
#pragma endregion Rendring
}
#endif // _WIN64