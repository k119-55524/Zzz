#include "pch.h"
export module GPUMeshDX;

#if defined(_WIN64)

import IGAPI;
import DXAPI;
import CPUMesh;
import IMeshGPU;

using namespace zzz::platforms;
using namespace zzz::platforms::directx;

export namespace zzz
{
	export class GPUMeshDX final : public IMeshGPU
	{
		friend class GPUResourcesManager;

	public:
		GPUMeshDX() = delete;
		GPUMeshDX(std::shared_ptr<CPUMesh> meshCPU);

		~GPUMeshDX() = default;

	private:
		ComPtr<ID3D12Resource> vertexBuffer;
		ComPtr<ID3D12Resource> indexBuffer;
		ComPtr<ID3D12Resource> uploadVertexBuffer;
		ComPtr<ID3D12Resource> uploadIndexBuffer;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;

		result<> Initialize(std::shared_ptr<IGAPI> _IGAPI) override;
	};

	GPUMeshDX::GPUMeshDX(std::shared_ptr<CPUMesh> meshCPU) :
		IMeshGPU(meshCPU)
	{
	}

	result<> GPUMeshDX::Initialize(std::shared_ptr<IGAPI> _IGAPI)
	{
		std::shared_ptr<DXAPI> m_DXAPI = std::dynamic_pointer_cast<DXAPI>(_IGAPI);
		ensure(m_DXAPI != nullptr);

		auto vertices = m_MeshCPU->GetMesh();
		auto indices = m_MeshCPU->GetIndicies();

		const UINT vertexBufferSize = (UINT)vertices->SizeInBytes();
		UINT indexBufferSize = 0;
		if (indices != nullptr)
			indexBufferSize = static_cast<UINT>(indices->GetSizeInBytes());

		// Создание GPU-буфера для вершин (HEAP_TYPE_DEFAULT)
		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		HRESULT hr = m_DXAPI->GetDevice()->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer));
		if (hr != S_OK)
			return Unexpected(eResult::failure, std::format(L">>>>> #0 [GPUMeshDX::Initialize]. Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

		// Создание промежуточного буфера для загрузки вершин (HEAP_TYPE_UPLOAD)
		CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		hr = m_DXAPI->GetDevice()->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadVertexBuffer));
		if (hr != S_OK)
			return Unexpected(eResult::failure, std::format(L">>>>> #1 [GPUMeshDX::Initialize]. Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = static_cast<UINT>(vertices->Stride());
		vertexBufferView.SizeInBytes = vertexBufferSize;

		if (indices != nullptr)
		{
			bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
			hr = m_DXAPI->GetDevice()->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&indexBuffer));
			if (hr != S_OK)
				return Unexpected(eResult::failure, std::format(L">>>>> 3 [GPUMeshDX::Initialize]. Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

			hr = m_DXAPI->GetDevice()->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadIndexBuffer));
			if (hr != S_OK)
				return Unexpected(eResult::failure, std::format(L">>>>> 3 [GPUMeshDX::Initialize]. Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

			indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
			indexBufferView.SizeInBytes = indexBufferSize;
			indexBufferView.Format = indices->GetFormat();
		}

		// Сброс командного списка (предполагается, что commandAllocator уже сброшен)
		//commandList->Reset(commandAllocator.Get(), nullptr);  // Без PSO, так как это только для копирования

		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		hr = uploadVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		if (hr != S_OK)
			return Unexpected(eResult::failure, std::format(L">>>>> 4 [GPUMeshDX::Initialize]. Failed Map. HRESULT = 0x{:08X}", hr));

		memcpy(pVertexDataBegin, vertices->GetData(), sizeof(vertices));
		uploadVertexBuffer->Unmap(0, nullptr);

		if (indices != nullptr)
		{
			// Копирование индексов в промежуточный буфер
			UINT8* pIndexDataBegin;
			hr = uploadIndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin));
			if (hr != S_OK)
				return Unexpected(eResult::failure, std::format(L">>>>> 5 [GPUMeshDX::Initialize]. Failed Map. HRESULT = 0x{:08X}", hr));

			memcpy(pIndexDataBegin, indices->GetData(), sizeof(indices));
			uploadIndexBuffer->Unmap(0, nullptr);
		}

		_IGAPI->AddTransferResource(
			[&](const ComPtr<ID3D12GraphicsCommandList>& commandList)
			{
			},
			[&](bool res)
			{
			}
		);

		// Барьеры для перехода буферов в состояние копирования (если нужно; по умолчанию уже в COPY_DEST)
		// CD3DX12_RESOURCE_BARRIER vertexBarrier = CD3DX12_RESOURCE_BARRIER::Transition(vertexBufferDefault.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		// commandList->ResourceBarrier(1, &vertexBarrier);
		// Аналогично для индексов

		// Команда копирования на GPU
		//commandList->CopyBufferRegion(vertexBufferDefault.Get(), 0, uploadVertexBuffer.Get(), 0, sizeof(vertices));
		//commandList->CopyBufferRegion(indexBufferDefault.Get(), 0, uploadIndexBuffer.Get(), 0, sizeof(indices));

		// Барьеры для перехода обратно в состояние чтения (для рендеринга)
		//CD3DX12_RESOURCE_BARRIER vertexBarrierBack = CD3DX12_RESOURCE_BARRIER::Transition(vertexBufferDefault.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		//commandList->ResourceBarrier(1, &vertexBarrierBack);

		//CD3DX12_RESOURCE_BARRIER indexBarrierBack = CD3DX12_RESOURCE_BARRIER::Transition(indexBufferDefault.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		//commandList->ResourceBarrier(1, &indexBarrierBack);

		// Закрытие и выполнение командного списка
		//commandList->Close();
		//ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
		//commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Синхронизация: Ожидание завершения загрузки перед использованием в рендеринге
		//WaitForGPU();  // Функция из предыдущего примера с fence

		return {};
	}
}
#endif // _WIN64