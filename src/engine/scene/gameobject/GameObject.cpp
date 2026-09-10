#include "core/utils/Ensure.h"
#include "engine/scene/gameobject/GameObject.h"
#include "core/userscripts/base_script/Script.h"
#include <algorithm>

using namespace zzz::core;
using namespace zzz::math;

namespace zzz::engine
{
	GameObject::GameObject(const Guid& guid, std::string name)
		: m_Guid(guid)
		, m_Name(std::move(name))
	{
	}

	uint32_t GameObject::GetSpatialHandle() const noexcept
	{
		ensure(m_SceneTree != nullptr, "GameObject::GetSpatialHandle: объект не привязан к сцене");
		return m_SceneTree->GetSpatialHandle(m_NodeHandle);
	}

	void GameObject::SetSpatialHandle(uint32_t handle) noexcept
	{
		ensure(m_SceneTree != nullptr, "GameObject::SetSpatialHandle: объект не привязан к сцене");
		m_SceneTree->SetSpatialHandle(m_NodeHandle, handle);
	}

	bool GameObject::IsActive() const noexcept
	{
		ensure(m_SceneTree != nullptr, "GameObject::IsActive: объект не привязан к сцене");
		return m_SceneTree->IsActive(m_NodeHandle);
	}

	void GameObject::SetActive(bool active) noexcept
	{
		ensure(m_SceneTree != nullptr, "GameObject::SetActive: объект не привязан к сцене");
		m_SceneTree->SetActive(m_NodeHandle, active);
	}

	void GameObject::SetLocalPosition(const Vec3<zF32>& pos)
	{
		ensure(m_SceneTree != nullptr, "GameObject::SetLocalPosition: объект не привязан к сцене");
		m_SceneTree->SetLocalPosition(m_NodeHandle, pos);
	}

	Vec3<zF32> GameObject::GetLocalPosition() const
	{
		ensure(m_SceneTree != nullptr, "GameObject::GetLocalPosition: объект не привязан к сцене");
		return m_SceneTree->GetLocalPosition(m_NodeHandle);
	}

	void GameObject::SetLocalRotation(const Quat<zF32>& rot)
	{
		ensure(m_SceneTree != nullptr, "GameObject::SetLocalRotation: объект не привязан к сцене");
		m_SceneTree->SetLocalRotation(m_NodeHandle, rot);
	}

	Quat<zF32> GameObject::GetLocalRotation() const
	{
		ensure(m_SceneTree != nullptr, "GameObject::GetLocalRotation: объект не привязан к сцене");
		return m_SceneTree->GetLocalRotation(m_NodeHandle);
	}

	void GameObject::SetLocalScale(const Vec3<zF32>& scale)
	{
		ensure(m_SceneTree != nullptr, "GameObject::SetLocalScale: объект не привязан к сцене");
		m_SceneTree->SetLocalScale(m_NodeHandle, scale);
	}

	Vec3<zF32> GameObject::GetLocalScale() const
	{
		ensure(m_SceneTree != nullptr, "GameObject::GetLocalScale: объект не привязан к сцене");
		return m_SceneTree->GetLocalScale(m_NodeHandle);
	}

	const Mat4<zF32>& GameObject::GetLocalMatrix() const
	{
		ensure(m_SceneTree != nullptr, "GameObject::GetLocalMatrix: объект не привязан к сцене");
		return m_SceneTree->GetLocalMatrix(m_NodeHandle);
	}

	const Mat4<zF32>& GameObject::GetWorldMatrix() const
	{
		ensure(m_SceneTree != nullptr, "GameObject::GetWorldMatrix: объект не привязан к сцене");
		return m_SceneTree->GetWorldMatrix(m_NodeHandle);
	}

	GameObject* GameObject::GetParent() const noexcept
	{
		ensure(m_SceneTree != nullptr, "GameObject::GetParent: объект не привязан к сцене");
		const NodeHandle parentHandle = m_SceneTree->GetParent(m_NodeHandle);
		if (parentHandle.IsValid())
		{
			return m_SceneTree->GetNodeOwner(parentHandle);
		}
		return nullptr;
	}

	void GameObject::SetParent(GameObject* newParent, bool keepWorldTransform) noexcept
	{
		ensure(m_SceneTree != nullptr, "GameObject::SetParent: объект не привязан к сцене");
		const NodeHandle parentHandle = (newParent != nullptr) ? newParent->GetNodeHandle() : NodeHandle{};
		m_SceneTree->SetParent(m_NodeHandle, parentHandle, keepWorldTransform);
	}

	void GameObject::AddScript(std::shared_ptr<Script> script)
	{
		if (script != nullptr)
		{
			m_Scripts.push_back(std::move(script));
		}
	}

	void GameObject::RemoveScript(const std::shared_ptr<Script>& script)
	{
		std::erase(m_Scripts, script);
	}

	void GameObject::RemoveAllScripts()
	{
		m_Scripts.clear();
	}
}
