
export module SceneEntity;

import Material;
import IMeshGPU;
import Transform;

using namespace zzz::math;

export namespace zzz
{
	export class SceneEntity final
	{
	public:
		SceneEntity() = delete;
		explicit SceneEntity(
			const std::shared_ptr<IMeshGPU> mesh,
			const std::shared_ptr<Material> material);
		~SceneEntity() = default;

		[[nodiscard]] inline std::shared_ptr<IMeshGPU> GetMesh() const noexcept { return m_Mesh; }
		[[nodiscard]] inline std::shared_ptr<Material> GetMaterial() const noexcept { return m_Material; }
		inline void SetTransform(Transform& transform) noexcept { m_Transform = transform; }
		[[nodiscard]] inline Transform& GetTransform() noexcept { return m_Transform; }

	protected:
		Transform m_Transform;

	private:
		const std::shared_ptr<IMeshGPU> m_Mesh;
		const std::shared_ptr<Material> m_Material;
	};

	SceneEntity::SceneEntity(
		const std::shared_ptr<IMeshGPU> mesh,
		const std::shared_ptr<Material> material) :
		m_Mesh{ mesh },
		m_Material{ material }
	{
		ensure(m_Mesh != nullptr, ">>>>> [SceneEntity::SceneEntity( ... )]. Mesh pointer cannot be null.");
		ensure(m_Material != nullptr, ">>>>> [SceneEntity::SceneEntity( ... )]. Material pointer cannot be null.");
	}
}