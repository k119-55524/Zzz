
#include "SurfView_DX.h"
#include "Swapchain_DX.h"
#include "DepthBuffer_DX.h"
#include "engine/utils/EngineLogFlags.h"

Z_SET_LOG_CATEGORY(::zzz::core::GAPI);

#if defined(Z_D3D12)
namespace zzz::engine
{
	SurfView_DX::SurfView_DX(std::shared_ptr<NativeWindow> window, std::shared_ptr<DirectX12API> gapi)
		: ISurfView(std::move(window), std::move(gapi))
	{
	}

	SurfView_DX::~SurfView_DX()
	{
		OnSurfaceDestroyed();
	}

#pragma region Initialize
	void SurfView_DX::OnSurfaceCreated(void* handle)
	{
		DOut("[SurfView_DX::OnSurfaceCreated] - Handle: {}", handle);
		Initialize();
	}

	void SurfView_DX::OnSurfaceDestroyed()
	{
		DOut("[SurfView_DX::OnSurfaceDestroyed]");

		std::lock_guard<std::mutex> lock(m_SubmitMutex);

		if (!m_GAPI)
			return;

		m_GAPI->WaitForGpu();

		for (size_t i = 0; i < zzz::core::c_FramesInFlight; ++i)
		{
			if (m_IsRecording[i] && m_CommandLists[i])
			{
				m_CommandLists[i]->Close();
				m_IsRecording[i] = false;
			}
			m_CommandLists[i].Reset();
			m_CommandAllocators[i].Reset();
		}

		if (m_DepthBuffer)
		{
			m_DepthBuffer->Release();
			m_DepthBuffer.reset();
		}

		if (m_Swapchain)
		{
			m_Swapchain->Release();
			m_Swapchain.reset();
		}
	}

	void SurfView_DX::Initialize()
	{
		m_Swapchain = std::make_unique<Swapchain_DX>(m_GAPI, m_Window);
		m_OldSize = m_Swapchain->GetSize();
		m_DepthBuffer = std::make_unique<DepthBuffer_DX>(m_GAPI, m_OldSize);

		ID3D12Device* device = m_GAPI->GetDevice();
		ensure(device, "DirectX12 Device не должен быть null.");

		for (size_t i = 0; i < zzz::core::c_FramesInFlight; ++i)
		{
			HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[i]));
			if (FAILED(hr))
				THROW_RUNTIME("Failed to create Command Allocator [{}] in SurfView_DX: 0x{:08X}", i, static_cast<uint32_t>(hr));

			m_GAPI->SetDebugName(m_CommandAllocators[i], std::format("SurfViewCommandAllocator[{}]", i).c_str());

			hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[i].Get(), nullptr, IID_PPV_ARGS(&m_CommandLists[i]));
			if (FAILED(hr))
				THROW_RUNTIME("Failed to create Command List [{}] in SurfView_DX: 0x{:08X}", i, static_cast<uint32_t>(hr));

			m_GAPI->SetDebugName(m_CommandLists[i], std::format("SurfViewCommandList[{}]", i).c_str());

			m_CommandLists[i]->Close();
			m_IsRecording[i] = false;
		}
	}
#pragma endregion // Initialize

	void SurfView_DX::PreRender()
	{
		if (!m_Swapchain || !m_DepthBuffer)
			return;

		if (m_OldSize.width == 0 || m_OldSize.height == 0)
			return;

		const uint32_t prepIdx = GetPrepareIndex();

		// 1. Ожидаем завершения работы GPU с ресурсами этого логического слота (аллокаторы и т.д.)
		m_GAPI->WaitForFenceValue(m_FrameFenceValues[prepIdx]);

		m_FrameReady[prepIdx] = false;

		// 2. Получаем физический индекс буфера из Swapchain (аналог vkAcquireNextImage)
		auto dxSwapchain = static_cast<Swapchain_DX*>(m_Swapchain.get());
		const uint32_t physIdx = dxSwapchain->AcquireNextImage();
		m_PhysicalIndices[prepIdx] = physIdx;

		ID3D12Resource* backBuffer = dxSwapchain->GetBackBuffer(physIdx);
		if (!backBuffer)
			return;

		m_FrameBackBuffer[prepIdx] = backBuffer;
		m_FrameReady[prepIdx] = true;
	}

	void SurfView_DX::PrepareFrame(const ClearConfig& clearConfig)
	{
		if (!m_Swapchain || !m_DepthBuffer)
			return;

		const uint32_t prepIdx = GetPrepareIndex();
		if (!m_FrameReady[prepIdx])
			return;

		if (!m_CommandAllocators[prepIdx] || !m_CommandLists[prepIdx])
			return;

		if (m_IsRecording[prepIdx])
			return;

		auto dxSwapchain = static_cast<Swapchain_DX*>(m_Swapchain.get());
		auto dxDepthBuffer = static_cast<DepthBuffer_DX*>(m_DepthBuffer.get());
		ID3D12Resource* backBuffer = m_FrameBackBuffer[prepIdx];
		const uint32_t physIdx = m_PhysicalIndices[prepIdx];

		HRESULT hr = m_CommandAllocators[prepIdx]->Reset();
		if (FAILED(hr))
		{
			DOutError("[SurfView_DX::PrepareFrame] Failed to reset Command Allocator [{}]: 0x{:08X}", prepIdx, static_cast<uint32_t>(hr));
			return;
		}

		hr = m_CommandLists[prepIdx]->Reset(m_CommandAllocators[prepIdx].Get(), nullptr);
		if (FAILED(hr))
		{
			DOutError("[SurfView_DX::PrepareFrame] Failed to reset Command List [{}]: 0x{:08X}", prepIdx, static_cast<uint32_t>(hr));
			return;
		}

		m_IsRecording[prepIdx] = true;

		// 1. Transition BackBuffer: PRESENT -> RENDER_TARGET
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = backBuffer;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		m_CommandLists[prepIdx]->ResourceBarrier(1, &barrier);

		// 2. Set Viewport & Scissor Rect
		D3D12_VIEWPORT viewport{ 0.0f, 0.0f, static_cast<FLOAT>(m_OldSize.width), static_cast<FLOAT>(m_OldSize.height), 0.0f, 1.0f };
		D3D12_RECT scissorRect{ 0, 0, static_cast<LONG>(m_OldSize.width), static_cast<LONG>(m_OldSize.height) };
		m_CommandLists[prepIdx]->RSSetViewports(1, &viewport);
		m_CommandLists[prepIdx]->RSSetScissorRects(1, &scissorRect);

		// 3. Clear Color Surface and Depth-Stencil Buffer (конфиг очистки приходит per-frame от активной сцены)
		dxSwapchain->SetClearConfig(clearConfig.surface);
		dxDepthBuffer->SetClearConfig(clearConfig.depthBuffer);
		dxSwapchain->Clear(m_CommandLists[prepIdx].Get(), physIdx);
		dxDepthBuffer->Clear(m_CommandLists[prepIdx].Get());

		// 4. Bind Render Target and Depth Stencil View
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxSwapchain->GetRTVHandle(physIdx);
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxDepthBuffer->GetDSVHandle();
		m_CommandLists[prepIdx]->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	}

	void SurfView_DX::SubmitRenderTree(const SceneRenderTree& renderTree)
	{
		// SceneRenderTree - пока заглушка (пустое дерево), трансляция бакетов Материалы -> Меши
		// в нативные Draw-вызовы появится вместе с ResourceManager/MeshRenderer.
		(void)renderTree;
	}

	void SurfView_DX::RenderFrame()
	{
		if (!m_Swapchain)
			return;

		const uint32_t renderIdx = GetRenderIndex();
		if (!m_CommandLists[renderIdx] || !m_IsRecording[renderIdx])
			return;

		ID3D12Resource* backBuffer = m_FrameBackBuffer[renderIdx];
		if (!backBuffer)
		{
			m_CommandLists[renderIdx]->Close();
			m_IsRecording[renderIdx] = false;
			return;
		}

		// 1. Transition BackBuffer: RENDER_TARGET -> PRESENT
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = backBuffer;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		m_CommandLists[renderIdx]->ResourceBarrier(1, &barrier);

		// 2. Close and execute Command List
		m_IsRecording[renderIdx] = false;
		HRESULT hr = m_CommandLists[renderIdx]->Close();

		if (FAILED(hr))
		{
			DOutError("[SurfView_DX::RenderFrame] Failed to close Command List [{}]: 0x{:08X}", renderIdx, static_cast<uint32_t>(hr));
			return;
		}

		ID3D12CommandList* cmds[] = { m_CommandLists[renderIdx].Get() };

		{
			std::lock_guard lock(m_SubmitMutex);
			m_GAPI->GetCommandQueue()->ExecuteCommandLists(1, cmds);
		}

		// 3. Signal Fence for this logical frame
		m_FrameFenceValues[renderIdx] = m_GAPI->SignalFence();

		// 4. Present
		m_Swapchain->Present(true);
	}

	void SurfView_DX::OnResize(const Size2D<>& size)
	{
		if (!m_Swapchain || !m_DepthBuffer)
			return;

		if (size.width == 0 || size.height == 0)
		{
			DOut("[SurfView_DX::OnResize] Width or height is zero.");
			return;
		}

		if (m_OldSize == size)
		{
			DOut("[SurfView_DX::OnResize] Dimensions are unchanged ({}x{}).", size.width, size.height);
			return;
		}

		std::lock_guard<std::mutex> lock(m_SubmitMutex);

		m_GAPI->WaitForGpu();

		for (size_t i = 0; i < zzz::core::c_FramesInFlight; ++i)
		{
			if (m_IsRecording[i] && m_CommandLists[i])
			{
				m_CommandLists[i]->Close();
				m_IsRecording[i] = false;
			}
		}

		m_Swapchain->OnResize(size);
		m_DepthBuffer->OnResize(size);

		m_OldSize = size;
		DOut(!Z_LOG_GET(g_IsResizing), "[SurfView_DX::OnResize] Resize completed from old size to {}x{}.", size.width, size.height);
	}
}
#endif // Z_D3D12
