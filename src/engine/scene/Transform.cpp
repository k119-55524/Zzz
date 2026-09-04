#include "engine/scene/Transform.h"
#include "engine/scene/GameObject.h"

namespace zzz
{
	Transform::Transform(GameObject& owner) noexcept
		: m_Owner(owner)
	{
	}

	void Transform::SetLocalPosition(const ::zzz::math::Vec3<zF32>& position) noexcept
	{
		m_LocalPosition = position;
		SetDirty();
	}

	void Transform::SetLocalRotation(const ::zzz::math::Quat<zF32>& rotation) noexcept
	{
		m_LocalRotation = rotation;
		SetDirty();
	}

	void Transform::SetLocalScale(const ::zzz::math::Vec3<zF32>& scale) noexcept
	{
		m_LocalScale = scale;
		SetDirty();
	}

	::zzz::math::Vec3<zF32> Transform::GetWorldPosition() const noexcept
	{
		const auto& world = GetWorldMatrix();
		return ::zzz::math::Vec3<zF32>{ world._41, world._42, world._43 };
	}

	::zzz::math::Quat<zF32> Transform::GetWorldRotation() const noexcept
	{
		if (m_IsDirty)
		{
			UpdateMatrices();
		}
		return m_WorldRotation;
	}

	::zzz::math::Vec3<zF32> Transform::GetWorldScale() const noexcept
	{
		if (m_IsDirty)
		{
			UpdateMatrices();
		}
		return m_WorldScale;
	}

	void Transform::SetWorldPosition(const ::zzz::math::Vec3<zF32>& position) noexcept
	{
		const GameObject* parent = m_Owner.GetParent();
		if (parent != nullptr)
		{
			const auto invParentWorld = parent->GetTransform().GetWorldMatrix().Inverse();
			// В Row-Vector конвенции: v_local = v_world * M_inv
			const auto localPos = invParentWorld.TransformPoint(position);
			SetLocalPosition(localPos);
		}
		else
		{
			SetLocalPosition(position);
		}
	}

	void Transform::SetWorldRotation(const ::zzz::math::Quat<zF32>& rotation) noexcept
	{
		const GameObject* parent = m_Owner.GetParent();
		if (parent != nullptr)
		{
			const auto invParentRot = parent->GetTransform().GetWorldRotation().Inverse();
			SetLocalRotation((invParentRot * rotation).Normalized());
		}
		else
		{
			SetLocalRotation(rotation);
		}
	}

	::zzz::math::Vec3<zF32> Transform::GetForward() const noexcept
	{
		return GetWorldRotation().RotateVector(::zzz::math::Vec3<zF32>::Forward());
	}

	::zzz::math::Vec3<zF32> Transform::GetUp() const noexcept
	{
		return GetWorldRotation().RotateVector(::zzz::math::Vec3<zF32>::Up());
	}

	::zzz::math::Vec3<zF32> Transform::GetRight() const noexcept
	{
		return GetWorldRotation().RotateVector(::zzz::math::Vec3<zF32>::Right());
	}

	const ::zzz::math::Mat4<zF32>& Transform::GetLocalMatrix() const noexcept
	{
		if (m_IsDirty)
		{
			UpdateMatrices();
		}
		return m_LocalMatrix;
	}

	const ::zzz::math::Mat4<zF32>& Transform::GetWorldMatrix() const noexcept
	{
		if (m_IsDirty)
		{
			UpdateMatrices();
		}
		return m_WorldMatrix;
	}

	void Transform::SetDirty() noexcept
	{
		if (!m_IsDirty)
		{
			m_IsDirty = true;
			for (size_t i = 0; i < m_Owner.GetChildCount(); ++i)
			{
				auto* child = m_Owner.GetChild(i);
				if (child != nullptr)
				{
					child->GetTransform().SetDirty();
				}
			}
		}
	}

	void Transform::UpdateMatrices() const noexcept
	{
		// В Row-Vector: v * M = v * (S * R * T)
		const auto s = ::zzz::math::Mat4<zF32>::Scaling(m_LocalScale);
		const auto r = m_LocalRotation.ToMat4();
		const auto t = ::zzz::math::Mat4<zF32>::Translation(m_LocalPosition);
		m_LocalMatrix = s * r * t;

		const GameObject* parent = m_Owner.GetParent();
		if (parent != nullptr)
		{
			// M_world = M_local * M_parent_world в Row-Vector конвенции (v * M_local * M_parent_world)
			m_WorldMatrix = m_LocalMatrix * parent->GetTransform().GetWorldMatrix();

			m_WorldRotation = (parent->GetTransform().GetWorldRotation() * m_LocalRotation).Normalized();

			const auto parentScale = parent->GetTransform().GetWorldScale();
			m_WorldScale = ::zzz::math::Vec3<zF32>{
				m_LocalScale.x * parentScale.x,
				m_LocalScale.y * parentScale.y,
				m_LocalScale.z * parentScale.z
			};
		}
		else
		{
			m_WorldMatrix = m_LocalMatrix;
			m_WorldRotation = m_LocalRotation;
			m_WorldScale = m_LocalScale;
		}

		m_IsDirty = false;
	}
}
