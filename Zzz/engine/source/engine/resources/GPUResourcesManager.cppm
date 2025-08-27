#include "pch.h"
export module GPUResourcesManager;

import IGAPI;
import result;
import IMeshGPU;
import GPUMeshDX;
import CPUResourcesManager;

using namespace zzz::platforms;

namespace zzz
{
#if defined(_WIN64)
	typedef GPUMeshDX GPUMesh;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif
}

export namespace zzz
{
	export class GPUResourcesManager final
	{
	public:
		GPUResourcesManager() = delete;
		GPUResourcesManager(const GPUResourcesManager&) = delete;
		GPUResourcesManager(GPUResourcesManager&&) = delete;
		GPUResourcesManager(std::shared_ptr<IGAPI> _IGAPI, std::shared_ptr<CPUResourcesManager> resCPU);

		~GPUResourcesManager() = default;

		result<std::shared_ptr<IMeshGPU>> GetGenericMesh(MeshType type);

	private:
		std::shared_ptr<IGAPI> m_GAPI;
		std::shared_ptr<CPUResourcesManager> m_ResCPU;
	};

	GPUResourcesManager::GPUResourcesManager(std::shared_ptr<IGAPI> _IGAPI, std::shared_ptr<CPUResourcesManager> resCPU) :
		m_GAPI{ _IGAPI },
		m_ResCPU{ resCPU }
	{
		ensure(m_ResCPU, ">>>>> [GPUResourcesManager::GPUResourcesManager()]. Resource system CPU cannot be null.");
	}

	result<std::shared_ptr<IMeshGPU>> GPUResourcesManager::GetGenericMesh(MeshType type)
	{
		auto meshCPU = m_ResCPU->GetGenericMesh(type);
		if (!meshCPU)
			return meshCPU.error();

		std::shared_ptr<IMeshGPU> meshGPU = safe_make_shared<GPUMesh>(meshCPU.value());
		result<> res = meshGPU->Initialize(m_GAPI);
		if (!res)
			return Unexpected(res.error());

		return meshGPU;
	}
}