#include "pch.h"
export module ResourcesManagerGPU;

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

	private:
		std::shared_ptr<ResourcesManagerCPU> m_ResCPU;
	};

	ResourcesManagerGPU::ResourcesManagerGPU(std::shared_ptr<ResourcesManagerCPU> resCPU) :
		m_ResCPU{ resCPU }
	{
		ensure(m_ResCPU, ">>>>> [ResourcesManagerGPU::ResourcesManagerGPU()]. Resource system CPU cannot be null.");
	}
}