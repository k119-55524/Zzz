
export module MeshGPU_Vulkan;

#if defined(ZRENDER_API_VULKAN)
import IGAPI;
import Result;
import CPUMesh;
import IMeshGPU;

namespace zzz
{
	export class GPUResManager;
}

namespace zzz::vk
{
	export class MeshGPU_Vulkan final : public IMeshGPU
	{
		friend class GPUResManager;

	public:
		MeshGPU_Vulkan() = delete;
		MeshGPU_Vulkan(std::shared_ptr<CPUMesh> meshCPU);
		~MeshGPU_Vulkan() = default;

	private:
		Result<> Initialize(std::shared_ptr<IGAPI> _IGAPI) override;
	};

	MeshGPU_Vulkan::MeshGPU_Vulkan(std::shared_ptr<CPUMesh> meshCPU) :
		IMeshGPU(meshCPU)
	{
	}

	Result<> MeshGPU_Vulkan::Initialize(std::shared_ptr<IGAPI> _IGAPI)
	{
		return {};
	}
}
#endif // ZRENDER_API_VULKAN