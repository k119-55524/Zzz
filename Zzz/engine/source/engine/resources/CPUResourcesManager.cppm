#include "pch.h"
export module CPUResourcesManager;

import result;
import Colors;
import CPUMesh;
import Settings;
import MeshData;
import CPUIndexBuffer;

export namespace zzz
{
	export enum MeshType
	{
		eGenericTriangle,
		eGenericBox
	};

	export class CPUResourcesManager final
	{
	public:
		CPUResourcesManager() = delete;
		CPUResourcesManager(const CPUResourcesManager&) = delete;
		CPUResourcesManager(CPUResourcesManager&&) = delete;
		CPUResourcesManager& operator=(const CPUResourcesManager&) = delete;
		CPUResourcesManager& operator=(CPUResourcesManager&&) = delete;
		explicit CPUResourcesManager(const std::shared_ptr<Settings> _settings);

		result<std::shared_ptr<CPUMesh>> GetGenericMesh(MeshType type);

		~CPUResourcesManager();

		private:
			const std::shared_ptr<Settings> m_settings;

			result<std::shared_ptr<CPUMesh>> GetGenericTriangle();
			result<std::shared_ptr<CPUMesh>> GetGenericBox();
	};

	export CPUResourcesManager::CPUResourcesManager(const std::shared_ptr<Settings> _settings)
		: m_settings{ _settings }
	{
		ensure(m_settings, ">>>>> [CPUResourcesManager::CPUResourcesManager()]. Settings cannot be null.");
	}

	export CPUResourcesManager::~CPUResourcesManager()
	{
	}

	result<std::shared_ptr<CPUMesh>> CPUResourcesManager::GetGenericMesh(MeshType type)
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

	result<std::shared_ptr<CPUMesh>> CPUResourcesManager::GetGenericTriangle()
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
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [CPUResourcesManager::GetDefaultTriangleMesh()]. Failed to create vertexBufferCPU.");

		auto mesh = std::make_shared<CPUMesh>(vertexBuffer);
		if (!mesh)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [CPUResourcesManager::GetDefaultTriangleMesh()]. Failed to create meshCPU.");

		return mesh;
	}

	result<std::shared_ptr<CPUMesh>> CPUResourcesManager::GetGenericBox()
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
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [CPUResourcesManager::GetDefaultTriangleMesh()]. Failed to create vertexBufferCPU.");

		std::shared_ptr<zzz::ICPUIndexBuffer> indices =
			std::make_shared<IndexBuffer16>(std::initializer_list<zU16>{
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

		auto mesh = std::make_shared<CPUMesh>(vertex, indices);
		if (!mesh)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [CPUResourcesManager::GetDefaultTriangleMesh()]. Failed to create meshCPU.");

		return mesh;
	}
}