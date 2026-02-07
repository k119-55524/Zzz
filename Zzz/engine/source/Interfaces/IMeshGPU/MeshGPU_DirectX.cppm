
export module MeshGPU_DirectX;

#if defined(ZRENDER_API_D3D12)
export import IGAPI;
export import DXAPI;
export import CPUMesh;
export import IMeshGPU;
export import CPUIndexBuffer;
export import CPUVertexBuffer;

export namespace zzz::directx
{
	export class MeshGPU_DirectX final : public IMeshGPU
	{
		friend class GPUResManager;

	public:
		MeshGPU_DirectX() = delete;
		MeshGPU_DirectX(std::shared_ptr<CPUMesh> meshCPU);
		~MeshGPU_DirectX() = default;

		const D3D12_VERTEX_BUFFER_VIEW* VertexBufferView() const { return &vertexBufferView; };
		const D3D12_INDEX_BUFFER_VIEW* IndexBufferView() const { return indices ? &indexBufferView : nullptr; };

	private:
		ComPtr<ID3D12Resource> vertexBuffer;
		ComPtr<ID3D12Resource> indexBuffer;
		ComPtr<ID3D12Resource> uploadVertexBuffer;
		ComPtr<ID3D12Resource> uploadIndexBuffer;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;

		std::shared_ptr<ICPUVertexBuffer> vertices;
		std::shared_ptr<ICPUIndexBuffer> indices;

		Result<> Initialize(std::shared_ptr<IGAPI> _IGAPI) override;

		// Копирует данные из промежуточных буферов на GPU
		void CopyBuffersToGPU(const ComPtr<ID3D12GraphicsCommandList>& commandList);
		// Настраивает барьеры ресурсов перед копированием данных на GPU
		void SetupResourceBarriers(const ComPtr<ID3D12GraphicsCommandList>& commandList);
		// Вызывается после завершения передачи данных на GPU
		void OnTransferComplete(bool isComplete);
	};

	MeshGPU_DirectX::MeshGPU_DirectX(std::shared_ptr<CPUMesh> meshCPU) :
		IMeshGPU(meshCPU)
	{
		vertices = m_MeshCPU->GetMesh();
		if (!vertices || vertices->GetData() == nullptr || vertices->SizeInBytes() == 0)
			throw_runtime_error("Invalid vertex data");

		indices = m_MeshCPU->GetIndicies();
		if (indices != nullptr && (indices->GetData() == nullptr || indices->GetSizeInBytes() == 0))
			throw_runtime_error("Invalid index data");
	}

	Result<> MeshGPU_DirectX::Initialize(std::shared_ptr<IGAPI> _IGAPI)
	{
		std::shared_ptr<DXAPI> dxAPI = std::dynamic_pointer_cast<DXAPI>(_IGAPI);
		ensure(dxAPI, "Failed to cast IGAPI to DXAPI.");

		CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices->SizeInBytes());
		HRESULT hr = dxAPI->GetDevice()->CreateCommittedResource(
			&defaultHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COMMON,  // Правильное начальное состояние
			nullptr,
			IID_PPV_ARGS(&vertexBuffer)
		);
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

		// Создание промежуточного буфера для загрузки вершин (HEAP_TYPE_UPLOAD)
		CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		hr = dxAPI->GetDevice()->CreateCommittedResource(
			&uploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadVertexBuffer)
		);
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"#1 Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = static_cast<UINT>(vertices->Stride());
		vertexBufferView.SizeInBytes = static_cast<UINT>(vertices->SizeInBytes());

		if (indices != nullptr)
		{
			bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indices->GetSizeInBytes());
			hr = dxAPI->GetDevice()->CreateCommittedResource(
				&defaultHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&indexBuffer));
			if (FAILED(hr))
				return Unexpected(eResult::failure, std::format(L"#2 Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

			hr = dxAPI->GetDevice()->CreateCommittedResource(
				&uploadHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&uploadIndexBuffer));
			if (FAILED(hr))
				return Unexpected(eResult::failure, std::format(L"#3 Failed to CreateCommittedResource. HRESULT = 0x{:08X}", hr));

			indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
			indexBufferView.SizeInBytes = static_cast<UINT>(indices->GetSizeInBytes());
			indexBufferView.Format = indices->GetFormat();
		}

		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		hr = uploadVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		if (FAILED(hr))
			return Unexpected(eResult::failure, std::format(L"#4 Failed Map. HRESULT = 0x{:08X}", hr));

		memcpy(pVertexDataBegin, vertices->GetData(), vertices->SizeInBytes());
		uploadVertexBuffer->Unmap(0, nullptr);

		if (indices != nullptr)
		{
			UINT8* pIndexDataBegin;
			hr = uploadIndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin));
			if (FAILED(hr))
				return Unexpected(eResult::failure, std::format(L"#5 Failed Map. HRESULT = 0x{:08X}", hr));

			memcpy(pIndexDataBegin, indices->GetData(), indices->GetSizeInBytes());
			uploadIndexBuffer->Unmap(0, nullptr);
		}

		// Передача ресурсов на GPU асинхронно
		dxAPI->AddTransferResource(
			[this](const ComPtr<ID3D12GraphicsCommandList>& commandList)
			{
				CopyBuffersToGPU(commandList);
			},
			[this](const ComPtr<ID3D12GraphicsCommandList>& commandList)
			{
				SetupResourceBarriers(commandList);
			},
			[this](bool res)
			{
				OnTransferComplete(res);
			}
		);

		return {};
	}

	void MeshGPU_DirectX::CopyBuffersToGPU(const ComPtr<ID3D12GraphicsCommandList>& commandList)
	{
		// Переход vertex buffer в состояние для копирования
		auto vbBarrierToCopy = CD3DX12_RESOURCE_BARRIER::Transition(
			vertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->ResourceBarrier(1, &vbBarrierToCopy);

		// Копирование vertex buffer
		commandList->CopyBufferRegion(vertexBuffer.Get(), 0, uploadVertexBuffer.Get(), 0, vertices->SizeInBytes());

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

			commandList->CopyBufferRegion(indexBuffer.Get(), 0, uploadIndexBuffer.Get(), 0, indices->GetSizeInBytes());

			CD3DX12_RESOURCE_BARRIER ibBarrierToCommon = CD3DX12_RESOURCE_BARRIER::Transition(
				indexBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_COMMON); // Только COMMON допустимо в copy command list
			commandList->ResourceBarrier(1, &ibBarrierToCommon);
		}
	}

	void MeshGPU_DirectX::SetupResourceBarriers(const ComPtr<ID3D12GraphicsCommandList>& commandList)
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
	}

	void MeshGPU_DirectX::OnTransferComplete(bool isComplete)
	{
		uploadVertexBuffer.Reset();
		if (uploadIndexBuffer != nullptr)
			uploadIndexBuffer.Reset();

		// TODO: Cделать нормальную обработку
		if (!isComplete)
			DebugOutput(L"Transfer mesh resource: FAILED.");
		else
			DebugOutput(L"Transfer mesh resource: SUCESS.");
	}
}
#endif // ZRENDER_API_D3D12