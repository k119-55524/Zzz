#pragma once

#include <vector>
#include <string>

#include "core/utils/macros/MiscMacros.h"
#include "engine/scene/storage/NodeStorageBlock.h"

using namespace zzz::math;

namespace zzz::engine
{
	class ISpatialStorage;

	/**
	 * @class SceneTreeContainer
	 * @brief Двухбуферный владелец и менеджер иерархии сцены.
	 *
	 * @details Реализует ISceneTreeAccessor. Инкапсулирует Front Buffer (m_PrimaryNodes)
	 * для Render Thread и Back Buffer (m_SecondaryNodes) для Game Logic.
	 */
	class SceneTreeContainer final : public ISceneTreeAccessor
	{
		Z_NO_COPY_MOVE(SceneTreeContainer);

	public:
		SceneTreeContainer();
		~SceneTreeContainer() override = default;

		// --- Создание и уничтожение узлов ---
		NodeHandle CreateNode(std::string name = "Node", GameObject* owner = nullptr);
		void DestroySubtree(NodeHandle root, ISpatialStorage* spatialStorage = nullptr);

		// --- ISceneTreeAccessor: Пространственные координаты ---
		void SetLocalPosition(NodeHandle handle, const Vec3<zF32>& pos) override;
		[[nodiscard]] const Vec3<zF32>& GetLocalPosition(NodeHandle handle) const override;

		void SetLocalRotation(NodeHandle handle, const Quat<zF32>& rot) override;
		[[nodiscard]] const Quat<zF32>& GetLocalRotation(NodeHandle handle) const override;

		void SetLocalScale(NodeHandle handle, const Vec3<zF32>& scale) override;
		[[nodiscard]] const Vec3<zF32>& GetLocalScale(NodeHandle handle) const override;

		[[nodiscard]] const Mat4<zF32>& GetWorldMatrix(NodeHandle handle) const override;

		// --- ISceneTreeAccessor: Топология и иерархия ---
		void SetParent(NodeHandle child, NodeHandle parent, bool keepWorldTransform = true) override;
		[[nodiscard]] NodeHandle GetParent(NodeHandle handle) const override;
		[[nodiscard]] GameObject* GetNodeOwner(NodeHandle handle) const override;
		void MarkDirty(NodeHandle handle) override;

		// --- ISceneTreeAccessor: Свойства узла ---
		void SetActive(NodeHandle handle, bool active) override;
		[[nodiscard]] bool IsActive(NodeHandle handle) const override;

		void SetName(NodeHandle handle, std::string name) override;
		[[nodiscard]] const std::string& GetName(NodeHandle handle) const override;

		void SetSpatialHandle(NodeHandle handle, SpatialHandle spHandle) override;
		[[nodiscard]] SpatialHandle GetSpatialHandle(NodeHandle handle) const noexcept override;

		// --- Жизненный цикл кадра и валидация ---
		[[nodiscard]] bool IsValid(NodeHandle handle) const noexcept;
		void BeginFrame();
		void ResolveTransforms();
		void ApplyHandoverBarrier();

		// --- Доступ к первичному буферу (для рендера) ---
		[[nodiscard]] const NodeStorageBlock& GetPrimaryNodes() const noexcept { return m_PrimaryNodes; }
		[[nodiscard]] NodeStorageBlock& GetSecondaryNodes() noexcept { return m_SecondaryNodes; }

	private:
		NodeStorageBlock m_PrimaryNodes;   // Front Buffer: статичный снимок для параллельного чтения рендером
		NodeStorageBlock m_SecondaryNodes; // Back Buffer: рабочий буфер для мутаций текущего кадра
		std::vector<uint32_t> m_FreeIndices;
		bool m_TopologyDirty{ false };
	};
}
