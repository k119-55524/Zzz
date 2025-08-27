#include "pch.h"
export module ResourcesManagerCPU;

import result;
import Colors;
import MeshCPU;
import Settings;
import MeshData;

export namespace zzz
{
	export enum MeshType
	{
		eGenericTriangle,
		eGenericBox
	};

	export class ResourcesManagerCPU final
	{
	public:
		ResourcesManagerCPU() = delete;
		ResourcesManagerCPU(const ResourcesManagerCPU&) = delete;
		ResourcesManagerCPU(ResourcesManagerCPU&&) = delete;
		ResourcesManagerCPU& operator=(const ResourcesManagerCPU&) = delete;
		ResourcesManagerCPU& operator=(ResourcesManagerCPU&&) = delete;
		explicit ResourcesManagerCPU(const std::shared_ptr<Settings> _settings);

		result<std::shared_ptr<MeshCPU>> GetGenericMesh(MeshType type);

		~ResourcesManagerCPU();

		private:
			const std::shared_ptr<Settings> m_settings;

			result<std::shared_ptr<MeshCPU>> GetGenericTriangle();
			result<std::shared_ptr<MeshCPU>> GetGenericBox();
	};

	export ResourcesManagerCPU::ResourcesManagerCPU(const std::shared_ptr<Settings> _settings)
		: m_settings{ _settings }
	{
		ensure(m_settings, ">>>>> [ResourcesManagerCPU::ResourcesManagerCPU()]. Settings cannot be null.");
	}

	export ResourcesManagerCPU::~ResourcesManagerCPU()
	{
	}

	result<std::shared_ptr<MeshCPU>> ResourcesManagerCPU::GetGenericMesh(MeshType type)
	{
		switch (type)
		{
		case MeshType::eGenericTriangle:
			return GetGenericTriangle();
		case MeshType::eGenericBox:
			return GetGenericBox();
		default:
			throw_invalid_argument("Unknown type mesh.");
		}
	}

	result<std::shared_ptr<MeshCPU>> ResourcesManagerCPU::GetGenericTriangle()
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

	result<std::shared_ptr<MeshCPU>> ResourcesManagerCPU::GetGenericBox()
	{
		std::shared_ptr<VB_P3C3> vertex = std::make_shared<VB_P3C3>(std::initializer_list<zzz::VB_P3C3::VertexT>{
			{
				{ { -1.0f, -1.0f, -1.0f } }, // Position
				{ { Colors::White } } // Color
			},
			{
				{ { -1.0f, +1.0f, -1.0f } },
				{ { Colors::Black } }
			},
			{
				{ { +1.0f, +1.0f, -1.0f } },
				{ { Colors::Red } }
			},
			{
				{ { +1.0f, -1.0f, -1.0f } },
				{ { Colors::Green } }
			},
			{
				{ { -1.0f, -1.0f, +1.0f } },
				{ { Colors::Blue } }
			},
			{
				{ { -1.0f, +1.0f, +1.0f } },
				{ { Colors::Yellow } }
			},
			{
				{ { +1.0f, +1.0f, +1.0f } },
				{ { Colors::Cyan } }
			},
			{
				{ { +1.0f, -1.0f, +1.0f } },
				{ { Colors::Magenta } }
			}
		});

		if (!vertex)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [ResourcesManagerCPU::GetDefaultTriangleMesh()]. Failed to create vertexBufferCPU.");

		auto indices = std::make_shared<std::vector<zU16>>(std::initializer_list<zU16>
		{
			// front face
			0, 1, 2,
			0, 2, 3,

			// back face
			4, 6, 5,
			4, 7, 6,

			// left face
			4, 5, 1,
			4, 1, 0,

			// right face
			3, 2, 6,
			3, 6, 7,

			// top face
			1, 5, 6,
			1, 6, 2,

			// bottom face
			4, 0, 3,
			4, 3, 7
		});

		auto mesh = std::make_shared<MeshCPU>(vertex, indices);
		if (!mesh)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [ResourcesManagerCPU::GetDefaultTriangleMesh()]. Failed to create meshCPU.");

		return mesh;
	}
}