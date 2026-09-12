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

	zU32 GameObject::GetSpatialHandle() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetSpatialHandle: объект не привязан к сцене");
		return m_NodeStorage->GetSpatialHandle(m_NodeIndex);
	}

	bool GameObject::IsActive() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::IsActive: объект не привязан к сцене");
		return m_NodeStorage->IsActive(m_NodeIndex);
	}

	void GameObject::SetActive(bool active)
	{
		ensure(m_NodeStorage != nullptr, "GameObject::SetActive: объект не привязан к сцене");
		m_NodeStorage->SetActive(m_NodeIndex, active);
	}

	void GameObject::SetLocalPosition(const Vec3<zF32>& pos)
	{
		ensure(m_NodeStorage != nullptr, "GameObject::SetLocalPosition: объект не привязан к сцене");
		m_NodeStorage->SetLocalPosition(m_NodeIndex, pos);
	}

	Vec3<zF32> GameObject::GetLocalPosition() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetLocalPosition: объект не привязан к сцене");
		return m_NodeStorage->GetLocalPosition(m_NodeIndex);
	}

	void GameObject::SetLocalRotation(const Quat<zF32>& rot)
	{
		ensure(m_NodeStorage != nullptr, "GameObject::SetLocalRotation: объект не привязан к сцене");
		m_NodeStorage->SetLocalRotation(m_NodeIndex, rot);
	}

	Quat<zF32> GameObject::GetLocalRotation() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetLocalRotation: объект не привязан к сцене");
		return m_NodeStorage->GetLocalRotation(m_NodeIndex);
	}

	void GameObject::SetLocalScale(const Vec3<zF32>& scale)
	{
		ensure(m_NodeStorage != nullptr, "GameObject::SetLocalScale: объект не привязан к сцене");
		m_NodeStorage->SetLocalScale(m_NodeIndex, scale);
	}

	Vec3<zF32> GameObject::GetLocalScale() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetLocalScale: объект не привязан к сцене");
		return m_NodeStorage->GetLocalScale(m_NodeIndex);
	}

	Mat4<zF32> GameObject::GetLocalMatrix() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetLocalMatrix: объект не привязан к сцене");
		return m_NodeStorage->GetLocalMatrix(m_NodeIndex);
	}

	const Mat4<zF32>& GameObject::GetWorldMatrix() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetWorldMatrix: объект не привязан к сцене");
		return m_NodeStorage->GetWorldMatrix(m_NodeIndex);
	}

	zU32 GameObject::GetParentIndex() const
	{
		ensure(m_NodeStorage != nullptr, "GameObject::GetParentIndex: объект не привязан к сцене");
		return m_NodeStorage->GetParent(m_NodeIndex);
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
