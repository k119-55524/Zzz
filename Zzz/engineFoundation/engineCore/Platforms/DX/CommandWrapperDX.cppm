
export module CommandWrapperDX;

#if defined(ZRENDER_API_D3D12)
import Result;
import Ensure;

using namespace zzz;

export namespace zzz::dx
{
	class CommandWrapperDX
	{
	public:
		CommandWrapperDX() = delete;
		CommandWrapperDX(CommandWrapperDX&) = delete;
		CommandWrapperDX(CommandWrapperDX&&) = delete;
		CommandWrapperDX(ComPtr<ID3D12Device>& m_device, D3D12_COMMAND_LIST_TYPE _type) :
			type{ _type }
		{
			Initialize(m_device);
		};

		inline const ComPtr<ID3D12CommandAllocator>& GetCommandAllocator() const noexcept { return m_commandAllocator; };
		inline const ComPtr<ID3D12GraphicsCommandList>& GetCommandList() const noexcept { return m_commandList; };

		inline void Reset() noexcept
		{
			if (m_commandList) m_commandList.Reset();
			if (m_commandAllocator) m_commandAllocator.Reset();
		}
		[[nodiscard]] inline Result<> Reinitialize(const ComPtr<ID3D12Device>& device)
		{
			Reset();
			return Initialize(device);
		}

	private:
		Result<> Initialize(const ComPtr<ID3D12Device>& m_device)
		{
			ensure(S_OK == m_device->CreateCommandAllocator(type, IID_PPV_ARGS(&m_commandAllocator)));
			SET_RESOURCE_DEBUG_NAME(m_commandAllocator, std::format(L"Command Allocator").c_str());

			// К одному ID3D12CommandAllocator(m_commandAllocator) можно привязать несколько ID3D12GraphicsCommandList(m_commandList) но одновременно записывать можно только в один.
			// А остальные ID3D12GraphicsCommandList, привязанные к ID3D12CommandAllocator, должны быть закрыты.
			// Так как ID3D12GraphicsCommandList при создании всегда открыт, сразу закрываем его.
			ensure(S_OK == m_device->CreateCommandList(0, type, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
			ensure(S_OK == m_commandList->Close());
			SET_RESOURCE_DEBUG_NAME(m_commandList, std::format(L"Command List").c_str());

			return {};
		}

		D3D12_COMMAND_LIST_TYPE type;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
	};
}
#endif // ZRENDER_API_D3D12