#include "pch.h"
export module CPUtoGPUDataTransferDX;

import result;
import ICPUtoGPUDataTransfer;

using namespace zzz;

#if defined(_WIN64)
export namespace zzz::platforms::directx
{
	export class CPUtoGPUDataTransferDX final : public ICPUtoGPUDataTransfer
	{
	public:
		CPUtoGPUDataTransferDX() = delete;
		CPUtoGPUDataTransferDX(CPUtoGPUDataTransferDX&) = delete;
		CPUtoGPUDataTransferDX(CPUtoGPUDataTransferDX&&) = delete;

		CPUtoGPUDataTransferDX(const ComPtr<ID3D12Device> m_device);

		~CPUtoGPUDataTransferDX() = default;

	private:
		const ComPtr<ID3D12Device> m_device;
	};

	CPUtoGPUDataTransferDX::CPUtoGPUDataTransferDX(const ComPtr<ID3D12Device> device) :
		m_device{ device }
	{
		ensure(m_device != nullptr);
	}
}
#endif // _WIN64