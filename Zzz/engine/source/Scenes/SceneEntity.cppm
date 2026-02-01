
export module SceneEntity;

import Result;
import Material;
import IMeshGPU;
import Transform;
import IBehavior;

using namespace zzz::core;
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

		inline void OnUpdate(float deltaTime) { if (m_Script != nullptr) m_Script->OnUpdate(deltaTime); };

		template<typename T, typename... Args>
		Result<> SetScript(Args&&... args)
		{
			static_assert(std::is_base_of_v<IBehavior, T>, "T must derive from IBehavior");

			m_Script = safe_make_unique<T>(std::forward<Args>(args)...);

			return {};
		}

	protected:
		Transform m_Transform;
		std::unique_ptr<IBehavior> m_Script;

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
		ensure(m_Mesh != nullptr, "Mesh pointer cannot be null.");
		ensure(m_Material != nullptr, "Material pointer cannot be null.");
	}
}