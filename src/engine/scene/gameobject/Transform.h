#pragma once

#include "math/vector/Vec3.h"
#include "math/matrix/Mat4.h"
#include "math/quat/Quat.h"
#include "core/utils/Defines.h"

namespace zzz
{
	class GameObject;

	/**
	 * @class Transform
	 * @brief Пространственная трансформация игрового объекта в 3D мире.
	 *
	 * @details Управляет локальным положением, вращением (кватернион), масштабом и вычислением TRS.
	 * Реализует кэширование мировой матрицы через dirty flag.
	 */
	class Transform final
	{
	public:
		explicit Transform(GameObject& owner) noexcept;
		~Transform() = default;

		Z_NO_COPY_MOVE(Transform);

		// --- Локальные параметры ---
		[[nodiscard]] const ::zzz::math::Vec3<zF32>& GetLocalPosition() const noexcept { return m_LocalPosition; }
		[[nodiscard]] const ::zzz::math::Quat<zF32>& GetLocalRotation() const noexcept { return m_LocalRotation; }
		[[nodiscard]] const ::zzz::math::Vec3<zF32>& GetLocalScale() const noexcept { return m_LocalScale; }

		void SetLocalPosition(const ::zzz::math::Vec3<zF32>& position) noexcept;
		void SetLocalRotation(const ::zzz::math::Quat<zF32>& rotation) noexcept;
		void SetLocalScale(const ::zzz::math::Vec3<zF32>& scale) noexcept;

		// --- Мировые параметры ---
		[[nodiscard]] ::zzz::math::Vec3<zF32> GetWorldPosition() const noexcept;
		[[nodiscard]] ::zzz::math::Quat<zF32> GetWorldRotation() const noexcept;
		[[nodiscard]] ::zzz::math::Vec3<zF32> GetWorldScale() const noexcept;

		void SetWorldPosition(const ::zzz::math::Vec3<zF32>& position) noexcept;
		void SetWorldRotation(const ::zzz::math::Quat<zF32>& rotation) noexcept;

		// --- Базисные векторы направления (в мировой системе) ---
		[[nodiscard]] ::zzz::math::Vec3<zF32> GetForward() const noexcept;
		[[nodiscard]] ::zzz::math::Vec3<zF32> GetUp() const noexcept;
		[[nodiscard]] ::zzz::math::Vec3<zF32> GetRight() const noexcept;

		// --- Матрицы трансформации ---
		[[nodiscard]] const ::zzz::math::Mat4<zF32>& GetLocalMatrix() const noexcept;
		[[nodiscard]] const ::zzz::math::Mat4<zF32>& GetWorldMatrix() const noexcept;

		// --- Владелец и инвалидация кэша ---
		[[nodiscard]] GameObject& GetOwner() noexcept { return m_Owner; }
		[[nodiscard]] const GameObject& GetOwner() const noexcept { return m_Owner; }

		[[nodiscard]] bool IsDirty() const noexcept { return m_IsDirty; }
		void SetDirty() noexcept;

	private:
		void UpdateMatrices() const noexcept;

		GameObject& m_Owner;

		::zzz::math::Vec3<zF32> m_LocalPosition{ 0.0f, 0.0f, 0.0f };
		::zzz::math::Quat<zF32> m_LocalRotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		::zzz::math::Vec3<zF32> m_LocalScale{ 1.0f, 1.0f, 1.0f };

		mutable ::zzz::math::Mat4<zF32> m_LocalMatrix{ ::zzz::math::Mat4<zF32>::Identity() };
		mutable ::zzz::math::Mat4<zF32> m_WorldMatrix{ ::zzz::math::Mat4<zF32>::Identity() };
		mutable ::zzz::math::Quat<zF32> m_WorldRotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		mutable ::zzz::math::Vec3<zF32> m_WorldScale{ 1.0f, 1.0f, 1.0f };
		mutable bool m_IsDirty{ true };
	};
}
