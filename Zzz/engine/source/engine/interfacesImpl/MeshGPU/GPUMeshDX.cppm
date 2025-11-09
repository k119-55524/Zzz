#include "pch.h"
export module GPUMeshDX;

#if defined(ZRENDER_API_D3D12)

import IGAPI;
import DXAPI;
import CPUMesh;
import IMeshGPU;
import CPUIndexBuffer;
import CPUVertexBuffer;

using namespace zzz::platforms;

export namespace zzz::platforms::directx
{
	export class GPUMeshDX final : public IMeshGPU
	{
		friend class GPUResManager;

	public:
		GPUMeshDX() = delete;
		GPUMeshDX(std::shared_ptr<CPUMesh> meshCPU);

		~GPUMeshDX() = default;

		const D3D12_VERTEX_BUFFER_VIEW* VertexBufferView() const override { return &vertexBufferView; };
		const D3D12_INDEX_BUFFER_VIEW* IndexBufferView() const override { return indices ? &indexBufferView : nullptr; };

	private:
		ComPtr<ID3D12Resource> vertexBuffer;
		ComPtr<ID3D12Resource> indexBuffer;
		ComPtr<ID3D12Resource> uploadVertexBuffer;
		ComPtr<ID3D12Resource> uploadIndexBuffer;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;

		std::shared_ptr<ICPUVertexBuffer> vertices;
		std::shared_ptr<ICPUIndexBuffer> indices;
		UINT vertexBufferSize;
		UINT indexBufferSize;

		result<> Initialize(std::shared_ptr<IGAPI> _IGAPI) override;
	};

	GPUMeshDX::GPUMeshDX(std::shared_ptr<CPUMesh> meshCPU) :
		IMeshGPU(meshCPU)
	{
		vertices = m_MeshCPU->GetMesh();
		if (!vertices || vertices->GetData() == nullptr || vertices->SizeInBytes() == 0)
			throw_runtime_error(">>>>> [GPUMeshDX::GPUMeshDX()]. Invalid vertex data");

		indices = m_MeshCPU->GetIndicies();
		if (indices != nullptr && (indices->GetData() == nullptr || indices->GetSizeInBytes() == 0))
			throw_runtime_error(">>>>> [GPUMeshDX::GPUMeshDX()]. Invalid index data");

		vertexBufferSize = static_cast<UINT>(vertices->SizeInBytes());
		DebugOutput(std::format(L">>>>> [GPUMeshDX::GPUMeshDX( ... )]. vertexBufferSize: {}", vertexBufferSize));
		indexBufferSize = indices ? static_cast<UINT>(indices->GetSizeInBytes()) : 0;
		DebugOutput(std::format(L">>>>> [GPUMeshDX::GPUMeshDX( ... )]. indexBufferSize: {}", indexBufferSize));
	}

	result<> GPUMeshDX::Initialize(std::shared_ptr<IGAPI> _IGAPI)
	{
		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		HRESULT hr = _IGAPI->GetDevice()->CreateCommittedResource(
			&defaultHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COMMON,  // Правильное начальное состояние
			nullptr,
			IID_PPV_ARGS(&vertexBuffer)
		);
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> #0 [GPUMeshDX::Initialize]. Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

		// Создание промежуточного буфера для загрузки вершин (HEAP_TYPE_UPLOAD)
		CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		hr = _IGAPI->GetDevice()->CreateCommittedResource(
			&uploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadVertexBuffer)
		);
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> #1 [GPUMeshDX::Initialize]. Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = static_cast<UINT>(vertices->Stride());
		vertexBufferView.SizeInBytes = vertexBufferSize;

		if (indices != nullptr)
		{
			bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
			hr = _IGAPI->GetDevice()->CreateCommittedResource(
				&defaultHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&indexBuffer));
			if (FAILED(hr))
				return Unexpected(eResult::failure, std::format(L">>>>> #2 [GPUMeshDX::Initialize]. Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

			hr = _IGAPI->GetDevice()->CreateCommittedResource(
				&uploadHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&uploadIndexBuffer));
			if (FAILED(hr))
				return Unexpected(eResult::failure, std::format(L">>>>> #3 [GPUMeshDX::Initialize]. Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

			indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
			indexBufferView.SizeInBytes = indexBufferSize;
			indexBufferView.Format = indices->GetFormat();
		}

		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		hr = uploadVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L">>>>> #4 [GPUMeshDX::Initialize]. Failed Map. HRESULT = 0x{:08X}", hr));

		memcpy(pVertexDataBegin, vertices->GetData(), vertexBufferSize);
		uploadVertexBuffer->Unmap(0, nullptr);

		if (indices != nullptr)
		{
			UINT8* pIndexDataBegin;
			hr = uploadIndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin));
			if (FAILED(hr))
				return Unexpected(eResult::failure, std::format(L">>>>> #5 [GPUMeshDX::Initialize]. Failed Map. HRESULT = 0x{:08X}", hr));

			memcpy(pIndexDataBegin, indices->GetData(), indexBufferSize);
			uploadIndexBuffer->Unmap(0, nullptr);
		}

		_IGAPI->AddTransferResource(
			[&](const ComPtr<ID3D12GraphicsCommandList>& commandList)
			{
				// Переход vertex buffer в состояние для копирования
				auto vbBarrierToCopy = CD3DX12_RESOURCE_BARRIER::Transition(
					vertexBuffer.Get(),
					D3D12_RESOURCE_STATE_COMMON,
					D3D12_RESOURCE_STATE_COPY_DEST);
				commandList->ResourceBarrier(1, &vbBarrierToCopy);

				// Копирование vertex buffer
				commandList->CopyBufferRegion(vertexBuffer.Get(), 0, uploadVertexBuffer.Get(), 0, vertexBufferSize);

				// Переход vertex buffer обратно в COMMON (единственное допустимое финальное состояние для copy command list)
				CD3DX12_RESOURCE_BARRIER vbBarrierToCommon = CD3DX12_RESOURCE_BARRIER::Transition(
					vertexBuffer.Get(),
					D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_COMMON); // Только COMMON допустимо в copy command list
				commandList->ResourceBarrier(1, &vbBarrierToCommon);

				if (indices != nullptr)
				{
					auto ibBarrierToCopy = CD3DX12_RESOURCE_BARRIER::Transition(
						indexBuffer.Get(),
						D3D12_RESOURCE_STATE_COMMON,
						D3D12_RESOURCE_STATE_COPY_DEST);
					commandList->ResourceBarrier(1, &ibBarrierToCopy);

					commandList->CopyBufferRegion(indexBuffer.Get(), 0, uploadIndexBuffer.Get(), 0, indexBufferSize);

					CD3DX12_RESOURCE_BARRIER ibBarrierToCommon = CD3DX12_RESOURCE_BARRIER::Transition(
						indexBuffer.Get(),
						D3D12_RESOURCE_STATE_COPY_DEST,
						D3D12_RESOURCE_STATE_COMMON); // Только COMMON допустимо в copy command list
					commandList->ResourceBarrier(1, &ibBarrierToCommon);
				}
			},
			[&](const ComPtr<ID3D12GraphicsCommandList>& commandList)
			{
				// Перевод вершинного буфера
				auto vbBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
					vertexBuffer.Get(),
					D3D12_RESOURCE_STATE_COMMON,
					D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
				commandList->ResourceBarrier(1, &vbBarrier);

				// Если есть индексный буфер
				if (indexBuffer != nullptr)
				{
					auto ibBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
						indexBuffer.Get(),
						D3D12_RESOURCE_STATE_COMMON,
						D3D12_RESOURCE_STATE_INDEX_BUFFER);
					commandList->ResourceBarrier(1, &ibBarrier);
				}
			},
			[&](bool res)
			{
				// Освобождаем промежуточные буферы
				uploadVertexBuffer.Reset();
				if (uploadIndexBuffer != nullptr)
					uploadIndexBuffer.Reset();

				if (!res)
					std::cerr << "Transfer resource failed" << std::endl;
				else
				{

				}
			}
		);

		return {};
	}
}
#endif // ZRENDER_API_D3D12