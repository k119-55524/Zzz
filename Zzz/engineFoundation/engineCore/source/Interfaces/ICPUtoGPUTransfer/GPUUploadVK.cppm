
export module GPUUploadVK;

#if defined(ZRENDER_API_VULKAN)

import Result;
import QueueArray;
import IGPUUpload;
import ThreadSafeSwapBuffer;

namespace zzz::vk
{
	export class GPUUploadVK final : public IGPUUpload
	{
		//Z_NO_CREATE_COPY(GPUUploadVK);

	public:
		void TransferResourceToGPU() noexcept override {};
	};
}
#endif // ZRENDER_API_VULKAN