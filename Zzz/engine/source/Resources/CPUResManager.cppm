
#include "pch.h"
export module CPUResManager;

import Result;
import Colors;
import CPUMesh;
import Settings;
import CPUIndexBuffer;
import CPUVertexBuffer;

using namespace zzz::core;

export namespace zzz
{
	export enum MeshType
	{
		Triangle,
		Box
	};

	export class CPUResManager final
	{
		Z_NO_CREATE_COPY(CPUResManager);

	public:
		explicit CPUResManager(const std::shared_ptr<Settings> _settings);

		Result<std::shared_ptr<CPUMesh>> GetGenericMesh(MeshType type);

		~CPUResManager();

		private:
			const std::shared_ptr<Settings> m_settings;

			Result<std::shared_ptr<CPUMesh>> GetGenericTriangle();
			Result<std::shared_ptr<CPUMesh>> GetGenericBox();
	};

	export CPUResManager::CPUResManager(const std::shared_ptr<Settings> _settings)
		: m_settings{ _settings }
	{
		ensure(m_settings, ">>>>> [CPUResourcesManager::CPUResourcesManager()]. Settings cannot be null.");
	}

	export CPUResManager::~CPUResManager()
	{
	}

	Result<std::shared_ptr<CPUMesh>> CPUResManager::GetGenericMesh(MeshType type)
	{
		switch (type)
		{
		case MeshType::Triangle:
			return GetGenericTriangle();
		case MeshType::Box:
			return GetGenericBox();
		default:
			throw_invalid_argument("Unknown type mesh.");
		}
	}

	Result<std::shared_ptr<CPUMesh>> CPUResManager::GetGenericTriangle()
	{
		std::shared_ptr<VB_P3C3> vertexBuffer = std::make_shared<VB_P3C3>(std::initializer_list<VB_P3C3::VertexT>{
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

	Result<std::shared_ptr<CPUMesh>> CPUResManager::GetGenericBox()
	{
		std::shared_ptr<VB_P3C3> vertex = std::make_shared<VB_P3C3>(std::initializer_list<VB_P3C3::VertexT>{
			{
				{ { -1.0f, -1.0f, -1.0f } },	// Position
				{ { colors::White } }			// Color
			},
			{
				{ { -1.0f, +1.0f, -1.0f } },
				{ { colors::Black } }
			},
			{
				{ { +1.0f, +1.0f, -1.0f } },
				{ { colors::Red } }
			},
			{
				{ { +1.0f, -1.0f, -1.0f } },
				{ { colors::Green } }
			},
			{
				{ { -1.0f, -1.0f, +1.0f } },
				{ { colors::Blue } }
			},
			{
				{ { -1.0f, +1.0f, +1.0f } },
				{ { colors::Yellow } }
			},
			{
				{ { +1.0f, +1.0f, +1.0f } },
				{ { colors::Cyan } }
			},
			{
				{ { +1.0f, -1.0f, +1.0f } },
				{ { colors::Magenta } }
			}
		});

		if (!vertex)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [CPUResourcesManager::GetDefaultTriangleMesh()]. Failed to create vertexBufferCPU.");

		std::shared_ptr<ICPUIndexBuffer> indices =
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