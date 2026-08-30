#pragma once

#include <vector>

namespace zzz::engine
{
	/**
	 * @struct MaterialInstanceParams
	 * @brief Заглушка конкретных параметров экземпляра материала (константные буферы, текстуры и т.п.).
	 */
	struct MaterialInstanceParams
	{
	};

	/**
	 * @struct MeshInstanceData
	 * @brief Заглушка данных одного экземпляра меша (world-матрица и прочие per-instance параметры).
	 */
	struct MeshInstanceData
	{
	};

	/**
	 * @struct MeshBucket
	 * @brief Заглушка бакета одного меша — набор экземпляров этого меша для отрисовки.
	 */
	struct MeshBucket
	{
		std::vector<MeshInstanceData> instances;
	};

	/**
	 * @struct MaterialInstanceBucket
	 * @brief Заглушка бакета одного экземпляра материала — его параметры и меши, которые им рисуются.
	 */
	struct MaterialInstanceBucket
	{
		MaterialInstanceParams params;
		std::vector<MeshBucket> meshes;
	};

	/**
	 * @struct BaseMaterialBucket
	 * @brief Заглушка бакета базового материала/шейдера — группирует экземпляры материалов, использующих один шейдер/PSO.
	 */
	struct BaseMaterialBucket
	{
		std::vector<MaterialInstanceBucket> materialInstances;
	};

	/**
	 * @struct SceneRenderTree
	 * @brief Дерево отрисовки кадра: Материалы -> Меши, разбитое на проходы (Opaque / Transparent).
	 *
	 * На текущем этапе заглушка: проходы формируются пустыми (см. RenderManager::BuildRenderTree),
	 * реальное наполнение появится вместе с ResourceManager/MeshRenderer.
	 */
	struct SceneRenderTree
	{
		std::vector<BaseMaterialBucket> opaquePass;
		std::vector<BaseMaterialBucket> transparentPass;

		void Clear()
		{
			opaquePass.clear();
			transparentPass.clear();
		}
	};
}
