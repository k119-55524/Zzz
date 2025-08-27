#include "pch.h"
export module ResourcesManagerCPU;

import result;
import MeshCPU;
import Settings;
import MeshData;

export namespace zzz
{
	export class ResourcesManagerCPU final
	{
	public:
		ResourcesManagerCPU() = delete;
		ResourcesManagerCPU(const ResourcesManagerCPU&) = delete;
		ResourcesManagerCPU(ResourcesManagerCPU&&) = delete;
		ResourcesManagerCPU& operator=(const ResourcesManagerCPU&) = delete;
		ResourcesManagerCPU& operator=(ResourcesManagerCPU&&) = delete;
		explicit ResourcesManagerCPU(const std::shared_ptr<Settings> _settings);

		result<std::shared_ptr<MeshCPU>> GetDefaultTriangleMesh();

		~ResourcesManagerCPU();

		private:
			const std::shared_ptr<Settings> m_settings;
	};

	export ResourcesManagerCPU::ResourcesManagerCPU(const std::shared_ptr<Settings> _settings)
		: m_settings{ _settings }
	{
		ensure(m_settings, ">>>>> [ResourcesManagerCPU::ResourcesManagerCPU()]. Settings cannot be null.");
	}

	export ResourcesManagerCPU::~ResourcesManagerCPU()
	{
	}

	result<std::shared_ptr<MeshCPU>> ResourcesManagerCPU::GetDefaultTriangleMesh()
	{
		std::shared_ptr<VB_P3C3> vertexBuffer = std::make_shared<VB_P3C3>(std::initializer_list<zzz::VB_P3C3::VertexT>{
			{
				{ { 0.0f,  0.5f, 0.0f } }, // Position
				{ { 1.0f,  0.0f, 0.0f } }  // Color (Red)
			},
			{
				{ { 0.5f, -0.5f, 0.0f } },
				{ { 0.0f, 1.0f, 0.0f } }   // Green
			},
			{
				{ { -0.5f, -0.5f, 0.0f } },
				{ { 0.0f,  0.0f, 1.0f } }  // Blue
			}
		});

		if (!vertexBuffer)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [ResourcesManagerCPU::GetDefaultTriangleMesh()]. Failed to create vertexBufferCPU.");

		auto mesh = std::make_shared<MeshCPU>(vertexBuffer);
		if (!mesh)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [ResourcesManagerCPU::GetDefaultTriangleMesh()]. Failed to create meshCPU.");

		return mesh;
	}
}