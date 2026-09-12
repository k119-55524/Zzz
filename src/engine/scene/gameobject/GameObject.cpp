#include "core/utils/Ensure.h"
#include "engine/scene/gameobject/GameObject.h"
#include "engine/scene/storage/NodeStorage.h"
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

	uint32_t GameObject::GetSpatialHandle() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetSpatialHandle: объект не привязан к сцене");
		return m_NodeStorage->GetSpatialHandle(m_NodeHandle);
	}

	void GameObject::SetSpatialHandle(uint32_t handle)
	{
		ensure(m_NodeStorage != nullptr, "GameObject::SetSpatialHandle: объект не привязан к сцене");
		m_NodeStorage->SetSpatialHandle(m_NodeHandle, handle);
	}

	bool GameObject::IsActive() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::IsActive: объект не привязан к сцене");
		return m_NodeStorage->IsActive(m_NodeHandle);
	}

	void GameObject::SetActive(bool active)
	{
		ensure(m_NodeStorage != nullptr, "GameObject::SetActive: объект не привязан к сцене");
		m_NodeStorage->SetActive(m_NodeHandle, active);
	}

	void GameObject::SetLocalPosition(const Vec3<zF32>& pos)
	{
		ensure(m_NodeStorage != nullptr, "GameObject::SetLocalPosition: объект не привязан к сцене");
		m_NodeStorage->SetLocalPosition(m_NodeHandle, pos);
	}

	Vec3<zF32> GameObject::GetLocalPosition() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetLocalPosition: объект не привязан к сцене");
		return m_NodeStorage->GetLocalPosition(m_NodeHandle);
	}

	void GameObject::SetLocalRotation(const Quat<zF32>& rot)
	{
		ensure(m_NodeStorage != nullptr, "GameObject::SetLocalRotation: объект не привязан к сцене");
		m_NodeStorage->SetLocalRotation(m_NodeHandle, rot);
	}

	Quat<zF32> GameObject::GetLocalRotation() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetLocalRotation: объект не привязан к сцене");
		return m_NodeStorage->GetLocalRotation(m_NodeHandle);
	}

	void GameObject::SetLocalScale(const Vec3<zF32>& scale)
	{
		ensure(m_NodeStorage != nullptr, "GameObject::SetLocalScale: объект не привязан к сцене");
		m_NodeStorage->SetLocalScale(m_NodeHandle, scale);
	}

	Vec3<zF32> GameObject::GetLocalScale() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetLocalScale: объект не привязан к сцене");
		return m_NodeStorage->GetLocalScale(m_NodeHandle);
	}

	const Mat4<zF32>& GameObject::GetLocalMatrix() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetLocalMatrix: объект не привязан к сцене");
		return m_NodeStorage->GetLocalMatrix(m_NodeHandle);
	}

	const Mat4<zF32>& GameObject::GetWorldMatrix() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetWorldMatrix: объект не привязан к сцене");
		return m_NodeStorage->GetWorldMatrix(m_NodeHandle);
	}

	GameObject* GameObject::GetParent() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetParent: объект не привязан к сцене");
		const NodeHandle parentHandle = m_NodeStorage->GetParent(m_NodeHandle);
		if (parentHandle.IsValid())
		{
			return m_NodeStorage->GetNodeOwner(parentHandle);
		}
		return nullptr;
	}

	void GameObject::SetParent(GameObject* newParent, bool keepWorldTransform)
	{
		ensure(m_NodeStorage != nullptr, "GameObject::SetParent: объект не привязан к сцене");
		const NodeHandle parentHandle = (newParent != nullptr) ? newParent->GetNodeHandle() : NodeHandle{};
		m_NodeStorage->SetParent(m_NodeHandle, parentHandle, keepWorldTransform);
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
