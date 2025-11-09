#include "pch.h"
export module SceneEntitySpawner;

import result;
import Material;
import SceneEntity;
import GPUResManager;

namespace zzz
{
	export class SceneEntitySpawner final
	{
	public:
		SceneEntitySpawner() = delete;
		SceneEntitySpawner(const SceneEntitySpawner&) = delete;
		SceneEntitySpawner(SceneEntitySpawner&&) = delete;
		SceneEntitySpawner& operator=(const SceneEntitySpawner&) = delete;
		SceneEntitySpawner& operator=(SceneEntitySpawner&&) = delete;
		explicit SceneEntitySpawner(const std::shared_ptr<GPUResManager> resGPU);
		~SceneEntitySpawner() = default;

		result<std::shared_ptr<SceneEntity>> SpawnGenericBox();

	private:
		const std::shared_ptr<GPUResManager> m_ResGPU;
	};

	SceneEntitySpawner::SceneEntitySpawner(const std::shared_ptr<GPUResManager> resGPU) :
		m_ResGPU{ resGPU }
	{
		ensure(m_ResGPU, ">>>>> [SceneEntitySpawner::SceneEntitySpawner()]. Resource system GPU cannot be null.");
	}

	result<std::shared_ptr<SceneEntity>> SceneEntitySpawner::SpawnGenericBox()
	{
		result<std::shared_ptr<IMeshGPU>> meshGPU = m_ResGPU->GetGenericMesh(MeshType::Box);
		if (!meshGPU)
			return meshGPU.error();

		result<std::shared_ptr<Material>> material = m_ResGPU->GetGenericMaterial(meshGPU.value());
		if (!material)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [SceneEntitySpawner::SpawnGenerateBox()]. Failed to create Material.");

		result<std::shared_ptr<SceneEntity>> entity = safe_make_shared<SceneEntity>(meshGPU.value(), material.value());
		if (!entity)
			return Unexpected(eResult::no_make_shared_ptr, L">>>>> [SceneEntitySpawner::SpawnGenerateBox()]. Failed to create SceneEntity.");

		return entity;
	}
}