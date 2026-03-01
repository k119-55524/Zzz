
export module SurfView_DX;

#if defined(ZRENDER_API_D3D12)

import Math;
import IPSO;
import IGAPI;
import DXAPI;
import Scene;
import PSO_DX;
import Result;
import Ensure;
import Size2D;
import Colors;
import Camera;
import IAppWin;
import Helpers;
import IMeshGPU;
import ISurfView;
import StrConvert;
import RenderQueue;
import DebugOutput;
import RenderVolume;
import ViewportDesc;
import AppWin_MSWin;
import EngineConstants;
import MeshGPU_DirectX;
import PrimitiveTopology;

using namespace zzz::math;
using namespace zzz::core;
using namespace zzz::colors;

namespace zzz::dx
{
	template<typename T>
	class UploadBuffer
	{
	public:
		UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) :
			mIsConstantBuffer(isConstantBuffer)
		{
			mElementByteSize = sizeof(T);
			if (isConstantBuffer)
				mElementByteSize = CalcBufferSize32(sizeof(T));

			CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount);
			HRESULT hr = device->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&mUploadBuffer));
			ensure(hr == S_OK, "Failed to create upload buffer resource.");

			hr = mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData));
			ensure(hr == S_OK, "Failed to map upload buffer resource.");

			// Нам не нужно вызывать unmap, пока мы не закончим работу с ресурсом. Однако мы не должны выполнять запись в
			// ресурс, пока он используется графическим процессором (поэтому необходимо использовать методы синхронизации).
		}

		UploadBuffer(const UploadBuffer& rhs) = delete;
		UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
		~UploadBuffer()
		{
			if (mUploadBuffer != nullptr)
				mUploadBuffer->Unmap(0, nullptr);

			mMappedData = nullptr;
		}

		ID3D12Resource* Resource()const
		{
			return mUploadBuffer.Get();
		}

		void CopyData(int elementIndex, const T& data)
		{
			memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
		BYTE* mMappedData = nullptr;

		UINT mElementByteSize = 0;
		bool mIsConstantBuffer = false;
	};

	struct GPU_LayerConstants
	{
		Matrix4x4 WorldViewProj;
		Matrix4x4 View;
		Matrix4x4 Proj;
		Matrix4x4 ViewProj;
		//Matrix4x4 CameraPos;
	};

	struct GPU_MaterialConstants
	{
		Vector4 BaseColor;
		float Roughness;
		float Metallic;
		float Padding[2]; // выравнивание до 16 байт
	};

	struct GPU_ObjectConstants
	{
		Matrix4x4 World;
		Matrix4x4 WorldViewProj;
	};

	export class SurfView_DX final : public ISurfView
	{
		Z_NO_COPY_MOVE(SurfView_DX);

	public:
		explicit SurfView_DX(
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);
		~SurfView_DX() override = default;

		[[nodiscard]] Result<> Initialize() override;
		void PrepareFrame(const std::shared_ptr<RenderQueue> renderQueue) override;
		void RenderFrame() override;
		void OnResize(const Size2D<>& size) override;

	private:
		std::shared_ptr<AppWin_MSWin> m_iAppWin;
		std::shared_ptr<DXAPI> m_DXGAPI;

		static constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT; // При использовании трафаарета: DXGI_FORMAT_D24_UNORM_S8_UINT;
		static constexpr UINT SWAP_CHAIN_FLAGS = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		bool m_IgnoreResize;
		std::mutex m_FrameMutex;
		std::condition_variable m_FrameCV;
		bool m_FrameReady;

		std::unique_ptr<UploadBuffer<GPU_LayerConstants>> m_CB_Layer = nullptr;
		std::unique_ptr<UploadBuffer<GPU_MaterialConstants>> m_CB_Material = nullptr;
		std::unique_ptr<UploadBuffer<GPU_ObjectConstants>> m_CB_Object = nullptr;

		ComPtr<IDXGISwapChain3> m_swapChain;
		UINT m_RtvDescrSize;
		UINT m_DsvDescrSize;
		UINT m_CbvSrvDescrSize;
		ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_SrvHeap;
		ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
		ComPtr<ID3D12Resource> m_renderTargets[BACK_BUFFER_COUNT];
		ComPtr<ID3D12Resource> m_depthStencil[BACK_BUFFER_COUNT];

		void BuildConstantBuffers();
		[[nodiscard]] Result<> CreateRTVHeap();
		[[nodiscard]] Result<> CreateSRVHeap();
		[[nodiscard]] Result<> CreateDSVHeap();
		[[nodiscard]] Result<> CreateDS(const Size2D<>& size);
		[[nodiscard]] Result<> CreateRTV(ComPtr<ID3D12Device>& m_device);
		[[nodiscard]] Result<> InitializeSwapChain();

		void ResetRTVandDS();
		[[nodiscard]] Result<> RecreateRenderTargetsAndDepth();
	};

	SurfView_DX::SurfView_DX(
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI)
		: ISurfView(_iGAPI),
		m_IgnoreResize{ false },
		m_RtvDescrSize{ 0 },
		m_DsvDescrSize{ 0 },
		m_CbvSrvDescrSize{ 0 },
		m_FrameReady{ false }
	{
		ensure(_iAppWin, "App window cannot be null.");
		m_iAppWin = std::dynamic_pointer_cast<AppWin_MSWin>(_iAppWin);
		ensure(m_iAppWin, "App window must be of type AppWin_MSWin.");

		m_DXGAPI = std::dynamic_pointer_cast<DXAPI>(_iGAPI);
		ensure(m_DXGAPI, "Failed to cast IGAPI to DXAPI.");
	}

#pragma region Initialize
	Result<> SurfView_DX::Initialize()
	{
		auto m_device = m_DXGAPI->GetDevice();
		ensure(m_device, "Device cannot be null.");
		auto m_commandQueue = m_DXGAPI->GetCommandQueue();
		ensure(m_commandQueue, "Command queue cannot be null.");
		auto m_factory = m_DXGAPI->GetFactory();
		ensure(m_factory, "Factory cannot be null.");

		m_RtvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		auto winSize = m_iAppWin->GetWinSize();
		m_SurfSize.SetFrom(winSize);

		auto res = InitializeSwapChain();
		if (!res)
			return UNEXPECTED(eResult::failure, L"Failed to initialize swap chain.");

		HRESULT hr = m_factory->MakeWindowAssociation(m_iAppWin->GetHWND(), DXGI_MWA_NO_ALT_ENTER);
		if (FAILED(hr))
			return UNEXPECTED(eResult::failure, L"Failed to make window association");

		res = CreateRTVHeap()
			.and_then([&]() { return CreateSRVHeap(); })
			.and_then([&]() { BuildConstantBuffers(); })
			.and_then([&]() { return CreateDSVHeap(); })
			.and_then([&]() { return CreateRTV(m_device); });

		res = CreateDS(winSize);
		if (!res)
			return Unexpected(eResult::failure, res.error().getMessage());

		return {};
	}

	Result<> SurfView_DX::CreateRTV(ComPtr<ID3D12Device>& m_device)
	{
		ensure(m_device, "Device cannot be null.");
		ensure(m_RtvHeap, "RTV Heap cannot be null.");

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create RTV for each frame.
		for (UINT n = 0; n < BACK_BUFFER_COUNT; n++)
		{
			HRESULT hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
			if (FAILED(hr))
				return UNEXPECTED(eResult::failure, L"Failed to get back buffer. HRESULT = 0x{:08X}", hr);

			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_RtvDescrSize);

			SET_RESOURCE_DEBUG_NAME(m_renderTargets[n], std::format(L"RTV Frame {}", n).c_str());
		}

		return {};
	}

	Result<> SurfView_DX::InitializeSwapChain()
	{
		auto winSize = m_iAppWin->GetWinSize();
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = static_cast<UINT>(winSize.width);
		swapChainDesc.Height = static_cast<UINT>(winSize.height);
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = BACK_BUFFER_COUNT;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDesc.Flags = m_GAPI->IsCanDisableVSync() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ComPtr<IDXGISwapChain1> swapChain1;
		ensure(S_OK == m_DXGAPI->GetFactory()->CreateSwapChainForHwnd(
			m_DXGAPI->GetCommandQueue().Get(),
			m_iAppWin->GetHWND(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1));

		ensure(S_OK == swapChain1.As(&m_swapChain));

		return {};
	}

	Result<> SurfView_DX::CreateRTVHeap()
	{
		auto m_device = m_DXGAPI->GetDevice();
		ensure(m_device, "Device cannot be null.");

		// Describe and create a render target View (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = BACK_BUFFER_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap));
		if (FAILED(hr))
			return UNEXPECTED(eResult::failure, L"Failed to create RTV heap. HRESULT = 0x{:08X}", hr);

		SET_RESOURCE_DEBUG_NAME(m_RtvHeap, L"RTV Heap");

		return {};
	}

	Result<> SurfView_DX::CreateSRVHeap()
	{
		auto m_device = m_DXGAPI->GetDevice();
		ensure(m_device, "Device cannot be null.");

		// Describe and create a shader resource View (SRV) heap for the texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvHeap));
		if (FAILED(hr))
			return UNEXPECTED(eResult::failure, L"Failed to create SRV heap. HRESULT = 0x{:08X}", hr);

		SET_RESOURCE_DEBUG_NAME(m_SrvHeap, L"SRV Heap");

		return {};
	}

	void SurfView_DX::BuildConstantBuffers()
	{
		auto device = m_DXGAPI->GetDevice();
		m_CB_Layer		= safe_make_unique<UploadBuffer<GPU_LayerConstants>>(device.Get(), 1, true);	// b0 – Layer
		m_CB_Material	= safe_make_unique<UploadBuffer<GPU_MaterialConstants>>(device.Get(), 1, true);	// b1 – Material
		m_CB_Object		= safe_make_unique<UploadBuffer<GPU_ObjectConstants>>(device.Get(), 1, true);	// b2 – Object
	}

	Result<> SurfView_DX::CreateDSVHeap()
	{
		auto m_device = m_DXGAPI->GetDevice();
		ensure(m_device, "Device cannot be null.");

		const D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			.NumDescriptors = BACK_BUFFER_COUNT,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask = 0
		};

		HRESULT hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DsvHeap));
		if (FAILED(hr))
			return UNEXPECTED(eResult::failure, L"Failed to create DSV heap. HRESULT = 0x{:08X}", hr);

		SET_RESOURCE_DEBUG_NAME(m_DsvHeap, L"DSV Heap");

		return {};
	}

	Result<> SurfView_DX::CreateDS(const Size2D<>& size)
	{
		auto m_device = m_DXGAPI->GetDevice();
		ensure(m_device, "Device cannot be null.");

		// Очищаем старые ресурсы
		for (auto& ds : m_depthStencil)
			ds.Reset();

		// Описание вида глубины/трафарета
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DEPTH_FORMAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		// Значение по умолчанию для очистки глубины
		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DEPTH_FORMAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		// Свойства кучи для текстуры глубины
		const CD3DX12_HEAP_PROPERTIES depthStencilHeapProps(D3D12_HEAP_TYPE_DEFAULT);

		// Описание ресурса глубины
		const CD3DX12_RESOURCE_DESC depthStencilTextureDesc =
			CD3DX12_RESOURCE_DESC::Tex2D(
				DEPTH_FORMAT,
				static_cast<UINT>(size.width),
				static_cast<UINT>(size.height),
				1, // массив из одной текстуры
				0, // mip-уровни
				1, 0,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
			);

		// Создаём ресурсы глубины по количеству кадров
		for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
		{
			HRESULT hr = m_device->CreateCommittedResource(
				&depthStencilHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&depthStencilTextureDesc,
				D3D12_RESOURCE_STATE_COMMON,
				&depthOptimizedClearValue,
				IID_PPV_ARGS(&m_depthStencil[i]));
			
			if (S_OK != hr)
				return UNEXPECTED(eResult::failure, L"Failed to create depth stencil resource. HRESULT = 0x{:08X}", hr);

			SET_RESOURCE_DEBUG_NAME(m_depthStencil[i], std::format(L"DepthStencil_{}", i).c_str());

			// Создаём DSV
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
				m_DsvHeap->GetCPUDescriptorHandleForHeapStart(),
				i,
				m_DsvDescrSize);

			m_device->CreateDepthStencilView(
				m_depthStencil[i].Get(),
				&depthStencilDesc,
				dsvHandle);

			SET_RESOURCE_DEBUG_NAME(m_depthStencil[i], std::format(L"DSV Frame {}", i).c_str());
		}

		return {};
	}

	void SurfView_DX::ResetRTVandDS()
	{
		for (auto& rt : m_renderTargets)
			rt.Reset();

		for (auto& ds : m_depthStencil)
			ds.Reset();
	}

	Result<> SurfView_DX::RecreateRenderTargetsAndDepth()
	{
		auto m_device = m_DXGAPI->GetDevice();

		auto res = CreateRTV(m_device);
		if (!res)
			return UNEXPECTED(eResult::failure, L"Failed to create RTV. {}", res.error().getMessage());

		DXGI_SWAP_CHAIN_DESC desc{};
		HRESULT hr = m_swapChain->GetDesc(&desc);
		if (S_OK != hr)
			UNEXPECTED(eResult::failure, L"Failed to get swap chain description. HRESULT = 0x{:08X}", hr);

		Size2D<> size{ desc.BufferDesc.Width, desc.BufferDesc.Height };
		res = CreateDS(size);
		if (!res)
			UNEXPECTED(eResult::failure, L"Failed to create depth stencil View. {}", res.error().getMessage());

		return {};
	}
#pragma endregion Initialize

#pragma region Rendring
	void SurfView_DX::PrepareFrame(const std::shared_ptr<RenderQueue> renderQueue)
	{
		// Синхронизируемся с рендерингом чтобы frameIndex остался валидным
		zU64 frameIndex = (m_swapChain->GetCurrentBackBufferIndex() + 1) % BACK_BUFFER_COUNT;
		{
			std::lock_guard<std::mutex> lock(m_FrameMutex);
			m_FrameReady = true;
			m_FrameCV.notify_one(); // Уведомляем ОДИН ожидающий поток
		}

		auto commandList = m_DXGAPI->GetCommandListUpdate();
		ensure(commandList, "Command list cannot be null.");

		{
			commandList->SetGraphicsRootSignature(m_DXGAPI->GetRootSignature().Get());

			// Привязываем root-параметры
			commandList->SetGraphicsRootConstantBufferView(0, m_CB_Layer->Resource()->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(1, m_CB_Material->Resource()->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(2, m_CB_Object->Resource()->GetGPUVirtualAddress());
	
			// Пока не используем текстуры - закомментировано
			//commandList->SetGraphicsRootDescriptorTable(2, m_srvHeap->GetGPUDescriptorHandleForHeapStart()); // SRV таблица начинается с t0
			//ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvHeap.Get() };
			//commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		}

		commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));
		commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencil[frameIndex].Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE)));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(frameIndex), m_RtvDescrSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DsvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(frameIndex), m_DsvDescrSize);
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		// Выполняем рендринг очереди
		renderQueue->PrepareQueue(
			m_SurfSize,
			// Очистка поверхности
			[&](const eSurfClearType surfClearType, const Color& color, bool isClearDepth)
			{
				switch (surfClearType)
				{
				case eSurfClearType::Color:
					commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
					break;
				}

				if(isClearDepth)
					commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
			},
			// Установка viewport и scissor rect
			[&](const ViewportDesc& vp, const ScissorDesc& sc)
			{
				D3D12_VIEWPORT viewport = vp.ToD3D12();
				D3D12_RECT scissor = sc.ToD3D12();
				commandList->RSSetViewports(1, &viewport);
				commandList->RSSetScissorRects(1, &scissor);
			},
			// Установка глобальных констант
			[&](const Matrix4x4& viewProj)
			{
				GPU_LayerConstants objConstants;
				std::memcpy(&objConstants.ViewProj, &viewProj, sizeof(Matrix4x4));
				m_CB_Layer->CopyData(0, objConstants);
			},
			// Установка PSO(set material)
			[&](const std::shared_ptr<IPSO> pso)
			{
				std::shared_ptr<PSO_DX> psoDX = static_pointer_cast<PSO_DX>(pso);
				commandList->SetPipelineState(psoDX->GetPSO().Get());
			},
			// Установка топологии примитивов
			[&](const PrimitiveTopology& topo)
			{
				commandList->IASetPrimitiveTopology(topo.ToD3D12());
			},
			// Установка констант объекта
			[&](const Matrix4x4& worldViewProj)
			{
				GPU_ObjectConstants gpuObj{};
				std::memcpy(&gpuObj.WorldViewProj, &worldViewProj, sizeof(Matrix4x4));
				m_CB_Object->CopyData(0, gpuObj);
			},
			// Отрисовка меша с инексным буффером
			[&](const std::shared_ptr<IMeshGPU> mesh, size_t count)
			{
				std::shared_ptr<MeshGPU_DirectX> meshDX = static_pointer_cast<MeshGPU_DirectX>(mesh);
				commandList->IASetVertexBuffers(0, 1, meshDX->VertexBufferView());
				commandList->IASetIndexBuffer(meshDX->IndexBufferView());
				commandList->DrawIndexedInstanced(static_cast<UINT>(count), 1, 0, 0, 0);
			}
		);

		// Завершение подготовки (из EndRender, до закрытия командного списка)
		commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));
		commandList->ResourceBarrier(1, &keep(CD3DX12_RESOURCE_BARRIER::Transition( m_depthStencil[frameIndex].Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON)));
	}

	void SurfView_DX::RenderFrame()
	{
		// Ожидаем(синхронизируемся) получения валидного frameIndex в PrepareFrame
		{
			std::unique_lock<std::mutex> lock(m_FrameMutex);
			m_FrameCV.wait(lock, [this] { return m_FrameReady; }); // Ждем, пока фрейм будет готов
			m_FrameReady = false; // Сбрасываем флаг для следующего кадра
		}

		// Выполняем командный список
		m_GAPI->SubmitCommandLists();

		// Настраиваем параметры для Present
		BOOL fullscreen = FALSE;
		ensure(S_OK == m_swapChain->GetFullscreenState(&fullscreen, nullptr));

		UINT syncInterval = 0;
		UINT presentFlags = 0;

		if (fullscreen)
		{
			// В fullscreen tearing запрещён
			syncInterval = m_GAPI->IsVSyncEnabled() ? 1 : 0;
		}
		else
		{
			if (m_GAPI->IsVSyncEnabled())
			{
				syncInterval = 1;
			}
			else
			{
				syncInterval = 0;
				presentFlags = DXGI_PRESENT_ALLOW_TEARING;
			}
		}

		HRESULT hr = m_swapChain->Present(syncInterval, presentFlags);
		if (FAILED(hr))
		{
			std::string errMsg;
			switch (hr)
			{
			case DXGI_ERROR_DEVICE_HUNG:
				// Драйвер устройства перестал отвечать
				errMsg = "GPU device hung - driver issues";
				break;
			case DXGI_ERROR_DEVICE_REMOVED:
				// Устройство было физически удалено
				errMsg = "GPU device physically removed";
				break;
			case DXGI_ERROR_DEVICE_RESET:
				// Устройство было сброшено
				errMsg = "GPU device reset";
				break;
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
				// Внутренняя ошибка драйвера
				errMsg = "GPU driver internal error";
				break;
			case DXGI_ERROR_INVALID_CALL:
				// Неправильный вызов API
				errMsg = "Invalid API call";
				break;
			default:
				errMsg = std::format("Unknown device removed reason: {:#x}", hr);
				break;
			}

			throw_runtime_error(errMsg);
		}
	}
#pragma endregion Rendring

	void SurfView_DX::OnResize(const Size2D<>& size)
	{
		//DOut(std::format(L">>>>> [SurfView_DX::OnResize({}x{})]. b_IgnoreResize: {}.", size.width, size.height, b_IgnoreResize).c_str());
		if (m_GAPI->GetInitState() != eInitState::InitOK && !m_swapChain || m_IgnoreResize)
			return;

		if (size.width == 0 || size.height == 0)
		{
			DOut(L"Invalid size for resize. Width or height is zero.\n");
			return;
		}

		auto m_device = m_DXGAPI->GetDevice();
		ensure(m_device, "Device cannot be null.");
		ensure(m_swapChain, "Swap chain cannot be null.");

		DXGI_SWAP_CHAIN_DESC1 desc;
		HRESULT hr = m_swapChain->GetDesc1(&desc);
		if (SUCCEEDED(hr))
		{
			// Сравниваем с новым размером
			if (desc.Width == static_cast<UINT>(size.width) &&
				desc.Height == static_cast<UINT>(size.height))
			{
				DOut(std::format(L"No resize needed, dimensions are unchanged.", size.width, size.height).c_str());
				return; // Размеры не изменились
			}
		}

		m_DXGAPI->CommandRenderReset();
		ResetRTVandDS();

		BOOL fullscreen = FALSE;
		ensure(S_OK == m_swapChain->GetFullscreenState(&fullscreen, nullptr));
		if (fullscreen)
		{
			DXGI_MODE_DESC modeDesc = {};
			modeDesc.Width = static_cast<UINT>(size.width);
			modeDesc.Height = static_cast<UINT>(size.height);
			modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			modeDesc.RefreshRate.Numerator = 60;
			modeDesc.RefreshRate.Denominator = 1;

			hr = m_swapChain->ResizeTarget(&modeDesc);
		}
		else
		{
			hr = m_swapChain->ResizeBuffers(
				BACK_BUFFER_COUNT,
				static_cast<UINT>(size.width),
				static_cast<UINT>(size.height),
				DXGI_FORMAT_R8G8B8A8_UNORM,
				m_GAPI->IsCanDisableVSync() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);
		}
		if (S_OK != hr)
			throw_runtime_error(std::format("[SurfView_DX::OnResize({}x{})].", size.width, size.height));

		auto res = CreateRTVHeap()
			.and_then([&]() { return CreateRTV(m_device); })
			.and_then([&]() { return CreateDS(size); })
			.and_then([&]() { return m_DXGAPI->CommandRenderReinitialize(); })
			.or_else([&](const Unexpected& error) { throw_runtime_error(std::format("[SurfView_DX::OnResize({}x{})]. {}.", size.width, size.height, wstring_to_string(error.getMessage()))); });

		m_SurfSize = size;
	}

#pragma region Deprecated
	//[[nodiscard]] Result<> SurfView_DX::SetFullScreenState(bool fss)
	//{
	//	BOOL fullscreen = FALSE;
	//	HRESULT hr = m_swapChain->GetFullscreenState(&fullscreen, nullptr);
	//	if (FAILED(hr))
	//		return UNEXPECTED(L"Failed to get fullscreen state. HRESULT = 0x{:08X}\n", fss, hr);

	//	if (fss == static_cast<bool>(fullscreen))
	//	{
	//		DOut(L"Fullscreen state is already set.\n", fss);
	//		return {};
	//	}

	//	m_DXGAPI->CommandRenderReset();
	//	ResetRTVandDS();

	//	m_IgnoreResize = true;
	//	hr = m_swapChain->SetFullscreenState(fss, nullptr);
	//	if (FAILED(hr))
	//	{
	//		m_IgnoreResize = false;
	//		DOut(L"Failed to set {} in fullscreen state.HRESULT = 0x{:08X}\n", fss, hr);
	//		auto res = RecreateRenderTargetsAndDepth();
	//		if (!res)
	//			throw_runtime_error(std::format("#0 [SurfView_DX::SetFullScreen({})]. Failed to recreate render targets and depth stencil View. {}.", fss, wstring_to_string(res.error().getMessage())));

	//		return {};
	//	}
	//	m_IgnoreResize = false;

	//	DXGI_SWAP_CHAIN_DESC desc{};
	//	hr = m_swapChain->GetDesc(&desc);
	//	if (FAILED(hr))
	//	{
	//		DOut(L"[SurfView_DX::SetFullScreen({})] Failed to get swap chain description. HRESULT = 0x{:08X}\n", fss, hr);
	//		return {};
	//	}

	//	hr = m_swapChain->ResizeBuffers(
	//		desc.BufferCount,
	//		0, 0, // авто определение размеров
	//		desc.BufferDesc.Format,
	//		desc.Flags);
	//	if (FAILED(hr))
	//	{
	//		DOut(L"[SurfView_DX::SetFullScreen({})] Failed to resize buffers. HRESULT = 0x{:08X}\n", fss, hr);
	//		return {};
	//	}

	//	ComPtr<ID3D12Resource> backBuffer;
	//	ensure(S_OK == m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));

	//	auto bbDesc = backBuffer->GetDesc();

	//	m_SurfSize.Set(bbDesc.Width, bbDesc.Height);

	//	return RecreateRenderTargetsAndDepth()
	//		.and_then([&]() { return m_DXGAPI->CommandRenderReinitialize(); })
	//		.and_then([&]() { DOut(L"SetFullScreenState({}): succeeded.\n", fss); })
	//		.or_else([&](const UNEXPECTED& error) { throw_runtime_error(std::format("SetFullScreenState({}): Failed: {}.", fss, wstring_to_string(error.getMessage()))); });
	//}

	//[[nodiscard]] Result<bool> SurfView_DX::GetFullScreenState() const
	//{
	//	BOOL fullscreen = FALSE;
	//	HRESULT hr = m_swapChain->GetFullscreenState(&fullscreen, nullptr);
	//	if (FAILED(hr))
	//		return UNEXPECTED(L"Failed to get fullscreen state. HRESULT = 0x{:08X}\n", hr);

	//	return static_cast<bool>(fullscreen);
	//}
#pragma endregion
}
#endif // defined(ZRENDER_API_D3D12)