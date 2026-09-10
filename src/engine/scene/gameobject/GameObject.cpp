#include "engine/scene/gameobject/GameObject.h"
#include "core/userscripts/base_script/Script.h"
#include <algorithm>

namespace zzz::engine
{
	namespace
	{
		const math::Vec3<zF32> kZeroPos{ 0.0f, 0.0f, 0.0f };
		const math::Quat<zF32> kIdentityRot{ 0.0f, 0.0f, 0.0f, 1.0f };
		const math::Vec3<zF32> kOneScale{ 1.0f, 1.0f, 1.0f };
		const math::Mat4<zF32> kIdentityMat = math::Mat4<zF32>::Identity();
	}

	GameObject::GameObject(const ::zzz::core::Guid& guid, std::string name)
		: m_Guid(guid)
		, m_FallbackName(std::move(name))
	{
	}

	const std::string& GameObject::GetName() const noexcept
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			return m_SceneTree->GetName(m_NodeHandle);
		}
		return m_FallbackName;
	}

	void GameObject::SetName(std::string name)
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			m_SceneTree->SetName(m_NodeHandle, std::move(name));
		}
		else
		{
			m_FallbackName = std::move(name);
		}
	}

	uint32_t GameObject::GetSpatialHandle() const noexcept
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			return m_SceneTree->GetSpatialHandle(m_NodeHandle);
		}
		return 0xFFFFFFFF;
	}

	void GameObject::SetSpatialHandle(uint32_t handle) noexcept
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			m_SceneTree->SetSpatialHandle(m_NodeHandle, handle);
		}
	}

	bool GameObject::IsActive() const noexcept
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			return m_SceneTree->IsActive(m_NodeHandle);
		}
		return true;
	}

	void GameObject::SetActive(bool active) noexcept
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			m_SceneTree->SetActive(m_NodeHandle, active);
		}
	}

	void GameObject::SetLocalPosition(const ::zzz::math::Vec3<zF32>& pos)
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			m_SceneTree->SetLocalPosition(m_NodeHandle, pos);
		}
	}

	const ::zzz::math::Vec3<zF32>& GameObject::GetLocalPosition() const
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			return m_SceneTree->GetLocalPosition(m_NodeHandle);
		}
		return kZeroPos;
	}

	void GameObject::SetLocalRotation(const ::zzz::math::Quat<zF32>& rot)
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			m_SceneTree->SetLocalRotation(m_NodeHandle, rot);
		}
	}

	const ::zzz::math::Quat<zF32>& GameObject::GetLocalRotation() const
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			return m_SceneTree->GetLocalRotation(m_NodeHandle);
		}
		return kIdentityRot;
	}

	void GameObject::SetLocalScale(const ::zzz::math::Vec3<zF32>& scale)
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			m_SceneTree->SetLocalScale(m_NodeHandle, scale);
		}
	}

	const ::zzz::math::Vec3<zF32>& GameObject::GetLocalScale() const
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			return m_SceneTree->GetLocalScale(m_NodeHandle);
		}
		return kOneScale;
	}

	const ::zzz::math::Mat4<zF32>& GameObject::GetWorldMatrix() const
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			return m_SceneTree->GetWorldMatrix(m_NodeHandle);
		}
		return kIdentityMat;
	}

	GameObject* GameObject::GetParent() const noexcept
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			const NodeHandle parentHandle = m_SceneTree->GetParent(m_NodeHandle);
			if (parentHandle.IsValid())
			{
				return m_SceneTree->GetNodeOwner(parentHandle);
			}
		}
		return nullptr;
	}

	void GameObject::SetParent(GameObject* newParent, bool keepWorldTransform) noexcept
	{
		if (m_SceneTree != nullptr && m_NodeHandle.IsValid())
		{
			const NodeHandle parentHandle = (newParent != nullptr) ? newParent->GetNodeHandle() : NodeHandle{};
			m_SceneTree->SetParent(m_NodeHandle, parentHandle, keepWorldTransform);
		}
	}

	void GameObject::AddScript(std::shared_ptr<::zzz::core::Script> script)
	{
		if (script != nullptr)
		{
			m_Scripts.push_back(std::move(script));
		}
	}

	void GameObject::RemoveScript(const std::shared_ptr<::zzz::core::Script>& script)
	{
		std::erase(m_Scripts, script);
	}

	void GameObject::RemoveAllScripts()
	{
		m_Scripts.clear();
	}
}
