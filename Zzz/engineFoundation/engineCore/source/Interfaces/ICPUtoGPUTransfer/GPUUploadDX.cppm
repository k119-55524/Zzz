
export module GPUUploadDX;

#if defined(ZRENDER_API_D3D12)

import Result;
import QueueArray;
import IGPUUpload;
import CommandWrapperDX;
import ThreadSafeSwapBuffer;

using namespace zzz;
using namespace zzz::templates;

namespace zzz::dx
{
	export class GPUUploadDX final : public IGPUUpload
	{
		Z_NO_CREATE_COPY(GPUUploadDX);

	public:
		GPUUploadDX(ComPtr<ID3D12Device>& m_device, ThreadSafeSwapBuffer<std::shared_ptr<GPUUploadCB>>& _Prepared);
		~GPUUploadDX() override;

		void TransferResourceToGPU() override;

	private:
		void Initialize(ComPtr<ID3D12Device>& device);

		std::unique_ptr<CommandWrapperDX> m_CommandTransfer[2];
		ComPtr<ID3D12CommandQueue> m_CopyQueue;
		UINT64 m_CopyFenceValue;
		ComPtr<ID3D12Fence> m_CopyFence;
		unique_handle m_FenceEvent;

		ThreadSafeSwapBuffer<std::shared_ptr<GPUUploadCB>>& m_Prepared;
	};

	GPUUploadDX::GPUUploadDX(ComPtr<ID3D12Device>& device, ThreadSafeSwapBuffer<std::shared_ptr<GPUUploadCB>>& _Prepared) :
		m_Prepared{ _Prepared },
		m_CopyFenceValue{ 0 }
	{
		ensure(device != nullptr);
		Initialize(device);
	}

	GPUUploadDX::~GPUUploadDX()
	{
	}

	void GPUUploadDX::Initialize(ComPtr<ID3D12Device>& device)
	{
		m_CommandTransfer[0] = safe_make_unique<CommandWrapperDX>(device, D3D12_COMMAND_LIST_TYPE_COPY);
		m_CommandTransfer[1] = safe_make_unique<CommandWrapperDX>(device, D3D12_COMMAND_LIST_TYPE_COPY);

		D3D12_COMMAND_QUEUE_DESC copyQueueDesc = {};
		copyQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;  // Ключевой тип
		copyQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		HRESULT hr = device->CreateCommandQueue(&copyQueueDesc, IID_PPV_ARGS(&m_CopyQueue));
		if (hr != S_OK)
			throw_runtime_error(std::format("Failed to CreateCommandQueue. HRESULT = 0x{:08X}", hr));

		// Создание объектов синхронизации
		{
			ComPtr<ID3D12Fence> fence;
			HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
			if (FAILED(hr))
				throw_runtime_error(std::format("Failed to CreateFence. HRESULT = 0x{:08X}", hr));

			m_CopyFence = fence;

			// RAII для события
			unique_handle fenceEvent{ CreateEvent(nullptr, FALSE, FALSE, nullptr) };
			if (!fenceEvent)
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
				throw_runtime_error(std::format("Failed to CreateEvent. HRESULT = 0x{:08X}", hr));
			}

			m_FenceEvent = fenceEvent.release();
		}
	}

	// Не потоко безопасна. Предназначена для вызова строго из одного потока.
	void GPUUploadDX::TransferResourceToGPU()
	{
		if (m_TransferCallbacks[m_TransferIndex].Size() > 0)
		{
			const ComPtr<ID3D12CommandAllocator>& copyAllocator = m_CommandTransfer[m_TransferIndex]->GetCommandAllocator();
			const ComPtr<ID3D12GraphicsCommandList>& copyList = m_CommandTransfer[m_TransferIndex]->GetCommandList();

			HRESULT hr = copyAllocator->Reset();
			if (S_OK != hr)
				throw_runtime_error("copyAllocator->Reset()...");

			hr = copyList->Reset(copyAllocator.Get(), nullptr);
			if (S_OK != hr)
				throw_runtime_error("copyList->Reset( ... )...");

			for (int i = 0; i < m_TransferCallbacks[m_TransferIndex].Size(); i++)
			{
				try
				{
					m_TransferCallbacks[m_TransferIndex][i]->OnFill(copyList);
				}
				catch ( ... )
				{
					m_TransferCallbacks[m_TransferIndex][i]->Success = false;
					m_TransferCallbacks[m_TransferIndex][i]->OnComplete(false);
				}
			}

			hr = copyList->Close();
			if (S_OK != hr)
				throw_runtime_error("copyList->Close()");

			ID3D12CommandList* ppLists[] = { copyList.Get() };
			m_CopyQueue->ExecuteCommandLists(1, ppLists);

			// Sync
			{
				m_CopyFenceValue++;
				ensure(S_OK == m_CopyQueue->Signal(m_CopyFence.Get(), m_CopyFenceValue));

				if (m_CopyFence->GetCompletedValue() < m_CopyFenceValue)
				{
					ensure(S_OK == m_CopyFence->SetEventOnCompletion(m_CopyFenceValue, m_FenceEvent.handle));
					WaitForSingleObject(m_FenceEvent.handle, INFINITE);
				}
			}

			for (int i = 0; i < m_TransferCallbacks[m_TransferIndex].Size(); i++)
			{
				if (m_TransferCallbacks[m_TransferIndex][i]->Success)
				{
					m_Prepared.Add(m_TransferCallbacks[m_TransferIndex][i]);
				}
			}

			m_TransferCallbacks[m_TransferIndex].Clear();
		}

		std::lock_guard<std::mutex> lock(hasMutex);
		m_TransferIndex = 1 - m_TransferIndex;
	}
}
#endif // ZRENDER_API_D3D12