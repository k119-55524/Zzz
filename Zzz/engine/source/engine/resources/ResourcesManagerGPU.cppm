#include "pch.h"
export module ResourcesManagerGPU;

import result;
import IMeshGPU;
import MeshGPU_DX;
import ResourcesManagerCPU;

export namespace zzz
{
	export class ResourcesManagerGPU final
	{
	public:
		ResourcesManagerGPU() = delete;
		ResourcesManagerGPU(const ResourcesManagerGPU&) = delete;
		ResourcesManagerGPU(ResourcesManagerGPU&&) = delete;
		ResourcesManagerGPU(std::shared_ptr<ResourcesManagerCPU> resCPU);

		~ResourcesManagerGPU() = default;

		result<std::shared_ptr<IMeshGPU>> GetGenericMesh(MeshType type);

	private:
		std::shared_ptr<ResourcesManagerCPU> m_ResCPU;
	};

	ResourcesManagerGPU::ResourcesManagerGPU(std::shared_ptr<ResourcesManagerCPU> resCPU) :
		m_ResCPU{ resCPU }
	{
		ensure(m_ResCPU, ">>>>> [ResourcesManagerGPU::ResourcesManagerGPU()]. Resource system CPU cannot be null.");
	}

	result<std::shared_ptr<IMeshGPU>> ResourcesManagerGPU::GetGenericMesh(MeshType type)
	{
		auto meshCPU = m_ResCPU->GetGenericMesh(type);
		if (!meshCPU)
			return meshCPU.error();

		std::shared_ptr<IMeshGPU> meshGPU = safe_make_shared<MeshGPU_DX>(meshCPU.value());

		return meshGPU;
	}
}