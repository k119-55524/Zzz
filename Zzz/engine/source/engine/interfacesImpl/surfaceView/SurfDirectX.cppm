#include "pch.h"
#include <Headers/headerDX.h>
export module SurfDirectX;

#if defined(ZRENDER_API_D3D12)
import Math;
import IGAPI;
import DXAPI;
import Scene;
import result;
import size2D;
import Camera;
import IAppWin;
import helpers;
import Vector4;
import Settings;
import Matrix4x4;
import StrConvert;
import RenderArea;
import ISurfaceView;
import AppWindowMsWin;

using namespace zzz::math;
using namespace zzz::helpers;
using namespace zzz::platforms;
using namespace zzz::engineCore;
using namespace zzz::platforms::directx;

namespace zzz
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
			ensure(hr == S_OK, ">>>>> [UploadBuffer::UploadBuffer(...)]. Failed to create upload buffer resource.");

			hr = mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData));
			ensure(hr == S_OK, ">>>>> [UploadBuffer::UploadBuffer(...)]. Failed to map upload buffer resource.");

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

	struct ObjectConstants
	{
		Matrix4x4 WorldViewProj;
		//XMFLOAT4X4 _WorldViewProj	= Identity4x4();
		
		//XMFLOAT4X4 view				= Identity4x4();
		//XMFLOAT4X4 proj				= Identity4x4();
		//XMFLOAT4X4 viewProj			= Identity4x4();
		//XMFLOAT4X4 cameraPos		= Identity4x4();
	};

	export class SurfDirectX final : public ISurfaceView
	{
	public:
		SurfDirectX() = delete;
		SurfDirectX(const SurfDirectX&) = delete;
		SurfDirectX(SurfDirectX&&) = delete;
		SurfDirectX& operator=(const SurfDirectX&) = delete;
		SurfDirectX& operator=(SurfDirectX&&) = delete;

		explicit SurfDirectX(
			std::shared_ptr<Settings> _settings,
			std::shared_ptr<IAppWin> _iAppWin,
			std::shared_ptr<IGAPI> _iGAPI);

		[[nodiscard]] result<> Initialize() override;
		void PrepareFrame(std::shared_ptr<Scene> scene) override;
		void RenderFrame() override;
		void OnResize(const size2D<>& size) override;

		void SetFullScreen(bool fs) override;

	private:
		static constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT; // При использовании трафаарета: DXGI_FORMAT_D24_UNORM_S8_UINT;
		static constexpr UINT SWAP_CHAIN_FLAGS = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		bool b_IgnoreResize;
		bool m_tearingSupported;
		std::mutex m_frameMutex;
		std::condition_variable m_frameCV;
		bool m_frameReady;

		std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

		ComPtr<IDXGISwapChain3> m_swapChain;
		UINT m_RtvDescrSize;
		UINT m_DsvDescrSize;
		UINT m_CbvSrvDescrSize;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12Resource> m_renderTargets[BACK_BUFFER_COUNT];
		ComPtr<ID3D12Resource> m_depthStencil[BACK_BUFFER_COUNT];

		[[nodiscard]] result<> BuildConstantBuffers();
		[[nodiscard]] result<> CreateRTVHeap();
		[[nodiscard]] result<> CreateSRVHeap();
		[[nodiscard]] result<> CreateDSVHeap();
		[[nodiscard]] result<> CreateDS(const size2D<>& size);
		[[nodiscard]] result<> CreateRTV(ComPtr<ID3D12Device>& m_device);
		[[nodiscard]] result<> InitializeSwapChain();

		void ResetRTVandDS();
		[[nodiscard]] result<> RecreateRenderTargetsAndDepth();
	};

	SurfDirectX::SurfDirectX(
		std::shared_ptr<Settings> _settings,
		std::shared_ptr<IAppWin> _iAppWin,
		std::shared_ptr<IGAPI> _iGAPI)
		: ISurfaceView(_settings, _iAppWin, _iGAPI),
		b_IgnoreResize{ false },
		m_tearingSupported{ false },
		m_RtvDescrSize{ 0 },
		m_DsvDescrSize{ 0 },
		m_CbvSrvDescrSize{ 0 },
		m_frameReady{ false }
	{
	}

#pragma region Initialize
	result<> SurfDirectX::Initialize()
	{
		auto m_device = m_iGAPI->GetDevice();
		ensure(m_device, ">>>>> [SurfDirectX::Initialize()]. Device cannot be null.");
		auto m_commandQueue = m_iGAPI->GetCommandQueue();
		ensure(m_commandQueue, ">>>>> [SurfDirectX::Initialize()]. Command queue cannot be null.");
		auto m_factory = m_iGAPI->GetFactory();
		ensure(m_factory, ">>>>> [SurfDirectX::Initialize()]. Factory cannot be null.");

		m_RtvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvDescrSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		auto winSize = m_iAppWin->GetWinSize();
		m_SurfSize.SetSize(static_cast<zU32>(winSize.width), static_cast<zU32>(winSize.height));

		auto res = InitializeSwapChain();
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. Failed to initialize swap chain.");

		HRESULT hr = m_factory->MakeWindowAssociation(m_iAppWin->GetHWND(), DXGI_MWA_NO_ALT_ENTER);
		if (FAILED(hr))
			return Unexpected(eResult::failure, L"Failed to make window association");

		res = CreateRTVHeap()
			.and_then([&]() { return CreateSRVHeap(); })
			.and_then([&]() { return BuildConstantBuffers(); })
			.and_then([&]() { return CreateDSVHeap(); })
			.and_then([&]() { return CreateRTV(m_device); });

		res = CreateDS(winSize);
		if (!res)
			return Unexpected(eResult::failure, L">>>>> [DXAPI::InitializePipeline()]. -> " + res.error().getMessage());

		return {};
	}

	result<> SurfDirectX::CreateRTV(ComPtr<ID3D12Device>& m_device)
	{
		ensure(m_device, ">>>>> [SurfDirectX::CreateRTV()]. Device cannot be null.");
		ensure(m_rtvHeap, ">>>>> [SurfDirectX::CreateRTV()]. RTV Heap cannot be null.");

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < BACK_BUFFER_COUNT; n++)
		{
			HRESULT hr = m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
			if (FAILED(hr))
				return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::InitializePipeline()]. Failed to get back buffer. HRESULT = 0x{:08X}", hr));

			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_RtvDescrSize);

			SET_RESOURCE_DEBUG_NAME(m_renderTargets[n], std::format(L"RTV Frame {}", n).c_str());
		}

		return {};
	}

	result<> SurfDirectX::InitializeSwapChain()
	{
		BOOL allowTearing = FALSE;
		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(m_iGAPI->GetFactory().As(&factory5)))
		{
			if (SUCCEEDED(factory5->CheckFeatureSupport(
				DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allowTearing,
				sizeof(allowTearing))))
			{
				m_tearingSupported = allowTearing == TRUE;
			}
		}

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
		swapChainDesc.Flags = m_tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ComPtr<IDXGISwapChain1> swapChain1;
		ensure(S_OK == m_iGAPI->GetFactory()->CreateSwapChainForHwnd(
			m_iGAPI->GetCommandQueue().Get(),
			m_iAppWin->GetHWND(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1));

		ensure(S_OK == swapChain1.As(&m_swapChain));

		return {};
	}

	result<> SurfDirectX::CreateRTVHeap()
	{
		auto m_device = m_iGAPI->GetDevice();
		ensure(m_device, ">>>>> [SurfDirectX::CreateRTVHeap()]. Device cannot be null.");

		// Describe and create a render target View (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = BACK_BUFFER_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to create RTV heap. HRESULT = 0x{:08X}", hr));

		SET_RESOURCE_DEBUG_NAME(m_rtvHeap, L"RTV Heap");

		return {};
	}

	result<> SurfDirectX::CreateSRVHeap()
	{
		auto m_device = m_iGAPI->GetDevice();
		ensure(m_device, ">>>>> [SurfDirectX::CreateSRVHeap()]. Device cannot be null.");

		// Describe and create a shader resource View (SRV) heap for the texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to create SRV heap. HRESULT = 0x{:08X}", hr));

		SET_RESOURCE_DEBUG_NAME(m_srvHeap, L"SRV Heap");

		return {};
	}

	result<> SurfDirectX::BuildConstantBuffers()
	{
		auto m_device = m_iGAPI->GetDevice();
		mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_device.Get(), 1, true);

		UINT objCBByteSize = CalcBufferSize32(sizeof(ObjectConstants));

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
		// Offset to the ith object constant buffer in the buffer.
		int boxCBufIndex = 0;
		cbAddress += boxCBufIndex * objCBByteSize;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = CalcBufferSize32(sizeof(ObjectConstants));

		m_device->CreateConstantBufferView(&cbvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

		return {};
	}

	result<> SurfDirectX::CreateDSVHeap()
	{
		auto m_device = m_iGAPI->GetDevice();
		ensure(m_device, ">>>>> [SurfDirectX::CreateDSVHeap()]. Device cannot be null.");

		const D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			.NumDescriptors = BACK_BUFFER_COUNT,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask = 0
		};

		HRESULT hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to create DSV heap. HRESULT = 0x{:08X}", hr));

		SET_RESOURCE_DEBUG_NAME(m_dsvHeap, L"DSV Heap");

		return {};
	}

	result<> SurfDirectX::CreateDS(const size2D<>& size)
	{
		auto m_device = m_iGAPI->GetDevice();
		ensure(m_device, ">>>>> [SurfDirectX::CreateDS()]. Device cannot be null.");

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
				return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::CreateDS()]. Failed to create depth stencil resource. HRESULT = 0x{:08X}", hr));

			SET_RESOURCE_DEBUG_NAME(m_depthStencil[i], std::format(L"DepthStencil_{}", i).c_str());

			// Создаём DSV
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
				m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
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

	void SurfDirectX::ResetRTVandDS()
	{
		for (auto& rt : m_renderTargets)
			rt.Reset();

		for (auto& ds : m_depthStencil)
			ds.Reset();
	}

	result<> SurfDirectX::RecreateRenderTargetsAndDepth()
	{
		auto m_device = m_iGAPI->GetDevice();

		auto res = CreateRTV(m_device);
		if (!res)
			return Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::RecreateRenderTargetsAndDepth()]. Failed to create RTV. {}", res.error().getMessage()).c_str());

		DXGI_SWAP_CHAIN_DESC desc{};
		HRESULT hr = m_swapChain->GetDesc(&desc);
		if (S_OK != hr)
			Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::RecreateRenderTargetsAndDepth()]. Failed to get swap chain description. HRESULT = 0x{:08X}", hr));

		size2D<> size{ desc.BufferDesc.Width, desc.BufferDesc.Height };
		res = CreateDS(size);
		if (!res)
			Unexpected(eResult::failure, std::format(L">>>>> [DXAPI::RecreateRenderTargetsAndDepth()]. Failed to create depth stencil View. {}", res.error().getMessage()).c_str());

		return {};
	}
#pragma endregion Initialize

#pragma region Rendring
	void SurfDirectX::PrepareFrame(std::shared_ptr<Scene> scene)
	{
		{
			//Matrix4x4 mView;
			//Matrix4x4 mProj;
			//
			//mProj = Matrix4x4::perspective(
			//	0.25f * Pi,		// FoV 45 градусов
			//	16.0f / 9.0f,	// Aspect ratio
			//	1.0f,			// Near plane
			//	1000.0f);		// Far plane
			//
			//float mTheta = 1.5f * Pi;
			//float mPhi = Pi / 4.0f;  // 45 градусов
			//float mRadius = 5.0f;
			//
			//float x = mRadius * std::sin(mPhi) * std::cos(mTheta);
			//float z = mRadius * std::sin(mPhi) * std::sin(mTheta);
			//float y = mRadius * std::cos(mPhi);
			//
			//Vector4 pos(x, y, z, 1.0f);
			//Vector4 target(0.0f, 0.0f, 0.0f, 1.0f);
			//Vector4 up(0.0f, 1.0f, 0.0f, 0.0f);
			//mView = Matrix4x4::lookAt(pos, target, up);
			//Matrix4x4 worldViewProj = mProj * mView * mWorld;
		}

		Camera& primaryCamera = scene->GetPrimaryCamera();

		// Обновляем константный буфер. Пока для 1-го объекта
		{
			Matrix4x4 mWorld;
			Matrix4x4 camViewProj = primaryCamera.GetViewProjectionMatrix();
			Matrix4x4 worldViewProj = camViewProj * mWorld;
			ObjectConstants objConstants;
			std::memcpy(&objConstants.WorldViewProj, &worldViewProj, sizeof(Matrix4x4));
			mObjectCB->CopyData(0, objConstants);
		}

		zU64 frameIndex = (m_swapChain->GetCurrentBackBufferIndex() + 1) % BACK_BUFFER_COUNT;

		// Синхронизируемся с рендерингом чтобы frameIndex остался валидным
		{
			std::lock_guard<std::mutex> lock(m_frameMutex);
			m_frameReady = true;
			m_frameCV.notify_one(); // Уведомляем ОДИН ожидающий поток
		}

		auto commandList = m_iGAPI->GetCommandListUpdate();
		ensure(commandList, ">>>>> [SurfDirectX::PrepareFrame()]. Command list cannot be null.");
		commandList->SetGraphicsRootSignature(m_iGAPI->GetRootSignature().Get());

		{
			// Привязываем root-параметры
			commandList->SetGraphicsRootConstantBufferView(0, mObjectCB->Resource()->GetGPUVirtualAddress());

			//commandList->SetGraphicsRootConstantBufferView(
			//	1,
			//	materialCBV_GPUHandle  // если есть материал (b1)
			//);

			//commandList->SetGraphicsRootDescriptorTable(
			//	2,
			//	m_srvHeap->GetGPUDescriptorHandleForHeapStart()  // SRV таблица начинается с t0
			//	// Если CBV занимает слот 0, то SRV начинаются с 1 -> нужно сместить!
			//);
		}

		{
			ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvHeap.Get() };
			commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		}

		// Настраиваем область рендеринга исходя из настроек камеры
		{
			RenderArea renderArea = primaryCamera.CalculateRenderArea(static_cast<zU32>(m_SurfSize.width), static_cast<zU32>(m_SurfSize.height));
			D3D12_VIEWPORT viewport = renderArea.GetViewport();
			D3D12_RECT scissor = renderArea.GetScissor();
			commandList->RSSetViewports(1, &viewport);
			commandList->RSSetScissorRects(1, &scissor);
		}

		commandList->ResourceBarrier(
			1,
			&keep(CD3DX12_RESOURCE_BARRIER::Transition(
				m_renderTargets[frameIndex].Get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET)));

		commandList->ResourceBarrier(
			1,
			&keep(CD3DX12_RESOURCE_BARRIER::Transition(
				m_depthStencil[frameIndex].Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_DEPTH_WRITE)));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(frameIndex), m_RtvDescrSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(frameIndex), m_DsvDescrSize);

		// Очищаем всю поверхность перед рендерингом если нужно
		switch (m_SurfClearType)
		{
		case SurfClearType::Color:
			commandList->ClearRenderTargetView(rtvHandle, m_ClearColor, 0, nullptr);
			break;
		}

		// Очистка буфера глубины, если нужно
		if(b_IsClearDepth)
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		{
			// Дополнительные команды рендеринга, если нужно
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			auto entity = scene->GetEntity();
			auto material = entity->GetMaterial();
			commandList->SetPipelineState(material->GetPSO()->GetPSO().Get());

			auto mesh = entity->GetMesh();
			commandList->IASetVertexBuffers(0, 1, mesh->VertexBufferView());
			commandList->IASetIndexBuffer(mesh->IndexBufferView());

			//commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

			commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
		}

		// Завершение подготовки (из EndRender, до закрытия командного списка)
		commandList->ResourceBarrier(
			1,
			&keep(CD3DX12_RESOURCE_BARRIER::Transition(
				m_renderTargets[frameIndex].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT)));

		commandList->ResourceBarrier(
			1,
			&keep(CD3DX12_RESOURCE_BARRIER::Transition(
				m_depthStencil[frameIndex].Get(),
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				D3D12_RESOURCE_STATE_COMMON)));
	}

	void SurfDirectX::RenderFrame()
	{
		// Ожидаем(синхронизируемся) получения валидного frameIndex в PrepareFrame
		{
			std::unique_lock<std::mutex> lock(m_frameMutex);
			m_frameCV.wait(lock, [this] { return m_frameReady; }); // Ждем, пока фрейм будет готов
			m_frameReady = false; // Сбрасываем флаг для следующего кадра
		}

		// Выполняем командный список
		m_iGAPI->SubmitCommandLists();

		// Настраиваем параметры для Present
		BOOL fullscreen = FALSE;
		ensure(S_OK == m_swapChain->GetFullscreenState(&fullscreen, nullptr));
		UINT syncInterval = b_IsVSync ? 1 : 0;
		UINT presentFlags = (!b_IsVSync && !fullscreen && m_tearingSupported) ? DXGI_PRESENT_ALLOW_TEARING : 0;
		HRESULT hr = m_swapChain->Present(syncInterval, presentFlags);
		if (FAILED(hr))
		{
			std::string errMsg;
			switch (hr)
			{
			case DXGI_ERROR_DEVICE_HUNG:
				// Драйвер устройства перестал отвечать
				errMsg = ">>>>> [SurfDirectX::RenderFrame()]. GPU device hung - driver issues";
				break;
			case DXGI_ERROR_DEVICE_REMOVED:
				// Устройство было физически удалено
				errMsg = ">>>>> [SurfDirectX::RenderFrame()]. GPU device physically removed";
				break;
			case DXGI_ERROR_DEVICE_RESET:
				// Устройство было сброшено
				errMsg = ">>>>> [SurfDirectX::RenderFrame()]. GPU device reset";
				break;
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
				// Внутренняя ошибка драйвера
				errMsg = ">>>>> [SurfDirectX::RenderFrame()]. GPU driver internal error";
				break;
			case DXGI_ERROR_INVALID_CALL:
				// Неправильный вызов API
				errMsg = ">>>>> [SurfDirectX::RenderFrame()]. Invalid API call";
				break;
			default:
				errMsg = std::format(">>>>> [SurfDirectX::RenderFrame()]. Unknown device removed reason: {:#x}", hr);
				break;
			}

			throw_runtime_error(errMsg);
		}
	}
#pragma endregion Rendring

	void SurfDirectX::OnResize(const size2D<>& size)
	{
		//DebugOutput(std::format(L">>>>> [SurfDirectX::OnResize({}x{})]. b_IgnoreResize: {}.", size.width, size.height, b_IgnoreResize).c_str());
		if (m_iGAPI->GetInitState() != eInitState::InitOK && !m_swapChain || b_IgnoreResize)
			return;

		if (size.width == 0 || size.height == 0)
		{
			DebugOutput(L">>>>> [SurfDirectX::OnResize()]. Invalid size for resize. Width or height is zero.\n");
			return;
		}

		auto m_device = m_iGAPI->GetDevice();
		ensure(m_device, ">>>>> [SurfDirectX::CreateRTVHeap()]. Device cannot be null.");
		ensure(m_swapChain, ">>>>> [SurfDirectX::CreateRTVHeap()]. Swap chain cannot be null.");

		DXGI_SWAP_CHAIN_DESC1 desc;
		HRESULT hr = m_swapChain->GetDesc1(&desc);
		if (SUCCEEDED(hr))
		{
			// Сравниваем с новым размером
			if (desc.Width == static_cast<UINT>(size.width) &&
				desc.Height == static_cast<UINT>(size.height))
			{
				DebugOutput(std::format(L">>>>> [SurfDirectX::OnResize({}x{})]. No resize needed, dimensions are unchanged.", size.width, size.height).c_str());
				return; // Размеры не изменились
			}
		}

		m_iGAPI->CommandRenderReset();
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
				m_tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);
		}
		if (S_OK != hr)
			throw_runtime_error(std::format(">>>>> [SurfDirectX::OnResize({}x{})].", size.width, size.height));

		auto res = CreateRTVHeap()
			.and_then([&]() { return CreateRTV(m_device); })
			.and_then([&]() { return CreateDS(size); })
			.and_then([&]() { return m_iGAPI->CommandRenderReinitialize(); })
			.or_else([&](const Unexpected& error) { throw_runtime_error(std::format(">>>>> [SurfDirectX::OnResize({}x{})]. {}.", size.width, size.height, wstring_to_string(error.getMessage()))); });

		m_SurfSize.SetSize(size.width, size.height);
	}

	void SurfDirectX::SetFullScreen(bool fs)
	{
		BOOL fullscreen = FALSE;
		HRESULT hr = m_swapChain->GetFullscreenState(&fullscreen, nullptr);
		if (FAILED(hr))
		{
			DebugOutput(std::format(L">>>>> [SurfDirectX::SetFullScreen({})] Failed to get fullscreen state. HRESULT = 0x{:08X}\n", fs, hr).c_str());
			return;
		}

		if (fs == static_cast<bool>(fullscreen))
		{
			DebugOutput(std::format(L">>>>> [SurfDirectX::SetFullScreen({})] Fullscreen state is already set.\n", fs).c_str());
			return;
		}

		m_iGAPI->CommandRenderReset();
		ResetRTVandDS();

		b_IgnoreResize = true;
		hr = m_swapChain->SetFullscreenState(fs, nullptr);
		if (S_OK != hr)
		{
			b_IgnoreResize = false;
			DebugOutput(std::format(L">>>>> [SurfDirectX::SetFullScreen({})] Failed to set fullscreen state. HRESULT = 0x{:08X}\n", fs, hr).c_str());
			auto res = RecreateRenderTargetsAndDepth();
			if (!res)
				throw_runtime_error(std::format(">>>>> #0 [SurfDirectX::SetFullScreen({})]. Failed to recreate render targets and depth stencil View. {}.", fs, wstring_to_string(res.error().getMessage())));

			return;
		}
		b_IgnoreResize = false;

		DXGI_SWAP_CHAIN_DESC desc{};
		hr = m_swapChain->GetDesc(&desc);
		if (S_OK != hr)
		{
			DebugOutput(std::format(L">>>>> [SurfDirectX::SetFullScreen({})] Failed to get swap chain description. HRESULT = 0x{:08X}\n", fs, hr).c_str());
			return;
		}

		hr = m_swapChain->ResizeBuffers(
			desc.BufferCount,
			0, 0, // авто определение размеров
			desc.BufferDesc.Format,
			desc.Flags);
		if (S_OK != hr)
		{
			DebugOutput(std::format(L">>>>> [SurfDirectX::SetFullScreen({})] Failed to resize buffers. HRESULT = 0x{:08X}\n", fs, hr).c_str());
			return;
		}

		auto res = RecreateRenderTargetsAndDepth()
			.and_then([&]() { return m_iGAPI->CommandRenderReinitialize(); })
			.or_else([&](const Unexpected& error) { throw_runtime_error(std::format(">>>>> #1 [SurfDirectX::SetFullScreen({})]. Failed: {}.", fs, wstring_to_string(error.getMessage()))); });
		

		DebugOutput(std::format(L">>>>> [SurfDirectX::SetFullScreen({})].\n", fs).c_str());
	}
}
#endif // defined(ZRENDER_API_D3D12)