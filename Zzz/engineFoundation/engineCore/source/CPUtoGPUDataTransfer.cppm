
#include "pch.h"

export module CPUtoGPUDataTransfer;

export namespace zzz::engineCore
{
#if defined(ZRENDER_API_D3D12)
	using FillCallback = std::function<void(const ComPtr<ID3D12GraphicsCommandList>&)>;
	using PreparedCallback = std::function<void(const ComPtr<ID3D12GraphicsCommandList>&)>;
	using CompleteCallback = std::function<void(bool)>;
#elif defined(ZRENDER_API_VULKAN)
#error ">>>>> [Compile error]. This branch requires implementation for Vulkan"
#elif defined(ZRENDER_API_METAL)
#error ">>>>> [Compile error]. This branch requires implementation for Metal"
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	export struct sInTransfersCallbacks
	{
		FillCallback fillCallback;
		PreparedCallback preparedCallback;
		CompleteCallback completeCallback;
		bool isCorrect = true;
	};
}